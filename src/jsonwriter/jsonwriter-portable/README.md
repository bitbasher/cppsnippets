# JsonWriter Portable

A small, portable library for writing JSON files via Qt with detailed error reporting, designed as a companion to JsonReader.

- Minimal API: `JsonWriter::writeFile`, `writeObject`, `writeArray` and `JsonWriteErrorInfo`
- Dependency: Qt Core (Qt5 or Qt6)
- Atomic writes using `QSaveFile` for safety
- Tests: QtTest-based unit tests

---

## Features

- **Atomic writes**: Uses `QSaveFile` to write to a temporary file and atomically rename on success
- **Format options**: Compact (single-line) or Indented (pretty-printed) output
- **Error reporting**: Consistent `JsonWriteErrorInfo` structure with filename and error messages
- **Pure QtCore**: No additional dependencies beyond Qt Core + C++17

---

## Build & Test

```bash
cmake -S . -B build -G Ninja
cmake --build build -j
ctest --test-dir build -V
```

Qt detection tries Qt6 first, then falls back to Qt5 automatically.

Notes:

- If you don't need to build tests, turn them off: `-DJSONWRITER_BUILD_TESTS=OFF`
- With tests off, only `Qt(Core)` is required

---

## Integration Options

- As a subdirectory (recommended):
  ```cmake
  add_subdirectory(contrib/jsonwriter-portable)
  target_link_libraries(your_target PRIVATE JsonWriterPortable)
  ```

- As a copied pair of files:
  - Copy `include/JsonWriter/JsonWriter.h` and `src/JsonWriter.cc` into your project
  - Link against Qt Core

---

## API Overview

```cpp
#include "JsonWriter/JsonWriter.h"

// Write a QJsonDocument
QJsonDocument doc = ...;
JsonWriteErrorInfo error;
bool success = JsonWriter::writeFile("/path/to/file.json", doc, error);
if (!success) {
    std::cerr << error.formatError() << std::endl;
}

// Write a QJsonObject directly (pretty-printed)
QJsonObject obj;
obj["name"] = "example";
obj["version"] = 1;
JsonWriter::writeObject("/path/to/config.json", obj, error, JsonWriter::Indented);

// Write compact (single-line) format
JsonWriter::writeArray("/path/to/data.json", array, error, JsonWriter::Compact);
```

---

## Format Styles

- `JsonWriter::Compact` - Single line, minimal whitespace
- `JsonWriter::Indented` - Pretty-printed with 4-space indentation (default)

---

## Error Handling

`JsonWriteErrorInfo` provides:
- `message`: Human-readable error description
- `filename`: Path to the file being written
- `hasError()`: Returns true if an error occurred
- `formatError()`: Returns formatted string "filename: message"

---

## Atomic Writes

JsonWriter uses Qt's `QSaveFile` which:
1. Writes to a temporary file in the same directory
2. Only replaces the target file if the write succeeds completely
3. Preserves the original file if any error occurs during writing

This ensures you never end up with a corrupted or partially-written JSON file.

---

## Tests

The test suite includes:
- Basic write/read round-trip verification
- Compact vs. Indented format validation
- Error handling (write permissions, invalid paths)
- Object and Array convenience methods
- Atomic write behavior verification

Run tests with:
```bash
ctest --test-dir build --output-on-failure
```
