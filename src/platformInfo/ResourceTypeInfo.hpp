/**
 * @file ResourceTypeInfo.h
 * @brief Information about OpenSCAD resources
 *
 */

#pragma once

#include "export.hpp"

#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>

namespace platformInfo {

/**
 * @brief Enumeration of OpenSCAD resource types
 *
 * These correspond to the subdirectories under the resource directory
 * that contain different types of resources.
 * Names should be plural, except for Group
 */
enum class ResourceType {
  Unknown = 0,  ///< Unknown/invalid resource type
  Group,        ///< Container holde groups of a resource
  Examples,     ///< Example .scad files ($RESOURCEDIR/examples)
  Tests,        ///< Test .scad files ($RESOURCEDIR/tests) - may have .json/.txt/.dat
                ///< attachments
  Fonts,        ///< Font files - .ttf, .otf ($RESOURCEDIR/fonts)
  ColorSchemes, ///< Container folder for color schemes
  EditorColors, ///< Color scheme files for the editor - .json
                ///< ($RESOURCEDIR/color-schemes)
  RenderColors, ///< Color scheme files for 3D View - .json
                ///< ($RESOURCEDIR/color-schemes)
  Shaders,      ///< Shader files - .frag, .vert ($RESOURCEDIR/shaders)
  Templates,    ///< Template files ($RESOURCEDIR/templates)
  Libraries,    ///< OpenSCAD library .scad scripts ($RESOURCEDIR/libraries) -
                ///< extend OpenSCAD features
  Translations, ///< Translation/locale files ($RESOURCEDIR/locale)
  NewResources  ///< For resources dropped in and not yet categorized
};

// All top-level resource types that can be discovered/scanned
//   excludes EditorColors/RenderColors which are sub-resources)
inline static const QList<ResourceType> s_topLevel = {
    ResourceType::Examples,  ResourceType::Tests,
    ResourceType::Fonts,     ResourceType::ColorSchemes,
    ResourceType::Shaders,   ResourceType::Templates,
    ResourceType::Libraries, ResourceType::Translations};

inline static const QList<ResourceType> s_nonContainer = {
    ResourceType::Fonts, ResourceType::EditorColors, ResourceType::RenderColors,
    ResourceType::Shaders, ResourceType::Templates};

inline static const QList<ResourceType> s_exampleSub = {
    ResourceType::Group, ResourceType::Templates, ResourceType::Tests};

inline static const QList<ResourceType> s_testSub = {ResourceType::Templates};

inline static const QStringList s_attachments = {
    QStringLiteral(".json"), QStringLiteral(".txt"), QStringLiteral(".dat"),
    QStringLiteral(".png"),  QStringLiteral(".jpg"), QStringLiteral(".jpeg"),
    QStringLiteral(".svg"),  QStringLiteral(".gif"), QStringLiteral(".csv"),
    QStringLiteral(".stl"),  QStringLiteral(".off"), QStringLiteral(".dxf")};

/**
 * @brief Resource type metadata
 *
 * Contains the subdirectory name, description, and file extensions for a
 * resource type.
 */
class PLATFORMINFO_API ResourceTypeInfo {
private:
  ResourceType type;    ///< The resource type enum
  QString subdirectory; ///< Subdirectory name under resource dir
  QString description;  ///< Human-readable description
  QList<ResourceType>
      subResTypes; ///< Sub-resource types contained within this resource type
  QStringList primaryExtensions; ///< Primary file extensions for this resource
  QStringList attachmentExtensions; ///< Optional attachment file extensions

public:
  ResourceTypeInfo() = default;

  // Constructor
  ResourceTypeInfo(ResourceType t, const QString &subdir, const QString &desc,
                   const QList<ResourceType> &subRes = {},
                   const QStringList &primary = {},
                   const QStringList &attachments = {})
      : type(t), subdirectory(subdir), description(desc), subResTypes(subRes),
        primaryExtensions(primary), attachmentExtensions(attachments) {}

  // Copy constructor
  ResourceTypeInfo(const ResourceTypeInfo &rti)
      : type(rti.type), subdirectory(rti.subdirectory),
        description(rti.description), subResTypes(rti.subResTypes),
        primaryExtensions(rti.primaryExtensions),
        attachmentExtensions(rti.attachmentExtensions) {}

  ResourceType gettype(const ResourceType t) { return type; }
  QString getSubDir(const QString &subdir) { return subdirectory; }
  QString getDescription(const QString &desc) { return description; }
  QList<ResourceType> getSubResTypes(const QList<ResourceType> &subRes) {
    return subResTypes;
  }
  QStringList getPrimaryExtensions(const QStringList &primary) {
    return primaryExtensions;
  }
  QStringList getAttachmentExtensions(const QStringList &attachments) {
    return attachmentExtensions;
  }

  ResourceTypeInfo &operator=(const ResourceTypeInfo &other) {
    if (this != &other) {
      type = other.type;
      subdirectory = other.subdirectory;
      description = other.description;
      subResTypes = other.subResTypes;
      primaryExtensions = other.primaryExtensions;
      attachmentExtensions = other.attachmentExtensions;
    }
    return *this;
  }

  /**
   * @brief Get all resource type definitions
   * @return Vector of all resource type info
   */
  static QList<ResourceTypeInfo> allResourceTypes(){
    QList<ResourceTypeInfo> resTypeInfos;
    for (auto it = s_resourceTypes.constBegin(); it != s_resourceTypes.constEnd(); ++it) {
        resTypeInfos.append(it.value());
    }
    return resTypeInfos;
  }

  inline static const QString groupNameCapture = QStringLiteral("__capture__");

  // Static resource type definitions with file extensions
  inline const QMap<ResourceType, ResourceTypeInfo> s_resourceTypes = {
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

      {ResourceType::NewResources,
       ResourceTypeInfo(
           ResourceType::NewResources, QStringLiteral("newresources"),
           QStringLiteral("Drop Targets"), s_nonContainer, {}, {})},

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
           allTopLevelResTypes, // libraries can contain any top-level resource
           {QStringLiteral(".scad")}, {})},

      {ResourceType::Translations,
       ResourceTypeInfo(ResourceType::Translations, QStringLiteral("locale"),
                        QStringLiteral("Translation files"),
                        {}, // no sub-resources
                        {QStringLiteral(".qm"), QStringLiteral(".ts")}, {})}};
};
} // namespace platformInfo
