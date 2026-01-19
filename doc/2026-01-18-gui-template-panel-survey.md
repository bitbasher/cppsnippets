# GUI Template Panel Architecture Survey

**Date:** January 18, 2026  
**Branch:** resScannerOption1  
**Purpose:** Document existing GUI before refactoring to new model/view architecture  
**Focus:** Template editing panel and resource location preferences

---

## Executive Summary

The current GUI has a **dual-panel layout** with template management on the left and a main text editor on the right. The template panel is partially functional but uses old APIs that were removed during backend refactoring. The preferences dialog manages resource locations across Installation/Machine/User tiers.

**Key Finding:** The **template editing widgets and layout are well-designed** and can be preserved. Only the data connection needs updating to use the new TemplatesInventory + ResourceIndexer backend.

---

## Main Window Layout

### Overall Structure

```
MainWindow (QMainWindow)
├── Menu Bar
│   ├── File (New, Open, Save, Save As, Exit)
│   ├── Edit (Undo, Redo, Cut, Copy, Paste, Select All, Preferences)
│   ├── Templates (Load Templates, Save Templates)
│   └── Help (About)
├── Central Widget (QWidget with QHBoxLayout)
    └── QSplitter (Horizontal, ratio 1:2)
        ├── Left Panel (400px)
        │   ├── Templates QGroupBox
        │   │   ├── Search QLineEdit (clearable)
        │   │   ├── Template List (QTreeView + QStandardItemModel)
        │   │   └── Action Buttons (QHBoxLayout)
        │   │       ├── New QPushButton
        │   │       ├── Delete QPushButton
        │   │       ├── Copy QPushButton
        │   │       └── Edit QPushButton
        │   └── Template Editor QGroupBox
        │       ├── Name QLineEdit (m_prefixEdit)
        │       ├── Source QLineEdit (m_sourceEdit, read-only)
        │       ├── Description QTextEdit (m_descriptionEdit, maxHeight=80)
        │       ├── Body QTextEdit (m_bodyEdit, maxHeight=150, fixed-width font)
        │       └── Editor Buttons (QHBoxLayout)
        │           ├── Save QPushButton
        │           └── Cancel QPushButton
        └── Right Panel (800px)
            └── Editor QGroupBox
                └── Main QPlainTextEdit (m_editor, fixed-width font)
```

### Splitter Configuration

```cpp
// Initial sizes (1:2 ratio)
splitter->setSizes({400, 800});
```

---

## Template Panel Widgets

### 1. Search Bar

**Widget:** `m_searchEdit` (QLineEdit)

**Properties:**
- Placeholder: "Search templates..."
- Clear button enabled: Yes
- Position: Top of template list

**Signal:** 
```cpp
connect(m_searchEdit, &QLineEdit::textChanged, 
        this, &MainWindow::onSearch);
```

**Handler:** `void MainWindow::onSearch(const QString& text)`
- **Status:** Stub implementation
- **TODO:** Filter tree based on search text

---

### 2. Template List

**Widget:** `m_templateTree` (QTreeView)

**Model:** `QStandardItemModel* m_inventory`
- Columns: "Name", "Tier", "Path", "ID"
- Populated by ResourceScanner::populateModel(ResourceType::Templates)

**Properties:**
- Selection mode: SingleSelection
- Alternating row colors: Yes
- Root decorated: No (flat list)
- Sorting enabled: Yes

**Signal:**
```cpp
connect(m_templateTree->selectionModel(), 
        &QItemSelectionModel::selectionChanged,
        this, &MainWindow::onInventorySelectionChanged);
```

**Handler:** `void MainWindow::onInventorySelectionChanged()`
- Gets selected row index
- Retrieves ResourceItem from Qt::UserRole
- Calls `onInventoryItemSelected(resItem)`
- **Status:** ⚠️ Broken - tries to convert to ResourceItem instead of ResourceTemplate

---

### 3. Template Action Buttons

**Layout:** Horizontal button row below template list

#### New Button
**Widget:** `m_newBtn` (QPushButton)  
**Label:** "New"  
**Signal:** `clicked → MainWindow::onNewTemplate()`  
**Enabled:** When NOT in edit mode  
**Action:**
- Enters edit mode
- Clears all editor fields
- Sets focus to name field
- Clears tree selection

