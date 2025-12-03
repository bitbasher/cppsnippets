/**
 * @file test_snippet_parser.cpp
 * @brief Unit tests for the TemplateParser class
 */

#include <gtest/gtest.h>
#include <scadtemplates/template_parser.h>

using namespace scadtemplates;

TEST(TemplateParserTest, ParseEmptyJson) {
    TemplateParser parser;
    auto result = parser.parseJson("");
    
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, "Empty JSON content");
}

TEST(TemplateParserTest, ParseValidJson) {
    TemplateParser parser;
    auto result = parser.parseJson("{}");
    
    EXPECT_TRUE(result.success);
}

TEST(TemplateParserTest, ParseNonExistentFile) {
    TemplateParser parser;
    auto result = parser.parseFile("/nonexistent/path/templates.json");
    
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.errorMessage.find("Failed to open file") != std::string::npos);
}

TEST(TemplateParserTest, ToJsonSingleTemplate) {
    TemplateParser parser;
    Template tmpl("log", "console.log($1);", "Log message");
    
    std::string json = parser.toJson(tmpl);
    
    EXPECT_TRUE(json.find("\"log\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"prefix\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"body\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"description\"") != std::string::npos);
}

TEST(TemplateParserTest, ToJsonMultipleTemplates) {
    TemplateParser parser;
    std::vector<Template> templates;
    templates.emplace_back("log", "console.log($1);", "Log message");
    templates.emplace_back("for", "for(;;)", "For loop");
    
    std::string json = parser.toJson(templates);
    
    EXPECT_TRUE(json.find("\"log\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"for\"") != std::string::npos);
}

TEST(TemplateParserTest, ToJsonEmptyVector) {
    TemplateParser parser;
    std::vector<Template> templates;
    
    std::string json = parser.toJson(templates);
    
    EXPECT_EQ(json, "{\n}");
}
