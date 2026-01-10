/**
 * @file ResourceTypeInfo.cpp
 * @brief Implementation of ResourceTypeInfo static members
 */

#include "ResourceTypeInfo.hpp"

namespace resourceMetadata {

// Static resource type definitions with file extensions
const QMap<ResourceType, ResourceTypeInfo> ResourceTypeInfo::s_resourceTypes = {
    {ResourceType::Unknown,
     ResourceTypeInfo(ResourceType::Unknown, QStringLiteral("unknown"),
                      QStringLiteral("Unknown resource type"),
                      {}, // no sub-resources
                      {}, {})},

    {ResourceType::Examples,
     ResourceTypeInfo(ResourceType::Examples, QStringLiteral("examples"),
                      QStringLiteral("Example scripts"),
                      {}, // no sub-resources
                      {QStringLiteral(".scad")}, s_attachments)},

    {ResourceType::Group,
     ResourceTypeInfo(ResourceType::Group, groupNameCapture,
                      QStringLiteral("A Category or Group"), {},
                      {QStringLiteral(".scad")}, {})},

    {ResourceType::Tests,
     ResourceTypeInfo(
         ResourceType::Tests, QStringLiteral("tests"),
         QStringLiteral("Test OpenSCAD scripts"),
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
         QStringLiteral("OpenGL shader files"), {}, // no sub-resources
         {QStringLiteral(".frag"), QStringLiteral(".vert")}, {})},

    {ResourceType::Templates,
     ResourceTypeInfo(ResourceType::Templates, QStringLiteral("templates"),
                      QStringLiteral("Template files"),
                      {}, // no sub-resources
                      {QStringLiteral(".json")}, {})},

    {ResourceType::Libraries,
     ResourceTypeInfo(
         ResourceType::Libraries, QStringLiteral("libraries"),
         QStringLiteral("OpenSCAD library scripts that extend features"),
         s_topLevel, // libraries can contain any top-level resource
         {QStringLiteral(".scad")}, {})},

    {ResourceType::Translations,
     ResourceTypeInfo(ResourceType::Translations, QStringLiteral("locale"),
                      QStringLiteral("Translation files"),
                      {}, // no sub-resources
                      {QStringLiteral(".qm"), QStringLiteral(".ts")}, {})}
};

} // namespace resourceMetadata