#### Delete Button
**Widget:** `m_deleteBtn` (QPushButton)  
**Label:** "Delete"  
**Signal:** `clicked → MainWindow::onDeleteTemplate()`  
**Enabled:** When NOT editing AND has selection  
**Action:**
- Shows confirmation dialog
- **TODO:** Implement delete logic
- Updates status bar

#### Copy Button
**Widget:** `m_copyBtn` (QPushButton)  
**Label:** "Copy"  
**Signal:** `clicked → MainWindow::onCopyTemplate()`  
**Enabled:** When NOT editing AND has selection  
**Action:**
- Enters edit mode
- Copies selected template fields
- Appends "-copy" to name
- Sets source to "cppsnippet-made"
- Clears tree selection
- Sets focus to name field

#### Edit Button
**Widget:** `m_editBtn` (QPushButton)  
**Label:** "Edit"  
**Signal:** `clicked → MainWindow::onEditTemplate()`  
**Enabled:** When NOT editing AND has selection  
**Action:**
- Enters edit mode
- Makes name and description fields editable
- Updates button states

---

### 4. Template Editor Fields

#### Name Field
**Widget:** `m_prefixEdit` (QLineEdit)  
**Label:** "Name:"  
**Read-only:** Yes (except in edit mode)  
**Purpose:** Template name/prefix

#### Source Field
**Widget:** `m_sourceEdit` (QLineEdit)  
**Label:** "Source:"  
**Read-only:** Yes (always)  
**Purpose:** Shows tier (Installation/Machine/User) or source tag

#### Description Field
**Widget:** `m_descriptionEdit` (QTextEdit)  
**Label:** "Description:"  
**Max Height:** 80px  
**Read-only:** Yes (except in edit mode)  
**Purpose:** Template description

#### Body Field
**Widget:** `m_bodyEdit` (QTextEdit)  
**Label:** "Body:"  
**Max Height:** 150px  
**Font:** Fixed-width (QFontDatabase::systemFont(QFontDatabase::FixedFont))  
**Read-only:** No (editable in edit mode)  
**Purpose:** Template OpenSCAD code with placeholders

---

### 5. Editor Control Buttons

#### Save Button
**Widget:** `m_saveBtn` (QPushButton)  
**Label:** "Save"  
**Signal:** `clicked → MainWindow::onSaveTemplate()`  
**Enabled:** Only when in edit mode  
**Tooltip:** 
- Edit mode: "Save changes"
- Normal: "Save only active in Edit mode"

**Action:**
```cpp
void MainWindow::onSaveTemplate() {
    // Validate name and body not empty
    // Create ResourceTemplate object
    // Set prefix, body, description, format, source
    // Call saveTemplateToUser(tmpl)
    // Exit edit mode
    // Refresh inventory
    // Update button states
}
```

**TODO:** `bool MainWindow::saveTemplateToUser(const ResourceTemplate& tmpl)`

#### Cancel Button
**Widget:** `m_cancelBtn` (QPushButton)  
**Label:** "Cancel"  
**Signal:** `clicked → MainWindow::onCancelEdit()`  
**Enabled:** Only when in edit mode  
**Tooltip:**
- Edit mode: "Cancel changes"
- Normal: "Cancel only active in Edit mode"

**Action:**
- Exits edit mode
- Restores fields to read-only
- Reloads selection (if any)
- Clears fields (if no selection)
- Updates button states

---

## Edit Mode State Machine

### States

```
┌─────────────┐
│   NORMAL    │
│             │
│ - Viewing   │
│ - Read-only │
└─────────────┘
       │
       │ New/Copy/Edit button
       ▼
┌─────────────┐
│  EDIT MODE  │
│             │
│ - Editing   │
│ - Fields    │
│   writable  │
└─────────────┘
       │
       │ Save/Cancel button
       ▼
┌─────────────┐
│   NORMAL    │
└─────────────┘
```

### State Variable

```cpp
bool m_editMode = false;  // Member variable
```

### Button State Matrix

