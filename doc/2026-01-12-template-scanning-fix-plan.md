# Template Scanning Fix Plan

**Date:** January 12, 2026  
**Status:** Planning  
**Author:** GitHub Copilot (Claude Sonnet 4.5)

---

## Problem Statement

Current template scanner ([templateScanner.cpp](../src/resourceDiscovery/templateScanner.cpp)) rejects most templates because:

1. **Format Mismatch:** Scanner expects new schema format, but finds:
   - Legacy format templates (`"key"` + `"content"`)
   - VS Code snippet format templates (converted from legacy)
   - New schema format templates (with required fields per lib/snippet.schema.json)

2. **No Schema Validation:** Despite having nlohmann/json-schema-validator in vcpkg, scanner doesn't use it

3. **Missing Format Detection:** No auto-detection of which format a template uses

4. **Incomplete Metadata:** ResourceTemplate class doesn't store all schema fields (`filetype`, `varient`, `language`, `encoding`)

---

## User Requirements (from conversation)

✅ **Must Support:**
- Legacy format (for backward compatibility)
- VS Code snippet format (already converted)
- New schema format (lib/snippet.schema.json)

✅ **Design Principles:**
- No caching - discovery/scanning done at startup every time
- In-memory objects only during runtime
- Can save edited templates back to filesystem
- Use existing JsonReader/JsonWriter concepts from bad-refactoring (but not code directly)

