# Drag and Drop Implementation Plan for Template Resources

**Date:** December 25, 2025  
**Feature:** Drop JSON template files into scadtemplates.exe left panel  
**Scope:** Templates only (other resource types to follow)

---

## 1. Overview

Enable users to drag .json template files from their file system and drop them into the template editor pane. Files will be validated, optionally converted from legacy format, and copied to the `newresources/templates` folder in the user's personal OpenSCAD directory.

---

## 2. Target Widget Architecture

**Drop Target:** `leftPanel` widget in MainWindow  
- Contains both `m_templateTree` (QTreeView) and template editor group
- Call `leftPanel->setAcceptDrops(true)` in `MainWindow::setupUI()`
- Override `dragEnterEvent()` and `dropEvent()` in MainWindow

**Alternative considered:** Direct drop on `m_templateTree`, but leftPanel provides larger drop zone and better UX.

---

## 3. Qt Drag and Drop Mechanics

### 3.1 Drop Action
- **Accept only:** `Qt::CopyAction`
- **Reject:** `Qt::MoveAction`, `Qt::LinkAction`
- **Rationale:** External file drops from OS file managers always provide CopyAction; we have no control over the source and must not delete user files.

### 3.2 MIME Type
- **Primary:** `text/uri-list` (URLs to dropped files)
- Check via `event->mimeData()->hasUrls()`
- Extract URLs via `event->mimeData()->urls()`

### 3.3 Event Flow
```cpp
void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    // Accept only if over left panel and has file URLs
    if (leftPanel->geometry().contains(event->position().toPoint())) {
        if (event->mimeData()->hasUrls() && 
            event->proposedAction() == Qt::CopyAction) {
            event->acceptProposedAction();
        }
    }
}

void MainWindow::dropEvent(QDropEvent* event) {
    // Process dropped files
    QList<QUrl> urls = event->mimeData()->urls();
    processDroppedTemplates(urls);
    event->acceptProposedAction();
}
```

---

## 4. Validation Pipeline

### Phase 1: Extension Filter
```
For each dropped URL:
  ├─ Convert to local file path
  ├─ Check extension == ".json" (case-insensitive)
  ├─ If not: add to rejected list ("Not a .json file")
  └─ If yes: proceed to Phase 2
```

### Phase 2: JSON Parsing
```
For each .json file:
  ├─ Open file for reading
  ├─ Parse as JSON (QJsonDocument)
  ├─ If parse error: add to rejected list ("Invalid JSON: <error>")
  └─ If valid: proceed to Phase 3
```

### Phase 3: Template Format Detection & Validation
```
Use TemplateParser::parseJson():
  ├─ Detect format (legacy vs modern vs invalid)
  ├─ If legacy: auto-convert with LegacyTemplateConverter
  │   └─ Set source="legacy-converted", version=1
  ├─ If modern: validate VS Code snippet structure
  ├─ If invalid: add to rejected list ("Not a valid template: <reason>")
  └─ If valid: add to accepted list with parsed templates
```

**Existing code leveraged:**
- `scadtemplates::TemplateParser::parseJson()` - already handles legacy detection
- `scadtemplates::LegacyTemplateConverter` - conversion logic
- Format markers: `_format: "vscode-snippet"`, `_source`, `_version`

---

## 5. Destination Folder Policy

### 5.1 Folder Requirements
- Must exist: `newresources/templates` subdirectory
- Location: One of two personal tier paths (in order)

**Windows:**
1. `%USERPROFILE%/Documents/OpenSCAD/newresources/templates`
2. `%USERPROFILE%/AppData/Local/OpenSCAD/newresources/templates`

**macOS:**
1. `~/Documents/OpenSCAD/newresources/templates`
2. `~/Library/Application Support/OpenSCAD/newresources/templates`

**Linux:**
1. `~/Documents/OpenSCAD/newresources/templates`
2. `~/.local/share/openscad/newresources/templates`

### 5.2 Folder Discovery
```cpp
QString MainWindow::findNewResourcesTemplatesFolder() const {
    QStringList candidates;
    
    // Platform-specific candidate paths (Documents first, then AppData/config)
    QString docs = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (!docs.isEmpty()) {
        candidates << QDir(docs).filePath("OpenSCAD/newresources/templates");
    }
    
    QString appData = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    if (!appData.isEmpty()) {
        candidates << QDir(appData).filePath("OpenSCAD/newresources/templates");
    }
    
    // Return first existing path
    for (const QString& path : candidates) {
        if (QDir(path).exists()) {
            return path;
        }
    }
    
    return QString(); // Not found
}
```

### 5.3 Missing Folder Handling
**Policy:** Do NOT auto-create folders (follows OpenSCAD convention)

