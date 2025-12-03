// TemplateSession.cpp
#include "scadtemplates/scad_template_session.h"
#include <regex>

#ifdef HAS_QSCINTILLA
#include <QString>
#endif

namespace scadtemplates {

TemplateSession::TemplateSession(EditorWidget* editor, const Template& tmpl)
    : m_editor(editor), m_template(tmpl), m_currentIndex(0) {
    parsePlaceholders();
}

void TemplateSession::insert() {
    // Replace current selection with template body
    if (!m_editor) return;
#ifdef HAS_QSCINTILLA
    m_editor->replaceSelectedText(QString::fromStdString(m_template.getBody()));
    // Move cursor to first placeholder if any
    if (!m_placeholders.empty()) {
        const auto& ph = m_placeholders[0];
        // QScintilla uses (line, index) pairs for selection
        // For simplicity, treat as single-line positions
        m_editor->setSelection(0, ph.start, 0, ph.end);
        m_currentIndex = 0;
    }
#endif
}

void TemplateSession::nextPlaceholder() {
    if (m_currentIndex + 1 < static_cast<int>(m_placeholders.size())) {
        ++m_currentIndex;
#ifdef HAS_QSCINTILLA
        const auto& ph = m_placeholders[m_currentIndex];
        m_editor->setSelection(0, ph.start, 0, ph.end);
#endif
    }
}

void TemplateSession::prevPlaceholder() {
    if (m_currentIndex > 0) {
        --m_currentIndex;
#ifdef HAS_QSCINTILLA
        const auto& ph = m_placeholders[m_currentIndex];
        m_editor->setSelection(0, ph.start, 0, ph.end);
#endif
    }
}

void TemplateSession::cancel() {
    // Optionally restore original text or clear selection
    m_placeholders.clear();
    m_currentIndex = 0;
}

void TemplateSession::merge(const Template& tmpl) {
    // For simplicity, just replace the body and re-parse placeholders
    m_template = tmpl;
    parsePlaceholders();
    insert();
}

bool TemplateSession::isAtLastPlaceholder() const {
    return m_currentIndex == static_cast<int>(m_placeholders.size()) - 1;
}

bool TemplateSession::isAtFirstPlaceholder() const {
    return m_currentIndex == 0;
}

int TemplateSession::getCurrentPlaceholderIndex() const {
    return m_currentIndex;
}

std::vector<Placeholder> TemplateSession::getAllPlaceholders() const {
    return m_placeholders;
}

void TemplateSession::parsePlaceholders() {
    m_placeholders.clear();
    const std::string& body = m_template.getBody();
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

} // namespace scadtemplates
