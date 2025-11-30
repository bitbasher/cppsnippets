/**
 * @file snippet.h
 * @brief Core snippet data structure
 */

#ifndef CPPSNIPPETS_SNIPPET_H
#define CPPSNIPPETS_SNIPPET_H

#include "export.h"
#include <string>
#include <vector>

namespace cppsnippets {

/**
 * @brief Represents a single code snippet
 * 
 * A Snippet contains a prefix (trigger), body (content), description,
 * and optional scope information for language-specific filtering.
 */
class CPPSNIPPETS_API Snippet {
public:
    /**
     * @brief Default constructor
     */
    Snippet() = default;

    /**
     * @brief Construct a snippet with prefix, body, and description
     * @param prefix The trigger text for the snippet
     * @param body The content of the snippet
     * @param description A human-readable description
     */
    Snippet(const std::string& prefix, const std::string& body, 
            const std::string& description = "");

    /**
     * @brief Get the snippet prefix (trigger)
     * @return The prefix string
     */
    const std::string& getPrefix() const;

    /**
     * @brief Set the snippet prefix
     * @param prefix The new prefix
     */
    void setPrefix(const std::string& prefix);

    /**
     * @brief Get the snippet body (content)
     * @return The body string
     */
    const std::string& getBody() const;

    /**
     * @brief Set the snippet body
     * @param body The new body content
     */
    void setBody(const std::string& body);

    /**
     * @brief Get the snippet description
     * @return The description string
     */
    const std::string& getDescription() const;

    /**
     * @brief Set the snippet description
     * @param description The new description
     */
    void setDescription(const std::string& description);

    /**
     * @brief Get the snippet scopes
     * @return Vector of scope strings
     */
    const std::vector<std::string>& getScopes() const;

    /**
     * @brief Add a scope to the snippet
     * @param scope The scope to add (e.g., "cpp", "python")
     */
    void addScope(const std::string& scope);

    /**
     * @brief Clear all scopes
     */
    void clearScopes();

    /**
     * @brief Check if snippet is valid
     * @return true if the snippet has at least a prefix and body
     */
    bool isValid() const;

private:
    std::string m_prefix;
    std::string m_body;
    std::string m_description;
    std::vector<std::string> m_scopes;
};

} // namespace cppsnippets

#endif // CPPSNIPPETS_SNIPPET_H
