/**
 * @file snippet_parser.h
 * @brief Template file parsing functionality
 */

#pragma once

#include "export.hpp"
#include "../resourceInventory/resourceItem.hpp"
#include <QString>
#include <QList>
#include <optional>
#include <QJsonObject>

using resourceInventory::ResourceTemplate;

namespace scadtemplates {

/**
 * @brief Result of a parse operation
 */
struct SCADTEMPLATES_API ParseResult {
    bool success;
    QString errorMessage;
    QList<ResourceTemplate> templates;
};

/**
 * @brief Parses template files in various formats
 * 
 * Supports parsing template files in JSON format (VS Code compatible)
 * and other common template formats.
 */
class SCADTEMPLATES_API TemplateParser {
public:
    /**
     * @brief Default constructor
     */
    TemplateParser() = default;

    /**
     * @brief Parse snippets from a JSON string
     * @param jsonContent The JSON content to parse
     * @return ParseResult containing success status and parsed snippets
     */
    ParseResult parseJson(const QString& jsonContent);

    /**
     * @brief Parse templates from a file
     * @param filePath Path to the template file
     * @return ParseResult containing success status and parsed templates
     */
    ParseResult parseFile(const QString& filePath);

    /**
     * @brief Convert a template to JSON format with source provenance
     * @param tmpl The template to convert
     * @param source The provenance source (default: "cppsnippet-made")
     * @return JSON object in modern format with _source field
     */
    static QJsonObject templateToJson(const ResourceTemplate& tmpl, 
                const QString& source = QStringLiteral("cppsnippet-made"));

    /**
     * @brief Convert a template to JSON format
     * @param tmpl The template to convert
     * @return JSON string representation
     */
    QString toJson(const ResourceTemplate& tmpl);

    /**
     * @brief Convert multiple templates to JSON format
     * @param templates List of templates to convert
     * @return JSON string representation
     */
    QString toJson(const QList<ResourceTemplate>& templates);
};

} // namespace scadtemplates
