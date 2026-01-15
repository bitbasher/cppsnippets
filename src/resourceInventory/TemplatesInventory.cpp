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
                                      const QString& tier)
{
    if (!entry.isFile() || !entry.fileName().endsWith(".json")) {
        return false;
    }
    
    QString path = entry.filePath();
    QFileInfo fi(path);
    QString baseName = fi.baseName();
    
    // Generate hierarchical key: tier-name
    QString key = QString("%1-%2").arg(tier, baseName);
    
    // Check for duplicates
    if (m_templates.contains(key)) {
        qWarning() << "TemplatesInventory: Duplicate template key:" << key;
        return false;
    }
    
    // Create ResourceTemplate and populate from JSON
    ResourceTemplate tmpl(path);
    tmpl.setType(ResourceType::Templates);
    tmpl.setTier(resourceMetadata::stringToTier(tier));
    tmpl.setName(baseName);
    tmpl.setDisplayName(baseName);  // Override with prefix if JSON has one
    
    // Store in hash
    m_templates.insert(key, QVariant::fromValue(tmpl));
    
    // Parse and cache JSON content for fast editor access
    QJsonObject jsonContent = parseJsonFile(path);
    if (!jsonContent.isEmpty()) {
        m_jsonCache.insert(key, jsonContent);
    }
    
    return true;
}

int TemplatesInventory::addFolder(const QDirListing::DirEntry& entry, 
                                   const QString& tier)
{
    if (!entry.isDir()) {
        return 0;
    }
    
    QString folderPath = entry.filePath();
    int addedCount = 0;
    
    // Scan folder for .json files
    for (const auto& fileEntry : QDirListing(folderPath, {"*.json"})) {
        if (fileEntry.isFile()) {
            if (addTemplate(fileEntry, tier)) {
                addedCount++;
            }
        }
    }
    
    return addedCount;
}

QVariant TemplatesInventory::get(const QString& key) const
{
    return m_templates.value(key, QVariant());
}

QVariant TemplatesInventory::getByPath(const QString& path) const
{
    // Slower O(n) search by path
    for (const QVariant& var : m_templates) {
        if (var.canConvert<ResourceItem>()) {
            ResourceItem tmpl = var.value<ResourceItem>();
            if (tmpl.path() == path) {
                return var;
            }
        }
    }
    return QVariant();
}

bool TemplatesInventory::contains(const QString& key) const
{
    return m_templates.contains(key);
}

QList<QVariant> TemplatesInventory::getAll() const
{
    return m_templates.values();
}

// ============================================================================
// JSON Content Access
// ============================================================================

QJsonObject TemplatesInventory::getJsonContent(const QString& key) const
{
    // Check cache first
    if (m_jsonCache.contains(key)) {
        return m_jsonCache.value(key);
    }
    
    // Not cached - load from disk
    QVariant var = m_templates.value(key);
    if (!var.isValid() || !var.canConvert<ResourceItem>()) {
        qWarning() << "TemplatesInventory::getJsonContent: Template not found:" << key;
        return QJsonObject();
    }
    
    ResourceItem item = var.value<ResourceItem>();
    QJsonObject json = parseJsonFile(item.path());
    
    // Cache for next access
    if (!json.isEmpty()) {
        m_jsonCache.insert(key, json);
    }
    
    return json;
}

bool TemplatesInventory::loadJsonContent(const QString& key)
{
    // Already cached?
    if (m_jsonCache.contains(key)) {
        return true;
    }
    
    // Get metadata
    QVariant var = m_templates.value(key);
    if (!var.isValid() || !var.canConvert<ResourceItem>()) {
        return false;
    }
    
    ResourceItem item = var.value<ResourceItem>();
    QJsonObject json = parseJsonFile(item.path());
    
    if (json.isEmpty()) {
        return false;
    }
    
    m_jsonCache.insert(key, json);
    return true;
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
    
    // Get metadata for file path
    QVariant var = m_templates.value(key);
    if (!var.isValid() || !var.canConvert<ResourceItem>()) {
        errorMsg = QString("Template not found: %1").arg(key);
        return false;
    }
    
    ResourceItem item = var.value<ResourceItem>();
    QString filePath = item.path();
    
    // Use JsonWriter for atomic write
    JsonWriteErrorInfo writeError;
    bool success = JsonWriter::writeObject(filePath.toStdString(), json, writeError, JsonWriter::Indented);
    
    if (!success) {
        errorMsg = QString::fromStdString(writeError.formatError());
        return false;
    }
    
    // Update cache with new content
    m_jsonCache.insert(key, json);
    
    return true;
}

} // namespace resourceInventory
