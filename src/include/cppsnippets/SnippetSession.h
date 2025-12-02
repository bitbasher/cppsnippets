// SnippetSession.h
#pragma once

#include <vector>
#include <string>
#include <memory>
#include "snippet.h"

class QScintillaEditor; // Forward declaration for editor integration

namespace cppsnippets {

struct Placeholder {
    int index; // Placeholder number (e.g., 1 for $1)
    int start; // Start position in the text
    int end;   // End position in the text
    std::string defaultValue;
};

class SnippetSession {
public:
    SnippetSession(QScintillaEditor* editor, const Snippet& snippet);
    void insert();
    void nextPlaceholder();
    void prevPlaceholder();
    void cancel();
    void merge(const Snippet& snippet);
    bool isAtLastPlaceholder() const;
    bool isAtFirstPlaceholder() const;
    int getCurrentPlaceholderIndex() const;
    std::vector<Placeholder> getAllPlaceholders() const;
private:
    QScintillaEditor* m_editor;
    Snippet m_snippet;
    std::vector<Placeholder> m_placeholders;
    int m_currentIndex;
    void parsePlaceholders();
};

} // namespace cppsnippets
