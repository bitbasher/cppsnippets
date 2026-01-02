/**
 * @file snippet.cpp
 * @brief Template class implementation
 */

#include "scadtemplates/template.hpp"

namespace scadtemplates {

Template::Template(const std::string& prefix, const std::string& body, 
                 const std::string& description)
    : m_prefix(prefix)
    , m_body(body)
    , m_description(description) {
}

const std::string& Template::getPrefix() const {
    return m_prefix;
}

void Template::setPrefix(const std::string& prefix) {
    m_prefix = prefix;
}

const std::string& Template::getBody() const {
    return m_body;
}

void Template::setBody(const std::string& body) {
    m_body = body;
}

const std::string& Template::getDescription() const {
    return m_description;
}

void Template::setDescription(const std::string& description) {
    m_description = description;
}

const std::vector<std::string>& Template::getScopes() const {
    return m_scopes;
}

void Template::addScope(const std::string& scope) {
    m_scopes.push_back(scope);
}

void Template::clearScopes() {
    m_scopes.clear();
}

bool Template::isValid() const {
    return !m_prefix.empty() && !m_body.empty();
}

EditType Template::getType() const {
    return m_type;
}

void Template::setType(EditType type) {
    m_type = type;
}

EditSubtype Template::getSubtype() const {
    return m_subtype;
}

void Template::setSubtype(EditSubtype subtype) {
    m_subtype = subtype;
    m_type = typeFromSubtype(subtype);
}

} // namespace scadtemplates
