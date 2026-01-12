/**
 * @file snippet_manager.cpp
 * @brief TemplateManager class implementation
 */

#include "scadtemplates/template_manager.hpp"
#include "scadtemplates/template_parser.hpp"
#include <algorithm>
#include <QFile>

namespace scadtemplates {

bool TemplateManager::addTemplate(const ResourceTemplate& tmpl) {
    if (!tmpl.isValid()) {
        return false;
    }
    
    // Check for duplicate prefix
    auto it = std::find_if(m_templates.begin(), m_templates.end(),
        [&tmpl](const ResourceTemplate& t) {
            return t.prefix() == tmpl.prefix();
        });
    
    if (it != m_templates.end()) {
        // Update existing template
        *it = tmpl;
    } else {
        m_templates.append(tmpl);
    }
    
    return true;
}

bool TemplateManager::removeTemplate(const QString& prefix) {
    auto it = std::find_if(m_templates.begin(), m_templates.end(),
        [&prefix](const ResourceTemplate& t) {
            return t.prefix() == prefix;
        });
    
    if (it != m_templates.end()) {
        m_templates.erase(it);
        return true;
    }
    
    return false;
}

std::optional<ResourceTemplate> TemplateManager::findByPrefix(const QString& prefix) const {
    auto it = std::find_if(m_templates.begin(), m_templates.end(),
        [&prefix](const ResourceTemplate& t) {
            return t.prefix() == prefix;
        });
    
    if (it != m_templates.end()) {
        return *it;
    }
    
    return std::nullopt;
}

QList<ResourceTemplate> TemplateManager::findByScope(const QString& scope) const {
    QList<ResourceTemplate> result;
    
    for (const auto& tmpl : m_templates) {
        const auto& scopes = tmpl.scopes();
        if (scopes.contains(scope)) {
            result.append(tmpl);
        }
    }
    
    return result;
}

QList<ResourceTemplate> TemplateManager::search(const QString& keyword) const {
    QList<ResourceTemplate> result;
    
    for (const auto& tmpl : m_templates) {
        if (tmpl.prefix().contains(keyword, Qt::CaseInsensitive) ||
            tmpl.description().contains(keyword, Qt::CaseInsensitive)) {
            result.append(tmpl);
        }
    }
    
    return result;
}

QList<ResourceTemplate> TemplateManager::getAllTemplates() const {
    return m_templates;
}

size_t TemplateManager::count() const {
    return m_templates.size();
}

void TemplateManager::clear() {
    m_templates.clear();
}

bool TemplateManager::loadFromFile(const QString& filePath) {
    TemplateParser parser;
    auto result = parser.parseFile(filePath);
    
    if (result.success) {
        for (const auto& tmpl : result.templates) {
            addTemplate(tmpl);
        }
    }
    
    return result.success;
}

bool TemplateManager::saveToFile(const QString& filePath) const {
    TemplateParser parser;
    QString json = parser.toJson(m_templates);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    file.write(json.toUtf8());
    return file.error() == QFile::NoError;
}

} // namespace scadtemplates
