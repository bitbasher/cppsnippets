/**
 * @file snippet_manager.h
 * @brief Template management and storage functionality
 */

#pragma once

#include "export.hpp"
#include "template.hpp"
#include <string>
#include <vector>
#include <memory>
#include <optional>

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
    TemplateManager();

    /**
     * @brief Destructor
     */
    ~TemplateManager();

    /**
     * @brief Add a template to the manager
     * @param tmpl The template to add
     * @return true if the template was added successfully
     */
    bool addTemplate(const Template& tmpl);

    /**
     * @brief Remove a template by prefix
     * @param prefix The prefix of the template to remove
     * @return true if a template was removed
     */
    bool removeTemplate(const std::string& prefix);

    /**
     * @brief Find a template by prefix
     * @param prefix The prefix to search for
     * @return Optional containing the template if found
     */
    std::optional<Template> findByPrefix(const std::string& prefix) const;

    /**
     * @brief Find templates matching a scope
     * @param scope The scope to filter by
     * @return Vector of matching templates
     */
    std::vector<Template> findByScope(const std::string& scope) const;

    /**
     * @brief Search templates by keyword in prefix or description
     * @param keyword The search keyword
     * @return Vector of matching templates
     */
    std::vector<Template> search(const std::string& keyword) const;

    /**
     * @brief Get all templates
     * @return Vector of all templates
     */
    std::vector<Template> getAllTemplates() const;

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
    bool loadFromFile(const std::string& filePath);

    /**
     * @brief Save templates to a file
     * @param filePath Path to save the templates
     * @return true if saving was successful
     */
    bool saveToFile(const std::string& filePath) const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace scadtemplates
