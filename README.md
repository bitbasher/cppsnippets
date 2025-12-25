# ScadTemplates

**A code template management solution for OpenSCAD**

ScadTemplates provides a comprehensive system for defining, organizing, and using code templates in OpenSCAD projects. Features include a Qt-based GUI application, tiered resource management, and VS Code-compatible JSON snippet format.

## Features

- **Cross-platform support** - Windows, macOS, Linux
- **Qt 6 GUI application** - Modern template browser with search, edit, and preview
- **Tiered resource management** - Installation, Machine, and User-level templates
- **JSON template format** - VS Code snippet compatible
- **Environment variable expansion** - `${VAR}` and `%VAR%` syntax support
- **Comprehensive testing** - GoogleTest unit tests + Qt TestLib GUI tests

---

## Project Structure

```
scadtemplates/
├── CMakeLists.txt              # Main CMake build configuration
├── cmake/                      # CMake configuration files
├── doc/                        # Documentation
│   ├── 2025-12-25-GUI-Testing-Framework.md
│   └── Resource-Discovery-Specifications.md
├── src/
│   ├── app/                    # Qt application (MainWindow)
│   ├── gui/                    # Preferences dialog components
│   ├── scadtemplates/          # Template management library
│   ├── platformInfo/           # Resource location management
│   ├── resInventory/           # Resource discovery and inventory
│   ├── jsonreader/             # JSON file reading library
│   └── jsonwriter/             # JSON file writing library
└── tests/
    ├── *.cpp                   # GoogleTest unit tests
    └── qt/                     # Qt TestLib GUI tests
```

---

## Building

### Prerequisites

- **CMake** 3.16 or higher
- **C++17** compatible compiler (MSVC 2022, GCC, Clang)
- **Qt 6.x** (6.10.1 recommended)

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_SHARED_LIBS` | ON | Build as shared/dynamic library |
| `BUILD_TESTS` | ON | Build unit tests |
| `BUILD_APP` | ON | Build Qt GUI application |
| `USE_QSCINTILLA` | OFF | Enable QScintilla editor integration |

### Windows (PowerShell)

```powershell
# Create build directory
mkdir build; cd build

# Configure with Qt application
cmake .. -DBUILD_APP=ON

# Build
cmake --build . --config Debug --parallel

# Run unit tests
ctest -C Debug --output-on-failure

# Run GUI tests
.\bin\Debug\test_mainwindow_gui.exe
.\bin\Debug\test_envvarstab_gui.exe
```

### Linux/macOS

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake .. -DBUILD_APP=ON

# Build
cmake --build . --parallel

# Run tests
ctest --output-on-failure
```

---

## Application Overview

### Main Window

The main application window provides:

- **Template Browser** - Tree view showing templates organized by tier (Installation, Machine, User)
- **Search** - Real-time search with case-insensitive matching and wrap-around
- **Template Editor** - Edit prefix, description, version, and body
- **Button State Management** - Context-aware enable/disable based on selection tier

### Preferences Dialog

Configurable settings organized in tabs:

| Tab | Purpose |
|-----|---------|
| **Installation** | View/manage installation-level template locations |
| **Machine** | Configure machine-wide template directories |
| **User** | Manage user-specific template locations |
| **Env Vars** | Configure environment variable overrides |

### Resource Tiers

Templates are organized in a three-tier hierarchy:

1. **Installation** - Bundled with application (read-only)
2. **Machine** - System-wide locations (typically read-only)
3. **User** - User-specific locations (editable)

User templates can override machine/installation templates with the same prefix.

---

## GUI Architecture

The application follows the **TabDialog pattern** where each component is a self-contained `QWidget` subclass.

### Key GUI Components

```
src/gui/
├── preferencesdialog.cpp/.h      # Main preferences dialog
├── envVarsTab.cpp/.h             # Environment variables editor
├── installationTab.cpp/.hpp      # Installation tier tab
├── machineTab.cpp/.hpp           # Machine tier tab
├── userTab.cpp/.hpp              # User tier tab
├── resourcelocationwidget.cpp/.hpp  # Reusable location list
├── dialogButtonBar.cpp/.hpp      # Standard dialog buttons
├── platformInfoWidget.cpp/.hpp   # Platform info display
├── aboutDialog.cpp/.h            # About dialog
└── sysInfoDialog.cpp/.h          # System information dialog
```

### Widget Pattern

Each widget class:
- Inherits from `QWidget` with `Q_OBJECT` macro
- Creates child widgets in constructor with `setObjectName()` for testability
- Applies layout at end of constructor
- Emits signals for parent communication

