/**
 * @file legacy_template_converter.cpp
 * @brief Implementation of legacy template converter
 */

#include "legacy_template_converter.hpp"
#include "pathDiscovery/ResourcePaths.hpp"
#include "platformInfo/ResourceLocation.hpp"
#include "jsonreader/JsonReader.hpp"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>
#include <QRegularExpression>

namespace scadtemplates {

LegacyTemplateConverter::ConversionResult 
LegacyTemplateConverter::convertFromLegacyJson(const QJsonObject& legacyJson, const QString& sourceFilePath)
{
    ConversionResult result;
    result.sourceFilePath = sourceFilePath;
    
    // Validate legacy format
    if (!isLegacyFormat(legacyJson)) {
        result.errorMessage = QStringLiteral("Not a legacy format (missing 'key' or 'content')");
        return result;
    }
    
    QString key = legacyJson.value("key").toString();
    QString content = legacyJson.value("content").toString();
    
    if (key.isEmpty()) {
        result.errorMessage = QStringLiteral("Empty 'key' field");
        return result;
    }
    
    // Store raw content
    result.rawContent = content;
    
    // Convert content to snippet body
    QStringList bodyLines = convertContentToBody(content);
    
    // Create ResourceTemplate
    ResourceTemplate tmpl;
    tmpl.setPrefix(key);
    
    // Join lines into single body string with newlines
    QString bodyJoined = bodyLines.join('\n');
    tmpl.setBody(bodyJoined);
    
    // Set description
    QString desc = QStringLiteral("Converted from legacy template");
    if (!sourceFilePath.isEmpty()) {
        desc += QStringLiteral(" (") + QFileInfo(sourceFilePath).fileName() + QStringLiteral(")");
    }
    tmpl.setDescription(desc);
    
    // Set metadata
    tmpl.setFormat(QStringLiteral("text/scad.template"));
    tmpl.setSource(QStringLiteral("legacy-converted"));
    tmpl.setName(key);
    
    result.convertedTemplate = tmpl;
    result.success = true;
    
    return result;
}

LegacyTemplateConverter::ConversionResult 
LegacyTemplateConverter::convertFromLegacyFile(const QString& filePath)
{
    ConversionResult result;
    result.sourceFilePath = filePath;
    
    // Use JsonReader to read the file
    QJsonObject jsonObj;
    JsonErrorInfo error;
    
    if (!JsonReader::readObject(filePath.toStdString(), jsonObj, error)) {
        result.errorMessage = QString::fromStdString(error.formatError());
        return result;
    }
    
    return convertFromLegacyJson(jsonObj, filePath);
}

QStringList LegacyTemplateConverter::convertContentToBody(const QString& content)
{
    // First, convert cursor marker
    QString processed = convertCursorMarker(content);
    
    // Then unescape newlines
    processed = unescapeNewlines(processed);
    
    // Split into lines
    return processed.split('\n');
}

QString LegacyTemplateConverter::convertCursorMarker(const QString& text)
{
    // Replace ^~^ with $0 (final cursor position)
    QString result = text;
    result.replace(QStringLiteral("^~^"), QStringLiteral("$0"));
    return result;
}

QString LegacyTemplateConverter::unescapeNewlines(const QString& text)
{
    // Replace literal \n with actual newline
    QString result = text;
    result.replace(QStringLiteral("\\n"), QStringLiteral("\n"));
    return result;
}

QList<LegacyTemplateConverter::ConversionResult> 
LegacyTemplateConverter::discoverAndConvertTemplates(
    const pathDiscovery::ResourcePaths& resourcePaths,
    const QString& outputDir)
{
    QList<ConversionResult> results;
    
    // Structure: outputDir/tier/mangled-filename.json
    QDir baseDir(outputDir);
    if (!baseDir.exists()) {
        baseDir.mkpath(".");
    }
    
    // Get all qualified paths and convert to ResourceLocations grouped by tier
    QList<pathDiscovery::PathElement> allPaths = resourcePaths.qualifiedSearchPaths();
    
    struct TierInfo {
        QString name;
        QList<platformInfo::ResourceLocation> locations;
    };
    
    // Group paths by tier
    QList<platformInfo::ResourceLocation> installationLocs;
    QList<platformInfo::ResourceLocation> machineLocs;
    QList<platformInfo::ResourceLocation> userLocs;
    
    for (const auto& pe : allPaths) {
        QFileInfo info(pe.path());
        if (!info.exists() || !info.isDir()) {
            continue;  // Skip non-existent paths
        }
        
        platformInfo::ResourceLocation loc(pe.path(), pe.tier());
        
        switch (pe.tier()) {
            case resourceMetadata::ResourceTier::Installation:
                installationLocs.append(loc);
                break;
            case resourceMetadata::ResourceTier::Machine:
                machineLocs.append(loc);
                break;
            case resourceMetadata::ResourceTier::User:
                userLocs.append(loc);
                break;
            default:
                break;
        }
    }
    
    std::vector<TierInfo> tiers = {
        { QStringLiteral("installation"), installationLocs },
        { QStringLiteral("machine"), machineLocs },
        { QStringLiteral("user"), userLocs }
    };
    
    for (const auto& tier : tiers) {
        // Create tier subdirectory
        QString tierPath = baseDir.filePath(tier.name);
        QDir tierDir(tierPath);
        if (!tierDir.exists()) {
            tierDir.mkpath(".");
        }
        
        // Scan each location in this tier
        for (const auto& location : tier.locations) {
            QString basePath = location.path();
            
            // Look for templates subdirectory
            QDir templateDir(basePath + QStringLiteral("/templates"));
            if (!templateDir.exists()) {
                continue;
            }
            
            // Find all .json files
            QStringList jsonFiles = templateDir.entryList(
                QStringList() << QStringLiteral("*.json"),
                QDir::Files | QDir::Readable
            );
            
            for (const QString& filename : jsonFiles) {
                QString fullPath = templateDir.filePath(filename);
                
                // Convert the file
                ConversionResult result = convertFromLegacyFile(fullPath);
                
                if (result.success) {
                    // Generate output filename
                    QString mangledName = manglePathToFilename(fullPath);
                    QString outputPath = tierDir.filePath(mangledName);
                    
                    // Save the converted template
                    bool saved = saveAsModernJson(result.convertedTemplate, outputPath);
                    if (saved) {
                        qDebug() << "Converted and saved:" << fullPath << "->" << outputPath;
                    } else {
                        qWarning() << "Failed to save:" << outputPath;
                        result.success = false;
                        result.errorMessage = QStringLiteral("Failed to write output file");
                    }
                }
                
                results.append(result);
            }
        }
    }
    
    return results;
}

QString LegacyTemplateConverter::manglePathToFilename(const QString& filePath)
{
    // Convert: C:/Program Files/OpenSCAD/templates/function.json
    // To: program-files-openscad-function.json
    
    QFileInfo fileInfo(filePath);
    QString baseName = fileInfo.completeBaseName();  // "function"
    QString dirPath = fileInfo.absolutePath();       // "C:/Program Files/OpenSCAD/templates"
    
    // Remove drive letter and normalize
    QString normalized = dirPath;
    normalized.remove(QRegularExpression(QStringLiteral("^[A-Za-z]:")));  // Remove C:
    normalized.replace('\\', '/');
    normalized = normalized.toLower();
    
    // Remove leading slashes and "templates" folder name
    normalized.remove(QRegularExpression(QStringLiteral("^/+")));
    normalized.remove(QRegularExpression(QStringLiteral("/templates$")));
    
    // Replace path separators and spaces with dashes
    normalized.replace('/', '-');
    normalized.replace(' ', '-');
    
    // Combine with base name
    if (!normalized.isEmpty()) {
        return normalized + QStringLiteral("-") + baseName + QStringLiteral(".json");
    } else {
        return baseName + QStringLiteral(".json");
    }
}

bool LegacyTemplateConverter::isLegacyFormat(const QJsonObject& jsonObj)
{
    return jsonObj.contains("key") && jsonObj.contains("content");
}

QJsonObject LegacyTemplateConverter::templateToModernJson(const ResourceTemplate& tmpl)
{
    QJsonObject snippetObj;
    
    // Get prefix as QString
    QString prefix = tmpl.prefix();
    
    // Add provenance markers for tracking resource source
    // legacy-converted = converted from old OpenSCAD template format by this tool
    snippetObj["_format"] = QStringLiteral("vscode-snippet");
    snippetObj["_source"] = QStringLiteral("legacy-converted");
    snippetObj["_version"] = 1;
    
    // Add prefix (same as name for now)
    snippetObj["prefix"] = prefix;
    
    // Add description
    QString desc = tmpl.description();
    if (desc.isEmpty()) {
        desc = QStringLiteral("Converted from legacy OpenSCAD template");
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

bool LegacyTemplateConverter::saveAsModernJson(const ResourceTemplate& tmpl, const QString& outputPath)
{
    // Convert to JSON
    QJsonObject json = templateToModernJson(tmpl);
    
    // Write to file
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for writing:" << outputPath;
        return false;
    }
    
    QJsonDocument doc(json);
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);
    
    qint64 written = file.write(jsonData);
    file.close();
    
    if (written == -1) {
        qWarning() << "Failed to write JSON to file:" << outputPath;
        return false;
    }
    
    return true;
}

} // namespace scadtemplates