**Warning Dialog:**
```
Title: Cannot Accept Templates

Message:
The 'newresources/templates' folder does not exist in your personal OpenSCAD directory.

Please create it in one of these locations:
  • ~/Documents/OpenSCAD/newresources/templates
  • ~/AppData/Local/OpenSCAD/newresources/templates

Then try dropping the files again.

Buttons: [OK]
```

After showing warning, abort the drop operation.

---

## 6. File Copying

### 6.1 Copy Logic
```cpp
QString destFolder = findNewResourcesTemplatesFolder();

for (const auto& [sourcePath, templates] : acceptedFiles) {
    QString fileName = QFileInfo(sourcePath).fileName();
    QString targetPath = QDir(destFolder).filePath(fileName);
    
    // Handle name conflicts
    if (QFile::exists(targetPath)) {
        targetPath = generateUniqueFileName(destFolder, fileName);
    }
    
    if (QFile::copy(sourcePath, targetPath)) {
        copiedFiles.append(targetPath);
    } else {
        copyFailures.append({sourcePath, "Failed to copy file"});
    }
}
```

### 6.2 Name Conflict Resolution
**Strategy:** Rename with numeric suffix (non-destructive)

```cpp
QString generateUniqueFileName(const QString& folder, const QString& baseName) {
    QString name = baseName;
    QString base = QFileInfo(baseName).completeBaseName();
    QString ext = QFileInfo(baseName).suffix();
    
    int counter = 1;
    while (QFile::exists(QDir(folder).filePath(name))) {
        name = QString("%1_%2.%3").arg(base).arg(counter++).arg(ext);
    }
    
    return QDir(folder).filePath(name);
}
```

**Examples:**
- `template.json` exists → save as `template_1.json`
- `template_1.json` exists → save as `template_2.json`

---

## 7. User Feedback

### 7.1 Results Popup (Single Dialog)
**Trigger:** After all files processed (accept/reject/copy)

**Structure:**
```
Title: Drop Results

Message:
[If accepted files:]
Successfully added N template(s):
  • template1.json
  • legacy_template.json (converted)
  • template2_1.json (renamed)

[If rejected files:]
M file(s) rejected:
  • not_a_json.txt
    Reason: Not a .json file
  • invalid.json
    Reason: Invalid JSON: Unexpected token at line 5
  • wrong_format.json
    Reason: Not a valid template: missing required fields

Buttons: [OK]
```

### 7.2 Partial Success
- **Policy:** Accept all valid files, reject invalid ones
- Valid files are copied even if some are rejected
- User gets complete feedback in one popup

---

## 8. Inventory Refresh

After successful copy:
```cpp
// Refresh the resource store and tree model
refreshInventory();
```

This triggers:
1. `ResourceStore` rescan of user tier locations
2. `TemplateTreeModel` rebuild
3. UI update showing new templates

---

## 9. Implementation Steps

### Step 1: Enable Drop Support
**Files:** `mainwindow.h`, `mainwindow.cpp`

- Add `setAcceptDrops(true)` to `leftPanel` in `setupUI()`
- Declare override methods:
  ```cpp
  protected:
      void dragEnterEvent(QDragEnterEvent* event) override;
      void dropEvent(QDropEvent* event) override;
  ```

### Step 2: Create Helper Methods
**File:** `mainwindow.cpp`

```cpp
private:
    QString findNewResourcesTemplatesFolder() const;
    
    struct ValidationResult {
        QString filePath;
        QString errorReason;
        std::vector<scadtemplates::Template> templates;
        bool isValid() const { return errorReason.isEmpty(); }
    };
    
    ValidationResult validateTemplateFile(const QString& filePath);
    QString generateUniqueFileName(const QString& folder, const QString& baseName);
    void processDroppedTemplates(const QList<QUrl>& urls);
    void showDropResults(const QStringList& accepted, 
                         const QList<QPair<QString, QString>>& rejected);
```

### Step 3: Implement Validation Pipeline
**Logic flow:**
1. Filter by extension
2. Parse JSON
3. Validate template format (use existing TemplateParser)
4. Return structured result

