#pragma once

#include <QList>
#include <QString>
#include <QStringList>

namespace resourceInfo {

// Canonical resource types used across resource handling
enum class ResourceType {
  Examples,
  Group,
  Tests,
  Fonts,
  ColorSchemes,
  EditorColors,
  RenderColors,
  Shaders,
  Templates,
  Libraries,
  Translations,
  NewResources
};

// Resource metadata (header-only registry)
struct ResourceTypeInfo {
  ResourceType type;
  QString subdirectory;
  QString description;
  QList<ResourceType> subResTypes;
  QStringList primaryExtensions;
  QStringList attachmentExtensions;
};

class ResourceTypeRegistry {
public:
  static const QList<ResourceTypeInfo> &allTypes() { return s_types; }

  static const ResourceTypeInfo *info(ResourceType t) {
    for (const auto &i : s_types) {
      if (i.type == t)
        return &i;
    }
    return nullptr;
  }

  static QString subdir(ResourceType t) {
    const auto *i = info(t);
    return i ? i->subdirectory : QString();
  }

  static QStringList extensions(ResourceType t) {
    const auto *i = info(t);
    return i ? i->primaryExtensions : QStringList();
  }

  static QStringList attachments(ResourceType t) {
    const auto *i = info(t);
    return i ? i->attachmentExtensions : QStringList();
  }

  static const QList<ResourceType> &topLevelTypes() { return s_topLevel; }

private:
  inline static const QString groupNameCapture = QStringLiteral("__capture__");

  inline static const QList<ResourceType> s_topLevel = {
      ResourceType::Examples,     ResourceType::Tests,
      ResourceType::Fonts,        ResourceType::ColorSchemes,
      ResourceType::Shaders,      ResourceType::Templates,
      ResourceType::Libraries,    ResourceType::Translations};

  inline static const QList<ResourceType> s_nonContainer = {
      ResourceType::Fonts, ResourceType::ColorSchemes,
      ResourceType::Shaders, ResourceType::Templates};

  inline static const QList<ResourceType> s_exampleSub = {
      ResourceType::Group, ResourceType::Templates};

  inline static const QList<ResourceType> s_testSub = {ResourceType::Templates};

  inline static const QStringList s_attachments = {
      QStringLiteral(".json"), QStringLiteral(".txt"),
      QStringLiteral(".dat"),  QStringLiteral(".png"),
      QStringLiteral(".jpg"),  QStringLiteral(".jpeg"),
      QStringLiteral(".svg"),  QStringLiteral(".gif"),
      QStringLiteral(".csv"),  QStringLiteral(".stl"),
      QStringLiteral(".off"),  QStringLiteral(".dxf")};

  inline static const QList<ResourceTypeInfo> s_types = {
      {ResourceType::Examples, QStringLiteral("examples"),
       QStringLiteral("Example scripts"), s_exampleSub,
       {QStringLiteral(".scad")}, s_attachments},

      {ResourceType::NewResources, QStringLiteral("newresources"),
       QStringLiteral("Drop Targets"), s_nonContainer, {}, {}},

      {ResourceType::Group, groupNameCapture,
       QStringLiteral("Editor Categories"), {},
       {QStringLiteral(".scad")}, s_attachments},

      {ResourceType::Tests, QStringLiteral("tests"),
       QStringLiteral("Test OpenSCAD scripts"), s_testSub,
       {QStringLiteral(".scad")}, s_attachments},

      {ResourceType::Fonts, QStringLiteral("fonts"),
       QStringLiteral("Font files (supplements OS fonts)"), {},
       {QStringLiteral(".ttf"), QStringLiteral(".otf")}, {}},

      {ResourceType::ColorSchemes, QStringLiteral("color-schemes"),
       QStringLiteral("Color scheme definitions"),
       QList<ResourceType>{ResourceType::EditorColors,
                           ResourceType::RenderColors},
       {}, {}},

      {ResourceType::EditorColors, QStringLiteral("editor"),
       QStringLiteral("Editor color schemes"), {},
       {QStringLiteral(".json")}, {}},

      {ResourceType::RenderColors, QStringLiteral("render"),
       QStringLiteral("Render color schemes"), {},
       {QStringLiteral(".json")}, {}},

      {ResourceType::Shaders, QStringLiteral("shaders"),
       QStringLiteral("OpenGL shader files"), {},
       {QStringLiteral(".frag"), QStringLiteral(".vert")}, {}},

      {ResourceType::Templates, QStringLiteral("templates"),
       QStringLiteral("Template files"), {},
       {QStringLiteral(".json")}, {}},

      {ResourceType::Libraries, QStringLiteral("libraries"),
       QStringLiteral("OpenSCAD library scripts that extend features"),
       s_topLevel, {QStringLiteral(".scad")}, {}},

      {ResourceType::Translations, QStringLiteral("locale"),
       QStringLiteral("Translation files"), {},
       {QStringLiteral(".qm"), QStringLiteral(".ts")}, {}}};
};

} // namespace resourceInfo
