/**
 * @file TemplatesInventory.cpp
 * @brief Implementation of template inventory storage
 */

#include "TemplatesInventory.hpp"
#include "ResourceIndexer.hpp"
#include "../resourceMetadata/ResourceTypeInfo.hpp"
#include "../scadtemplates/legacy_template_converter.hpp"
#include "JsonWriter/JsonWriter.h"

#include <QDir>
#include <QDirListing>
#include <QFileInfo>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QCoreApplication>
#include <QVariant>

#include <nlohmann/json.hpp>
#include <nlohmann/json-schema.hpp>

using nlohmann::json;
using nlohmann::json_schema::json_validator;

namespace resourceInventory {

TemplatesInventory::TemplatesInventory(QObject* parent)
    : QAbstractItemModel(parent)
{
}

bool TemplatesInventory::addTemplate(const QDirListing::DirEntry& entry, 
                                      const platformInfo::ResourceLocation& location)
{
    QFileInfo fi(entry.filePath());
    QString baseName = fi.baseName();
    
    // Create ResourceTemplate with convenience constructor
    ResourceTemplate tmpl(entry.filePath(), baseName);
    tmpl.setTier(location.tier());
    tmpl.setDisplayName(baseName);
    
    // Read and validate JSON FIRST before generating ID
    if (!tmpl.readJson(entry.fileInfo())) {
        qWarning() << "TemplatesInventory: Failed to read template JSON:" << entry.filePath()
                   << "-" << tmpl.lastError();
        return false;
    }
    
    // NOW generate unique ID after validation succeeds - use ResourceIndexer
    QString uniqueID = ResourceIndexer::getUniqueIDString(baseName);
    tmpl.setUniqueID(uniqueID);
    
    // Try to insert - fails if uniqueID already exists (atomic duplicate detection)
    if (m_templates.contains(uniqueID)) {
        qWarning() << "TemplatesInventory: Duplicate template ID:" << uniqueID
                   << "at" << entry.filePath();
        return false;
    }
    
    // Insert into inventory
    int row = m_keys.size();
    beginInsertRows(QModelIndex(), row, row);
    m_templates.insert(uniqueID, tmpl);
    m_keys.append(uniqueID);
    endInsertRows();
    
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

int TemplatesInventory::scanLocation(const platformInfo::ResourceLocation& location)
{
    const QString& folder = resourceMetadata::ResourceTypeInfo::s_resourceTypes[resourceMetadata::ResourceType::Templates].getSubDir();
    QString templatesPath = location.path() + "/" + folder;
    return addFolder(templatesPath, location);
}

int TemplatesInventory::scanLocations(const QList<platformInfo::ResourceLocation>& locations)
{
    int total = 0;
    for (const auto& location : locations) {
        total += scanLocation(location);
    }
    return total;
}

QJsonObject TemplatesInventory::getJsonContent(const QString& key) const
{
    ResourceTemplate tmpl = m_templates.value(key, ResourceTemplate());
    if (tmpl.path().isEmpty()) {
        qWarning() << "TemplatesInventory::getJsonContent: Template not found:" << key;
        return QJsonObject();
    }
    
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
    ResourceTemplate tmpl = m_templates.value(key, ResourceTemplate());
    if (tmpl.path().isEmpty()) {
        errorMsg = QString("Template not found: %1").arg(key);
        return false;
    }
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

QList<QVariant> TemplatesInventory::getAll() const
{
    QList<QVariant> result;
    result.reserve(m_templates.size());
    for (const auto& key : m_keys) {
        result.append(QVariant::fromValue(m_templates.value(key)));
    }
    return result;
}

void TemplatesInventory::clear()
{
    beginResetModel();
    m_templates.clear();
    m_keys.clear();
    endResetModel();
}

QModelIndex TemplatesInventory::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return QModelIndex();
    }
    if (row < 0 || column < 0 || row >= m_keys.size() || column >= 2) {
        return QModelIndex();
    }
    return createIndex(row, column);
}

QModelIndex TemplatesInventory::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int TemplatesInventory::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_keys.size();
}

int TemplatesInventory::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 2; // Name, ID
}

QVariant TemplatesInventory::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_keys.size()) {
        return QVariant();
    }
    const QString& key = m_keys.at(index.row());
    const ResourceTemplate tmpl = m_templates.value(key, ResourceTemplate());
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: return tmpl.displayName();
            case 1: return tmpl.uniqueID();
            default: return QVariant();
        }
    }
    if (role == Qt::UserRole) {
        return QVariant::fromValue(tmpl);
    }
    if (role == Qt::ToolTipRole && index.column() == 0) {
        return QString("Path: %1\nTier: %2\nID: %3")
            .arg(tmpl.path())
            .arg(resourceMetadata::tierToString(tmpl.tier()))
            .arg(tmpl.uniqueID());
    }
    return QVariant();
}

QVariant TemplatesInventory::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return QStringLiteral("Name");
            case 1: return QStringLiteral("ID");
            default: return QVariant();
        }
    }
    return QVariant();
}

} // namespace resourceInventory
