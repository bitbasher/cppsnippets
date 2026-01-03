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

  ResourceType getType() const { return type; }
  const QString &getSubDir() const { return subdirectory; }
  const QString &getDescription() const { return description; }
  const QList<ResourceType> &getSubResTypes() const { return subResTypes; }
  const QStringList &getPrimaryExtensions() const { return primaryExtensions; }
  const QStringList &getAttachmentExtensions() const {
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

  // Static resource type registry
  static const QMap<ResourceType, ResourceTypeInfo> s_resourceTypes;
};

inline static const QString groupNameCapture = QStringLiteral("__capture__");

} // namespace platformInfo