### Step 4: Implement dropEvent Handler
**Core logic:**
```cpp
void MainWindow::dropEvent(QDropEvent* event) {
    QList<QUrl> urls = event->mimeData()->urls();
    
    // Validate all files
    QList<ValidationResult> results;
    for (const QUrl& url : urls) {
        QString filePath = url.toLocalFile();
        if (!filePath.endsWith(".json", Qt::CaseInsensitive)) {
            results.append({filePath, "Not a .json file", {}});
            continue;
        }
        results.append(validateTemplateFile(filePath));
    }
    
    // Check destination exists
    QString destFolder = findNewResourcesTemplatesFolder();
    if (destFolder.isEmpty()) {
        showMissingFolderWarning();
        return;
    }
    
    // Copy valid files
    QStringList accepted;
    QList<QPair<QString, QString>> rejected;
    
    for (const auto& result : results) {
        if (result.isValid()) {
            QString target = copyToDestination(result.filePath, destFolder);
            if (!target.isEmpty()) {
                accepted.append(QFileInfo(target).fileName());
            }
        } else {
            rejected.append({QFileInfo(result.filePath).fileName(), result.errorReason});
        }
    }
    
    // Show results and refresh
    showDropResults(accepted, rejected);
    if (!accepted.isEmpty()) {
        refreshInventory();
    }
    
    event->acceptProposedAction();
}
```

### Step 5: Legacy Conversion Integration
**Ensure:** `LegacyTemplateConverter` sets correct metadata
- Override `_source` to `"legacy-converted"`
- Set `_version` to `1`
- May require small update to `templateToModernJson()`

### Step 6: Manual Testing
**Test cases:**
1. ✅ Drop single modern template → success
2. ✅ Drop single legacy template → auto-convert → success
3. ✅ Drop multiple mixed files → partial success
4. ✅ Drop with missing `newresources/templates` → warning
5. ✅ Drop with name conflicts → rename with suffix
6. ✅ Drop non-JSON files → reject with reason
7. ✅ Drop invalid JSON → reject with parse error
8. ✅ Drop JSON that isn't a template → reject with validation error

### Step 7: Automated Tests (Future)
**Framework:** Qt Test + QSignalSpy

**Test fixtures:**
- Sample modern templates
- Sample legacy templates
- Invalid JSON files
- Non-JSON files

**Test scenarios:**
```cpp
void TestDragDrop::testSingleModernTemplate() {
    // Create test file
    // Simulate drop event
    // Verify file copied
    // Verify inventory updated
}

void TestDragDrop::testLegacyConversion() {
    // Create legacy template
    // Simulate drop
    // Verify converted format
    // Verify metadata (_source, _version)
}

void TestDragDrop::testMissingFolder() {
    // Remove newresources folder
    // Simulate drop
    // Verify warning shown
    // Verify no files copied
}
```

---

## 10. Key Design Decisions Summary

| Decision | Choice | Rationale |
|----------|--------|-----------|
| **Drop target** | `leftPanel` widget | Larger drop zone, covers tree + editor |
| **Drop action** | CopyAction only | Safe for external drops, no risk of deleting user files |
| **Validation** | Use existing TemplateParser | Leverages tested code, handles legacy + modern |
| **Legacy handling** | Auto-convert on drop | Seamless UX, preserves backward compat |
| **Folder policy** | No auto-create | Follows OpenSCAD convention |
| **Folder order** | Documents, then AppData | User-visible location preferred |
| **Partial success** | Accept valid, reject invalid | User-friendly, shows all issues |
| **Name conflicts** | Rename with suffix | Non-destructive, preserves both |
| **Feedback** | Single results popup | Clean, comprehensive |
| **Inventory refresh** | Auto after success | Immediate visibility |

---

## 11. Future Enhancements (Out of Scope)

- Drag and drop for other resource types (fonts, color schemes, etc.)
- Drag from tree to reorder/move within app (would use MoveAction)
- Visual drop feedback (highlight on hover)
- Progress bar for large batches
- Undo support
- Drag out to export

---

## 12. References

**Qt Documentation:**
- [Drag and Drop](https://doc.qt.io/qt-6/dnd.html)
- [QDragEnterEvent](https://doc.qt.io/qt-6/qdragenterevent.html)
- [QDropEvent](https://doc.qt.io/qt-6/qdropevent.html)
- [QMimeData](https://doc.qt.io/qt-6/qmimedata.html)

**Existing Code:**
- `scadtemplates::TemplateParser` - JSON parsing
- `scadtemplates::LegacyTemplateConverter` - legacy conversion
- `ResourceStore` / `TemplateTreeModel` - inventory management
- `ResourceLocationManager` - path discovery

---

## 13. Testing Checklist

### Manual Testing
- [ ] Drop single modern template
- [ ] Drop single legacy template  
- [ ] Drop multiple files (mixed valid/invalid)
- [ ] Drop with missing folder
- [ ] Drop with name conflicts
- [ ] Drop non-JSON files
- [ ] Drop invalid JSON
- [ ] Drop wrong format JSON
- [ ] Verify inventory refresh
- [ ] Verify copied file content matches source

### Automated Testing (Phase 2)
- [ ] Unit tests for validation logic
- [ ] Unit tests for conflict resolution
- [ ] Integration tests with QTest
- [ ] Regression suite for other features

---

**End of Plan**
