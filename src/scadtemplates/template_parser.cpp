/**
 * @file snippet_parser.cpp
 * @brief TemplateParser class implementation
 */

#include "scadtemplates/template_parser.h"
#include "JsonReader/JsonReader.h"
#include <fstream>
#include <sstream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>

namespace scadtemplates {

/**
 * @brief Check if JSON is in legacy format (has "key" and "content" fields)
 */
static bool isLegacyFormat(const QJsonObject& json) {
    return json.contains("key") && json.contains("content");
}

/**
 * @brief Check if JSON is in modern VS Code snippet format
 */
static bool isModernFormat(const QJsonObject& json) {
    // Check for format marker first - this is the definitive check
    if (json.contains("_format")) {
        QString format = json["_format"].toString();
        if (format == "vscode-snippet") {
            return true;
        }
        // If _format exists but isn't vscode-snippet, definitely not a template
        return false;
    }
    
    // Fallback for templates without _format marker (old converted files)
    // Must have VS Code snippet structure: nested objects with prefix + body array
    // Be strict to avoid matching unrelated JSON
    bool hasValidStructure = false;
    for (auto it = json.begin(); it != json.end(); ++it) {
        // Skip any top-level fields starting with underscore
        if (it.key().startsWith('_')) {
            continue;
        }
        
        if (it.value().isObject()) {
            QJsonObject inner = it.value().toObject();
            // Must have "prefix" (string) and "body" (array or string)
            if (inner.contains("prefix") && inner.contains("body")) {
                // Additional validation: body should be array for modern format
                if (inner["body"].isArray() || inner["body"].isString()) {
                    hasValidStructure = true;
                    break;
                }
            }
        }
    }
    
    return hasValidStructure;
}
static Template parseLegacyTemplate(const QJsonObject& json) {
    QString key = json["key"].toString();
    QString content = json["content"].toString();
    
    // Convert escaped newlines to actual newlines
    content.replace("\\n", "\n");
    
    // Convert cursor marker from ^~^ to $0
    content.replace("^~^", "$0");
    
    Template tmpl(key.toStdString(), content.toStdString());
    return tmpl;
}

/**
 * @brief Parse modern template format
 * Modern format: { "template_name": { "prefix": "...", "body": [...], "description": "..." } }
 */
static std::vector<Template> parseModernTemplate(const QJsonObject& root) {
    std::vector<Template> results;
    
    // Each key in root is a template name
    for (auto it = root.begin(); it != root.end(); ++it) {
        QString templateName = it.key();
        QJsonObject templateObj = it.value().toObject();
        
        // Extract fields
        QString prefix = templateObj["prefix"].toString(templateName);
        QString description = templateObj["description"].toString("Converted from template");
        QJsonArray bodyArray = templateObj["body"].toArray();
        
        // Join body lines
        QStringList bodyLines;
        for (const QJsonValue& line : bodyArray) {
            bodyLines.append(line.toString());
        }
        QString body = bodyLines.join("\n");
        
        Template tmpl(prefix.toStdString(), body.toStdString(), description.toStdString());
        results.push_back(tmpl);
    }
    
    return results;
}

ParseResult TemplateParser::parseJson(const std::string& jsonContent) {
    ParseResult result;
    result.success = false;
    
    if (jsonContent.empty()) {
        result.errorMessage = "Empty JSON content";
        return result;
    }
    
    // Parse JSON
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(jsonContent));
    if (!doc.isObject()) {
        result.errorMessage = "Invalid JSON: not an object";
        return result;
    }
    
    QJsonObject root = doc.object();
    
    // Check format marker first for explicit identification
    // Supports formats: vscode-snippet (with _source indicating provenance)
    if (root.contains("_format")) {
        QString format = root["_format"].toString();
        if (format == "vscode-snippet") {
            // Parse as modern format (regardless of _source)
            auto templates = parseModernTemplate(root);
            if (!templates.empty()) {
                result.templates = templates;
                result.success = true;
                return result;
            }
        }
    }
    
    // Check if it's legacy format
    if (isLegacyFormat(root)) {
        // Parse as legacy template
        Template tmpl = parseLegacyTemplate(root);
        result.templates.push_back(tmpl);
        result.success = true;
        return result;
    }
    
