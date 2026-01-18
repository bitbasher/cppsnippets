# Template File I/O Architecture Design Discussion

**Date:** 2026-01-16
**Status:** Design Review - Awaiting User Feedback

## User Requirements Summary

1. Use "read" and "save" naming convention for file I/O (not "load")
2. Template name comes from JSON content, not filename
3. Separation of concerns: ResourceTemplate reads/writes files, TemplatesInventory manages collection
4. Support use cases:
   - Drag-and-drop file → read → add to inventory
   - Template editor creates new → add to inventory
   - Menu "Load Templates" → bulk import
   - Menu "Save Templates" → bulk export

## Design Questions & Recommendations

### 1. QDir + QFileInfo Parameter Design

**User Proposed:**

```cpp
readFromJson(QDir dir, QFileInfo filename)
saveToJson(QDir dir, QFileInfo filename)
```

there are several possible use cases

but the main reason for separate folder and filename is that with multiple installations on a machine there would be several places with templates that have the same filename, but may differ in their text or other details

and .. to be able to save a modified template into a writable folder we may have to offer the user a filedialog to select the directory .. but need to leave the filename untouched.

another important cased is when there is a complex command in the script language with lots of options. imagine that we have three different snippets based on the command

  color( "0xffffff" )   // hex totation for RGB
  color( "colorname" )  // named color
  color( [.3,.35,.66])   // RGB vector

so template names .. also good as template file names
  color-hex
  color-name
  color-vector

but for a particular use i want my colors to have an alpha channel set att 0.04
so i clone one of those and change it to

the clone will be named color-name-copy by the cloning operation
  color( "colorname", 0.4 )  // named color fixed alpha

so i want to save it to a file named "color-named-alpha4" 

the intention of prefix is that is what the user types in the editor to invoke the template, so that has to match a command or keyword in the scripting language and cannot be changed.
At the moment there is no use case that would require having an alternate name for a template so we can use prefix for that.
but obviously the name of the file will be used to distinguish versions of the template

Also, there might come a time when the filename might need to be modified to include a category prefix or other rule based name enhancement

**Recommendation:** Use single QFileInfo parameter <<< sorry .. no

reviewing our use cases:

readFromJson( QDir dir, QFileInfo filename )
and you see (from above) the need for the rawfile name as the name of the template should be set by the content,
but we need to know where the file came from to be able to replace it

and
saveToJson( QDir dir, QFileInfo filename )
to give the location it should be wrtten into
but the filename should default to the empty QString as it should normally use the name of the template
 .. having the file name provides an override for the file name
 -- or a future need to modify it according to a naming convention

now we need to think about other use cases

we have covered "have file to add to inventory" -- drag and drop info gives us folder and file name for reading
  and for writing we need only folder as we can make the filename from the template info

another use case is when we have an in-memory template that we want to add to the inventory
  we have a template editor that can create new ones 

Revisting the case of  resourceTemplate.readFromJson() 

Does it cross a boundary of responsibility to have it read in the file AND also add it to the inventory?
A newly instantiated resourceTemplate could pass itself to the templateInventory store using a method on the inventory class like

m_templateInv.addtemplate( this )

---

### 3. Template Name from Content

**User Requirement:** "filename should be set by the content"

not exactly

**My Understanding:**

- File: `cube.json`  <<< use case : read in from existing file, as in - not new made
- JSON content: `{ "prefix": "mycube", "body": "..." }`
- Template : "mycube" (from prefix field) <<< name changed by modifications to template
write to mycube.json



**Implementation:**

```cpp
bool ResourceTemplate::readFromJson(const QFileInfo& fileInfo) {
    // Parse JSON << need to add try-catch for possible json errors
    QJsonObject json = parseFile(fileInfo);
    
    // Extract name from JSON content
    QString prefix = json.value("prefix").toString();
    setName(prefix);           // Name from content
    setPrefix(prefix);
    setPath(fileInfo.filePath());  // << should be raw path i think
    // then process body array to make the text
    return true;
}
```

**Question for User:** Confirm this is correct behavior?

---

### 4. Save Method Filename Override

**User Proposed:**

```cpp
saveToJson(QDir dir, QFileInfo filename)
// filename defaults to empty QString → use template name
// filename provided → override with that name
```

**Alternative A: Separate methods for clarity:** << do this

```cpp
// Save with template's own name
bool saveToJson(const QDir& dir);  // Saves as "{name}.json"

// Save with override name
bool saveToJsonAs(const QDir& dir, const QString& filename);
```

