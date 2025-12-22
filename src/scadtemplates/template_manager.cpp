/**
 * @file snippet_manager.cpp
 * @brief TemplateManager class implementation
 */

#include "scadtemplates/template_manager.h"
#include "scadtemplates/template_parser.h"
#include <algorithm>
#include <fstream>

namespace scadtemplates {

class TemplateManager::Impl {
public:
    std::vector<Template> templates;
};

TemplateManager::TemplateManager()
    : m_impl(std::make_unique<Impl>()) {
}

TemplateManager::~TemplateManager() = default;

bool TemplateManager::addTemplate(const Template& tmpl) {
    if (!tmpl.isValid()) {
        return false;
    }
    
    // Check for duplicate prefix
    auto it = std::find_if(m_impl->templates.begin(), m_impl->templates.end(),
        [&tmpl](const Template& t) {
            return t.getPrefix() == tmpl.getPrefix();
        });
    
    if (it != m_impl->templates.end()) {
        // Update existing template
        *it = tmpl;
    } else {
        m_impl->templates.push_back(tmpl);
    }
    
    return true;
}

bool TemplateManager::removeTemplate(const std::string& prefix) {
    auto it = std::find_if(m_impl->templates.begin(), m_impl->templates.end(),
        [&prefix](const Template& t) {
            return t.getPrefix() == prefix;
        });
    
    if (it != m_impl->templates.end()) {
        m_impl->templates.erase(it);
        return true;
    }
    
    return false;
}

std::optional<Template> TemplateManager::findByPrefix(const std::string& prefix) const {
    auto it = std::find_if(m_impl->templates.begin(), m_impl->templates.end(),
        [&prefix](const Template& t) {
            return t.getPrefix() == prefix;
        });
    
    if (it != m_impl->templates.end()) {
        return *it;
    }
    
    return std::nullopt;
}

std::vector<Template> TemplateManager::findByScope(const std::string& scope) const {
    std::vector<Template> result;
    
    for (const auto& tmpl : m_impl->templates) {
        const auto& scopes = tmpl.getScopes();
        if (std::find(scopes.begin(), scopes.end(), scope) != scopes.end()) {
            result.push_back(tmpl);
        }
    }
    
    return result;
}

std::vector<Template> TemplateManager::search(const std::string& keyword) const {
    std::vector<Template> result;
    
    for (const auto& tmpl : m_impl->templates) {
        if (tmpl.getPrefix().find(keyword) != std::string::npos ||
            tmpl.getDescription().find(keyword) != std::string::npos) {
            result.push_back(tmpl);
        }
    }
    
    return result;
}

std::vector<Template> TemplateManager::getAllTemplates() const {
    return m_impl->templates;
}

size_t TemplateManager::count() const {
    return m_impl->templates.size();
}

void TemplateManager::clear() {
    m_impl->templates.clear();
}

bool TemplateManager::loadFromFile(const std::string& filePath) {
    TemplateParser parser;
    auto result = parser.parseFile(filePath);
    
    if (result.success) {
        for (const auto& tmpl : result.templates) {
            addTemplate(tmpl);
        }
    }
    
    return result.success;
}

bool TemplateManager::saveToFile(const std::string& filePath) const {
    TemplateParser parser;
    std::string json = parser.toJson(m_impl->templates);
    
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    file << json;
    return file.good();
}

} // namespace scadtemplates