| Button  | Normal + No Selection | Normal + Selection | Edit Mode |
|---------|----------------------|-------------------|-----------|
| New     | ✅ Enabled           | ✅ Enabled        | ❌ Disabled |
| Delete  | ❌ Disabled          | ✅ Enabled        | ❌ Disabled |
| Copy    | ❌ Disabled          | ✅ Enabled        | ❌ Disabled |
| Edit    | ❌ Disabled          | ✅ Enabled        | ❌ Disabled |
| Save    | ❌ Disabled          | ❌ Disabled       | ✅ Enabled |
| Cancel  | ❌ Disabled          | ❌ Disabled       | ✅ Enabled |

### Field Editability

| Field       | Normal Mode      | Edit Mode   |
|-------------|-----------------|-------------|
| Name        | Read-only       | Editable    |
| Source      | Read-only       | Read-only   |
| Description | Read-only       | Editable    |
| Body        | Read-only       | Editable    |

**Implementation:**
```cpp
void MainWindow::updateTemplateButtons() {
    bool hasSelection = !m_selectedItem.path().isEmpty();
    bool isEditing = m_editMode;
    
    m_newBtn->setEnabled(!isEditing);
    m_deleteBtn->setEnabled(!isEditing && hasSelection);
    m_copyBtn->setEnabled(!isEditing && hasSelection);
    m_editBtn->setEnabled(!isEditing && hasSelection);
    m_saveBtn->setEnabled(isEditing);
    m_cancelBtn->setEnabled(isEditing);
    
    // Update tooltips
    m_saveBtn->setToolTip(isEditing ? 
        tr("Save changes") : 
        tr("Save only active in Edit mode"));
    m_cancelBtn->setToolTip(isEditing ? 
        tr("Cancel changes") : 
        tr("Cancel only active in Edit mode"));
}
```

---

## Signal/Slot Catalog

### Template Panel Signals

| Signal Source | Signal | Slot Handler | Action |
|--------------|--------|--------------|--------|
| m_searchEdit | textChanged(QString) | MainWindow::onSearch(QString) | Filter template list |
| m_templateTree selectionModel | selectionChanged() | MainWindow::onInventorySelectionChanged() | Load template in editor |
| m_newBtn | clicked() | MainWindow::onNewTemplate() | Enter edit mode, clear fields |
| m_deleteBtn | clicked() | MainWindow::onDeleteTemplate() | Confirm and delete template |
| m_copyBtn | clicked() | MainWindow::onCopyTemplate() | Enter edit mode, copy template |
| m_editBtn | clicked() | MainWindow::onEditTemplate() | Enter edit mode |
| m_saveBtn | clicked() | MainWindow::onSaveTemplate() | Validate and save template |
| m_cancelBtn | clicked() | MainWindow::onCancelEdit() | Exit edit mode, restore |

### Main Editor Signals

| Signal Source | Signal | Slot Handler | Action |
|--------------|--------|--------------|--------|
| m_editor | textChanged() | Lambda (inline) | Set m_modified flag, update title |

### Menu Signals

#### File Menu
| Action | Shortcut | Slot | Action |
|--------|----------|------|--------|
| New | Ctrl+N | MainWindow::onNewFile() | Clear editor, prompt save |
| Open | Ctrl+O | MainWindow::onOpenFile() | Open .scad file dialog |
| Save | Ctrl+S | MainWindow::onSaveFile() | Save current file |
| Save As | Ctrl+Shift+S | MainWindow::onSaveFileAs() | Save with new name |
| Exit | Ctrl+Q | QMainWindow::close() | Close application |

#### Edit Menu
| Action | Shortcut | Slot | Action |
|--------|----------|------|--------|
| Undo | Ctrl+Z | m_editor::undo() | Undo editor change |
| Redo | Ctrl+Y | m_editor::redo() | Redo editor change |
| Cut | Ctrl+X | m_editor::cut() | Cut selection |
| Copy | Ctrl+C | m_editor::copy() | Copy selection |
| Paste | Ctrl+V | m_editor::paste() | Paste clipboard |
| Select All | Ctrl+A | m_editor::selectAll() | Select all text |
| Preferences | Ctrl+, | MainWindow::onPreferences() | Open preferences dialog |

#### Templates Menu
| Action | Slot | Action |
|--------|------|--------|
| Load Templates | Lambda | Load JSON template file |
| Save Templates | Lambda | Save templates to JSON |

#### Help Menu
| Action | Slot | Action |
|--------|------|--------|
| About | Lambda | Show AboutDialog |