**Implementation:**

```cpp
bool ResourceTemplate::saveToJson(const QDir& dir, const QString& filenameOverride) {
    QString filename = filenameOverride.isEmpty() 
                       ? (name() + ".json") 
                       : filenameOverride;
    
    QString fullPath = dir.filePath(filename);
    QFileInfo outputFile(fullPath);
    
    // Write JSON to file
    // ...
}
```

### 5. With Respect to Tiers

templates can only be written to User folders .. folders of the other tiers will no be writtable
Templates are not orgnaised by tier .. they only look that way in the GUI but that comes from where they are in the file system and how we divide them into tiers according to out concept to organise resources

---

### 6. Bulk Operations for MainWindow Menus




### TemplatesInventory (Collection Management)

```cpp
class TemplatesInventory {
public:
    // Add in-memory template to inventory
    // - Uses template's tier() and name() to generate key
    // - Key format: "tier-name"
    // - Returns true if added, false if duplicate
    bool addTemplate(const ResourceTemplate& tmpl);
    
    // Load multiple templates from a JSON file (bulk import)
    // - Supports VSCode snippet format with multiple templates
    // - Returns number of templates successfully added
    int loadTemplatesFromFile(const QFileInfo& fileInfo);
    
    // Save all templates to a JSON file (bulk export)
    // - Writes in VSCode snippet format
    // - Returns true if successful
    bool saveAllToFile(const QFileInfo& fileInfo);
    
    // Existing methods remain unchanged
    QVariant get(const QString& key) const;
    bool contains(const QString& key) const;
    QList<QVariant> getAll() const;
    int count() const;
    void clear();
    
    // JSON content cache methods (existing)
    QJsonObject getJsonContent(const QString& key) const;
    bool writeJsonContent(const QString& key, const QJsonObject& json, QString& errorMsg);
};
```

---

## Usage Examples

### Use Case 1: Drag-and-Drop Single Template File

```cpp
void MainWindow::onTemplateFileDropped(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    
    // Create and read template
    ResourceTemplate tmpl;
    if (!tmpl.readFromJson(fileInfo)) {
        QMessageBox::warning(this, tr("Error"), tmpl.lastError());
        return;
    }
    
    // Add to inventory (tier already set by readFromJson)
    if (!m_templateInv->addTemplate(tmpl)) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to add template"));
        return;
    }
    
    statusBar()->showMessage(tr("Added template: %1").arg(tmpl.name()));
}
```

### Use Case 2: Template Editor Creates New Template

```cpp
void TemplateEditor::onSaveNewTemplate() {
    ResourceTemplate tmpl;
    tmpl.setName(m_nameEdit->text());
    tmpl.setPrefix(m_prefixEdit->text());
    tmpl.setBody(m_bodyEdit->toPlainText());
    tmpl.setDescription(m_descEdit->toPlainText());
    tmpl.setTier(ResourceTier::User);  // New templates go to User tier
    tmpl.setType(ResourceType::Templates);
    
    // Add to inventory
    if (!m_templateInv->addTemplate(tmpl)) {
        QMessageBox::warning(this, tr("Error"), tr("Template already exists"));
        return;
    }
    
    // Optionally save to disk immediately
    QDir userTemplatesDir = /* ... */;
    if (!tmpl.saveToJson(userTemplatesDir)) {
        QMessageBox::warning(this, tr("Error"), tmpl.lastError());
    }
}
```

### Use Case 3: Menu "Load Templates" (Bulk Import)

```cpp
void MainWindow::onLoadTemplatesAction() {
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Templates File"), QString(),
        tr("JSON Files (*.json)"));
    
    if (fileName.isEmpty()) return;
    
    int count = m_templateInv->loadTemplatesFromFile(QFileInfo(fileName));
    
    if (count > 0) {
        statusBar()->showMessage(tr("Loaded %1 templates").arg(count));
    } else {
        QMessageBox::warning(this, tr("Error"), tr("No templates loaded"));
    }
}
```

### Use Case 4: Menu "Save Templates" (Bulk Export)

```cpp
void MainWindow::onSaveTemplatesAction() {
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Templates File"), QString(),
        tr("JSON Files (*.json)"));
    
    if (fileName.isEmpty()) return;
    
    if (m_templateInv->saveAllToFile(QFileInfo(fileName))) {
        statusBar()->showMessage(tr("Saved %1 templates").arg(m_templateInv->count()));
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to save templates"));
    }
}
```

