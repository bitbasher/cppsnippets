/**
 * @file ResourceTypeInfo.cpp
 * @brief Implementation of ResourceTypeInfo static members
 */

#include "ResourceTypeInfo.hpp"

namespace resourceMetadata {

// Convert ResourceType enum to string
QString ResourceTypeInfo::getResTypeString(ResourceType type)
{
    switch (type) {
        case ResourceType::Templates: return QStringLiteral("Templates");
        case ResourceType::Examples: return QStringLiteral("Examples");
        case ResourceType::Fonts: return QStringLiteral("Fonts");
        case ResourceType::Tests: return QStringLiteral("Tests");
        case ResourceType::Libraries: return QStringLiteral("Libraries");
        case ResourceType::Shaders: return QStringLiteral("Shaders");
        case ResourceType::Translations: return QStringLiteral("Translations");
        case ResourceType::ColorSchemes: return QStringLiteral("ColorSchemes");
        case ResourceType::EditorColors: return QStringLiteral("EditorColors");
        case ResourceType::RenderColors: return QStringLiteral("RenderColors");
        case ResourceType::Group: return QStringLiteral("Group");
        default: return QStringLiteral("Unknown");
    }
}

// Static resource type definitions with file extensions
const QMap<ResourceType, ResourceTypeInfo> ResourceTypeInfo::s_resourceTypes = {
    {ResourceType::Unknown,
     ResourceTypeInfo(ResourceType::Unknown, QStringLiteral("unknown"),
                      QStringLiteral("Unknown Resource Rype"),
                      {}, // no sub-resources
                      {}, {})},

    {ResourceType::Examples,
     ResourceTypeInfo(ResourceType::Examples, QStringLiteral("examples"),
                      QStringLiteral("Example Scripts"),
                      s_exampleSub,
                      {QStringLiteral(".scad")}, s_attachments)},

    {ResourceType::Group,
     ResourceTypeInfo(ResourceType::Group, groupNameCapture,
                      QStringLiteral("A Category or Group"),
                      {}, // no sub-resources
                      {QStringLiteral(".scad")}, {})},

    {ResourceType::Tests,
     ResourceTypeInfo(
         ResourceType::Tests, QStringLiteral("tests"),
         QStringLiteral("Test Scripts"),
         s_testSub, // possible to want tempates to build test scripts
         {QStringLiteral(".scad")}, s_attachments)},

    {ResourceType::Fonts,
     ResourceTypeInfo(ResourceType::Fonts, QStringLiteral("fonts"),
                      QStringLiteral("Font Files"), {}, // no sub-resources
                      {QStringLiteral(".ttf"), QStringLiteral(".otf")}, {})},

    {ResourceType::ColorSchemes,
     ResourceTypeInfo(
         ResourceType::ColorSchemes, QStringLiteral("color-schemes"),
         QStringLiteral("Color scheme definitions"),
         {ResourceType::EditorColors,
          ResourceType::RenderColors}, // contains editor and render colors
         {}, // no primary extensions (container only)
         {})},

    {ResourceType::EditorColors,
     ResourceTypeInfo(
         ResourceType::EditorColors, QStringLiteral("color-schemes"),
         QStringLiteral("Editor color schemes"), {}, // no sub-resources
         {QStringLiteral(".json")}, {})},

    {ResourceType::RenderColors,
     ResourceTypeInfo(
         ResourceType::RenderColors, QStringLiteral("color-schemes"),
         QStringLiteral("Render color schemes"), {}, // no sub-resources
         {QStringLiteral(".json")}, {})},

    {ResourceType::Shaders,
     ResourceTypeInfo(
         ResourceType::Shaders, QStringLiteral("shaders"),
         QStringLiteral("OpenGL Shaders"), {}, // no sub-resources
         {QStringLiteral(".frag"), QStringLiteral(".vert")}, {})},

    {ResourceType::Templates,
     ResourceTypeInfo(ResourceType::Templates, QStringLiteral("templates"),
                      QStringLiteral("Template Files"),
                      {}, // no sub-resources
                      {QStringLiteral(".json")}, {})},

    {ResourceType::Libraries,
     ResourceTypeInfo(
         ResourceType::Libraries, QStringLiteral("libraries"),
         QStringLiteral("OpenSCAD Library"),
         s_topLevel, // libraries can contain any top-level resource
         {QStringLiteral(".scad")},
         s_attachments)},

    {ResourceType::Translations,
     ResourceTypeInfo(ResourceType::Translations, QStringLiteral("locale"),
                      QStringLiteral("Translation Files"),
                      {}, // no sub-resources
                      {QStringLiteral(".qm"), QStringLiteral(".ts")}, {})}
};

} // namespace resourceMetadata
