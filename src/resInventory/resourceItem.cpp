#include "resInventory/resourceItem.h"
#include <QFileInfo>
#include <QMetaType>

namespace resInventory {

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
        case ResourceType::Unknown:     return QStringLiteral("Unknown");
        case ResourceType::ColorScheme: return QStringLiteral("ColorScheme");
        case ResourceType::Font:        return QStringLiteral("Font");
        case ResourceType::Library:     return QStringLiteral("Library");
        case ResourceType::Example:     return QStringLiteral("Example");
        case ResourceType::Test:        return QStringLiteral("Test");
        case ResourceType::Template:    return QStringLiteral("Template");
        case ResourceType::Shader:      return QStringLiteral("Shader");
        case ResourceType::Translation: return QStringLiteral("Translation");
    }
    return QStringLiteral("Unknown");
}

ResourceType stringToResourceType(const QString& str)
{
    if (str == QLatin1String("ColorScheme")) return ResourceType::ColorScheme;
    if (str == QLatin1String("Font"))        return ResourceType::Font;
    if (str == QLatin1String("Library"))     return ResourceType::Library;
    if (str == QLatin1String("Example"))     return ResourceType::Example;
    if (str == QLatin1String("Test"))        return ResourceType::Test;
    if (str == QLatin1String("Template"))    return ResourceType::Template;
    if (str == QLatin1String("Shader"))      return ResourceType::Shader;
    if (str == QLatin1String("Translation")) return ResourceType::Translation;
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

} // namespace resInventory
