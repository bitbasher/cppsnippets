#ifndef RESOURCEITEM_H
#define RESOURCEITEM_H

#include "../platformInfo/export.hpp"
#include "../resourceMetadata/ResourceTier.hpp"
#include "../resourceMetadata/ResourceTypeInfo.hpp"
#include "../resourceMetadata/ResourceAccess.hpp"
#include "../scadtemplates/edittype.hpp"
#include "../scadtemplates/editsubtype.hpp"

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QVariant>

namespace resourceInventory {

// Use Gold Standard enums from resourceMetadata
using ResourceTier = resourceMetadata::ResourceTier;
using ResourceType = resourceMetadata::ResourceType;
using ResourceAccess = resourceMetadata::Access;

/**
 * @brief Base class for all resource items
 * 
 * Represents a single resource with its location, metadata, and state.
 * Can be used directly for simple resources or subclassed for complex ones.
 */
class PLATFORMINFO_API ResourceItem {
public:
    ResourceItem() = default;
    explicit ResourceItem(const QString& path);
    ResourceItem(const QString& path, ResourceType type, ResourceTier tier);
    virtual ~ResourceItem() = default;
    
    // Identity
    QString path() const { return m_path; }
    void setPath(const QString& path) { m_path = path; }
    
    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }
    
    QString displayName() const { return m_displayName.isEmpty() ? m_name : m_displayName; }
    void setDisplayName(const QString& name) { m_displayName = name; }
    
    QString description() const { return m_description; }
    void setDescription(const QString& desc) { m_description = desc; }
    
    // Classification
    ResourceType type() const { return m_type; }
    void setType(ResourceType type) { m_type = type; }
    
    ResourceTier tier() const { return m_tier; }
    void setTier(ResourceTier tier) { m_tier = tier; }
    
    ResourceAccess access() const { return m_access; }
    void setAccess(ResourceAccess access) { m_access = access; }
    
    // Category (for templates: subfolder name; for libraries: category tag)
    QString category() const { return m_category; }
    void setCategory(const QString& category) { m_category = category; }
    
    // Source tracking (for updates)
    QString sourcePath() const { return m_sourcePath; }
    void setSourcePath(const QString& path) { m_sourcePath = path; }
    
    QString sourceLocationKey() const { return m_sourceLocationKey; }
    void setSourceLocationKey(const QString& key) { m_sourceLocationKey = key; }
    
    // State
    bool exists() const { return m_exists; }
    void setExists(bool exists) { m_exists = exists; }
    
    bool isEnabled() const { return m_isEnabled; }
    void setEnabled(bool enabled) { m_isEnabled = enabled; }
    
    bool isModified() const { return m_isModified; }
    void setModified(bool modified) { m_isModified = modified; }
    
    QDateTime lastModified() const { return m_lastModified; }
    void setLastModified(const QDateTime& dt) { m_lastModified = dt; }
    
    // Validation
    virtual bool isValid() const;
    
    // For QVariant storage
    static int metaTypeId();
    
protected:
    QString m_path;
    QString m_name;
    QString m_displayName;
    QString m_description;
    QString m_category;
    QString m_sourcePath;           // Original path where found
    QString m_sourceLocationKey;    // Key in ResLocMap for updates
    
    ResourceType m_type = ResourceType::Unknown;
    ResourceTier m_tier = ResourceTier::User;
    ResourceAccess m_access = ResourceAccess::ReadOnly;
    
    bool m_exists = false;
    bool m_isEnabled = true;
    bool m_isModified = false;
    QDateTime m_lastModified;
};

/**
 * @brief Resource with attachments (examples, tests)
 * 
 * Represents a script that may have associated files like
 * images, JSON data, or text files.
 */
class PLATFORMINFO_API ResourceScript : public ResourceItem {
public:
    ResourceScript() = default;
    explicit ResourceScript(const QString& path);
    
    // Main script file
    QString scriptPath() const { return m_scriptPath; }
    void setScriptPath(const QString& path) { m_scriptPath = path; }
    
    // Attachments (images, json, txt files in same folder)
    QStringList attachments() const { return m_attachments; }
    void setAttachments(const QStringList& files) { m_attachments = files; }
    void addAttachment(const QString& file) { m_attachments.append(file); }
    bool hasAttachments() const { return !m_attachments.isEmpty(); }
    
    bool isValid() const override;
    
private:
    QString m_scriptPath;
    QStringList m_attachments;
};

/**
 * @brief Resource representing a template
 * 
 * Extends ResourceItem with template-specific metadata
 * including format, source tag, and version.
 */
class PLATFORMINFO_API ResourceTemplate : public ResourceItem {
public:
    ResourceTemplate() = default;
    explicit ResourceTemplate(const QString& path);
    
    // Template metadata
    QString format() const { return m_format; }
    void setFormat(const QString& format) { m_format = format; }
    
    QString source() const { return m_source; }
    void setSource(const QString& source) { m_source = source; }
    
    QString version() const { return m_version; }
    void setVersion(const QString& version) { m_version = version; }
    
    // Template content
    QString body() const { return m_body; }
    void setBody(const QString& body) { m_body = body; }
    
    // Raw legacy content (for imported templates)
    QString rawText() const { return m_rawText; }
    void setRawText(const QString& text) { m_rawText = text; }
    
    // Template triggering
    QString prefix() const { return m_prefix; }
    void setPrefix(const QString& prefix) { m_prefix = prefix; }
    
    // Language scopes (e.g., "source.scad", "text.plain")
    QStringList scopes() const { return m_scopes; }
    void setScopes(const QStringList& scopes) { m_scopes = scopes; }
    void addScope(const QString& scope) { m_scopes.append(scope); }
    void clearScopes() { m_scopes.clear(); }
    
    // File type classification
    scadtemplates::EditType editType() const { return m_editType; }
    void setEditType(scadtemplates::EditType type) { m_editType = type; }
    
    scadtemplates::EditSubtype editSubtype() const { return m_editSubtype; }
    void setEditSubtype(scadtemplates::EditSubtype subtype);  // Also updates EditType
    
    bool isValid() const override;
    
private:
    QString m_format;       // MIME type, e.g., "text/scad.template"
    QString m_source;       // Source tag: "legacy-converted", "cppsnippet-made", "openscad-made"
    QString m_version;      // Version string
    QString m_body;         // Assembled template body
    QString m_rawText;      // Original legacy format text
    QString m_prefix;       // Trigger text for template insertion
    QStringList m_scopes;   // Language scopes for filtering
    scadtemplates::EditType m_editType = scadtemplates::EditType::Text;
    scadtemplates::EditSubtype m_editSubtype = scadtemplates::EditSubtype::Txt;
};

/**
 * @brief String conversion utilities
 */
PLATFORMINFO_API QString resourceTypeToString(ResourceType type);
PLATFORMINFO_API ResourceType stringToResourceType(const QString& str);

PLATFORMINFO_API QString resourceTierToString(ResourceTier tier);
PLATFORMINFO_API ResourceTier stringToResourceTier(const QString& str);

} // namespace resourceInventory

Q_DECLARE_METATYPE(resourceInventory::ResourceItem)
Q_DECLARE_METATYPE(resourceInventory::ResourceScript)
Q_DECLARE_METATYPE(resourceInventory::ResourceTemplate)

#endif // RESOURCEITEM_H
