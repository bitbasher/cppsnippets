/**
 * @file snippet_manager.h
 * @brief Template management and storage functionality
 */

#pragma once

#include "export.hpp"
#include "../resourceInventory/resourceItem.hpp"
#include <QString>
#include <QList>
#include <optional>

using resourceInventory::ResourceTemplate;

namespace scadtemplates {

/**
 * @brief Manages a collection of templates
 * 
 * The TemplateManager provides functionality for storing, retrieving,
 * searching, and organizing templates.
 */
class SCADTEMPLATES_API TemplateManager {
public:
    /**
     * @brief Default constructor
     */
    TemplateManager() = default;

    /**
     * @brief Add a template to the manager
     * @param tmpl The template to add
     * @return true if the template was added successfully
     */
    bool addTemplate(const ResourceTemplate& tmpl);

    /**
     * @brief Remove a template by prefix
     * @param prefix The prefix of the template to remove
     * @return true if a template was removed
     */
    bool removeTemplate(const QString& prefix);

    /**
     * @brief Find a template by prefix
     * @param prefix The prefix to search for
     * @return Optional containing the template if found
     */
    std::optional<ResourceTemplate> findByPrefix(const QString& prefix) const;

    /**
     * @brief Find templates matching a scope
     * @param scope The scope to filter by
     * @return List of matching templates
     */
    QList<ResourceTemplate> findByScope(const QString& scope) const;

    /**
     * @brief Search templates by keyword in prefix or description
     * @param keyword The search keyword
     * @return List of matching templates
     */
    QList<ResourceTemplate> search(const QString& keyword) const;

    /**
     * @brief Get all templates
     * @return List of all templates
     */
    QList<ResourceTemplate> getAllTemplates() const;

    /**
     * @brief Get the number of templates
     * @return Number of templates in the manager
     */
    size_t count() const;

    /**
     * @brief Clear all templates
     */
    void clear();

    /**
     * @brief Load templates from a file
     * @param filePath Path to the template file
     * @return true if loading was successful
     */
    bool loadFromFile(const QString& filePath);

    /**
     * @brief Save templates to a file
     * @param filePath Path to save the templates
     * @return true if saving was successful
     */
    bool saveToFile(const QString& filePath) const;

private:
    QList<ResourceTemplate> m_templates;
};

} // namespace scadtemplates