---

## Testing

### Unit Tests (GoogleTest)

Core library functionality tested with GoogleTest:

```powershell
cd build
ctest -C Debug --output-on-failure
```

Test files:
- `test_template.cpp` - Template class tests
- `test_template_manager.cpp` - Template manager tests
- `test_template_parser.cpp` - JSON parsing tests
- `test_resource_paths.cpp` - Path expansion tests
- `test_scad_template_session.cpp` - Session management tests

### GUI Tests (Qt TestLib)

Comprehensive GUI regression tests using Qt TestLib following official Qt tutorials 3 & 4:

| Test Suite | Tests | Purpose |
|------------|-------|---------|
| `test_mainwindow_gui` | 23 | Template browser, search, button states |
| `test_envvarstab_gui` | 9 | Environment variable editor |
| `test_resourcelocationwidget_gui` | 4 | Location management |

**Running GUI Tests:**

```powershell
cd build

# Build GUI tests
cmake --build . --target test_mainwindow_gui
cmake --build . --target test_envvarstab_gui
cmake --build . --target test_resourcelocationwidget_gui

# Run individually
.\bin\Debug\test_mainwindow_gui.exe
.\bin\Debug\test_envvarstab_gui.exe
.\bin\Debug\test_resourcelocationwidget_gui.exe
```

**Example Output:**
```
********* Start testing of TestMainWindowGUI *********
Config: Using QtTest library 6.10.1, Qt 6.10.1
PASS   : TestMainWindowGUI::initTestCase()
PASS   : TestMainWindowGUI::testSearchFindTemplate()
PASS   : TestMainWindowGUI::testSearchWrapAround()
...
Totals: 23 passed, 0 failed, 0 skipped, 0 blacklisted, 4518ms
********* Finished testing of TestMainWindowGUI *********
```

### GUI Test Coverage

**MainWindow Tests:**
- Search functionality (find, wrap-around, case-insensitive)
- Button state management by tier
- New/Copy/Edit template workflows
- Editor population from selection

**EnvVarsTab Tests:**
- List population from system environment
- Variable selection and editing
- Save, cancel, revert operations
- Button state management

For detailed GUI testing documentation, see [doc/2025-12-25-GUI-Testing-Framework.md](doc/2025-12-25-GUI-Testing-Framework.md).

---

## API Reference

### Template Classes

| Class | Description |
|-------|-------------|
| `scadtemplates::Template` | Single template with prefix, body, description, scopes |
| `scadtemplates::TemplateManager` | Collection management with search/filter |
| `scadtemplates::TemplateParser` | JSON serialization/deserialization |

### Resource Management

| Class | Description |
|-------|-------------|
| `platformInfo::ResourceLocationManager` | Manages template location discovery |
| `platformInfo::ResourcePaths` | Path expansion with env var support |
| `resInventory::ResourceStore` | Template inventory storage |
| `resInventory::TemplateTreeModel` | Qt model for tree view display |

### Example Usage

```cpp
#include <scadtemplates/template.h>
#include <scadtemplates/templateManager.h>

int main() {
    scadtemplates::TemplateManager manager;
    
    // Create and add a template
    scadtemplates::Template tmpl("module_basic", 
        "module ${1:name}() {\n    $0\n}", 
        "Basic OpenSCAD module");
    manager.addTemplate(tmpl);
    
    // Find by prefix
    auto found = manager.findByPrefix("module_basic");
    if (found) {
        std::cout << found->getBody() << std::endl;
    }
    
    return 0;
}
```

---

## Environment Variables

Templates and paths support environment variable expansion:

| Syntax | Platform |
|--------|----------|
| `${VAR}` | Cross-platform |
| `%VAR%` | Windows-style |

**Common Variables:**
- `${HOME}` / `${USERPROFILE}` - User home directory
- `${APPDATA}` - Application data (Windows)
- `${XDG_CONFIG_HOME}` - Config directory (Linux)

Variables can be overridden in the Env Vars preferences tab.

---

## Documentation

Additional documentation in the `doc/` folder:

| Document | Description |
|----------|-------------|
| [GUI Testing Framework](doc/2025-12-25-GUI-Testing-Framework.md) | Comprehensive GUI test documentation |
| [Resource Discovery Specs](doc/Resource-Discovery-Specifications.md) | Resource location discovery design |
| [Implementation Status](doc/2025-12-22-Implementation-Status-Report.md) | Development progress report |

---

## License

MIT License - See [LICENSE](LICENSE) for details.

Copyright (c) 2025 Jeff Hayes
