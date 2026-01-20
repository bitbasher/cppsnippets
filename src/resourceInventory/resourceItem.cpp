#include "resourceItem.hpp"
#include "../platformInfo/ResourceLocation.hpp"
#include "../scadtemplates/template_parser.hpp"
#include <QFileInfo>
#include <QMetaType>

namespace resourceInventory {

// ============================================================================
// ResourceItem
// ============================================================================

ResourceItem::ResourceItem(
    const QString& path,
    ResourceType type,
    ResourceTier tier,
    ResourceAccess access,
    const QString& name,
    const QString& displayName,
    const QString& description,
    const QString& category
)
    : m_path(path)
    , m_type(type)
    , m_tier(tier)
    , m_access(access)
    , m_name(name)
    , m_displayName(displayName)
    , m_description(description)
    , m_category(category)
{
    // No fallback - if subclass needs name derivation, it should do it explicitly
}

int ResourceItem::metaTypeId()
{
    static int id = qRegisterMetaType<ResourceItem>("resInventory::ResourceItem");
    return id;
}

// ============================================================================
// ResourceScript
// ============================================================================

// Convenience constructor for inventory use
ResourceScript::ResourceScript(const QString& path, const QString& name)
    : ResourceItem(
        path,
        ResourceType::Examples,
        ResourceTier::User,
        ResourceAccess::ReadOnly,
        name
    )
{
    m_scriptPath = path;
}

ResourceScript::ResourceScript(
    const QString& path,
    ResourceType type,
    ResourceTier tier,
    ResourceAccess access,
    const QString& name,
    const QString& displayName,
    const QString& description,
    const QString& category
)
    : ResourceItem(
        path, 
        type, 
        tier, 
        access, 
        name,
        displayName, 
        description, 
        category
    )
{
    m_scriptPath = path;
}

// ============================================================================
// ResourceTemplate
// ============================================================================

// Convenience constructor for inventory use
ResourceTemplate::ResourceTemplate(const QString& path, const QString& name)
    : ResourceItem(
        path,
        ResourceType::Templates,
        ResourceTier::User,
        ResourceAccess::ReadOnly,
        name
    )
    , m_format(QStringLiteral("text/scad.template"))
    , m_version(defaultVersion)
{
}

ResourceTemplate::ResourceTemplate(
    const QString& path,
    ResourceType type,
    ResourceTier tier,
    ResourceAccess access,
    const QString& name,
    const QString& displayName,
    const QString& description,
    const QString& category
)
    : ResourceItem(path, type, tier, access, name, displayName, description, category)
    , m_format(QStringLiteral("text/scad.template"))
    , m_version(defaultVersion)
{
}

bool ResourceTemplate::readJson(const QFileInfo& fileInfo)
{
    m_lastError.clear();

    if (!fileInfo.exists()) {
        m_lastError = QStringLiteral("File does not exist: %1").arg(fileInfo.filePath());
        return false;
    }

    scadtemplates::TemplateParser parser;
    auto result = parser.parseFile(fileInfo.filePath());

    if (!result.success) {
        m_lastError = result.errorMessage;
        return false;
    }

    if (result.templates.isEmpty()) {
        m_lastError = QStringLiteral("No template found in file");
        return false;
    }

    // Take the first template from the file
    const ResourceTemplate& tmpl = result.templates.first();

    // Copy all properties from parsed template
    setPath(fileInfo.filePath());
    setName(tmpl.name());
    setDisplayName(tmpl.displayName());
    setDescription(tmpl.description());
    setType(ResourceType::Templates);
    setTier(tmpl.tier());
    setAccess(tmpl.access());
    setCategory(tmpl.category());

    // Template-specific properties
    setFormat(tmpl.format());
    setSource(tmpl.source());
    setVersion(tmpl.version());
    setBody(tmpl.body());
    setRawText(tmpl.rawText());
    setPrefix(tmpl.prefix());
    setScopes(tmpl.scopes());

    return true;
}

} // namespace resourceInventory
