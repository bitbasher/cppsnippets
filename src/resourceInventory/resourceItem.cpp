#include "resourceItem.hpp"
#include "ResourceIndexer.hpp"
#include "../platformInfo/ResourceLocation.hpp"
#include "../scadtemplates/template_parser.hpp"
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
}

ResourceItem::ResourceItem(const QString& path, ResourceType type, ResourceTier tier)
    : m_path(path)
    , m_type(type)
    , m_tier(tier)
{
    QFileInfo fi(path);
    m_name = fi.baseName();
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

ResourceScript::ResourceScript(const QString& filePath, 
                               const platformInfo::ResourceLocation& location)
    : ResourceItem(filePath, ResourceType::Examples, location.tier())
{
    m_scriptPath = filePath;
    
    QFileInfo fi(filePath);
    QString baseName = fi.baseName();
    
    // Use ResourceIndexer for unique index generation
    QString indexString = ResourceIndexer::getOrCreateIndex(
        location, 
        ResourceType::Examples, 
        baseName
    );
    m_uniqueID = QString("%1-%2").arg(indexString, baseName);
    
    // Set display name
    m_displayName = baseName;
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

ResourceTemplate::ResourceTemplate(const QString& filePath, 
                                   const platformInfo::ResourceLocation& location)
    : ResourceItem(filePath, ResourceType::Templates, location.tier())
    , m_format(QStringLiteral("text/scad.template"))
    , m_version(QStringLiteral("1"))
{
    QFileInfo fi(filePath);
    QString baseName = fi.baseName();
    
    // Generate unique ID using unified ResourceIndexer
    // Ensures uniqueness across ALL resource types and locations
    QString indexString = ResourceIndexer::getOrCreateIndex(
        location, 
        ResourceType::Templates, 
        baseName
    );
    m_uniqueID = QString("%1-%2").arg(indexString, baseName);
    
    // Set display name (will be overridden by JSON prefix if present)
    m_displayName = baseName;
}

void ResourceTemplate::setEditSubtype(scadtemplates::EditSubtype subtype)
{
    m_editType = scadtemplates::typeFromSubtype(subtype);
    m_editSubtype = subtype;
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
    setSourcePath(fileInfo.filePath());
    setSourceLocationKey(tmpl.sourceLocationKey());

    // Template-specific properties
    setFormat(tmpl.format());
    setSource(tmpl.source());
    setVersion(tmpl.version());
    setBody(tmpl.body());
    setRawText(tmpl.rawText());
    setPrefix(tmpl.prefix());
    setScopes(tmpl.scopes());
    setEditType(tmpl.editType());
    setEditSubtype(tmpl.editSubtype());

    return true;
}

} // namespace resourceInventory