---

## Questions for User

1. **QDir + QFileInfo vs just QFileInfo**: << see above

2. **Template name from content**: << see above

3. **saveToJson filenameOverride**: Is optional parameter acceptable? << this way

4. **addTemplate tier source**: << tiers are irrelvant to templates

5. **Bulk operations**: skip these for now

6. **Free functions**: <<< remove
---

## Implementation Plan (Based on User Feedback)

### Phase 1: ResourceTemplate File I/O Methods

**Add to ResourceTemplate class:**

```cpp
// Read template from JSON file
// Parameters separated to handle multiple installations with same filename
bool readFromJson(const QDir& dir, const QString& filename);

// Save using template's own name (prefix + ".json")
bool saveToJson(const QDir& dir);

// Save with explicit filename override
bool saveToJsonAs(const QDir& dir, const QString& filename);

// Error reporting
QString lastError() const { return m_lastError; }
```

**Implementation notes:**
- `readFromJson()` extracts prefix from JSON content, uses as template name
- Does NOT automatically add to inventory (separation of concerns)
- Sets path to source location for tracking
- Handles JSON parsing errors with try-catch
- Only User tier directories are writable for saves

### Phase 2: TemplatesInventory Collection Management

**Add to TemplatesInventory class:**

```cpp
// Add in-memory template to inventory
// Generates key from tier-name format
bool addTemplate(const ResourceTemplate& tmpl);
```

**Note:** Bulk operations (loadTemplatesFromFile, saveAllToFile) are DEFERRED for now.

### Phase 3: Update MainWindow

**Simplify menu actions:**

```cpp
// Load Templates menu - single file import
void MainWindow::onLoadTemplatesAction() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("JSON Files (*.json)"));
    
    if (dialog.exec()) {
        QStringList files = dialog.selectedFiles();
        QFileInfo fileInfo(files.first());
        
        ResourceTemplate tmpl;
        if (!tmpl.readFromJson(fileInfo.dir(), fileInfo.fileName())) {
            QMessageBox::warning(this, tr("Error"), tmpl.lastError());
            return;
        }
        
        if (!m_templateInv->addTemplate(tmpl)) {
            QMessageBox::warning(this, tr("Error"), tr("Template already exists"));
            return;
        }
        
        statusBar()->showMessage(tr("Loaded template: %1").arg(tmpl.name()));
    }
}

// Save template - to User tier location
void MainWindow::onSaveTemplateAction() {
    // Get selected template from tree view
    ResourceTemplate tmpl = getSelectedTemplate();
    
    QDir userDir = getUserTemplatesDirectory();
    
    if (!tmpl.saveToJson(userDir)) {
        QMessageBox::warning(this, tr("Error"), tmpl.lastError());
        return;
    }
    
    statusBar()->showMessage(tr("Saved template: %1").arg(tmpl.name()));
}
```

### Phase 4: Remove Free Functions

**Files to clean up:**
- Remove or deprecate functions in `inventoryOperations.hpp/cpp`
- They're replaced by methods on ResourceTemplate and TemplatesInventory

### Phase 5: Testing

**Test scenarios:**
1. Drag-and-drop template file → read → add to inventory
2. Template editor create new → set properties → add to inventory → save to User dir
3. Clone existing template → modify → save with new filename
4. Multiple installations with same filename in different directories
5. Read from Installation tier → modify → save to User tier
6. Error handling for JSON parse failures
7. Error handling for unwritable directories

---

## Detailed Implementation

### ResourceTemplate::readFromJson()

