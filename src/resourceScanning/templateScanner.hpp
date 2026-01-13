#ifndef TEMPLATESCANNER_HPP
#define TEMPLATESCANNER_HPP

#include <QList>
#include <QString>
#include <QStringList>
#include <QJsonDocument>

#include "../resourceScanning/export.hpp"
#include "../platformInfo/ResourceLocation.hpp"
#include "../resourceInventory/resourceItem.hpp"

// Use resourceInventory namespace types
using resourceInventory::ResourceTemplate;
using resourceInventory::ResourceType;
using resourceInventory::ResourceAccess;

/**
 * @brief Scanner for OpenSCAD template resources (.json files)
 * 
 * Phase 2B.1 Implementation:
 * - Scans template folders for .json files
 * - Validates JSON structure (requires "name" field)
 * - Extracts metadata into ResourceTemplate objects
 * - Returns typed QList<ResourceTemplate> (GUI-independent)
 * - Tracks tier, location, and access permissions
 * 
 * JSON Structure Expected:
 * {
 *   "name": "Template Name",           // REQUIRED
 *   "category": "Category Name",       // optional
 *   "parameters": [...],               // optional array
 *   "body": "OpenSCAD code"           // optional
 * }
 */
class PLATFORMINFO_API TemplateScanner
{
public:
    /**
     * @brief Scan a single location for template resources
     * @param location The resource location to scan
     * @return List of discovered templates with metadata
     * 
     * Scans the "templates" subfolder within the location for .json files.
     * Each valid template file is parsed, validated, and converted to a ResourceTemplate.
     * Invalid files are logged but do not abort the scan.
     */
    static QList<ResourceTemplate> scanLocation(const platformInfo::ResourceLocation& location);
    
    /**
     * @brief Scan multiple locations for template resources
     * @param locations List of resource locations to scan
     * @return Combined list of all discovered templates
     */
    static QList<ResourceTemplate> scanLocations(const QList<platformInfo::ResourceLocation>& locations);
    
    /**
     * @brief Validate template JSON structure
     * @param json Parsed JSON document to validate
     * @return true if JSON has valid template structure, false otherwise
     * 
     * Validation rules:
     * - Must be a JSON object (not array or primitive)
     * - Must have "name" field (string, non-empty)
     * - Optional fields: "category" (string), "parameters" (array), "body" (string)
     */
    static bool validateTemplateJson(const QJsonDocument& json);
    
    /**
     * @brief Extract metadata from valid JSON into ResourceTemplate
     * @param json Validated JSON document
     * @param filePath Full path to the template file
     * @param location Source location (for tier tracking)
     * @return ResourceTemplate with extracted metadata
     * 
     * Sets:
     * - name, category, body, parameters from JSON
     * - filePath, tier, locationKey, access from parameters
     * - exists = true, lastModified from filesystem
     */
    static ResourceTemplate extractMetadata(const QJsonDocument& json,
                                           const QString& filePath,
                                           const platformInfo::ResourceLocation& location);
    
    /**
     * @brief Get list of file extensions for templates
     * @return QStringList of extensions (e.g., {"json"})
     */
    static QStringList templateExtensions();
    
    /**
     * @brief Get subfolder name where templates are stored
     * @return Subfolder name (e.g., "templates")
     */
    static QString templateSubfolder();
};

#endif // TEMPLATESCANNER_HPP
