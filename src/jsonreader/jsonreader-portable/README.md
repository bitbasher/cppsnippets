# JsonReader Portable

A small, portable library extracted from OpenSCAD providing robust JSON file reading via Qt, with detailed error reporting (filename:line:column) suitable for end-user messages.

- Minimal API: `JsonReader::readFile`, `readObject`, `readArray` and `JsonErrorInfo`
- Dependency: Qt Core (Qt5 or Qt6)
- Tests: QtTest-based unit tests (no Catch2 dependency)

---

## What Was Extracted (and Why)

This module is a standalone packaging of the OpenSCAD `JsonReader` utility, designed to be dropped into other projects (e.g., cppsnippets) to provide consistent JSON file parsing with human-friendly error locations. It removes any OpenSCAD-specific includes and logging, keeping only Qt Core + standard library dependencies.

Key points:

- No OpenSCAD headers or utilities are referenced
- Pure QtCore (Qt5 or Qt6) + C++17
- Same `JsonErrorInfo` structure and `formatError()` formatting used in OpenSCAD
- Byte offset → line/column mapping implemented locally

---

## Build & Test

```bash
cmake -S . -B build -G Ninja
cmake --build build -j
ctest --test-dir build -V
```

Qt detection tries Qt6 first, then falls back to Qt5 automatically.

Notes:

- If you don't need to build tests, turn them off to avoid a dependency on `Qt(Test)`: `-DJSONREADER_BUILD_TESTS=OFF`
- With tests off, only `Qt(Core)` is required

---

## Integration Options

- As a subdirectory (recommended):
  ```cmake
  add_subdirectory(contrib/jsonreader-portable)
  target_link_libraries(your_target PRIVATE JsonReaderPortable)
  ```

- As a copied pair of files:
  - Copy `include/JsonReader/JsonReader.h` and `src/JsonReader.cc` into your project
  - Link against Qt Core

- With CMake FetchContent (optional):
  ```cmake
  include(FetchContent)
  FetchContent_Declare(
    JsonReaderPortable
    GIT_REPOSITORY https://example.com/your-fork-or-mirror.git
    GIT_TAG        main
    SOURCE_SUBDIR  contrib/jsonreader-portable
  )
  FetchContent_MakeAvailable(JsonReaderPortable)
  target_link_libraries(your_target PRIVATE JsonReaderPortable)
  ```

  - As an installed package (system-wide or prefix)
    1. Install the package:
      ```bash
      cmake --install build --prefix /your/prefix
      ```
    2. In your consumer CMakeLists.txt:
      ```cmake
      find_package(JsonReaderPortable CONFIG REQUIRED)
      target_link_libraries(your_target PRIVATE JsonReaderPortable::JsonReaderPortable)
      ```

---

    ## Consumer Template (for Agents)

    For a copy-pasteable consumer setup, see:

    - examples/ConsumerCMakeLists.cmake

    It contains three ready-to-use blocks showing how to consume this library via:
    - add_subdirectory
    - find_package (installed config)
    - vendored sources

    Include that file in your agent notes or project docs to standardize adoption.

    ---

## Minimal Usage Example

```cpp
#include <QJsonDocument>
#include <QJsonObject>
#include "JsonReader/JsonReader.h"

JsonErrorInfo err;
QJsonObject obj;
if (!JsonReader::readObject("/path/to/template.json", obj, err)) {
  // Log or display err.formatError()  →  "/path/to/template.json:15:8: unterminated object"
}

// Access fields
auto name = obj.value("name").toString();
```

---

## API

```cpp
struct JsonErrorInfo {
  std::string message;
  std::string filename;
  int line = 0;    // 1-based
  int column = 0;  // 1-based
  int offset = 0;  // byte offset in file
  std::string formatError() const; // "file:line:column: message"
};

class JsonReader {
public:
  static bool readFile(const std::filesystem::path&, QJsonDocument&, JsonErrorInfo&);
  static bool readFile(const std::string&, QJsonDocument&, JsonErrorInfo&);
  static bool readObject(const std::filesystem::path&, QJsonObject&, JsonErrorInfo&);
  static bool readObject(const std::string&, QJsonObject&, JsonErrorInfo&);
  static bool readArray(const std::filesystem::path&, QJsonArray&, JsonErrorInfo&);
  static bool readArray(const std::string&, QJsonArray&, JsonErrorInfo&);
};
```

---

## Test Data

Tests use a minimal set of JSON files under `tests/data/json/`:
- `valid-array.json`
- `error-unterminated-object.json`
- `error-missing-comma.json`
- `error-unterminated-string.json`

## Notes

- This module intentionally avoids any OpenSCAD-specific headers or logging.
- Error locations are derived from `QJsonParseError::offset` mapped to line/column.

Compatibility:

- Platforms: Windows, macOS, Linux (anywhere Qt Core is available)
- Compilers: C++17 or later
- Qt: 5.x or 6.x (prefers Qt6 when available)

Limitations:

- JSON must be read from files (this utility focuses on file paths). You can trivially adapt it to parse from `QByteArray` if needed.

Toggles:

- `JSONREADER_BUILD_TESTS=OFF` to skip building the QtTest executable (avoids requiring `Qt(Test)`).

---

## Intended Use and Context

This portable package was extracted from OpenSCAD’s internal utilities to enable reuse in adjacent projects (e.g., the cppsnippets integration effort) for reading JSON templates and reporting actionable parse errors (file:line:column). It is designed to be light-weight, Qt-only, and easy to integrate either by vendoring two files or linking as a small static library.

When contributing to projects that already use Qt, prefer linking against the installed package or adding this directory as a subdirectory so updates flow naturally and improvements remain centralized.