```cpp
bool ResourceTemplate::readFromJson(const QDir& dir, const QString& filename) {
    m_lastError.clear();
    
    QString fullPath = dir.filePath(filename);
    QFileInfo fileInfo(fullPath);
    
    if (!fileInfo.exists()) {
        m_lastError = tr("File does not exist: %1").arg(fullPath);
        return false;
    }
    
    QFile file(fullPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = tr("Cannot open file: %1").arg(file.errorString());
        return false;
    }
    
    QByteArray jsonData = file.readAll();
    file.close();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        m_lastError = tr("JSON parse error at offset %1: %2")
                      .arg(parseError.offset)
                      .arg(parseError.errorString());
        return false;
    }
    
    if (!doc.isObject()) {
        m_lastError = tr("JSON root is not an object");
        return false;
    }
    
    QJsonObject json = doc.object();
    
    // Extract template data
    QString prefix = json.value("prefix").toString();
    if (prefix.isEmpty()) {
        m_lastError = tr("Missing 'prefix' field in JSON");
        return false;
    }
    
    // Set properties from JSON content
    setPrefix(prefix);
    setName(prefix);  // Use prefix as name
    setPath(fullPath);  // Track source location
    setSourcePath(fullPath);
    
    // Body can be array or string
    if (json.value("body").isArray()) {
        QJsonArray bodyArray = json.value("body").toArray();
        QStringList lines;
        for (const QJsonValue& v : bodyArray) {
            lines << v.toString();
        }
        setBody(lines.join('\n'));
    } else {
        setBody(json.value("body").toString());
    }
    
    setDescription(json.value("description").toString());
    
    // Scopes
    if (json.value("scope").isArray()) {
        QJsonArray scopeArray = json.value("scope").toArray();
        QStringList scopes;
        for (const QJsonValue& v : scopeArray) {
            scopes << v.toString();
        }
        setScopes(scopes);
    }
    
    // Source tag
    setSource(json.value("_source").toString("imported"));
    
    // Determine tier from directory path (if possible)
    // This is heuristic - may need refinement
    QString dirPath = dir.absolutePath();
    if (dirPath.contains("user", Qt::CaseInsensitive)) {
        setTier(ResourceTier::User);
    } else if (dirPath.contains("machine", Qt::CaseInsensitive)) {
        setTier(ResourceTier::Machine);
    } else {
        setTier(ResourceTier::Installation);
    }
    
    setType(ResourceType::Templates);
    setExists(true);
    setLastModified(fileInfo.lastModified());
    
    return true;
}
```

### ResourceTemplate::saveToJson()

```cpp
bool ResourceTemplate::saveToJson(const QDir& dir) {
    QString filename = name() + ".json";
    return saveToJsonAs(dir, filename);
}

bool ResourceTemplate::saveToJsonAs(const QDir& dir, const QString& filename) {
    m_lastError.clear();
    
    // Check directory is writable
    QFileInfo dirInfo(dir.absolutePath());
    if (!dirInfo.isWritable()) {
        m_lastError = tr("Directory is not writable: %1").arg(dir.absolutePath());
        return false;
    }
    
    QString fullPath = dir.filePath(filename);
    
    // Build JSON object
    QJsonObject json;
    json["prefix"] = prefix();
    
    // Body as array of lines
    QStringList lines = body().split('\n');
    QJsonArray bodyArray;
    for (const QString& line : lines) {
        bodyArray.append(line);
    }
    json["body"] = bodyArray;
    
    if (!description().isEmpty()) {
        json["description"] = description();
    }
    
    if (!scopes().isEmpty()) {
        QJsonArray scopeArray;
        for (const QString& scope : scopes()) {
            scopeArray.append(scope);
        }
        json["scope"] = scopeArray;
    }
    
    json["_source"] = source();
    
    // Write to file using atomic write
    JsonWriteErrorInfo writeError;
    bool success = JsonWriter::writeObject(
        fullPath.toStdString(), 
        json, 
        writeError, 
        JsonWriter::Indented
    );
    
    if (!success) {
        m_lastError = QString::fromStdString(writeError.formatError());
        return false;
    }
    
    // Update path after successful save
    setPath(fullPath);
    setSourcePath(fullPath);
    setModified(false);
    
    return true;
}
```

### TemplatesInventory::addTemplate()

```cpp
bool TemplatesInventory::addTemplate(const ResourceTemplate& tmpl) {
    if (!tmpl.isValid()) {
        qWarning() << "TemplatesInventory::addTemplate: Invalid template";
        return false;
    }
    
    // Generate key: tier-name
    QString tierName = resourceMetadata::tierToString(tmpl.tier());
    QString key = QString("%1-%2").arg(tierName, tmpl.name());
    
    // Check for duplicates
    if (m_templates.contains(key)) {
        qWarning() << "TemplatesInventory::addTemplate: Duplicate key:" << key;
        return false;
    }
    
    // Store in hash
    m_templates.insert(key, QVariant::fromValue(tmpl));
    
    return true;
}
```

---

## Notes

- Current implementation uses `TemplateParser` internally - this will continue
- JSON schema validation happens in `parseJsonFile()` - unchanged
- Legacy template conversion supported - unchanged
- Tier naming (installation/machine/user) handled by existing `resourceMetadata::tierToString()`

