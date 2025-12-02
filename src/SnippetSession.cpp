// SnippetSession.cpp
#include "cppsnippets/SnippetSession.h"
#include <QScintilla/qscintilla.h> // Adjust include as needed for your build
#include <regex>

namespace cppsnippets {

SnippetSession::SnippetSession(QScintillaEditor* editor, const Snippet& snippet)
    : m_editor(editor), m_snippet(snippet), m_currentIndex(0) {
    parsePlaceholders();
}

void SnippetSession::insert() {
    // Replace current selection with snippet body
    if (!m_editor) return;
    m_editor->replaceSelectedText(QString::fromStdString(m_snippet.getBody()));
    // Move cursor to first placeholder if any
    if (!m_placeholders.empty()) {
        const auto& ph = m_placeholders[0];
        m_editor->setSelection(ph.start, ph.end);
        m_currentIndex = 0;
    }
}

void SnippetSession::nextPlaceholder() {
    if (m_currentIndex + 1 < static_cast<int>(m_placeholders.size())) {
        ++m_currentIndex;
        const auto& ph = m_placeholders[m_currentIndex];
        m_editor->setSelection(ph.start, ph.end);
    }
}

void SnippetSession::prevPlaceholder() {
    if (m_currentIndex > 0) {
        --m_currentIndex;
        const auto& ph = m_placeholders[m_currentIndex];
        m_editor->setSelection(ph.start, ph.end);
    }
}

void SnippetSession::cancel() {
    // Optionally restore original text or clear selection
    m_placeholders.clear();
    m_currentIndex = 0;
}

void SnippetSession::merge(const Snippet& snippet) {
    // For simplicity, just replace the body and re-parse placeholders
    m_snippet = snippet;
    parsePlaceholders();
    insert();
}

bool SnippetSession::isAtLastPlaceholder() const {
    return m_currentIndex == static_cast<int>(m_placeholders.size()) - 1;
}

bool SnippetSession::isAtFirstPlaceholder() const {
    return m_currentIndex == 0;
}

int SnippetSession::getCurrentPlaceholderIndex() const {
    return m_currentIndex;
}

std::vector<Placeholder> SnippetSession::getAllPlaceholders() const {
    return m_placeholders;
}

void SnippetSession::parsePlaceholders() {
    m_placeholders.clear();
    const std::string& body = m_snippet.getBody();
    std::regex re(R"(\$(\d+)(?::([^}]+))?)");
    auto begin = std::sregex_iterator(body.begin(), body.end(), re);
    auto end = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
        int idx = std::stoi((*it)[1]);
        std::string def = (*it)[2];
        int start = static_cast<int>(it->position());
        int matchLen = static_cast<int>(it->length());
        m_placeholders.push_back({idx, start, start + matchLen, def});
    }
}

} // namespace cppsnippets
