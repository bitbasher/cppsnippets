#pragma once

#include <QString>
#include <QStringList>
#include <QList>

namespace resourceInfo {

// Consolidated, singular resource type names
// Folder names may be plural; type names remain singular
enum class ResourceType {
    Unknown,
    Example,
    Test,
    Font,
    ColorScheme,   // Container only; contains EditorColor + RenderColor
    EditorColor,
    RenderColor,
    Shader,
    Template,
    Library,
    Translation,
    Group          // Grouping/category folder
};

// Reference info for a resource type (TYPE metadata only)
struct ResourceTypeInfo {
    ResourceType type;
    QString subdirectory;           // e.g. "examples", "fonts", "color-schemes"
    QString description;            // Human-readable description
    QList<ResourceType> subResTypes; // Contained sub-resource types (if container)
    QString extension;              // Primary file extension (single)
    QStringList attachmentExtensions; // Optional attachment extensions
};

class ResourceTypeRegistry {
public:
    static const QList<ResourceTypeInfo>& allTypes() { return s_types; }

    static const ResourceTypeInfo* info(ResourceType t) {
        for (const auto& i : s_types) {
            if (i.type == t) return &i;
        }
        return nullptr;
    }

    static QString subdir(ResourceType t) {
        const auto* i = info(t);
        return i ? i->subdirectory : QString();
    }

    static QString extension(ResourceType t) {
        const auto* i = info(t);
        return i ? i->extension : QString();
    }

    static QStringList attachments(ResourceType t) {
        const auto* i = info(t);
        return i ? i->attachmentExtensions : QStringList();
    }

    static const QList<ResourceType>& topLevelTypes() { return s_topLevel; }

private:
    // Shared lists
    inline static const QStringList s_attachmentsList = {
        QStringLiteral(".json"), QStringLiteral(".txt"), QStringLiteral(".dat"),
        QStringLiteral(".png"),  QStringLiteral(".jpg"), QStringLiteral(".jpeg"),
        QStringLiteral(".svg"),  QStringLiteral(".gif"), QStringLiteral(".csv"),
        QStringLiteral(".stl"),  QStringLiteral(".off"), QStringLiteral(".dxf")
    };

    inline static const QList<ResourceType> s_exampleSub = {
        ResourceType::Group,
        ResourceType::Template
    };

    inline static const QList<ResourceType> s_testSub = {
        ResourceType::Template
    };

    inline static const QList<ResourceType> s_topLevel = {
        ResourceType::Example,
        ResourceType::Test,
        ResourceType::Font,
        ResourceType::ColorScheme,
        ResourceType::Shader,
        ResourceType::Template,
        ResourceType::Library,
        ResourceType::Translation
    };

    // Canonical registry (singular names, single extension per type)
    inline static const QList<ResourceTypeInfo> s_types = {
        { ResourceType::Example,
          QStringLiteral("examples"),
          QStringLiteral("Example scripts"),
          s_exampleSub,
          QStringLiteral(".scad"),
          s_attachmentsList },

        { ResourceType::Group,
          QStringLiteral("__capture__"),
          QStringLiteral("Editor categories (group folders)"),
          {},
          QStringLiteral(".scad"),
          s_attachmentsList },

        { ResourceType::Test,
          QStringLiteral("tests"),
          QStringLiteral("Test scripts"),
          s_testSub,
          QStringLiteral(".scad"),
          s_attachmentsList },

        { ResourceType::Font,
          QStringLiteral("fonts"),
          QStringLiteral("Font files (supplement OS fonts)"),
          {},
          QStringLiteral(".ttf"), // canonical; some fonts may be .otf but single-ext rule applies
          {} },

        { ResourceType::ColorScheme,
          QStringLiteral("color-schemes"),
          QStringLiteral("Color scheme definitions"),
          QList<ResourceType>{ ResourceType::EditorColor, ResourceType::RenderColor },
          QString(),
          {} },

        { ResourceType::EditorColor,
          QStringLiteral("editor"),
          QStringLiteral("Editor color schemes"),
          {},
          QStringLiteral(".json"),
          {} },

        { ResourceType::RenderColor,
          QStringLiteral("render"),
          QStringLiteral("Render color schemes"),
          {},
          QStringLiteral(".json"),
          {} },

        { ResourceType::Shader,
          QStringLiteral("shaders"),
          QStringLiteral("OpenGL shader files"),
          {},
          QStringLiteral(".frag"), // primary; secondary .vert not primary per single-ext rule
          {} },

        { ResourceType::Template,
          QStringLiteral("templates"),
          QStringLiteral("Template files"),
          {},
          QStringLiteral(".json"),
          {} },

        { ResourceType::Library,
          QStringLiteral("libraries"),
          QStringLiteral("OpenSCAD library scripts that extend features"),
          s_topLevel, // can contain any top-level resource
          QStringLiteral(".scad"),
          {} },

        { ResourceType::Translation,
          QStringLiteral("locale"),
          QStringLiteral("Translation files"),
          {},
          QStringLiteral(".qm"), // primary per single-ext rule
          {} }
    };
};

} // namespace resourceInfo
