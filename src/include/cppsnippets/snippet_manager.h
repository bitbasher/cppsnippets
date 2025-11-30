/**
 * @file snippet_manager.h
 * @brief Snippet management and storage functionality
 */

#ifndef CPPSNIPPETS_SNIPPET_MANAGER_H
#define CPPSNIPPETS_SNIPPET_MANAGER_H

#include "export.h"
#include "snippet.h"
#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace cppsnippets {

/**
 * @brief Manages a collection of snippets
 * 
 * The SnippetManager provides functionality for storing, retrieving,
 * searching, and organizing snippets.
 */
class CPPSNIPPETS_API SnippetManager {
public:
    /**
     * @brief Default constructor
     */
    SnippetManager();

    /**
     * @brief Destructor
     */
    ~SnippetManager();

    /**
     * @brief Add a snippet to the manager
     * @param snippet The snippet to add
     * @return true if the snippet was added successfully
     */
    bool addSnippet(const Snippet& snippet);

    /**
     * @brief Remove a snippet by prefix
     * @param prefix The prefix of the snippet to remove
     * @return true if a snippet was removed
     */
    bool removeSnippet(const std::string& prefix);

    /**
     * @brief Find a snippet by prefix
     * @param prefix The prefix to search for
     * @return Optional containing the snippet if found
     */
    std::optional<Snippet> findByPrefix(const std::string& prefix) const;

    /**
     * @brief Find snippets matching a scope
     * @param scope The scope to filter by
     * @return Vector of matching snippets
     */
    std::vector<Snippet> findByScope(const std::string& scope) const;

    /**
     * @brief Search snippets by keyword in prefix or description
     * @param keyword The search keyword
     * @return Vector of matching snippets
     */
    std::vector<Snippet> search(const std::string& keyword) const;

    /**
     * @brief Get all snippets
     * @return Vector of all snippets
     */
    std::vector<Snippet> getAllSnippets() const;

    /**
     * @brief Get the number of snippets
     * @return Number of snippets in the manager
     */
    size_t count() const;

    /**
     * @brief Clear all snippets
     */
    void clear();

    /**
     * @brief Load snippets from a file
     * @param filePath Path to the snippet file
     * @return true if loading was successful
     */
    bool loadFromFile(const std::string& filePath);

    /**
     * @brief Save snippets to a file
     * @param filePath Path to save the snippets
     * @return true if saving was successful
     */
    bool saveToFile(const std::string& filePath) const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace cppsnippets

#endif // CPPSNIPPETS_SNIPPET_MANAGER_H
