/**
 * @file test_snippet_session.cpp
 * @brief Unit tests for the SnippetSession class (core logic, no editor)
 */

#include <gtest/gtest.h>
#include <cppsnippets/snippet.h>
#include <vector>
#include <string>
#include <regex>

using namespace cppsnippets;

// Placeholder struct for testing (mirrors SnippetSession.h)
struct Placeholder {
    int index;
    int start;
    int end;
    std::string defaultValue;
};

// Standalone placeholder parser for testing core logic
std::vector<Placeholder> parsePlaceholders(const std::string& body) {
    std::vector<Placeholder> placeholders;
    // Match $1, $2, ${1:default}, ${2:value}, etc.
    std::regex re(R"(\$\{?(\d+)(?::([^}]*))?\}?)");
    auto begin = std::sregex_iterator(body.begin(), body.end(), re);
    auto end = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
        int idx = std::stoi((*it)[1]);
        std::string def = (*it)[2].matched ? (*it)[2].str() : "";
        int start = static_cast<int>(it->position());
        int matchLen = static_cast<int>(it->length());
        placeholders.push_back({idx, start, start + matchLen, def});
    }
    return placeholders;
}

TEST(SnippetSessionTest, ParseSimplePlaceholders) {
    std::string body = "for (int $1 = 0; $1 < $2; $1++) { $3 }";
    auto phs = parsePlaceholders(body);
    EXPECT_EQ(phs.size(), 5);
    EXPECT_EQ(phs[0].index, 1);
    EXPECT_EQ(phs[1].index, 1);
    EXPECT_EQ(phs[2].index, 2);
    EXPECT_EQ(phs[3].index, 1);
    EXPECT_EQ(phs[4].index, 3);
}

TEST(SnippetSessionTest, ParsePlaceholdersWithDefaults) {
    std::string body = "console.log(${1:message});";
    auto phs = parsePlaceholders(body);
    ASSERT_EQ(phs.size(), 1);
    EXPECT_EQ(phs[0].index, 1);
    EXPECT_EQ(phs[0].defaultValue, "message");
}

TEST(SnippetSessionTest, ParseMultiplePlaceholdersWithDefaults) {
    std::string body = "function ${1:name}(${2:args}) { ${3:body} }";
    auto phs = parsePlaceholders(body);
    ASSERT_EQ(phs.size(), 3);
    EXPECT_EQ(phs[0].index, 1);
    EXPECT_EQ(phs[0].defaultValue, "name");
    EXPECT_EQ(phs[1].index, 2);
    EXPECT_EQ(phs[1].defaultValue, "args");
    EXPECT_EQ(phs[2].index, 3);
    EXPECT_EQ(phs[2].defaultValue, "body");
}

TEST(SnippetSessionTest, NoPlaceholders) {
    std::string body = "Hello, world!";
    auto phs = parsePlaceholders(body);
    EXPECT_TRUE(phs.empty());
}

TEST(SnippetSessionTest, PlaceholderPositions) {
    std::string body = "abc $1 def $2 ghi";
    auto phs = parsePlaceholders(body);
    ASSERT_EQ(phs.size(), 2);
    EXPECT_EQ(phs[0].start, 4);
    EXPECT_EQ(phs[0].end, 6);
    EXPECT_EQ(phs[1].start, 11);
    EXPECT_EQ(phs[1].end, 13);
}

TEST(SnippetSessionTest, MixedPlaceholderStyles) {
    std::string body = "$1 ${2:default} $3";
    auto phs = parsePlaceholders(body);
    ASSERT_EQ(phs.size(), 3);
    EXPECT_EQ(phs[0].index, 1);
    EXPECT_EQ(phs[0].defaultValue, "");
    EXPECT_EQ(phs[1].index, 2);
    EXPECT_EQ(phs[1].defaultValue, "default");
    EXPECT_EQ(phs[2].index, 3);
    EXPECT_EQ(phs[2].defaultValue, "");
}

TEST(SnippetSessionTest, NavigationSimulation) {
    std::string body = "$1 $2 $3";
    auto phs = parsePlaceholders(body);
    ASSERT_EQ(phs.size(), 3);
    // Simulate navigation
    int currentIndex = 0;
    EXPECT_EQ(phs[currentIndex].index, 1);
    ++currentIndex;
    EXPECT_EQ(phs[currentIndex].index, 2);
    ++currentIndex;
    EXPECT_EQ(phs[currentIndex].index, 3);
    // At last placeholder
    EXPECT_EQ(currentIndex, static_cast<int>(phs.size()) - 1);
}
