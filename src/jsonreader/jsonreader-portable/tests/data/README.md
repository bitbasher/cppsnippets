# JsonReader Test Data

This directory contains test data files for the JsonReader module's standalone testing.

## Directory Structure

```
data/
├── inst/OpenSCAD/
│   ├── color-schemes/
│   │   ├── editor/          - Editor color scheme JSON files
│   │   └── render/          - Render color scheme JSON files
│   └── templates/           - Installation template files
├── pers/Jeff/Documents/OpenSCAD/
│   └── templates/           - User-specific template files
└── json/                    - General JSON test files
```

## Test Data Files

### Editor Color Schemes (`inst/OpenSCAD/color-schemes/editor/`)

- **high-contrast-dark.json** - Valid editor color scheme with all standard fields
  - Used by: `readsEditorColorScheme()`, `validatesEditorColorHexValues()`
  - Contains: keyword colors, comment colors, operators, variables, special colors

- **bad-invalid-hex.json** - Invalid hex color values (validation test)
  - Used by: `readsEditColorWithBadValues()`
  - Contains: `#gggggg` (invalid hex digits), `#ZZZ` (too short and invalid)

- **bad-negative-values.json** - Negative color values (validation test)
  - Used by: Available for range validation testing
  - Contains: `"-1"` as a string (semantically invalid)

### Render Color Schemes (`inst/OpenSCAD/color-schemes/render/`)

- **metallic.json** - Valid render color scheme for OpenSCAD's rendering engine
  - Used by: `readsRenderColorScheme()`, `validatesRenderColorHexValues()`
  - Contains: render-specific colors (background, cgal-face-*, cgal-edge-*)
  - Note: Render colors use 6-character hex only (no alpha channel)

- **bad-out-of-range.json** - Numeric values instead of hex strings (validation test)
  - Used by: Available for type validation testing
  - Contains: `256`, `-100` (invalid types and out-of-range values)

### Installation Templates (`inst/OpenSCAD/templates/`)

- **inst_template_sphere_param.json** - Modern VS Code snippet format
  - Used by: `readsModernTemplateJson()`
  - Format: `prefix`, `body` (array), optional `description` and `scope`
  - Body contains parametric sphere template with `${n:default}` placeholders

- **unicode_multilingual_template.json** - Unicode and multilingual content test
  - Used by: `readsUnicodeTemplate()`
  - Contains: 
    - English, Amharic, Bulgarian, Czech, Polish text
    - Mathematical symbols (∑, ∏, √, ∂, ∇, ∫)
    - Currency symbols (€, £, ¥, ₹, ₽, ₩)
    - Special symbols (✓, ✗, →, ←, ∞)
  - Tests: UTF-8 encoding preservation through JSON parse cycle

### User Templates (`pers/Jeff/Documents/OpenSCAD/templates/`)

- **jeffdoc.json** - Legacy template format (backward compatibility test)
  - Used by: `readsLegacyTemplateJson()`
  - Format: `key` and `content` (single string, not array)
  - Tests: Support for pre-VS Code snippet format

### General JSON Files (`json/`)

These files are used by the basic JsonReader tests (not file-specific tests):

- **valid-array.json** - Valid JSON array
- **error-unterminated-string.json** - Syntax error: unterminated string
- **error-missing-comma.json** - Syntax error: missing comma
- **error-unterminated-object.json** - Syntax error: unterminated object

## Test Data Mirroring

This test data directory mirrors the structure of the main project's `testFileStructure/` directory, allowing the JsonReader module to be:

1. **Standalone** - Can be used and tested independently
2. **Portable** - Data is packaged with the module
3. **Minimal** - Contains only essential test files (not the full testFileStructure)

## Adding New Test Data

To add new test data:

1. Create the appropriate subdirectory structure (e.g., `inst/OpenSCAD/color-schemes/editor/`)
2. Add your JSON file
3. Add a test method in `test_jsonreader_files.cpp`
4. Run tests to verify: `ctest -C Debug -R jsonreader_files_qttest`

## Color Hex Format Notes

### Editor Colors
- Support both **6-character** (`#RRGGBB`) and **8-character** (`#RRGGBBAA`) hex
- 8-character format includes alpha channel (opacity)
- Examples: `#ff7aaf` (pink), `#ff000000` (red with full opacity)

### Render Colors
- Support **6-character only** (`#RRGGBB`)
- No alpha channel in render colors
- Examples: `#aaaaff` (light blue), `#ff0000` (red)

## Unicode Test Data

The unicode_multilingual_template.json file tests:

- **Script coverage**:
  - Latin with diacritics (Czech, Polish)
  - Cyrillic (Bulgarian)
  - Ge'ez alphabet (Amharic)

- **Special characters**:
  - Mathematical operators and symbols
  - Currency symbols
  - Directional and logical symbols

- **Encoding survival**:
  - UTF-8 file → `QJsonDocument` parse → `QString` → output
  - Verifies no character loss through JSON processing

## Running Tests with This Data

```bash
# From the build directory
ctest -C Debug -R jsonreader_files_qttest --output-on-failure
```

The test discovery will automatically find this data directory via the `findTestData()` function in `test_jsonreader_files.cpp`, which searches for `inst/OpenSCAD` subdirectories.
