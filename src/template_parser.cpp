/**
 * @file snippet_parser.cpp
 * @brief TemplateParser class implementation
 */

#include "scadtemplates/template_parser.h"
#include <fstream>
#include <sstream>

namespace scadtemplates {

ParseResult TemplateParser::parseJson(const std::string& jsonContent) {
    ParseResult result;
    result.success = false;
    
    // Basic JSON parsing - a real implementation would use a proper JSON library
    // This is a placeholder implementation
    if (jsonContent.empty()) {
        result.errorMessage = "Empty JSON content";
        return result;
    }
    
    // For now, return success with empty templates
    // TODO: Implement proper JSON parsing (e.g., using nlohmann/json)
    result.success = true;
    return result;
}

ParseResult TemplateParser::parseFile(const std::string& filePath) {
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

std::string TemplateParser::toJson(const Template& tmpl) {
    std::stringstream ss;
    ss << "{\n";
    ss << "  \"" << tmpl.getPrefix() << "\": {\n";
    ss << "    \"prefix\": \"" << tmpl.getPrefix() << "\",\n";
    ss << "    \"body\": \"" << tmpl.getBody() << "\",\n";
    ss << "    \"description\": \"" << tmpl.getDescription() << "\"\n";
    ss << "  }\n";
    ss << "}";
    return ss.str();
}

std::string TemplateParser::toJson(const std::vector<Template>& templates) {
    std::stringstream ss;
    ss << "{\n";
    
    for (size_t i = 0; i < templates.size(); ++i) {
        const auto& tmpl = templates[i];
        ss << "  \"" << tmpl.getPrefix() << "\": {\n";
        ss << "    \"prefix\": \"" << tmpl.getPrefix() << "\",\n";
        ss << "    \"body\": \"" << tmpl.getBody() << "\",\n";
        ss << "    \"description\": \"" << tmpl.getDescription() << "\"\n";
        ss << "  }";
        if (i < templates.size() - 1) {
            ss << ",";
        }
        ss << "\n";
    }
    
    ss << "}";
    return ss.str();
}

} // namespace scadtemplates
