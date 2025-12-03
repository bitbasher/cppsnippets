/**
 * @file test_snippet_manager.cpp
 * @brief Unit tests for the TemplateManager class
 */

#include <gtest/gtest.h>
#include <scadtemplates/template_manager.h>

using namespace scadtemplates;

class TemplateManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<TemplateManager>();
    }
    
    std::unique_ptr<TemplateManager> manager;
};

TEST_F(TemplateManagerTest, InitiallyEmpty) {
    EXPECT_EQ(manager->count(), 0);
    EXPECT_TRUE(manager->getAllTemplates().empty());
}

TEST_F(TemplateManagerTest, AddTemplate) {
    Template tmpl("log", "console.log($1);", "Log message");
    EXPECT_TRUE(manager->addTemplate(tmpl));
    EXPECT_EQ(manager->count(), 1);
}

TEST_F(TemplateManagerTest, AddInvalidTemplate) {
    Template invalidTemplate;
    EXPECT_FALSE(manager->addTemplate(invalidTemplate));
    EXPECT_EQ(manager->count(), 0);
}

TEST_F(TemplateManagerTest, AddDuplicateTemplate) {
    Template tmpl1("log", "console.log($1);", "Log message");
    Template tmpl2("log", "console.log('updated');", "Updated log");
    
    manager->addTemplate(tmpl1);
    manager->addTemplate(tmpl2);
    
    EXPECT_EQ(manager->count(), 1);
    
    auto found = manager->findByPrefix("log");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->getBody(), "console.log('updated');");
}

TEST_F(TemplateManagerTest, RemoveTemplate) {
    Template tmpl("log", "console.log($1);", "Log message");
    manager->addTemplate(tmpl);
    
    EXPECT_TRUE(manager->removeTemplate("log"));
    EXPECT_EQ(manager->count(), 0);
}

TEST_F(TemplateManagerTest, RemoveNonExistentTemplate) {
    EXPECT_FALSE(manager->removeTemplate("nonexistent"));
}

TEST_F(TemplateManagerTest, FindByPrefix) {
    Template tmpl("log", "console.log($1);", "Log message");
    manager->addTemplate(tmpl);
    
    auto found = manager->findByPrefix("log");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->getPrefix(), "log");
    EXPECT_EQ(found->getBody(), "console.log($1);");
}

TEST_F(TemplateManagerTest, FindByPrefixNotFound) {
    auto found = manager->findByPrefix("nonexistent");
    EXPECT_FALSE(found.has_value());
}

TEST_F(TemplateManagerTest, FindByScope) {
    Template tmpl1("log", "console.log($1);", "Log message");
    tmpl1.addScope("javascript");
    
    Template tmpl2("for", "for(;;)", "For loop");
    tmpl2.addScope("cpp");
    
    Template tmpl3("while", "while(true)", "While loop");
    tmpl3.addScope("javascript");
    
    manager->addTemplate(tmpl1);
    manager->addTemplate(tmpl2);
    manager->addTemplate(tmpl3);
    
    auto jsTemplates = manager->findByScope("javascript");
    EXPECT_EQ(jsTemplates.size(), 2);
    
    auto cppTemplates = manager->findByScope("cpp");
    EXPECT_EQ(cppTemplates.size(), 1);
}

TEST_F(TemplateManagerTest, Search) {
    manager->addTemplate(Template("console_log", "console.log($1);", "Log to console"));
    manager->addTemplate(Template("console_warn", "console.warn($1);", "Warning to console"));
    manager->addTemplate(Template("for", "for(;;)", "For loop"));
    
    auto results = manager->search("console");
    EXPECT_EQ(results.size(), 2);
    
    auto forResults = manager->search("loop");
    EXPECT_EQ(forResults.size(), 1);
}

TEST_F(TemplateManagerTest, Clear) {
    manager->addTemplate(Template("log", "console.log($1);", "Log"));
    manager->addTemplate(Template("for", "for(;;)", "For loop"));
    
    EXPECT_EQ(manager->count(), 2);
    
    manager->clear();
    EXPECT_EQ(manager->count(), 0);
}

TEST_F(TemplateManagerTest, GetAllTemplates) {
    manager->addTemplate(Template("log", "console.log($1);", "Log"));
    manager->addTemplate(Template("for", "for(;;)", "For loop"));
    
    auto templates = manager->getAllTemplates();
    EXPECT_EQ(templates.size(), 2);
}
