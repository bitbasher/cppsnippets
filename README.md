# cppsnippets

A comprehensive snippets tool for use in C++ programs - defining, parsing, storing, and using code snippets.

## Features

- Cross-platform support (Windows, macOS, Linux)
- Qt-based GUI application
- Dynamic library (.dll/.so/.dylib) for integration into other projects
- JSON-compatible snippet format (VS Code compatible)
- GoogleTest-based unit testing

## Project Structure

```
cppsnippets/
├── CMakeLists.txt          # Main CMake build configuration
├── AppInfo.cmake           # Application metadata (name, version, author)
├── cmake/                  # CMake configuration files
├── doc/                    # Documentation
├── src/                    # Source code
│   ├── include/            # Public API headers
│   │   └── cppsnippets/    # Library headers
│   ├── app/                # Qt application source
│   └── *.cpp               # Library implementation
├── src-orig/               # Original/reference sources
└── tests/                  # Unit tests
```

## Application Metadata (Name & Version)

### Single Source of Truth: AppInfo.cmake

All application metadata is centralized in [AppInfo.cmake](AppInfo.cmake):

```cmake
set(APP_NAME "ScadTemplates")
set(APP_SUFFIX "" CACHE STRING "e.g., 'Nightly', 'Beta'")
set(APP_AUTHOR "Jeff Hayes")
set(APP_ORGANIZATION "jartisan")
set(APP_VERSION_MAJOR 2)
# VERSION_MINOR is auto-calculated from git commit count
set(APP_VERSION_PATCH 0)
```

### How It Works

1. **Edit [AppInfo.cmake](AppInfo.cmake)** to change name, version, or author
2. **CMake generates two files** during configuration:
   - `build/generated/applicationNameInfo.hpp` - For use in C++ code
   - `build/generated/version.rc` - For Windows version embedding
3. **Version info appears in:**
   - Application UI (About dialog, window title)
   - Windows File Explorer (right-click → Properties → Details)
   - Library exports and headers

### Version Numbering

- **Major:** Manual (set in AppInfo.cmake)
- **Minor:** Automatic (git commit count since repo creation)
- **Patch:** Manual (set in AppInfo.cmake)

Example: `2.51.0` means "Major version 2, commit #51, patch 0"

### Using in Code

```cpp
#include "applicationNameInfo.hpp"

// Access metadata
std::cout << appInfo::displayName << " v" << appInfo::version << std::endl;
// Output: ScadTemplates v2.51.0

QApplication::setApplicationName(appInfo::displayName);
QApplication::setApplicationVersion(appInfo::version);
QApplication::setOrganizationName(appInfo::organization);
```

All namespace constants:
- `appInfo::baseName` - "ScadTemplates"
- `appInfo::suffix` - "" (or " Beta", " Nightly")
- `appInfo::displayName` - Full name with suffix
- `appInfo::author` - "Jeff Hayes"
- `appInfo::organization` - "jartisan"
- `appInfo::version` - "2.51.0"
- `appInfo::versionMajor`, `versionMinor`, `versionPatch` - Integer components
- `appInfo::gitCommitHash` - Short git SHA

## Building

### Prerequisites

- CMake 3.16 or higher
- C++17 compatible compiler
- Qt 5 or Qt 6 (optional, for GUI application)

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_SHARED_LIBS` | ON | Build as shared/dynamic library |
| `BUILD_TESTS` | ON | Build unit tests |
| `BUILD_APP` | ON | Build Qt GUI application |

### Linux/macOS

```bash
# Create build directory
mkdir build && cd build

# Configure (library and tests only)
cmake .. -DBUILD_APP=OFF

# Or configure with Qt application
cmake .. -DBUILD_APP=ON

# Build
cmake --build . --parallel

# Run tests
ctest --output-on-failure

# Install (optional)
sudo cmake --install .
```

### Windows

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake .. -DBUILD_APP=OFF

# Build
cmake --build . --config Release --parallel

# Run tests
ctest -C Release --output-on-failure
```

## Using the Library

### CMake Integration

```cmake
find_package(cppsnippets REQUIRED)
target_link_libraries(your_target PRIVATE cppsnippets::cppsnippets_lib)
```

### Example Usage

```cpp
#include <cppsnippets/cppsnippets.h>
#include <iostream>

int main() {
    cppsnippets::SnippetManager manager;
    
    // Create and add a snippet
    cppsnippets::Snippet snippet("cout", "std::cout << $1 << std::endl;", "Output to console");
    manager.addSnippet(snippet);
    
    // Find a snippet
    auto found = manager.findByPrefix("cout");
    if (found) {
        std::cout << found->getBody() << std::endl;
    }
    
    return 0;
}
```

## API Reference

### Core Classes

- `Snippet` - Represents a single code snippet with prefix, body, description, and scopes
- `SnippetParser` - Parses and serializes snippets in JSON format
- `SnippetManager` - Manages collections of snippets with search and filtering

### Library Functions

- `getVersion()` - Returns the library version string
- `getVersionMajor()`, `getVersionMinor()`, `getVersionPatch()` - Version components

## Testing

The project uses GoogleTest for some unit tests and to build mock objects, and Qt Ctest for unit testing and GUI tests.
Tests are automatically discovered by CTest.

```bash
cd build
ctest --output-on-failure
```

## GUI Architecture

The Qt application follows the **TabDialog pattern** where each tab/dialog component is its own `QWidget` subclass, promoting separation of concerns and reusability.

### GUI File Structure

```
src/include/gui/
├── preferencesdialog.h           # Dialog that assembles all widgets
├── platformInfoWidget.hpp        # Platform information group box
├── dialogButtonBar.hpp           # Action buttons (Restore/Apply/OK/Cancel)
├── resourceLocationWidget.hpp    # Reusable location list widget
├── installationTab.hpp           # Installation tab class
├── machineTab.hpp                # Machine tab class
├── userTab.hpp                   # User tab class
├── aboutDialog.h                 # About dialog
└── sysInfoDialog.h               # System info dialog

src/gui/
├── preferencesdialog.cpp
├── platformInfoWidget.cpp
├── dialogButtonBar.cpp
├── resourcelocationwidget.cpp
├── installationTab.cpp
├── machineTab.cpp
├── userTab.cpp
├── aboutDialog.cpp
└── sysInfoDialog.cpp
```

### TabDialog Pattern

Each widget class:
- Inherits from `QWidget` with `Q_OBJECT` macro
- Creates its own child widgets in the constructor
- Applies layout at the end of the constructor
- Emits signals for parent dialog communication

The parent dialog (`PreferencesDialog`) simply:
- Creates instances of each widget class
- Adds tabs to a `QTabWidget`
- Connects signals between widgets

A reference implementation is available in `src/utils/` with its own CMake build:
- `tabDialog.hpp/cpp` - Example dialog assembling tabs
- `generalTab.hpp/cpp` - File info tab
- `permsTab.hpp/cpp` - Permissions tab  
- `appsTab.hpp/cpp` - Applications tab

## License

MIT License - See [LICENSE](LICENSE) for details.

Copyright (c) 2025 Jeff Hayes
