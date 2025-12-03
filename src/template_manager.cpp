/**
 * @file snippet_manager.cpp
 * @brief SnippetManager class implementation
 */

#include "cppsnippets/snippet_manager.h"
#include "cppsnippets/snippet_parser.h"
#include <algorithm>
#include <fstream>

namespace cppsnippets {

class SnippetManager::Impl {
public:
    std::vector<Snippet> snippets;
};

SnippetManager::SnippetManager()
    : m_impl(std::make_unique<Impl>()) {
}

SnippetManager::~SnippetManager() = default;

bool SnippetManager::addSnippet(const Snippet& snippet) {
    if (!snippet.isValid()) {
        return false;
    }
    
    // Check for duplicate prefix
    auto it = std::find_if(m_impl->snippets.begin(), m_impl->snippets.end(),
        [&snippet](const Snippet& s) {
            return s.getPrefix() == snippet.getPrefix();
        });
    
    if (it != m_impl->snippets.end()) {
        // Update existing snippet
        *it = snippet;
    } else {
        m_impl->snippets.push_back(snippet);
    }
    
    return true;
}

bool SnippetManager::removeSnippet(const std::string& prefix) {
    auto it = std::find_if(m_impl->snippets.begin(), m_impl->snippets.end(),
        [&prefix](const Snippet& s) {
            return s.getPrefix() == prefix;
        });
    
    if (it != m_impl->snippets.end()) {
        m_impl->snippets.erase(it);
        return true;
    }
    
    return false;
}

std::optional<Snippet> SnippetManager::findByPrefix(const std::string& prefix) const {
    auto it = std::find_if(m_impl->snippets.begin(), m_impl->snippets.end(),
        [&prefix](const Snippet& s) {
            return s.getPrefix() == prefix;
        });
    
    if (it != m_impl->snippets.end()) {
        return *it;
    }
    
    return std::nullopt;
}

std::vector<Snippet> SnippetManager::findByScope(const std::string& scope) const {
    std::vector<Snippet> result;
    
    for (const auto& snippet : m_impl->snippets) {
        const auto& scopes = snippet.getScopes();
        if (std::find(scopes.begin(), scopes.end(), scope) != scopes.end()) {
            result.push_back(snippet);
        }
    }
    
    return result;
}

std::vector<Snippet> SnippetManager::search(const std::string& keyword) const {
    std::vector<Snippet> result;
    
    for (const auto& snippet : m_impl->snippets) {
        if (snippet.getPrefix().find(keyword) != std::string::npos ||
            snippet.getDescription().find(keyword) != std::string::npos) {
            result.push_back(snippet);
        }
    }
    
    return result;
}

std::vector<Snippet> SnippetManager::getAllSnippets() const {
    return m_impl->snippets;
}

size_t SnippetManager::count() const {
    return m_impl->snippets.size();
}

void SnippetManager::clear() {
    m_impl->snippets.clear();
}

bool SnippetManager::loadFromFile(const std::string& filePath) {
    SnippetParser parser;
    auto result = parser.parseFile(filePath);
    
    if (result.success) {
        for (const auto& snippet : result.snippets) {
            addSnippet(snippet);
        }
    }
    
    return result.success;
}

bool SnippetManager::saveToFile(const std::string& filePath) const {
    SnippetParser parser;
    std::string json = parser.toJson(m_impl->snippets);
    
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    file << json;
    return file.good();
}

} // namespace cppsnippets
