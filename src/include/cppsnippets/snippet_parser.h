/**
 * @file snippet_parser.h
 * @brief Snippet file parsing functionality
 */

#pragma once

#include "export.h"
#include "snippet.h"
#include <string>
#include <vector>
#include <optional>

namespace cppsnippets {

/**
 * @brief Result of a parse operation
 */
struct CPPSNIPPETS_API ParseResult {
    bool success;
    std::string errorMessage;
    std::vector<Snippet> snippets;
};

/**
 * @brief Parses snippet files in various formats
 * 
 * Supports parsing snippet files in JSON format (VS Code compatible)
 * and other common snippet formats.
 */
class CPPSNIPPETS_API SnippetParser {
public:
    /**
     * @brief Default constructor
     */
    SnippetParser() = default;

    /**
     * @brief Parse snippets from a JSON string
     * @param jsonContent The JSON content to parse
     * @return ParseResult containing success status and parsed snippets
     */
    ParseResult parseJson(const std::string& jsonContent);

    /**
     * @brief Parse snippets from a file
     * @param filePath Path to the snippet file
     * @return ParseResult containing success status and parsed snippets
     */
    ParseResult parseFile(const std::string& filePath);

    /**
     * @brief Convert a snippet to JSON format
     * @param snippet The snippet to convert
     * @return JSON string representation
     */
    std::string toJson(const Snippet& snippet);

    /**
     * @brief Convert multiple snippets to JSON format
     * @param snippets Vector of snippets to convert
     * @return JSON string representation
     */
    std::string toJson(const std::vector<Snippet>& snippets);
};

} // namespace cppsnippets
