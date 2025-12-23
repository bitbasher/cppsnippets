#include <QtTest/QtTest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>
#include <QTemporaryDir>
#include <QFile>

#include "JsonWriter/JsonWriter.h"

#include <nlohmann/json.hpp>
#include <nlohmann/json-schema.hpp>

using nlohmann::json;
using nlohmann::json_schema::json_validator;

class JsonWriterValidatorQtTest : public QObject {
  Q_OBJECT

private:
  // Helper to load schema from file
  json loadSchema(const QString& schemaPath) {
    QFile file(schemaPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qWarning() << "Failed to open schema:" << schemaPath;
      return json();
    }
    
    QByteArray content = file.readAll();
    file.close();
    
    return json::parse(content.toStdString());
  }
  
  // Helper to validate JSON against schema
  bool validateAgainstSchema(const QString& jsonPath, const json& schema, QString& errorMsg) {
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      errorMsg = "Failed to open JSON file";
      return false;
    }
    
    QByteArray content = file.readAll();
    file.close();
    
    try {
      json_validator validator;
      validator.set_root_schema(schema);
      
      json doc = json::parse(content.toStdString());
      validator.validate(doc);
      return true;
    } catch (const std::exception& e) {
      errorMsg = QString::fromStdString(e.what());
      return false;
    }
  }

private slots:
  void validatorWorksWithGoodJSON() {
    // Test the validator itself with known good template JSON
    QString srcDir = QString::fromStdString(SRCDIR);
    QString templatePath = srcDir + "/src/jsonreader/jsonreader-portable/tests/data/inst/OpenSCAD/templates/inst_template_sphere_param.json";
    QString schemaPath = srcDir + "/src/jsonreader/schemas/modern-template.schema.json";
    
    QVERIFY(QFileInfo::exists(templatePath));
    QVERIFY(QFileInfo::exists(schemaPath));
    
    json schema = loadSchema(schemaPath);
    QVERIFY(!schema.is_null());
    
    QString error;
    bool valid = validateAgainstSchema(templatePath, schema, error);
    if (!valid) {
      qWarning() << "Validation error:" << error;
    }
    QVERIFY(valid);
  }
  
  void validatorRejectsBadJSON() {
    // Test with known bad JSON (missing comma)
    QString srcDir = QString::fromStdString(SRCDIR);
    QString badJsonPath = srcDir + "/src/jsonreader/jsonreader-portable/tests/data/json/error-missing-comma.json";
    
    QVERIFY(QFileInfo::exists(badJsonPath));
    
    QFile file(badJsonPath);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QByteArray content = file.readAll();
    file.close();
    
    // Try to parse - should fail
    try {
      json doc = json::parse(content.toStdString());
      QFAIL("Expected parse error for invalid JSON");
    } catch (const std::exception& e) {
      // Expected - bad JSON should throw parse error
      QVERIFY(true);
    }
  }
  
  void writtenTemplatePassesSchemaValidation() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    
    QString path = tempDir.path() + "/validated-template.json";
    
    // Create a template that matches modern-template.schema.json
    QJsonObject template1;
    template1["prefix"] = "test_template";
    template1["description"] = "Test template for validation";
    template1["scope"] = "source.scad";
    
    QJsonArray body;
    body.append("// Test template");
    body.append("cube([${1:10}, ${2:10}, ${3:10}]);");
    body.append("$0");
    template1["body"] = body;
    
    // Write it
    JsonWriteErrorInfo err;
    bool ok = JsonWriter::writeObject(path.toStdString(), template1, err);
    QVERIFY(ok);
    
    // Load schema
    QString srcDir = QString::fromStdString(SRCDIR);
    QString schemaPath = srcDir + "/src/jsonreader/schemas/modern-template.schema.json";
    QVERIFY(QFileInfo::exists(schemaPath));
    
    json schema = loadSchema(schemaPath);
    QVERIFY(!schema.is_null());
    
    // Validate written JSON
    QString error;
    bool valid = validateAgainstSchema(path, schema, error);
    if (!valid) {
      qWarning() << "Validation error:" << error;
    }
    QVERIFY(valid);
  }
  
  void writtenVSCodeSnippetPassesValidation() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    
    QString path = tempDir.path() + "/vscode-snippet.json";
    
    // Create a VSCode snippet wrapper (how ScadTemplates saves templates)
    QJsonObject snippetObj;
    snippetObj["prefix"] = "cube_param";
    
    QJsonArray body;
    body.append("cube([${1:10}, ${2:10}, ${3:10}]);");
    snippetObj["body"] = body;
    
    snippetObj["description"] = "Parametric cube";
    snippetObj["version"] = 1;
    snippetObj["_source"] = "cppsnippet-made";
    snippetObj["_format"] = "vscode-snippet";
    
    // Wrap in named object (VSCode snippet format)
    QJsonObject root;
    root["cube_param"] = snippetObj;
    
    // Write it
    JsonWriteErrorInfo err;
    bool ok = JsonWriter::writeObject(path.toStdString(), root, err, JsonWriter::Indented);
    QVERIFY(ok);
    
    // Read back and verify it's valid JSON
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QByteArray content = file.readAll();
    file.close();
    
    try {
      json doc = json::parse(content.toStdString());
      QVERIFY(doc.is_object());
      QVERIFY(doc.contains("cube_param"));
      
      auto snippet = doc["cube_param"];
      QVERIFY(snippet.contains("prefix"));
      QVERIFY(snippet.contains("body"));
      QVERIFY(snippet.contains("description"));
      QVERIFY(snippet.contains("version"));
    } catch (const std::exception& e) {
      QFAIL(QString("JSON parse error: %1").arg(e.what()).toUtf8().constData());
    }
  }
  
  void compactFormatIsStillValid() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    
    QString path = tempDir.path() + "/compact-template.json";
    
    // Create and write in compact format
    QJsonObject template1;
    template1["prefix"] = "sphere";
    template1["body"] = "sphere(r=${1:5});";
    template1["description"] = "Simple sphere";
    
    JsonWriteErrorInfo err;
    bool ok = JsonWriter::writeObject(path.toStdString(), template1, err, JsonWriter::Compact);
    QVERIFY(ok);
    
    // Load schema
    QString srcDir = QString::fromStdString(SRCDIR);
    QString schemaPath = srcDir + "/src/jsonreader/schemas/modern-template.schema.json";
    
    json schema = loadSchema(schemaPath);
    QVERIFY(!schema.is_null());
    
    // Validate
    QString error;
    bool valid = validateAgainstSchema(path, schema, error);
    if (!valid) {
      qWarning() << "Validation error:" << error;
    }
    QVERIFY(valid);
  }
};

QTEST_MAIN(JsonWriterValidatorQtTest)
#include "test_jsonwriter_validator.moc"