    // Try modern format (even without marker)
    if (isModernFormat(root)) {
        auto templates = parseModernTemplate(root);
        if (!templates.empty()) {
            result.templates = templates;
            result.success = true;
            return result;
        }
    }
    
    result.errorMessage = "Failed to identify JSON format (not legacy or modern template)";
    return result;
}

ParseResult TemplateParser::parseFile(const std::string& filePath) {
    ParseResult result;
    result.success = false;
    
    // Try using JsonReader first
    QJsonObject jsonObj;
    JsonErrorInfo error;
    if (JsonReader::readObject(filePath, jsonObj, error)) {
        // Successfully read JSON object, check format
        if (isLegacyFormat(jsonObj)) {
            // Legacy format with single template
            Template tmpl = parseLegacyTemplate(jsonObj);
            result.templates.push_back(tmpl);
            result.success = true;
            return result;
        }
        
        // Try as modern format (might have multiple templates)
        auto templates = parseModernTemplate(jsonObj);
        if (!templates.empty()) {
            result.templates = templates;
            result.success = true;
            return result;
        }
    }
    
    // Fallback: try reading as text and parsing JSON manually
    std::ifstream file(filePath);
    if (!file.is_open()) {
        result.errorMessage = "Failed to open file: " + filePath;
        return result;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return parseJson(buffer.str());
}

QJsonObject TemplateParser::templateToJson(const Template& tmpl, const std::string& source)
{
    QJsonObject snippetObj;
    
    // Get prefix as QString
    QString prefix = QString::fromStdString(tmpl.getPrefix());
    
    // Add provenance markers for tracking resource source
    // _source indicates where this resource originated:
    //   "legacy-converted" = converted from old OpenSCAD format
    //   "cppsnippet-made" = created/modified in cppsnippets
    //   "openscad-made" = created/modified in OpenSCAD
    snippetObj["_format"] = QStringLiteral("vscode-snippet");
    snippetObj["_source"] = QString::fromStdString(source);
    snippetObj["_version"] = 1;
    
    // Add prefix (same as name for now)
    snippetObj["prefix"] = prefix;
    
    // Add description
    QString desc = QString::fromStdString(tmpl.getDescription());
    if (desc.isEmpty()) {
        desc = QStringLiteral("Created in cppsnippets");
    }
    snippetObj["description"] = desc;
    
    // Add body as array of lines
    QString bodyStr = QString::fromStdString(tmpl.getBody());
    QStringList bodyLines = bodyStr.split('\n', Qt::KeepEmptyParts);
    
    QJsonArray bodyArray;
    for (const QString& line : bodyLines) {
        bodyArray.append(line);
    }
    snippetObj["body"] = bodyArray;
    
    // Wrap in outer object with template name as key (use prefix as key)
    QJsonObject rootObj;
    rootObj[prefix] = snippetObj;
    
    return rootObj;
}

std::string TemplateParser::toJson(const Template& tmpl) {
    std::stringstream ss;
    ss << "{\n";
    ss << "  \"" << tmpl.getPrefix() << "\": {\n";
    ss << "    \"_format\": \"vscode-snippet\",\n";
    ss << "    \"_source\": \"cppsnippet-made\",\n";
    ss << "    \"_version\": 1,\n";
    ss << "    \"prefix\": \"" << tmpl.getPrefix() << "\",\n";
    ss << "    \"body\": \"" << tmpl.getBody() << "\",\n";
    ss << "    \"description\": \"" << tmpl.getDescription() << "\"\n";
    ss << "  }\n";
    ss << "}";
    return ss.str();
}

std::string TemplateParser::toJson(const std::vector<Template>& templates) {
    std::stringstream ss;
    ss << "{\n";
    
    for (size_t i = 0; i < templates.size(); ++i) {
        const auto& tmpl = templates[i];
        ss << "  \"" << tmpl.getPrefix() << "\": {\n";
        ss << "    \"prefix\": \"" << tmpl.getPrefix() << "\",\n";
        ss << "    \"body\": \"" << tmpl.getBody() << "\",\n";
        ss << "    \"description\": \"" << tmpl.getDescription() << "\"\n";
        ss << "  }";
        if (i < templates.size() - 1) {
            ss << ",";
        }
        ss << "\n";
    }
    
    ss << "}";
    return ss.str();
}

} // namespace scadtemplates
