/**
 * @file test_snippet_manager.cpp
 * @brief Unit tests for the SnippetManager class
 */

#include <gtest/gtest.h>
#include <cppsnippets/snippet_manager.h>

using namespace cppsnippets;

class SnippetManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<SnippetManager>();
    }
    
    std::unique_ptr<SnippetManager> manager;
};

TEST_F(SnippetManagerTest, InitiallyEmpty) {
    EXPECT_EQ(manager->count(), 0);
    EXPECT_TRUE(manager->getAllSnippets().empty());
}

TEST_F(SnippetManagerTest, AddSnippet) {
    Snippet snippet("log", "console.log($1);", "Log message");
    EXPECT_TRUE(manager->addSnippet(snippet));
    EXPECT_EQ(manager->count(), 1);
}

TEST_F(SnippetManagerTest, AddInvalidSnippet) {
    Snippet invalidSnippet;
    EXPECT_FALSE(manager->addSnippet(invalidSnippet));
    EXPECT_EQ(manager->count(), 0);
}

TEST_F(SnippetManagerTest, AddDuplicateSnippet) {
    Snippet snippet1("log", "console.log($1);", "Log message");
    Snippet snippet2("log", "console.log('updated');", "Updated log");
    
    manager->addSnippet(snippet1);
    manager->addSnippet(snippet2);
    
    EXPECT_EQ(manager->count(), 1);
    
    auto found = manager->findByPrefix("log");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->getBody(), "console.log('updated');");
}

TEST_F(SnippetManagerTest, RemoveSnippet) {
    Snippet snippet("log", "console.log($1);", "Log message");
    manager->addSnippet(snippet);
    
    EXPECT_TRUE(manager->removeSnippet("log"));
    EXPECT_EQ(manager->count(), 0);
}

TEST_F(SnippetManagerTest, RemoveNonExistentSnippet) {
    EXPECT_FALSE(manager->removeSnippet("nonexistent"));
}

TEST_F(SnippetManagerTest, FindByPrefix) {
    Snippet snippet("log", "console.log($1);", "Log message");
    manager->addSnippet(snippet);
    
    auto found = manager->findByPrefix("log");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->getPrefix(), "log");
    EXPECT_EQ(found->getBody(), "console.log($1);");
}

TEST_F(SnippetManagerTest, FindByPrefixNotFound) {
    auto found = manager->findByPrefix("nonexistent");
    EXPECT_FALSE(found.has_value());
}

TEST_F(SnippetManagerTest, FindByScope) {
    Snippet snippet1("log", "console.log($1);", "Log message");
    snippet1.addScope("javascript");
    
    Snippet snippet2("for", "for(;;)", "For loop");
    snippet2.addScope("cpp");
    
    Snippet snippet3("while", "while(true)", "While loop");
    snippet3.addScope("javascript");
    
    manager->addSnippet(snippet1);
    manager->addSnippet(snippet2);
    manager->addSnippet(snippet3);
    
    auto jsSnippets = manager->findByScope("javascript");
    EXPECT_EQ(jsSnippets.size(), 2);
    
    auto cppSnippets = manager->findByScope("cpp");
    EXPECT_EQ(cppSnippets.size(), 1);
}

TEST_F(SnippetManagerTest, Search) {
    manager->addSnippet(Snippet("console_log", "console.log($1);", "Log to console"));
    manager->addSnippet(Snippet("console_warn", "console.warn($1);", "Warning to console"));
    manager->addSnippet(Snippet("for", "for(;;)", "For loop"));
    
    auto results = manager->search("console");
    EXPECT_EQ(results.size(), 2);
    
    auto forResults = manager->search("loop");
    EXPECT_EQ(forResults.size(), 1);
}

TEST_F(SnippetManagerTest, Clear) {
    manager->addSnippet(Snippet("log", "console.log($1);", "Log"));
    manager->addSnippet(Snippet("for", "for(;;)", "For loop"));
    
    EXPECT_EQ(manager->count(), 2);
    
    manager->clear();
    EXPECT_EQ(manager->count(), 0);
}

TEST_F(SnippetManagerTest, GetAllSnippets) {
    manager->addSnippet(Snippet("log", "console.log($1);", "Log"));
    manager->addSnippet(Snippet("for", "for(;;)", "For loop"));
    
    auto snippets = manager->getAllSnippets();
    EXPECT_EQ(snippets.size(), 2);
}
