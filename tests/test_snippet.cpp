/**
 * @file test_snippet.cpp
 * @brief Unit tests for the Snippet class
 */

#include <gtest/gtest.h>
#include <cppsnippets/snippet.h>

using namespace cppsnippets;

TEST(SnippetTest, DefaultConstructor) {
    Snippet snippet;
    EXPECT_TRUE(snippet.getPrefix().empty());
    EXPECT_TRUE(snippet.getBody().empty());
    EXPECT_TRUE(snippet.getDescription().empty());
    EXPECT_TRUE(snippet.getScopes().empty());
    EXPECT_FALSE(snippet.isValid());
}

TEST(SnippetTest, ParameterizedConstructor) {
    Snippet snippet("log", "console.log($1);", "Log to console");
    EXPECT_EQ(snippet.getPrefix(), "log");
    EXPECT_EQ(snippet.getBody(), "console.log($1);");
    EXPECT_EQ(snippet.getDescription(), "Log to console");
    EXPECT_TRUE(snippet.isValid());
}

TEST(SnippetTest, ConstructorWithEmptyDescription) {
    Snippet snippet("for", "for(int i = 0; i < $1; i++) {\n\t$2\n}");
    EXPECT_EQ(snippet.getPrefix(), "for");
    EXPECT_TRUE(snippet.getDescription().empty());
    EXPECT_TRUE(snippet.isValid());
}

TEST(SnippetTest, SetPrefix) {
    Snippet snippet;
    snippet.setPrefix("test");
    EXPECT_EQ(snippet.getPrefix(), "test");
}

TEST(SnippetTest, SetBody) {
    Snippet snippet;
    snippet.setBody("test body");
    EXPECT_EQ(snippet.getBody(), "test body");
}

TEST(SnippetTest, SetDescription) {
    Snippet snippet;
    snippet.setDescription("test description");
    EXPECT_EQ(snippet.getDescription(), "test description");
}

TEST(SnippetTest, AddScope) {
    Snippet snippet("test", "body");
    snippet.addScope("cpp");
    snippet.addScope("c");
    
    const auto& scopes = snippet.getScopes();
    EXPECT_EQ(scopes.size(), 2);
    EXPECT_EQ(scopes[0], "cpp");
    EXPECT_EQ(scopes[1], "c");
}

TEST(SnippetTest, ClearScopes) {
    Snippet snippet("test", "body");
    snippet.addScope("cpp");
    snippet.addScope("c");
    snippet.clearScopes();
    
    EXPECT_TRUE(snippet.getScopes().empty());
}

TEST(SnippetTest, IsValidWithPrefixOnly) {
    Snippet snippet;
    snippet.setPrefix("test");
    EXPECT_FALSE(snippet.isValid());
}

TEST(SnippetTest, IsValidWithBodyOnly) {
    Snippet snippet;
    snippet.setBody("test body");
    EXPECT_FALSE(snippet.isValid());
}

TEST(SnippetTest, IsValidWithBoth) {
    Snippet snippet;
    snippet.setPrefix("test");
    snippet.setBody("test body");
    EXPECT_TRUE(snippet.isValid());
}