✅ **Technical Constraints:**
- Current project has earlier version of scanner than bad-refactoring
- Cannot copy code from bad-refactoring (it's tainted with bad architectural choices)
- Must use concepts/approaches from bad-refactoring only
- Already have nlohmann-json and json-schema-validator in vcpkg

---

## Three Template Formats

### Format 1: Legacy (Original OpenSCAD)

**File:** `templates/jeffdoc.json`

```json
{
  "key": "jeffdoc",
  "content": "// jeff doc scad templates;\n"
}
```

**Characteristics:**
- One template per file
- `"key"` field = template name
- `"content"` field = raw text with `\n` escapes
- Cursor marker: `^~^` (to be converted to `$0`)

**Detection:** Has both `"key"` and `"content"` fields

---

### Format 2: VS Code Snippet (Converted)

**File:** `scadtemplates/user/users-jeff-appdata-local-openscad-module.json`

```json
{
  "module": {
    "_format": "vscode-snippet",
    "_source": "legacy-converted",
    "_version": 1,
    "body": ["", "module $0() {", "\t", "}", ""],
    "description": "Converted from legacy template (module.json)",
    "prefix": "module"
  }
}
```

**Characteristics:**
- Multiple templates per file (object of objects)
- Top-level keys = template names
- Each template has: `prefix`, `body` (array), `description`
- Metadata fields: `_format`, `_source`, `_version`
- Body uses VS Code placeholders: `$0`, `$1`, `${1:default}`

**Detection:** Top-level object with keys containing objects that have `"prefix"` and `"body"` fields

---

### Format 3: New Schema (lib/snippet.schema.json)

**File:** `testFileStructure/inst/OpenSCAD/templates/inst_template_basic_cube.json`

```json
{
  "name": "Basic Cube",
  "description": "Simple cube from installation OpenSCAD",
  "scope": "source.scad",
  "body": [
    "// Basic cube template from installation",
    "cube([${1:10}, ${2:10}, ${3:10}]);",
    "$0"
  ],
  "prefix": "cube_basic",
  "filetype": "openscad",
  "varient": "basic",
  "language": "en",
  "encoding": "UTF8"
}
```

**Schema Requirements (from lib/snippet.schema.json):**

**REQUIRED fields:**
- `prefix` (string)
- `body` (string OR array of strings)
- `filetype` (enum: "text", "openscad", "json", "markup")
- `varient` (string) - NOTE: intentional misspelling in schema
- `language` (string, default "en")
- `encoding` (string, default "UTF8")

**OPTIONAL fields:**
- `description` (string)
- `scope` (string OR array of strings)
- `minScadVersion` (string)

**Detection:** Has `"filetype"`, `"varient"`, `"language"`, `"encoding"` fields

---

## Multi-Phase Implementation Plan

### Phase 1: Format Detection & Multi-Format Support

**Goal:** Scanner accepts all three formats

**Estimated Effort:** 2-3 hours

#### Step 1.1: Add Format Detection

**New Class:** `TemplateFormatDetector`

**File:** `src/resourceDiscovery/TemplateFormatDetector.hpp`

```cpp
namespace resourceDiscovery {

enum class TemplateFormat {
    Unknown,
    Legacy,        // key + content
    VSCodeSnippet, // VS Code snippet format
    NewSchema      // lib/snippet.schema.json format
};

class TemplateFormatDetector {
public:
    static TemplateFormat detectFormat(const QJsonDocument& json);
    
private:
    static bool isLegacyFormat(const QJsonObject& obj);
    static bool isVSCodeSnippetFormat(const QJsonObject& obj);
    static bool isNewSchemaFormat(const QJsonObject& obj);
};

} // namespace resourceDiscovery
```

**Detection Logic:**

```cpp
TemplateFormat TemplateFormatDetector::detectFormat(const QJsonDocument& json) {
    if (!json.isObject()) return TemplateFormat::Unknown;
    
    QJsonObject obj = json.object();
    
    // Check for legacy format first (simplest)
    if (isLegacyFormat(obj)) return TemplateFormat::Legacy;
    
    // Check for new schema format (has specific required fields)
    if (isNewSchemaFormat(obj)) return TemplateFormat::NewSchema;
    
    // Check for VS Code snippet format (object of template objects)
    if (isVSCodeSnippetFormat(obj)) return TemplateFormat::VSCodeSnippet;
    
    return TemplateFormat::Unknown;
}

bool TemplateFormatDetector::isLegacyFormat(const QJsonObject& obj) {
    return obj.contains("key") && obj.contains("content");
}

bool TemplateFormatDetector::isNewSchemaFormat(const QJsonObject& obj) {
    // Must have all required fields from schema
    return obj.contains("prefix") &&
           obj.contains("body") &&
           obj.contains("filetype") &&
           obj.contains("varient") &&
           obj.contains("language") &&
           obj.contains("encoding");
}

bool TemplateFormatDetector::isVSCodeSnippetFormat(const QJsonObject& obj) {
    // Check if top-level keys contain objects with "prefix" and "body"
    if (obj.isEmpty()) return false;
    
    // Check first key
    QString firstKey = obj.keys().first();
    QJsonValue firstValue = obj.value(firstKey);
    
    if (!firstValue.isObject()) return false;
    
    QJsonObject tmplObj = firstValue.toObject();
    return tmplObj.contains("prefix") && tmplObj.contains("body");
}
```

#### Step 1.2: Add Format-Specific Parsers

**New Classes:** `LegacyTemplateParser`, `VSCodeSnippetParser`, `NewSchemaTemplateParser`

**File:** `src/resourceDiscovery/TemplateFormatParsers.hpp`

```cpp
namespace resourceDiscovery {

// Parse legacy format into ResourceTemplate
class LegacyTemplateParser {
public:
    static QVector<ResourceTemplate> parse(
        const QJsonObject& json,
        const QString& filePath,
        const platformInfo::ResourceLocation& location);
        
private:
    static QString convertCursorMarker(const QString& content);
    static QStringList contentToBodyLines(const QString& content);
};

// Parse VS Code snippet format into ResourceTemplate(s)
class VSCodeSnippetParser {
public:
    static QVector<ResourceTemplate> parse(
        const QJsonObject& json,
        const QString& filePath,
        const platformInfo::ResourceLocation& location);
        
private:
    static ResourceTemplate parseOneSnippet(
        const QString& name,
        const QJsonObject& snippetObj,
        const QString& filePath,
        const platformInfo::ResourceLocation& location);
};

// Parse new schema format into ResourceTemplate
class NewSchemaTemplateParser {
public:
    static QVector<ResourceTemplate> parse(
        const QJsonObject& json,
        const QString& filePath,
        const platformInfo::ResourceLocation& location);
        
private:
    static bool validateRequiredFields(const QJsonObject& obj);
};

} // namespace resourceDiscovery
```

#### Step 1.3: Refactor TemplateScanner

**Update:** `src/resourceDiscovery/templateScanner.cpp`

```cpp
QVector<ResourceTemplate> TemplateScanner::scanLocation(
    const platformInfo::ResourceLocation& location)
{
    QVector<ResourceTemplate> templates;
    QString templatesPath = location.path() + "/" + templateSubfolder();
    
    QDirIterator it(templatesPath, QStringList() << "*.json",
                    QDir::Files | QDir::Readable | QDir::NoDotAndDotDot);
    
    while (it.hasNext()) {
        QString filePath = it.next();
        
        // Read and parse JSON
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Cannot open:" << filePath;
            continue;
        }
        
        QByteArray data = file.readAll();
        file.close();
        
        QJsonParseError parseError;
        QJsonDocument json = QJsonDocument::fromJson(data, &parseError);
        
        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "JSON parse error in" << filePath 
                      << ":" << parseError.errorString();
            continue;
        }
        
        // Detect format
        TemplateFormat format = TemplateFormatDetector::detectFormat(json);
        
        if (format == TemplateFormat::Unknown) {
            qWarning() << "Unknown template format in" << filePath;
            continue;
        }
        
        QJsonObject obj = json.object();
        
        // Parse based on format
        QVector<ResourceTemplate> parsedTemplates;
        
        switch (format) {
            case TemplateFormat::Legacy:
                parsedTemplates = LegacyTemplateParser::parse(obj, filePath, location);
                qDebug() << "Parsed legacy template from" << filePath;
                break;
                
            case TemplateFormat::VSCodeSnippet:
                parsedTemplates = VSCodeSnippetParser::parse(obj, filePath, location);
                qDebug() << "Parsed VS Code snippet(s) from" << filePath;
                break;
                
            case TemplateFormat::NewSchema:
                parsedTemplates = NewSchemaTemplateParser::parse(obj, filePath, location);
                qDebug() << "Parsed new schema template from" << filePath;
                break;
                
            default:
                break;
        }
        
        templates.append(parsedTemplates);
    }
    
    qDebug() << "TemplateScanner: Found" << templates.size() 
            << "templates in" << templatesPath;
    
    return templates;
}
```

#### Step 1.4: Update ResourceTemplate Class

**Add missing fields to:** `src/resourceInventory/resourceItem.hpp`

```cpp
class ResourceTemplate : public ResourceItem {
public:
    // Existing fields remain...
    
    // NEW: Schema fields
    QString filetype() const { return m_filetype; }
    void setFiletype(const QString& ft) { m_filetype = ft; }
    
    QString varient() const { return m_varient; }  // Note: misspelling intentional
    void setVarient(const QString& v) { m_varient = v; }
    
    QString language() const { return m_language; }
    void setLanguage(const QString& lang) { m_language = lang; }
    
    QString encoding() const { return m_encoding; }
    void setEncoding(const QString& enc) { m_encoding = enc; }
    
    QString minScadVersion() const { return m_minScadVersion; }
    void setMinScadVersion(const QString& ver) { m_minScadVersion = ver; }
    
    // Body can be string OR array - store as QStringList internally
    QStringList bodyLines() const { return m_bodyLines; }
    void setBodyLines(const QStringList& lines) { m_bodyLines = lines; }
    void setBodyFromString(const QString& body);
    
    // Prefix for snippet triggering
    QString prefix() const { return m_prefix; }
    void setPrefix(const QString& pfx) { m_prefix = pfx; }
    
    // Scope (string or array)
    QStringList scopes() const { return m_scopes; }
    void setScopes(const QStringList& scopes) { m_scopes = scopes; }
    void setScopeFromString(const QString& scope);
    
private:
    QString m_filetype;
    QString m_varient;
    QString m_language{"en"};
    QString m_encoding{"UTF8"};
    QString m_minScadVersion;
    QStringList m_bodyLines;
    QString m_prefix;
    QStringList m_scopes;
};
```

#### Step 1.5: Testing

**New Test:** `tests/resourceDiscovery/test_template_formats.cpp`

**Test Categories:**

1. **Format Detection (9 tests)**
   - Detect legacy format correctly
   - Detect VS Code snippet format correctly
   - Detect new schema format correctly
   - Reject invalid JSON
   - Reject empty objects
   - Reject objects with missing required fields
   - Legacy format takes precedence (if overlapping fields)
   - New schema takes precedence over VS Code snippet
   - Unknown format returns Unknown

2. **Legacy Parsing (6 tests)**
   - Parse simple legacy template
   - Convert `^~^` to `$0`
   - Unescape `\n` to actual newlines
   - Split content into body lines
   - Handle empty content
   - Handle malformed legacy (missing key or content)

3. **VS Code Snippet Parsing (8 tests)**
   - Parse single snippet from file
   - Parse multiple snippets from one file
   - Extract prefix, body, description
   - Handle array body correctly
   - Handle metadata fields (`_format`, `_source`, `_version`)
   - Handle snippet with no description
   - Handle malformed snippet (missing prefix)
   - Handle malformed snippet (missing body)

4. **New Schema Parsing (10 tests)**
   - Parse valid new schema template
   - Validate all required fields present
   - Handle optional fields (description, scope, minScadVersion)
   - Handle body as string
   - Handle body as array
   - Handle scope as string
   - Handle scope as array
   - Validate filetype enum (text/openscad/json/markup)
   - Reject template missing required field
   - Handle default values (language=en, encoding=UTF8)

5. **Integration (5 tests)**
   - Scan testFileStructure and find all three formats
   - Legacy templates converted to ResourceTemplate
   - VS Code snippets converted to ResourceTemplate
   - New schema templates converted to ResourceTemplate
   - Mixed format folder scanned successfully

**Total Tests:** ~38 tests for Phase 1

---

### Phase 2: Schema Validation with nlohmann/json

**Goal:** Validate new schema templates against lib/snippet.schema.json

**Estimated Effort:** 3-4 hours

#### Step 2.1: Add JsonSchemaValidator Utility

**New Class:** `JsonSchemaValidator`

**File:** `src/resourceDiscovery/JsonSchemaValidator.hpp`

```cpp
#include <nlohmann/json.hpp>
#include <nlohmann/json-schema.hpp>
#include <QString>
#include <QJsonDocument>

namespace resourceDiscovery {

struct ValidationError {
    QString message;
    QString path;      // JSON path to error (e.g., "/body")
    int line = 0;
    int column = 0;
    
    QString formatError() const {
        return QString("Validation error at %1 (line %2, col %3): %4")
            .arg(path).arg(line).arg(column).arg(message);
    }
};

class JsonSchemaValidator {
public:
    // Load schema from file (call once at startup)
    static bool loadSchema(const QString& schemaFilePath);
    
    // Validate a JSON document against loaded schema
    static bool validate(const QJsonDocument& json, QVector<ValidationError>& errors);
    
    // Check if schema is loaded
    static bool isSchemaLoaded();
    
private:
    static nlohmann::json_schema::json_validator m_validator;
    static bool m_schemaLoaded;
    
    // Convert Qt JSON to nlohmann JSON
    static nlohmann::json qtToNlohmann(const QJsonDocument& doc);
    static nlohmann::json qtToNlohmann(const QJsonObject& obj);
    static nlohmann::json qtToNlohmann(const QJsonArray& arr);
};

} // namespace resourceDiscovery
```

#### Step 2.2: Integrate Validation into NewSchemaTemplateParser

**Update:** `src/resourceDiscovery/TemplateFormatParsers.cpp`

```cpp
QVector<ResourceTemplate> NewSchemaTemplateParser::parse(
    const QJsonObject& json,
    const QString& filePath,
    const platformInfo::ResourceLocation& location)
{
    QVector<ResourceTemplate> templates;
    
    // First, validate against schema if available
    if (JsonSchemaValidator::isSchemaLoaded()) {
        QJsonDocument doc(json);
        QVector<ValidationError> errors;
        
        if (!JsonSchemaValidator::validate(doc, errors)) {
            qWarning() << "Schema validation failed for" << filePath;
            for (const auto& error : errors) {
                qWarning() << "  " << error.formatError();
            }
            // Don't abort - still try to parse what we can
        }
    }
    
    // Check required fields manually
    if (!validateRequiredFields(json)) {
        qWarning() << "Missing required fields in" << filePath;
        return templates;
    }
    
    // Extract template data
    ResourceTemplate tmpl;
    
    // ... rest of parsing logic ...
    
    templates.append(tmpl);
    return templates;
}
```

#### Step 2.3: Load Schema at Application Startup

**Update:** Main application initialization (or TemplateScanner initialization)

```cpp
// In application startup or first scan
QString schemaPath = QCoreApplication::applicationDirPath() + "/lib/snippet.schema.json";

if (QFile::exists(schemaPath)) {
    if (JsonSchemaValidator::loadSchema(schemaPath)) {
        qDebug() << "Template schema loaded successfully from" << schemaPath;
    } else {
        qWarning() << "Failed to load template schema from" << schemaPath;
    }
} else {
    qWarning() << "Template schema not found at" << schemaPath;
}
```

#### Step 2.4: Testing

**New Test:** `tests/resourceDiscovery/test_schema_validation.cpp`

**Test Categories:**

1. **Schema Loading (4 tests)**
   - Load valid schema file
   - Reject invalid schema JSON
   - Reject non-existent schema file
   - Schema remains loaded across multiple validations

2. **Validation Success (6 tests)**
   - Valid new schema template passes
   - Template with all optional fields passes
   - Template with minimal required fields passes
   - Body as string passes
   - Body as array passes
   - Valid filetype enum value passes

3. **Validation Failures (10 tests)**
   - Missing `prefix` field fails
   - Missing `body` field fails
   - Missing `filetype` field fails
   - Missing `varient` field fails
   - Missing `language` field fails
   - Missing `encoding` field fails
   - Invalid `filetype` enum value fails
   - Wrong type for `body` (number instead of string/array) fails
   - Wrong type for `scope` (number instead of string/array) fails
   - Additional properties allowed (patternProperties: "^.*$")

4. **Error Reporting (5 tests)**
   - Error message contains field path
   - Error message contains line number
   - Error message contains column number
   - Multiple errors reported for multiple issues
   - Human-readable error formatting

**Total Tests:** ~25 tests for Phase 2

---

### Phase 3: Enhanced Template Metadata

**Goal:** Store all schema fields properly in ResourceTemplate

**Estimated Effort:** 2-3 hours

#### Step 3.1: Complete ResourceTemplate Implementation

All fields added in Step 1.4, now implement:

- Proper getters/setters
- Default values (language="en", encoding="UTF8")
- Conversion helpers (string body → QStringList, string scope → QStringList)
- Validation (filetype enum checking)

#### Step 3.2: Update All Parsers

Ensure all parsers populate all relevant fields:

- **LegacyTemplateParser:** Set defaults (filetype="openscad", varient="legacy", etc.)
- **VSCodeSnippetParser:** Set defaults, extract metadata fields if present
- **NewSchemaTemplateParser:** Extract all schema fields

#### Step 3.3: Testing

**Update:** Existing tests to verify all metadata fields populated correctly

**Total Tests:** ~10 additional tests for Phase 3

---

## Build Integration

### CMakeLists.txt Updates

```cmake
# Ensure nlohmann-json and json-schema-validator are found
find_package(nlohmann_json CONFIG REQUIRED)
find_package(nlohmann_json_schema_validator CONFIG REQUIRED)

# Add to resourceDiscovery library
target_link_libraries(resourceDiscovery PUBLIC
    Qt6::Core
    platformInfo
    resourceMetadata
    resourceInventory
    nlohmann_json::nlohmann_json
    nlohmann_json_schema_validator::nlohmann_json_schema_validator
)

# Add new test executables
add_executable(test_template_formats
    tests/resourceDiscovery/test_template_formats.cpp
)
target_link_libraries(test_template_formats PRIVATE
    resourceDiscovery
    GTest::gtest
    GTest::gtest_main
)

add_executable(test_schema_validation
    tests/resourceDiscovery/test_schema_validation.cpp
)
target_link_libraries(test_schema_validation PRIVATE
    resourceDiscovery
    nlohmann_json::nlohmann_json
    nlohmann_json_schema_validator::nlohmann_json_schema_validator
    GTest::gtest
    GTest::gtest_main
)
```

---

## File Structure Summary

```text
src/
├── resourceDiscovery/
│   ├── templateScanner.hpp            (UPDATED - orchestrates format detection)
│   ├── templateScanner.cpp            (UPDATED - calls parsers based on format)
│   ├── TemplateFormatDetector.hpp     (NEW - detects which format)
│   ├── TemplateFormatDetector.cpp     (NEW)
│   ├── TemplateFormatParsers.hpp      (NEW - legacy/VS Code/new schema parsers)
│   ├── TemplateFormatParsers.cpp      (NEW)
│   ├── JsonSchemaValidator.hpp        (NEW - nlohmann integration)
│   └── JsonSchemaValidator.cpp        (NEW)
│
├── resourceInventory/
│   └── resourceItem.hpp               (UPDATED - add schema fields to ResourceTemplate)
│
tests/
└── resourceDiscovery/
    ├── test_template_formats.cpp      (NEW - Phase 1 tests)
    └── test_schema_validation.cpp     (NEW - Phase 2 tests)
```

---

## Success Criteria

**Phase 1 Complete:**
1. ✅ All three formats detected correctly
2. ✅ Legacy templates parsed and converted
3. ✅ VS Code snippets parsed (multiple per file supported)
4. ✅ New schema templates parsed
5. ✅ 38 tests passing
6. ✅ Scanning testFileStructure finds all templates

**Phase 2 Complete:**
1. ✅ Schema validator integrated
2. ✅ lib/snippet.schema.json loaded at startup
3. ✅ New schema templates validated against schema
4. ✅ Validation errors reported with line/column info
5. ✅ Invalid templates logged but don't crash scanner
6. ✅ 25 tests passing

**Phase 3 Complete:**
1. ✅ All schema fields stored in ResourceTemplate
2. ✅ Default values applied correctly
3. ✅ String/array conversions working (body, scope)
4. ✅ 10 tests passing
5. ✅ Full metadata available for GUI/export

---

## Risks and Mitigations

### Risk 1: nlohmann/json integration with Qt JSON

**Problem:** Need to convert between Qt's QJsonDocument and nlohmann::json

**Mitigation:**
- Write conversion utilities (qtToNlohmann helpers)
- Test conversions thoroughly
- Consider caching conversions if performance issues

### Risk 2: Schema file not found at runtime

**Problem:** lib/snippet.schema.json might not be deployed with application

**Mitigation:**
- Check file exists before loading
- Log warning if missing
- Continue without schema validation (degraded mode)
- Document deployment requirements

### Risk 3: Breaking existing code

**Problem:** ResourceTemplate API changes might break GUI code

**Mitigation:**
- Add new fields without removing old ones
- Deprecate old fields instead of removing
- Update GUI code in separate phase
- Maintain backward compatibility

### Risk 4: Performance with large template collections

**Problem:** Validating hundreds of templates against schema might be slow

**Mitigation:**
- Profile first - measure actual impact
- Schema validation only in debug builds (optional)
- Cache validation results per file hash
- Consider lazy validation (on-demand)

---

## Dependencies

**vcpkg packages (already installed):**
- `nlohmann-json` ✅
- `json-schema-validator` ✅

**Qt modules:**
- Qt6::Core ✅

**Project libraries:**
- platformInfo ✅
- resourceMetadata ✅
- resourceInventory ✅

---

## Testing Strategy

### Unit Tests (Google Test)

- Format detection logic
- Individual parser functionality
- Schema validation
- Metadata extraction
- Error handling

### Integration Tests

- Scan testFileStructure folders
- Find templates in all three formats
- Populate ResourceTemplate objects correctly
- Handle mixed format folders

### Manual Testing

- Run scadtemplates.exe
- Load templates from discovery
- Verify all templates visible in GUI
- Check metadata displayed correctly
- Verify edit/save functionality

---

## Estimated Total Effort

| Phase | Component | Hours |
| ------- | ----------- | ------- |
| **Phase 1** | Format detection | 1 |
| | Legacy parser | 1 |
| | VS Code snippet parser | 2 |
| | New schema parser | 1 |
| | ResourceTemplate updates | 1 |
| | Testing (38 tests) | 3 |
| **Phase 2** | JsonSchemaValidator | 2 |
| | Integration with parsers | 1 |
| | Error reporting | 1 |
| | Testing (25 tests) | 2 |
| **Phase 3** | Metadata completion | 1 |
| | Parser updates | 1 |
| | Testing (10 tests) | 1 |
| **Total** | | **18 hours** |

**Recommended Approach:** Implement one phase at a time with full testing before moving to next.

---

## Next Steps

1. **Review this plan** with user
2. **Get approval** for approach
3. **Start Phase 1** with format detection
4. **Commit after each phase** completes with tests passing
5. **Validate with real templates** from testFileStructure
6. **Move to Phase 2** when Phase 1 stable

---

## Related Documents

- [Phase 2B Resource Scanning Design](2026-01-10-phase2b-resource-scanning.md)
- [Phase 2A Discovery Architecture](2026-01-10-phase2a-revised-after-reorganization.md)
- [Resource Refactoring Design](ResourceRefactoring-QDirListing-Design.md)
- [Legacy Template Converter](../src/scadtemplates/legacy_template_converter.hpp)
- [Snippet Schema](../lib/snippet.schema.json)

---

## Summary

**Problem:** Template scanner rejects most templates due to format mismatches and lack of schema validation.

**Solution:** Three-phase implementation:
1. Add multi-format support (legacy, VS Code, new schema)
2. Integrate schema validation using nlohmann/json
3. Complete metadata extraction for all schema fields

**Approach:** Incremental, test-driven, with clear commit points after each phase.

**Outcome:** Scanner accepts all template formats, validates against schema, provides rich metadata for GUI and export.

