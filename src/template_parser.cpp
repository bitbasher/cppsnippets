/**
 * @file snippet_parser.cpp
 * @brief SnippetParser class implementation
 */

#include "cppsnippets/snippet_parser.h"
#include <fstream>
#include <sstream>

namespace cppsnippets {

ParseResult SnippetParser::parseJson(const std::string& jsonContent) {
    ParseResult result;
    result.success = false;
    
    // Basic JSON parsing - a real implementation would use a proper JSON library
    // This is a placeholder implementation
    if (jsonContent.empty()) {
        result.errorMessage = "Empty JSON content";
        return result;
    }
    
    // For now, return success with empty snippets
    // TODO: Implement proper JSON parsing (e.g., using nlohmann/json)
    result.success = true;
    return result;
}

ParseResult SnippetParser::parseFile(const std::string& filePath) {
    ParseResult result;
    result.success = false;
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        result.errorMessage = "Failed to open file: " + filePath;
        return result;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    return parseJson(buffer.str());
}

std::string SnippetParser::toJson(const Snippet& snippet) {
    std::stringstream ss;
    ss << "{\n";
    ss << "  \"" << snippet.getPrefix() << "\": {\n";
    ss << "    \"prefix\": \"" << snippet.getPrefix() << "\",\n";
    ss << "    \"body\": \"" << snippet.getBody() << "\",\n";
    ss << "    \"description\": \"" << snippet.getDescription() << "\"\n";
    ss << "  }\n";
    ss << "}";
    return ss.str();
}

std::string SnippetParser::toJson(const std::vector<Snippet>& snippets) {
    std::stringstream ss;
    ss << "{\n";
    
    for (size_t i = 0; i < snippets.size(); ++i) {
        const auto& snippet = snippets[i];
        ss << "  \"" << snippet.getPrefix() << "\": {\n";
        ss << "    \"prefix\": \"" << snippet.getPrefix() << "\",\n";
        ss << "    \"body\": \"" << snippet.getBody() << "\",\n";
        ss << "    \"description\": \"" << snippet.getDescription() << "\"\n";
        ss << "  }";
        if (i < snippets.size() - 1) {
            ss << ",";
        }
        ss << "\n";
    }
    
    ss << "}";
    return ss.str();
}

} // namespace cppsnippets
