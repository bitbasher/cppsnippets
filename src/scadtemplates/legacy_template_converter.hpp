/**
 * @file legacy_template_converter.h
 * @brief Converter for legacy OpenSCAD template format
 */

#pragma once

#include "export.hpp"
#include "template.hpp"
#include <QString>
#include <QJsonObject>
#include <string>
#include <optional>
#include <vector>

namespace platformInfo {
class ResourceLocationManager;
}

namespace scadtemplates {

/**
 * @brief Converts legacy OpenSCAD template format to modern snippet format
 * 
 * Legacy format (one template per file):
 * {
 *   "key": "template_name",
 *   "content": "raw text with \\n escapes and ^~^ cursor marker"
 * }
 * 
 * Modern format (VS Code snippet):
 * {
 *   "template_name": {
 *     "prefix": "template_name",
 *     "body": ["line1", "line2"],
 *     "description": "Converted from legacy format"
 *   }
 * }
 * 
 * Cursor marker: ^~^ → $0 (final cursor position)
 */
class SCADTEMPLATES_API LegacyTemplateConverter {
public:
    /**
     * @brief Result of a conversion operation
     */
    struct ConversionResult {
        bool success = false;
        QString errorMessage;
        Template convertedTemplate;
        QString rawContent;  // Original content for re-conversion
        QString sourceFilePath;  // Where it came from
    };

    /**
     * @brief Convert a legacy template QJsonObject to a Template
     * @param legacyJson The legacy format JSON object (with "key" and "content")
     * @param sourceFilePath Original file path for metadata
     * @return Conversion result with template if successful
     */
    static ConversionResult convertFromLegacyJson(const QJsonObject& legacyJson, 
                                                   const QString& sourceFilePath = QString());

    /**
     * @brief Convert a legacy template file to a Template using JsonReader
     * @param filePath Path to the legacy template file
     * @return Conversion result with template if successful
     */
    static ConversionResult convertFromLegacyFile(const QString& filePath);

    /**
     * @brief Convert content string from legacy format to snippet body
     * @param content Legacy content with \n escapes and ^~^ cursor marker
     * @return Vector of body lines with snippet placeholders
     */
    static QStringList convertContentToBody(const QString& content);

    /**
     * @brief Convert cursor marker (^~^) to snippet placeholder ($0)
     * @param text Text containing ^~^ markers
     * @return Text with snippet placeholders
     */
    static QString convertCursorMarker(const QString& text);

    /**
     * @brief Unescape newlines (\n) in legacy content
     * @param text Text with escaped newlines
     * @return Text with actual newlines
     */
    static QString unescapeNewlines(const QString& text);

    /**
     * @brief Scan resource locations for legacy template files and convert them
     * @param resourceManager Resource location manager with tier locations
     * @param outputDir Base directory for converted templates (e.g., "templates/")
     * @return Vector of conversion results
     */
    static std::vector<ConversionResult> discoverAndConvertTemplates(
        const platformInfo::ResourceLocationManager& resourceManager,
        const QString& outputDir);

    /**
     * @brief Mangle a file path into a safe filename
     * @param filePath Original file path (e.g., "C:/Program Files/OpenSCAD/templates/function.json")
     * @return Mangled name (e.g., "program-files-openscad-function.json")
     */
    static QString manglePathToFilename(const QString& filePath);

    /**
     * @brief Check if a JSON file is in legacy format
     * @param jsonObj JSON object to check
     * @return true if it has "key" and "content" fields
     */
    static bool isLegacyFormat(const QJsonObject& jsonObj);

    /**
     * @brief Save a Template to modern VS Code snippet JSON format
     * @param tmpl The template to save
     * @param outputPath Path where to write the JSON file
     * @return true if successful, false on error
     */
    static bool saveAsModernJson(const Template& tmpl, const QString& outputPath);

    /**
     * @brief Convert a Template to modern JSON format
     * @param tmpl The template to convert
     * @return JSON object in modern format
     */
    static QJsonObject templateToModernJson(const Template& tmpl);
};

} // namespace scadtemplates
