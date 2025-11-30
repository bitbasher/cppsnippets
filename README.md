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

int main() {
    cppsnippets::SnippetManager manager;
    
    // Create and add a snippet
    cppsnippets::Snippet snippet("log", "console.log($1);", "Log to console");
    manager.addSnippet(snippet);
    
    // Find a snippet
    auto found = manager.findByPrefix("log");
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

The project uses GoogleTest for unit testing. Tests are automatically discovered by CTest.

```bash
cd build
ctest --output-on-failure
```

## License

MIT License - See [LICENSE](LICENSE) for details.

Copyright (c) 2025 Jeff Hayes