---

## Data Flow

### Template Selection Flow

```
User clicks template in QTreeView
    ↓
QItemSelectionModel::selectionChanged signal
    ↓
MainWindow::onInventorySelectionChanged()
    ↓
Get QModelIndex from selection
    ↓
Get QStandardItem from model
    ↓
Retrieve ResourceItem from Qt::UserRole
    ↓
MainWindow::onInventoryItemSelected(ResourceItem)
    ↓
MainWindow::populateEditorFromSelection(ResourceItem)
    ↓
Load JSON file from item.path()
    ↓
Parse JSON (legacy or modern format)
    ↓
Populate editor widgets:
    - m_prefixEdit ← prefix
    - m_bodyEdit ← body
    - m_descriptionEdit ← description
    - m_sourceEdit ← tier or source tag
```

### Template Save Flow

```
User clicks Save button
    ↓
MainWindow::onSaveTemplate()
    ↓
Validate name and body not empty
    ↓
Create ResourceTemplate object
    ↓
Set properties from editor widgets
    ↓
MainWindow::saveTemplateToUser(ResourceTemplate)
    ↓
TODO: Write JSON to user templates folder
    ↓
refreshInventory()  (TODO: implement)
    ↓
Exit edit mode
    ↓
Update button states
```

---

## Issues with Current Implementation

### 1. QVariant Type Mismatch

**Location:** mainwindow.cpp:463

```cpp
// ❌ WRONG - tries to convert to base class
resourceInventory::ResourceItem resItem = 
    itemData.value<resourceInventory::ResourceItem>();
```

**Fix:**
```cpp
// ✅ CORRECT - use derived class
resourceInventory::ResourceTemplate tmpl = 
    itemData.value<resourceInventory::ResourceTemplate>();
```

### 2. Model Population Type Error

**Location:** resourceScanner.cpp:141

```cpp
// ❌ WRONG - checking base class
if (!var.canConvert<resourceInventory::ResourceItem>()) {
    continue;
}
resourceInventory::ResourceItem tmpl = 
    var.value<resourceInventory::ResourceItem>();
```

**Fix:**
```cpp
// ✅ CORRECT - use derived class
if (!var.canConvert<resourceInventory::ResourceTemplate>()) {
    continue;
}
resourceInventory::ResourceTemplate tmpl = 
    var.value<resourceInventory::ResourceTemplate>();
```

### 3. Missing ResourceItem Storage in Model

**Location:** resourceScanner.cpp:148-152

```cpp
QList<QStandardItem*> row;
row.append(new QStandardItem(tmpl.displayName()));
row.append(new QStandardItem(resourceMetadata::tierToString(tmpl.tier())));
row.append(new QStandardItem(tmpl.path()));
row.append(new QStandardItem(tmpl.uniqueID()));
model->appendRow(row);
```

**Missing:** Store the ResourceTemplate object in Qt::UserRole

**Fix:**
```cpp
QList<QStandardItem*> row;
QStandardItem* nameItem = new QStandardItem(tmpl.displayName());
nameItem->setData(QVariant::fromValue(tmpl), Qt::UserRole);  // Store full object
row.append(nameItem);
row.append(new QStandardItem(resourceMetadata::tierToString(tmpl.tier())));
row.append(new QStandardItem(tmpl.path()));
row.append(new QStandardItem(tmpl.uniqueID()));
model->appendRow(row);
```

### 4. Stub Implementations (TODOs)

| Function | Status | Location |
|----------|--------|----------|
| `onSearch(QString)` | Stub | mainwindow.cpp:338 |
| `applyFilterToTree(QString)` | Stub | mainwindow.cpp:556 |
| `saveTemplateToUser(ResourceTemplate)` | Stub | mainwindow.cpp:552 |
| `refreshInventory()` | Missing | - |
| Template deletion | Stub | mainwindow.cpp:356 |

### 5. Removed APIs Used by Other Components

**PreferencesDialog** tries to use removed methods:
- ResourceLocation::setEnabled()
- ResourceLocation::setExists()
- ResourceLocation::setWritable()
- ResourceLocation::setHasResourceFolders()
- ResourceItem::isEnabled()
- ResourceItem::exists()
- ResourceItem::setEnabled()
- ResourceItem::setExists()

