/**
 * @file TemplatesInventory.cpp
 * @brief Implementation of template inventory storage
 */

#include "TemplatesInventory.hpp"
#include "../resourceMetadata/ResourceTypeInfo.hpp"
#include "../scadtemplates/legacy_template_converter.hpp"
#include "JsonWriter/JsonWriter.h"

#include <QDir>
#include <QFileInfo>
#include <QDirListing>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QCoreApplication>

#include <nlohmann/json.hpp>
#include <nlohmann/json-schema.hpp>

using nlohmann::json;
using nlohmann::json_schema::json_validator;

namespace resourceInventory {

bool TemplatesInventory::addTemplate(const QDirListing::DirEntry& entry, 
                                      const platformInfo::ResourceLocation& location)
{
    // Create ResourceTemplate with location-based constructor
    // This automatically sets type, tier, name, displayName, and uniqueID
    ResourceTemplate tmpl(entry.filePath(), location);
    
    // Read JSON to populate template body, prefix, scopes, etc.
    if (!tmpl.readJson(entry.fileInfo())) {
        qWarning() << "TemplatesInventory: Failed to read template JSON:" << entry.filePath()
                   << "-" << tmpl.lastError();
        return false;
    }
    
    // Try to insert - fails if uniqueID already exists (atomic duplicate detection)
    QString uniqueID = tmpl.uniqueID();
    auto result = m_templates.tryInsert(uniqueID, QVariant::fromValue(tmpl));
    
    if (!result.inserted) {
        qWarning() << "TemplatesInventory: Duplicate template ID:" << uniqueID
                   << "at" << entry.filePath();
        return false;
    }
    
    return true;
}

int TemplatesInventory::addFolder(const QString& folderPath, 
                                   const platformInfo::ResourceLocation& location)
{
    int sizeBefore = m_templates.size();
    
    // Scan folder for .json files (FilesOnly flag eliminates need for isFile() check)
    QDirListing listing(folderPath, {"*.json"}, QDirListing::IteratorFlag::FilesOnly);
    
    for (const auto& fileEntry : listing) {
        addTemplate(fileEntry, location);  // Failures logged internally
    }
    
    return m_templates.size() - sizeBefore;
}

QJsonObject TemplatesInventory::getJsonContent(const QString& key) const
{
    // Get template metadata
    QVariant var = m_templates.value(key);
    if (!var.canConvert<ResourceTemplate>()) {
        qWarning() << "TemplatesInventory::getJsonContent: Template not found:" << key;
        return QJsonObject();
    }
    
    ResourceTemplate tmpl = var.value<ResourceTemplate>();
    
    // Load JSON from disk (no caching)
    return parseJsonFile(tmpl.path());
}

QJsonObject TemplatesInventory::parseJsonFile(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "TemplatesInventory: Cannot open JSON file:" << filePath;
        return QJsonObject();
    }
    
