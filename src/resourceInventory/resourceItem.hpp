#ifndef RESOURCEITEM_H
#define RESOURCEITEM_H

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QVariant>

namespace resourceInventory {

/**
 * @brief Tier where a resource was found
 */
enum class ResourceTier {
    Installation,   ///< Built-in with application
    Machine,        ///< System-wide (all users)
    User            ///< User-specific (personal)
};

/**
 * @brief Type of resource
 */
enum class ResourceType {
    Unknown,
    ColorSchemes,   ///< Color scheme container folder (contains EditorColors and RenderColors)
    RenderColors,   ///< Render color scheme (.json)
    EditorColors,   ///< Editor color scheme (.json)
    Font,           ///< Font file (.ttf, .otf)
    Library,        ///< OpenSCAD library (folder with .scad files)
    Example,        ///< Example script (.scad) with optional attachments (.png, .jpg, .jpeg, .svg, .gif, .json, .txt, .csv, .stl, .off, .dxf, .dat)
    Test,           ///< Test script (.scad) with optional attachments (.png, .jpg, .jpeg, .svg, .gif, .json, .txt, .csv, .stl, .off, .dxf, .dat)
    Template,       ///< User template (.scad, writable)
    Shader,         ///< Shader file
    Translation     ///< Translation file (.ts, .qm)
};

/**
 * @brief Access mode for a resource
 */
enum class ResourceAccess {
    ReadOnly,       ///< Cannot be modified by user
    Writable        ///< Can be created/modified by user
};

/**
 * @brief Base class for all resource items
 * 
 * Represents a single resource with its location, metadata, and state.
 * Can be used directly for simple resources or subclassed for complex ones.
 */
class ResourceItem {
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
class ResourceScript : public ResourceItem {
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
class ResourceTemplate : public ResourceItem {
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
    
    bool isValid() const override;
    
private:
    QString m_format;       // MIME type, e.g., "text/scad.template"
    QString m_source;       // Source tag: "legacy-converted", "cppsnippet-made", "openscad-made"
    QString m_version;      // Version string
    QString m_body;         // Assembled template body
    QString m_rawText;      // Original legacy format text
};

/**
 * @brief String conversion utilities
 */
QString resourceTypeToString(ResourceType type);
ResourceType stringToResourceType(const QString& str);

QString resourceTierToString(ResourceTier tier);
ResourceTier stringToResourceTier(const QString& str);

} // namespace resourceInventory

Q_DECLARE_METATYPE(resourceInventory::ResourceItem)
Q_DECLARE_METATYPE(resourceInventory::ResourceScript)
Q_DECLARE_METATYPE(resourceInventory::ResourceTemplate)

#endif // RESOURCEITEM_H
