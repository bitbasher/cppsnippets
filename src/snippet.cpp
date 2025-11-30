/**
 * @file snippet.cpp
 * @brief Snippet class implementation
 */

#include "cppsnippets/snippet.h"

namespace cppsnippets {

Snippet::Snippet(const std::string& prefix, const std::string& body, 
                 const std::string& description)
    : m_prefix(prefix)
    , m_body(body)
    , m_description(description) {
}

const std::string& Snippet::getPrefix() const {
    return m_prefix;
}

void Snippet::setPrefix(const std::string& prefix) {
    m_prefix = prefix;
}

const std::string& Snippet::getBody() const {
    return m_body;
}

void Snippet::setBody(const std::string& body) {
    m_body = body;
}

const std::string& Snippet::getDescription() const {
    return m_description;
}

void Snippet::setDescription(const std::string& description) {
    m_description = description;
}

const std::vector<std::string>& Snippet::getScopes() const {
    return m_scopes;
}

void Snippet::addScope(const std::string& scope) {
    m_scopes.push_back(scope);
}

void Snippet::clearScopes() {
    m_scopes.clear();
}

bool Snippet::isValid() const {
    return !m_prefix.empty() && !m_body.empty();
}

} // namespace cppsnippets