**Missing Files:**
- platformInfo/resourceLocationManager.hpp
- export.hpp (used by inventoryOperations.hpp)

---

## Preserved vs. Rebuild Strategy

### ✅ PRESERVE (Works with new architecture)

1. **Layout Structure**
   - Left/Right splitter ratio (1:2)
   - Template panel positioning
   - Editor panel on right
   - All widgets and containers

2. **Widget Types**
   - QLineEdit for search
   - QTreeView for template list
   - QTextEdit for body/description
   - QPushButton for actions
   - QGroupBox for sections

3. **Visual Design**
   - Alternating row colors
   - Fixed-width fonts for code
   - Height constraints (80px description, 150px body)
   - Button layout and spacing

4. **Edit Mode State Machine**
   - Boolean flag m_editMode
   - Button enable/disable logic
   - Field read-only control
   - Tooltip updates

5. **Signal/Slot Structure**
   - All button connections
   - Selection change handler
   - Search text changed
   - Menu actions

### 🔧 FIX (Small changes needed)

1. **QVariant Conversions**
   - Change ResourceItem → ResourceTemplate
   - Store full object in Qt::UserRole
   - Update all value<>() calls

2. **Model Population**
   - Fix type checking in ResourceScanner
   - Add object storage to model rows
   - Update column headers if needed

3. **Selection Handler**
   - Update onInventorySelectionChanged() to use ResourceTemplate
   - Fix member variable type (m_selectedItem)

4. **JSON Parsing**
   - Update populateEditorFromSelection()
   - Use TemplatesInventory::parseJsonFile()
   - Handle modern wrapper format

### 🆕 IMPLEMENT (Missing functionality)

1. **Search/Filter**
   - QSortFilterProxyModel for tree filtering
   - Case-insensitive search
   - Search in name/description

2. **Save Template**
   - Write JSON to user templates folder
   - Create directory if needed
   - Generate unique filename
   - Update inventory after save

3. **Delete Template**
   - Delete JSON file from disk
   - Update inventory
   - Handle read-only locations

4. **Inventory Refresh**
   - Re-scan templates after save/delete
   - Update model without recreating
   - Preserve selection if possible

---

## Preferences Dialog Architecture

**Note:** This is separate from template panel but shares resource location concerns

### Structure

```
PreferencesDialog (QDialog)
├── PlatformInfoWidget (shows OS/arch/platform)
├── QTabWidget
│   ├── InstallationTab (shows detected installation paths)
│   ├── MachineTab (configure machine-wide resources)
│   └── UserTab (configure user-specific resources)
└── DialogButtonBar (OK, Cancel, Restore Defaults)
```

### Tabs

#### InstallationTab
- **Purpose:** Display detected OpenSCAD installation paths
- **Fields:** 
  - Installation path (read-only)
  - Exists checkbox (read-only)
  - Writable checkbox (read-only)
  - Has resources checkbox (read-only)

#### MachineTab
- **Purpose:** Configure machine-wide resource locations
- **Fields:**
  - Machine path (editable with browse button)
  - Enable checkbox
  - Exists indicator
  - Writable indicator
  - Has resources indicator

#### UserTab
- **Purpose:** Configure user-specific resource locations
- **Fields:**
  - User path (editable with browse button)
  - Enable checkbox
  - Has resources indicator

### Removed API Issues

All tabs use:
- `ResourceLocation::setEnabled()` - ❌ Removed
- `ResourceLocation::setExists()` - ❌ Removed
- `ResourceLocation::setWritable()` - ❌ Removed
- `ResourceLocation::setHasResourceFolders()` - ❌ Removed

These were removed during Phase 2 cleanup. Need to either:
1. Re-add as status fields (not setters)
2. Redesign preferences to work without them
3. Use ResourceScanner to check status instead

---

## Recommended Refactoring Plan

### Phase 1: Fix Template Panel Display (Simple)

**Goal:** Get template list showing with correct data

**Changes:**
1. Fix ResourceScanner::populateModel():
   - Change ResourceItem → ResourceTemplate
   - Store full object in Qt::UserRole
   - Add uniqueID column

2. Fix MainWindow::onInventorySelectionChanged():
   - Change ResourceItem → ResourceTemplate
   - Update member variable type

3. Build and test - templates should display

