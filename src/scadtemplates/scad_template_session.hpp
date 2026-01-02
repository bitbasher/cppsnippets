// TemplateSession.h
#pragma once

#include <vector>
#include <string>
#include <memory>
#include "template.hpp"

#ifdef HAS_QSCINTILLA
#include <Qsci/qsciscintilla.h>
using EditorWidget = QsciScintilla;
#else
// Stub for builds without QScintilla
class EditorWidget {
public:
    void replaceSelectedText(const QString&) {}
    void setSelection(int, int, int, int) {}
};
#endif

namespace scadtemplates {

struct Placeholder {
    int index; // Placeholder number (e.g., 1 for $1)
    int start; // Start position in the text
    int end;   // End position in the text
    std::string defaultValue;
};

class TemplateSession {
public:
    TemplateSession(EditorWidget* editor, const Template& tmpl);
    void insert();
    void nextPlaceholder();
    void prevPlaceholder();
    void cancel();
    void merge(const Template& tmpl);
    bool isAtLastPlaceholder() const;
    bool isAtFirstPlaceholder() const;
    int getCurrentPlaceholderIndex() const;
    std::vector<Placeholder> getAllPlaceholders() const;
private:
    EditorWidget* m_editor;
    Template m_template;
    std::vector<Placeholder> m_placeholders;
    int m_currentIndex;
    void parsePlaceholders();
};

} // namespace scadtemplates
