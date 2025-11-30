/**
 * @file test_snippet_parser.cpp
 * @brief Unit tests for the SnippetParser class
 */

#include <gtest/gtest.h>
#include <cppsnippets/snippet_parser.h>

using namespace cppsnippets;

TEST(SnippetParserTest, ParseEmptyJson) {
    SnippetParser parser;
    auto result = parser.parseJson("");
    
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, "Empty JSON content");
}

TEST(SnippetParserTest, ParseValidJson) {
    SnippetParser parser;
    auto result = parser.parseJson("{}");
    
    EXPECT_TRUE(result.success);
}

TEST(SnippetParserTest, ParseNonExistentFile) {
    SnippetParser parser;
    auto result = parser.parseFile("/nonexistent/path/snippets.json");
    
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.errorMessage.find("Failed to open file") != std::string::npos);
}

TEST(SnippetParserTest, ToJsonSingleSnippet) {
    SnippetParser parser;
    Snippet snippet("log", "console.log($1);", "Log message");
    
    std::string json = parser.toJson(snippet);
    
    EXPECT_TRUE(json.find("\"log\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"prefix\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"body\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"description\"") != std::string::npos);
}

TEST(SnippetParserTest, ToJsonMultipleSnippets) {
    SnippetParser parser;
    std::vector<Snippet> snippets;
    snippets.emplace_back("log", "console.log($1);", "Log message");
    snippets.emplace_back("for", "for(;;)", "For loop");
    
    std::string json = parser.toJson(snippets);
    
    EXPECT_TRUE(json.find("\"log\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"for\"") != std::string::npos);
}

TEST(SnippetParserTest, ToJsonEmptyVector) {
    SnippetParser parser;
    std::vector<Snippet> snippets;
    
    std::string json = parser.toJson(snippets);
    
    EXPECT_EQ(json, "{\n}");
}
