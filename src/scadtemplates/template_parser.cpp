/**
 * @file snippet_parser.cpp
 * @brief TemplateParser class implementation
 */

#include "scadtemplates/template_parser.hpp"
#include "jsonreader/JsonReader.hpp"
#include <QFile>
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
static ResourceTemplate parseLegacyTemplate(const QJsonObject& json) {
    QString key = json["key"].toString();
    QString content = json["content"].toString();
    
    // Convert escaped newlines to actual newlines
    content.replace("\\n", "\n");
    
    // Convert cursor marker from ^~^ to $0
    content.replace("^~^", "$0");
    
    ResourceTemplate tmpl;
    tmpl.setPrefix(key);
    tmpl.setBody(content);
    tmpl.setName(key);
    tmpl.setFormat(QStringLiteral("text/scad.template"));
    tmpl.setSource(QStringLiteral("legacy-converted"));
    return tmpl;
}

/**
 * @brief Parse modern template format
 * Modern format: { "template_name": { "prefix": "...", "body": [...], "description": "..." } }
 */
static QList<ResourceTemplate> parseModernTemplate(const QJsonObject& root) {
    QList<ResourceTemplate> results;
    
    // Each key in root is a template name
    for (auto it = root.begin(); it != root.end(); ++it) {
        QString templateName = it.key();
        
        // Skip metadata fields
        if (templateName.startsWith('_')) {
            continue;
        }
        
        QJsonObject templateObj = it.value().toObject();
        
        // Extract fields
        QString prefix = templateObj["prefix"].toString(templateName);
        QString description = templateObj["description"].toString(QStringLiteral("Converted from template"));
        QJsonArray bodyArray = templateObj["body"].toArray();
        
        // Join body lines
        QStringList bodyLines;
        for (const QJsonValue& line : bodyArray) {
            bodyLines.append(line.toString());
        }
        QString body = bodyLines.join('\n');
        
        ResourceTemplate tmpl;
        tmpl.setPrefix(prefix);
        tmpl.setBody(body);
        tmpl.setDescription(description);
        tmpl.setName(templateName);
        tmpl.setFormat(QStringLiteral("text/scad.template"));
        
        // Extract source if present
        if (templateObj.contains("_source")) {
            tmpl.setSource(templateObj["_source"].toString());
        } else {
            tmpl.setSource(QStringLiteral("vscode-snippet"));
        }
        
        results.append(tmpl);
    }
    
    return results;
}

ParseResult TemplateParser::parseJson(const QString& jsonContent) {
    ParseResult result;
    result.success = false;
    
    if (jsonContent.isEmpty()) {
        result.errorMessage = QStringLiteral("Empty JSON content");
        return result;
    }
    
    // Parse JSON
    QJsonDocument doc = QJsonDocument::fromJson(jsonContent.toUtf8());
    if (!doc.isObject()) {
        result.errorMessage = QStringLiteral("Invalid JSON: not an object");
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
        ResourceTemplate tmpl = parseLegacyTemplate(root);
        result.templates.append(tmpl);
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
    
    result.errorMessage = QStringLiteral("Failed to identify JSON format (not legacy or modern template)");
    return result;
}

ParseResult TemplateParser::parseFile(const QString& filePath) {
    ParseResult result;
    result.success = false;
    
    // Try using JsonReader first (expects std::string)
    QJsonObject jsonObj;
    JsonErrorInfo error;
    if (JsonReader::readObject(filePath.toStdString(), jsonObj, error)) {
        // Successfully read JSON object, check format
        if (isLegacyFormat(jsonObj)) {
            // Legacy format with single template
            ResourceTemplate tmpl = parseLegacyTemplate(jsonObj);
            result.templates.append(tmpl);
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
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.errorMessage = QStringLiteral("Failed to open file: ") + filePath;
        return result;
    }
    
    QString content = QString::fromUtf8(file.readAll());
    return parseJson(content);
}

QJsonObject TemplateParser::templateToJson(const ResourceTemplate& tmpl, const QString& source)
{
    QJsonObject snippetObj;
    
    // Get prefix as QString
    QString prefix = tmpl.prefix();
    
    // Add provenance markers for tracking resource source
    // _source indicates where this resource originated:
    //   "legacy-converted" = converted from old OpenSCAD format
    //   "cppsnippet-made" = created/modified in cppsnippets
    //   "openscad-made" = created/modified in OpenSCAD
    snippetObj["_format"] = QStringLiteral("vscode-snippet");
    snippetObj["_source"] = source;
    snippetObj["_version"] = 1;
    
    // Add prefix (same as name for now)
    snippetObj["prefix"] = prefix;
    
    // Add description
    QString desc = tmpl.description();
    if (desc.isEmpty()) {
        desc = QStringLiteral("Created in cppsnippets");
    }
    snippetObj["description"] = desc;
    
    // Add body as array of lines
    QString bodyStr = tmpl.body();
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

QString TemplateParser::toJson(const ResourceTemplate& tmpl) {
    QString json;
    json += "{\n";
    json += "  \"" + tmpl.prefix() + "\": {\n";
    json += "    \"_format\": \"vscode-snippet\",\n";
    json += "    \"_source\": \"cppsnippet-made\",\n";
    json += "    \"_version\": 1,\n";
    json += "    \"prefix\": \"" + tmpl.prefix() + "\",\n";
    json += "    \"body\": \"" + tmpl.body() + "\",\n";
    json += "    \"description\": \"" + tmpl.description() + "\"\n";
    json += "  }\n";
    json += "}";
    return json;
}

QString TemplateParser::toJson(const QList<ResourceTemplate>& templates) {
    QString json;
    json += "{\n";
    
    for (int i = 0; i < templates.size(); ++i) {
        const auto& tmpl = templates[i];
        json += "  \"" + tmpl.prefix() + "\": {\n";
        json += "    \"prefix\": \"" + tmpl.prefix() + "\",\n";
        json += "    \"body\": \"" + tmpl.body() + "\",\n";
        json += "    \"description\": \"" + tmpl.description() + "\"\n";
        json += "  }";
        if (i < templates.size() - 1) {
            json += ",";
        }
        json += "\n";
    }
    
    json += "}";
    return json;
}

} // namespace scadtemplates