    QByteArray jsonData = file.readAll();
    file.close();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "TemplatesInventory: JSON parse error in" << filePath 
                   << ":" << parseError.errorString();
        return QJsonObject();
    }
    
    if (!doc.isObject()) {
        qWarning() << "TemplatesInventory: JSON file is not an object:" << filePath;
        return QJsonObject();
    }
    
    QJsonObject jsonObj = doc.object();
    
    // Unwrap snippet: Modern format has wrapper key (e.g., {"cube_basic": {...}})
    // Extract the first (and only) snippet's content
    if (!jsonObj.isEmpty()) {
        QString firstKey = jsonObj.keys().first();
        QJsonValue snippetValue = jsonObj.value(firstKey);
        if (snippetValue.isObject()) {
            jsonObj = snippetValue.toObject();  // Use the snippet content, not wrapper
        }
    }
    
    // Validate against schema
    static json schemaJson;
    static json_validator validator;
    static bool schemaLoaded = false;
    
    if (!schemaLoaded) {
        // Load schema from lib/snippet.schema.json
        QString schemaPath = QDir(qApp->applicationDirPath()).filePath("../lib/snippet.schema.json");
        QFile schemaFile(schemaPath);
        
        if (!schemaFile.open(QIODevice::ReadOnly)) {
            // Try alternative path (build directory)
            schemaPath = "lib/snippet.schema.json";
            schemaFile.setFileName(schemaPath);
            if (!schemaFile.open(QIODevice::ReadOnly)) {
                qWarning() << "TemplatesInventory: Cannot load schema file - validation disabled";
                schemaLoaded = true; // Don't keep trying
            }
        }
        
        if (schemaFile.isOpen()) {
            try {
                QByteArray schemaData = schemaFile.readAll();
                schemaFile.close();
                schemaJson = json::parse(schemaData.toStdString());
                validator.set_root_schema(schemaJson);
                schemaLoaded = true;
                qDebug() << "TemplatesInventory: Schema loaded from" << schemaPath;
            } catch (const std::exception& e) {
                qWarning() << "TemplatesInventory: Schema load error:" << e.what();
                schemaLoaded = true; // Don't keep trying
            }
        }
    }
    
    // Convert QJsonObject to nlohmann::json for validation
    if (schemaLoaded && !schemaJson.empty()) {
        try {
            std::string jsonStr = QString(QJsonDocument(jsonObj).toJson()).toStdString();
            json nlohmannJson = json::parse(jsonStr);
            
            // Validate
            validator.validate(nlohmannJson);
            
            // Valid modern format
            return jsonObj;
            
        } catch (const std::exception& e) {
            // Validation failed - check format
            qDebug() << "TemplatesInventory: Schema validation failed for" << filePath 
                     << ":" << e.what();
            
            // Check if it's VS Code snippet format (has body, prefix, no filetype/varient)
            bool isVSCodeFormat = jsonObj.contains("body") && jsonObj.contains("prefix") 
                                  && !jsonObj.contains("filetype") && !jsonObj.contains("varient");
            
            if (isVSCodeFormat) {
                qDebug() << "TemplatesInventory: Detected VS Code snippet format (allowed)";
                return jsonObj;
            }
            
            // Check if it's legacy format
            if (scadtemplates::LegacyTemplateConverter::isLegacyFormat(jsonObj)) {
                qInfo() << "TemplatesInventory: Detected legacy format in" << filePath 
                        << "- converting...";
                
                auto result = scadtemplates::LegacyTemplateConverter::convertFromLegacyJson(
                    jsonObj, filePath);
                
                if (result.success) {
                    // Convert ResourceTemplate to modern JSON format
                    QJsonObject modernJson = scadtemplates::LegacyTemplateConverter::templateToModernJson(
                        result.convertedTemplate);
                    
                    qInfo() << "TemplatesInventory: Successfully converted legacy template";
                    return modernJson;
                } else {
                    qWarning() << "TemplatesInventory: Legacy conversion failed:" 
                               << result.errorMessage;
                    return QJsonObject();
                }
            } else {
                qWarning() << "TemplatesInventory: Not a valid template format (not legacy either)";
                return QJsonObject();
            }
        }
    }
    
    // Schema not loaded - return without validation
    return jsonObj;
}

bool TemplatesInventory::writeJsonContent(const QString& key, const QJsonObject& json, QString& errorMsg)
{
    errorMsg.clear();
    
    // Get template metadata for file path
    QVariant var = m_templates.value(key);
    if (!var.canConvert<ResourceTemplate>()) {
        errorMsg = QString("Template not found: %1").arg(key);
        return false;
    }
    
    ResourceTemplate tmpl = var.value<ResourceTemplate>();
    QString filePath = tmpl.path();
    
    // Use JsonWriter for atomic write
    JsonWriteErrorInfo writeError;
    bool success = JsonWriter::writeObject(filePath.toStdString(), json, writeError, JsonWriter::Indented);
    
    if (!success) {
        errorMsg = QString::fromStdString(writeError.formatError());
        return false;
    }
    
    return true;
}

} // namespace resourceInventory