**Estimated Changes:** 10 lines of code

### Phase 2: Implement Template Actions (Medium)

**Goal:** Make New/Edit/Save/Delete work

**Changes:**
1. Implement saveTemplateToUser():
   - Create user templates directory
   - Generate JSON file
   - Write modern format

2. Implement refreshInventory():
   - Rescan user templates
   - Update model incrementally

3. Implement deleteTemplate():
   - Delete JSON file
   - Update model

4. Implement search filter:
   - QSortFilterProxyModel
   - Filter by name/description

**Estimated Changes:** 150 lines of code

### Phase 3: Consider Model/View Redesign (Complex - Optional)

**Goal:** Replace QStandardItemModel with TemplateTreeModel

**Changes:**
1. Update TemplateTreeModel to work with TemplatesInventory
   - Remove ResourceStore dependency
   - Add TemplatesInventory* member
   - Build tree from inventory data

2. Update MainWindow to use TemplateTreeModel
   - Replace QStandardItemModel
   - Update selection handling
   - Add tier/location grouping

3. Add real-time updates:
   - Connect inventory signals
   - Auto-refresh on file changes

**Estimated Changes:** 500+ lines of code

**Benefits:**
- Proper model/view separation
- Automatic updates
- Better tree structure (tier → location → templates)
- No data duplication

**Drawbacks:**
- Major refactoring effort
- More complexity
- May not be needed for simple list

---

## Widget Measurements

### Template Panel Dimensions

| Widget | Min Width | Max Width | Min Height | Max Height |
|--------|-----------|-----------|------------|------------|
| Search field | - | - | - | 25px (default) |
| Template tree | - | - | 200px | - |
| Name field | - | - | - | 25px (default) |
| Source field | - | - | - | 25px (default) |
| Description | - | - | - | 80px |
| Body editor | - | - | - | 150px |
| Button bar | - | - | - | 30px (default) |

### Splitter Ratio

```cpp
splitter->setSizes({400, 800});
// Left panel: 400px (33%)
// Right panel: 800px (67%)
```

### Font Settings

```cpp
// Body editor and main editor
QFontDatabase::systemFont(QFontDatabase::FixedFont)
```

---

## Code Fragments for Reference

### Edit Mode Entry (New Template)

```cpp
void MainWindow::onNewTemplate() {
    m_editMode = true;
    m_prefixEdit->clear();
    m_bodyEdit->clear();
    m_descriptionEdit->clear();
    m_sourceEdit->setText(QStringLiteral("cppsnippet-made"));
    m_prefixEdit->setFocus();
    m_prefixEdit->setReadOnly(false);
    m_descriptionEdit->setReadOnly(false);
    m_bodyEdit->setReadOnly(false);
    m_templateTree->clearSelection();
    m_selectedItem = resourceInventory::ResourceItem();
    updateTemplateButtons();
}
```

### Template Validation

```cpp
void MainWindow::onSaveTemplate() {
    QString prefix = m_prefixEdit->text().trimmed();
    QString body = m_bodyEdit->toPlainText();
    QString description = m_descriptionEdit->toPlainText().trimmed();
    
    if (prefix.isEmpty() || body.isEmpty()) {
        QMessageBox::warning(this, tr("Error"),
            tr("Name and body are required."));
        return;
    }
    
    // ... save logic
}
```

### JSON Format Parsing

```cpp
// Modern VSCode format
// File: {"cube_basic": {"prefix": "cube", "body": "...", "description": "..."}}

// After unwrapping (TemplatesInventory does this):
// {"prefix": "cube", "body": "...", "description": "..."}

// Legacy format:
// {"key": "cube", "content": "cube([10, 10, 10]);"}
```

---

## Summary

**What Works:** Layout, widgets, edit mode state machine, visual design  
**What's Broken:** Data connection (wrong types), missing implementations  
**What's Needed:** Fix 3 QVariant conversions + implement 4 stub functions  

The GUI architecture is sound and doesn't need major redesign. The template panel widgets and layout can be preserved as-is. Only the data plumbing needs updating to work with the new TemplatesInventory + ResourceIndexer backend.

**Recommended Approach:** Phase 1 (fix display) → Phase 2 (implement actions) → Optionally Phase 3 (model/view redesign if needed)
