/**
 * @file snippet_parser.h
 * @brief Template file parsing functionality
 */

#pragma once

#include "export.h"
#include "template.h"
#include <string>
#include <vector>
#include <optional>

namespace scadtemplates {

/**
 * @brief Result of a parse operation
 */
struct SCADTEMPLATES_API ParseResult {
    bool success;
    std::string errorMessage;
    std::vector<Template> templates;
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
    ParseResult parseJson(const std::string& jsonContent);

    /**
     * @brief Parse templates from a file
     * @param filePath Path to the template file
     * @return ParseResult containing success status and parsed templates
     */
    ParseResult parseFile(const std::string& filePath);

    /**
     * @brief Convert a template to JSON format
     * @param tmpl The template to convert
     * @return JSON string representation
     */
    std::string toJson(const Template& tmpl);

    /**
     * @brief Convert multiple templates to JSON format
     * @param templates Vector of templates to convert
     * @return JSON string representation
     */
    std::string toJson(const std::vector<Template>& templates);
};

} // namespace scadtemplates
