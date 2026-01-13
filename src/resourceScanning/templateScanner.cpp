#include "templateScanner.hpp"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>
#include <QDateTime>

QList<ResourceTemplate> TemplateScanner::scanLocation(const platformInfo::ResourceLocation& location)
{
    QList<ResourceTemplate> templates;
    
    // Build path to templates subfolder
    QString templatesPath = location.path() + "/" + templateSubfolder();
    
    qDebug() << "TemplateScanner: Scanning" << templatesPath;
    
    // Check if templates folder exists
    QDir templatesDir(templatesPath);
    if (!templatesDir.exists()) {
        qDebug() << "TemplateScanner: Templates folder does not exist:" << templatesPath;
        return templates;
    }
    
    // Iterate through .json files in templates folder (flat scan, no recursion)
    QDirIterator it(templatesPath,
                    QStringList() << "*.json",
                    QDir::Files | QDir::Readable | QDir::NoDotAndDotDot,
                    QDirIterator::NoIteratorFlags);  // No recursion - templates are flat
    
    int filesScanned = 0;
    int filesValid = 0;
    
    while (it.hasNext()) {
        QString filePath = it.next();
        filesScanned++;
        
        qDebug() << "TemplateScanner: Processing" << filePath;
        
        // Read file contents
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "TemplateScanner: Cannot open file:" << filePath << file.errorString();
            continue;
        }
        
        QByteArray fileData = file.readAll();
        file.close();
        
        // Parse JSON
        QJsonParseError parseError;
        QJsonDocument json = QJsonDocument::fromJson(fileData, &parseError);
        
        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "TemplateScanner: JSON parse error in" << filePath 
                      << ":" << parseError.errorString() << "at offset" << parseError.offset;
            continue;
        }
        
        // Validate template structure
        if (!validateTemplateJson(json)) {
            qWarning() << "TemplateScanner: Invalid template structure in" << filePath;
            continue;
        }
        
        // Extract metadata into ResourceTemplate
        ResourceTemplate tmpl = extractMetadata(json, filePath, location);
        
        // Set filesystem metadata
        QFileInfo fileInfo(filePath);
        tmpl.setExists(true);
        tmpl.setLastModified(fileInfo.lastModified());
        
        templates.append(tmpl);
        filesValid++;
        
        qDebug() << "TemplateScanner: Valid template:" << tmpl.name() 
                << "category:" << tmpl.category();
    }
    
    qDebug() << "TemplateScanner: Scanned" << filesScanned << "files," 
            << filesValid << "valid templates found";
    
    return templates;
}

QList<ResourceTemplate> TemplateScanner::scanLocations(const QList<platformInfo::ResourceLocation>& locations)
{
    QList<ResourceTemplate> allTemplates;
    
    for (const auto& location : locations) {
        if (!location.exists() || !location.isEnabled()) {
            qDebug() << "TemplateScanner: Skipping disabled/non-existent location:" 
                    << location.getDisplayName();
            continue;
        }
        
        QList<ResourceTemplate> locationTemplates = scanLocation(location);
        allTemplates.append(locationTemplates);
    }
    
    qDebug() << "TemplateScanner: Total templates across all locations:" << allTemplates.size();
    
    return allTemplates;
}

bool TemplateScanner::validateTemplateJson(const QJsonDocument& json)
{
    // Must be an object
    if (!json.isObject()) {
        qWarning() << "TemplateScanner: JSON is not an object";
        return false;
    }
    
    QJsonObject obj = json.object();
    
    // REQUIRED: "name" field must exist and be a non-empty string
    if (!obj.contains("name")) {
        qWarning() << "TemplateScanner: Missing required 'name' field";
        return false;
    }
    
    QJsonValue nameValue = obj.value("name");
    if (!nameValue.isString()) {
        qWarning() << "TemplateScanner: 'name' field is not a string";
        return false;
    }
    
    QString name = nameValue.toString().trimmed();
    if (name.isEmpty()) {
        qWarning() << "TemplateScanner: 'name' field is empty";
        return false;
    }
    
    // OPTIONAL: "category" field - if present, must be string
    if (obj.contains("category")) {
        if (!obj.value("category").isString()) {
            qWarning() << "TemplateScanner: 'category' field is not a string";
            return false;
        }
    }
    
    // OPTIONAL: "parameters" field - if present, must be array
    if (obj.contains("parameters")) {
        if (!obj.value("parameters").isArray()) {
            qWarning() << "TemplateScanner: 'parameters' field is not an array";
            return false;
        }
    }
    
    // OPTIONAL: "body" field - if present, must be string
    if (obj.contains("body")) {
        if (!obj.value("body").isString()) {
            qWarning() << "TemplateScanner: 'body' field is not a string";
            return false;
        }
    }
    
    return true;
}

ResourceTemplate TemplateScanner::extractMetadata(const QJsonDocument& json,
                                                  const QString& filePath,
                                                  const platformInfo::ResourceLocation& location)
{
    ResourceTemplate tmpl;
    QJsonObject obj = json.object();
    
    // Extract REQUIRED field: name
    QString name = obj.value("name").toString().trimmed();
    tmpl.setName(name);
    tmpl.setDisplayName(name);  // Use name as display name
    
    // Extract OPTIONAL field: category
    if (obj.contains("category")) {
        QString category = obj.value("category").toString().trimmed();
        tmpl.setCategory(category);
    }
    
    // Extract OPTIONAL field: body
    if (obj.contains("body")) {
        QString body = obj.value("body").toString();
        tmpl.setBody(body);
    }
    
    // Extract OPTIONAL field: parameters
    if (obj.contains("parameters")) {
        QJsonArray paramsArray = obj.value("parameters").toArray();
        QString paramsStr;
        for (const QJsonValue& paramValue : paramsArray) {
            if (paramValue.isString()) {
                if (!paramsStr.isEmpty()) paramsStr += ", ";
                paramsStr += paramValue.toString();
            }
        }
        // Store as comma-separated string or in custom field if available
        // For now, we'll store in description
        if (!paramsStr.isEmpty()) {
            tmpl.setDescription("Parameters: " + paramsStr);
        }
    }
    
    // Set path metadata
    tmpl.setPath(filePath);
    
    // Set tier from location
    tmpl.setTier(location.tier());
    
    // Set source tracking (location path)
    tmpl.setSourcePath(location.path());
    tmpl.setSourceLocationKey(location.path());
    
    // Set access based on location writeability
    tmpl.setAccess(location.isWritable() ? ResourceAccess::ReadWrite : ResourceAccess::ReadOnly);
    
    // Set resource type
    tmpl.setType(ResourceType::Templates);
    
    // Store raw JSON text for potential future use
    tmpl.setRawText(json.toJson(QJsonDocument::Compact));
    
    return tmpl;
}

QStringList TemplateScanner::templateExtensions()
{
    return QStringList() << "json";
}

QString TemplateScanner::templateSubfolder()
{
    return "templates";
}
