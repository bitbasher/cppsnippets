/**
 * @file test_snippet.cpp
 * @brief Unit tests for the Template class
 */

#include <gtest/gtest.h>
#include <scadtemplates/template.h>

using namespace scadtemplates;

TEST(TemplateTest, DefaultConstructor) {
    Template tmpl;
    EXPECT_TRUE(tmpl.getPrefix().empty());
    EXPECT_TRUE(tmpl.getBody().empty());
    EXPECT_TRUE(tmpl.getDescription().empty());
    EXPECT_TRUE(tmpl.getScopes().empty());
    EXPECT_FALSE(tmpl.isValid());
}

TEST(TemplateTest, ParameterizedConstructor) {
    Template tmpl("log", "console.log($1);", "Log to console");
    EXPECT_EQ(tmpl.getPrefix(), "log");
    EXPECT_EQ(tmpl.getBody(), "console.log($1);");
    EXPECT_EQ(tmpl.getDescription(), "Log to console");
    EXPECT_TRUE(tmpl.isValid());
}

TEST(TemplateTest, ConstructorWithEmptyDescription) {
    Template tmpl("for", "for(int i = 0; i < $1; i++) {\n\t$2\n}");
    EXPECT_EQ(tmpl.getPrefix(), "for");
    EXPECT_TRUE(tmpl.getDescription().empty());
    EXPECT_TRUE(tmpl.isValid());
}

TEST(TemplateTest, SetPrefix) {
    Template tmpl;
    tmpl.setPrefix("test");
    EXPECT_EQ(tmpl.getPrefix(), "test");
}

TEST(TemplateTest, SetBody) {
    Template tmpl;
    tmpl.setBody("test body");
    EXPECT_EQ(tmpl.getBody(), "test body");
}

TEST(TemplateTest, SetDescription) {
    Template tmpl;
    tmpl.setDescription("test description");
    EXPECT_EQ(tmpl.getDescription(), "test description");
}

TEST(TemplateTest, AddScope) {
    Template tmpl("test", "body");
    tmpl.addScope("cpp");
    tmpl.addScope("c");
    
    const auto& scopes = tmpl.getScopes();
    EXPECT_EQ(scopes.size(), 2);
    EXPECT_EQ(scopes[0], "cpp");
    EXPECT_EQ(scopes[1], "c");
}

TEST(TemplateTest, ClearScopes) {
    Template tmpl("test", "body");
    tmpl.addScope("cpp");
    tmpl.addScope("c");
    tmpl.clearScopes();
    
    EXPECT_TRUE(tmpl.getScopes().empty());
}

TEST(TemplateTest, IsValidWithPrefixOnly) {
    Template tmpl;
    tmpl.setPrefix("test");
    EXPECT_FALSE(tmpl.isValid());
}

TEST(TemplateTest, IsValidWithBodyOnly) {
    Template tmpl;
    tmpl.setBody("test body");
    EXPECT_FALSE(tmpl.isValid());
}

TEST(TemplateTest, IsValidWithBoth) {
    Template tmpl;
    tmpl.setPrefix("test");
    tmpl.setBody("test body");
    EXPECT_TRUE(tmpl.isValid());
}
