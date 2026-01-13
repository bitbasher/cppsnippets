#include "resourceItem.hpp"
#include <QFileInfo>
#include <QMetaType>

namespace resourceInventory {

// ============================================================================
// ResourceItem
// ============================================================================

ResourceItem::ResourceItem(const QString& path)
    : m_path(path)
{
    QFileInfo fi(path);
    m_name = fi.baseName();
    m_exists = fi.exists();
    if (m_exists) {
        m_lastModified = fi.lastModified();
    }
}

ResourceItem::ResourceItem(const QString& path, ResourceType type, ResourceTier tier)
    : m_path(path)
    , m_type(type)
    , m_tier(tier)
{
    QFileInfo fi(path);
    m_name = fi.baseName();
    m_exists = fi.exists();
    if (m_exists) {
        m_lastModified = fi.lastModified();
    }
}

bool ResourceItem::isValid() const
{
    return !m_path.isEmpty() && m_type != ResourceType::Unknown;
}

int ResourceItem::metaTypeId()
{
    static int id = qRegisterMetaType<ResourceItem>("resInventory::ResourceItem");
    return id;
}

// ============================================================================
// ResourceScript
// ============================================================================

ResourceScript::ResourceScript(const QString& path)
    : ResourceItem(path)
{
    m_scriptPath = path;
}

bool ResourceScript::isValid() const
{
    return ResourceItem::isValid() && !m_scriptPath.isEmpty();
}

// ============================================================================
// String conversions
// ============================================================================

QString resourceTypeToString(ResourceType type)
{
    switch (type) {
        case ResourceType::Unknown:      return QStringLiteral("Unknown");
        case ResourceType::Group:        return QStringLiteral("Group");
        case ResourceType::RenderColors: return QStringLiteral("RenderColors");
        case ResourceType::EditorColors: return QStringLiteral("EditorColors");
        case ResourceType::Fonts:        return QStringLiteral("Fonts");
        case ResourceType::Libraries:    return QStringLiteral("Libraries");
        case ResourceType::Examples:     return QStringLiteral("Examples");
        case ResourceType::Tests:        return QStringLiteral("Tests");
        case ResourceType::Templates:    return QStringLiteral("Templates");
        case ResourceType::Shaders:      return QStringLiteral("Shaders");
        case ResourceType::Translations: return QStringLiteral("Translations");
        case ResourceType::ColorSchemes: return QStringLiteral("ColorSchemes");
    }
    return QStringLiteral("Unknown");
}

ResourceType stringToResourceType(const QString& str)
{
    if (str == QLatin1String("Group"))        return ResourceType::Group;
    if (str == QLatin1String("RenderColors")) return ResourceType::RenderColors;
    if (str == QLatin1String("EditorColors")) return ResourceType::EditorColors;
    if (str == QLatin1String("Fonts"))        return ResourceType::Fonts;
    if (str == QLatin1String("Libraries"))    return ResourceType::Libraries;
    if (str == QLatin1String("Examples"))     return ResourceType::Examples;
    if (str == QLatin1String("Tests"))        return ResourceType::Tests;
    if (str == QLatin1String("Templates"))    return ResourceType::Templates;
    if (str == QLatin1String("Shaders"))      return ResourceType::Shaders;
    if (str == QLatin1String("Translations")) return ResourceType::Translations;
    if (str == QLatin1String("ColorSchemes")) return ResourceType::ColorSchemes;
    return ResourceType::Unknown;
}

QString resourceTierToString(ResourceTier tier)
{
    switch (tier) {
        case ResourceTier::Installation: return QStringLiteral("Installation");
        case ResourceTier::Machine:      return QStringLiteral("Machine");
        case ResourceTier::User:         return QStringLiteral("User");
    }
    return QStringLiteral("User");
}

ResourceTier stringToResourceTier(const QString& str)
{
    if (str == QLatin1String("Installation")) return ResourceTier::Installation;
    if (str == QLatin1String("Machine"))      return ResourceTier::Machine;
    if (str == QLatin1String("User"))         return ResourceTier::User;
    return ResourceTier::User;
}

// ============================================================================
// ResourceTemplate
// ============================================================================

ResourceTemplate::ResourceTemplate(const QString& path)
    : ResourceItem(path)
    , m_format(QStringLiteral("text/scad.template"))
    , m_version(QStringLiteral("1"))
{
}

bool ResourceTemplate::isValid() const
{
    return ResourceItem::isValid() && !m_body.isEmpty() && !m_prefix.isEmpty();
}

void ResourceTemplate::setEditSubtype(scadtemplates::EditSubtype subtype)
{
    m_editSubtype = subtype;
    m_editType = scadtemplates::typeFromSubtype(subtype);
}

} // namespace resourceInventory
