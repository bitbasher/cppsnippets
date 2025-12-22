/**
 * @file snippet.h
 * @brief Core template data structure
 */

#pragma once

#include "export.h"
#include "edittype.h"
#include "editsubtype.h"
#include <string>
#include <vector>

namespace scadtemplates {

/**
 * @brief Represents a single code template
 * 
 * A Template contains a prefix (trigger), body (content), description,
 * and optional scope information for language-specific filtering.
 */
class SCADTEMPLATES_API Template {
public:
    /**
     * @brief Default constructor
     */
    Template() = default;

    /**
     * @brief Construct a template with prefix, body, and description
     * @param prefix The trigger text for the template
     * @param body The content of the template
     * @param description A human-readable description
     */
    Template(const std::string& prefix, const std::string& body, 
            const std::string& description = "");

    /**
     * @brief Get the template prefix (trigger)
     * @return The prefix string
     */
    const std::string& getPrefix() const;

    /**
     * @brief Set the template prefix
     * @param prefix The new prefix
     */
    void setPrefix(const std::string& prefix);

    /**
     * @brief Get the template body (content)
     * @return The body string
     */
    const std::string& getBody() const;

    /**
     * @brief Set the template body
     * @param body The new body content
     */
    void setBody(const std::string& body);

    /**
     * @brief Get the template description
     * @return The description string
     */
    const std::string& getDescription() const;

    /**
     * @brief Set the template description
     * @param description The new description
     */
    void setDescription(const std::string& description);

    /**
     * @brief Get the template scopes
     * @return Vector of scope strings
     */
    const std::vector<std::string>& getScopes() const;

    /**
     * @brief Add a scope to the template
     * @param scope The scope to add (e.g., "cpp", "python")
     */
    void addScope(const std::string& scope);

    /**
     * @brief Clear all scopes
     */
    void clearScopes();

    /**
     * @brief Check if template is valid
     * @return true if the template has at least a prefix and body
     */
    bool isValid() const;

    /**
     * @brief Get the template type
     * @return The EditType for this template
     */
    EditType getType() const;

    /**
     * @brief Set the template type
     * @param type The new type
     */
    void setType(EditType type);

    /**
     * @brief Get the template subtype
     * @return The EditSubtype for this template
     */
    EditSubtype getSubtype() const;

    /**
     * @brief Set the template subtype
     * @param subtype The new subtype
     * @note This also updates the type to match the subtype's parent type
     */
    void setSubtype(EditSubtype subtype);

private:
    std::string m_prefix;
    std::string m_body;
    std::string m_description;
    std::vector<std::string> m_scopes;
    EditType m_type = EditType::Text;
    EditSubtype m_subtype = EditSubtype::Txt;
};

} // namespace scadtemplates
