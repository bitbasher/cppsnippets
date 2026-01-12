// TemplateSession.h
#pragma once

#include <QList>
#include <QString>
#include <memory>
#include "../resourceInventory/resourceItem.hpp"

using resourceInventory::ResourceTemplate;

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
    QString defaultValue;
};

class TemplateSession {
public:
    TemplateSession(EditorWidget* editor, const ResourceTemplate& tmpl);
    void insert();
    void nextPlaceholder();
    void prevPlaceholder();
    void cancel();
    void merge(const ResourceTemplate& tmpl);
    bool isAtLastPlaceholder() const;
    bool isAtFirstPlaceholder() const;
    int getCurrentPlaceholderIndex() const;
    QList<Placeholder> getAllPlaceholders() const;
private:
    EditorWidget* m_editor;
    ResourceTemplate m_template;
    QList<Placeholder> m_placeholders;
    int m_currentIndex;
    void parsePlaceholders();
};

} // namespace scadtemplates
