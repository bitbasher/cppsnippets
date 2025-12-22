/**
 * @file resourceScannerDirListing.h
 * @brief QDirListing-based resource scanner (Qt 6.8+)
 * 
 * Proof-of-concept scanner using Qt's modern QDirListing API for
 * streaming directory iteration. This replaces QDir::entryList() with
 * a more memory-efficient, on-demand iteration pattern.
 * 
 * @see ResourceScanner for the original QDir-based implementation
 */

#ifndef RESOURCESCANNER_DIRLISTING_H
#define RESOURCESCANNER_DIRLISTING_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QDateTime>
#include <functional>

#include "platformInfo/export.h"
#include "resInventory/resourceItem.h"  // For ResourceType, ResourceTier enums

namespace resInventory {

/**
 * @brief Lightweight struct for discovered resources
 * 
 * Captures essential file metadata during scanning without
 * creating heavy ResourceItem objects. Consumers can convert
 * these to ResourceItem as needed.
 */
struct DiscoveredResource {
    QString path;           ///< Absolute file path
    QString name;           ///< File name (without path)
    QString category;       ///< Category/subfolder within resource type
    QString locationKey;    ///< Source location identifier
    ResourceType type;      ///< Resource type (from resInventory namespace)
    ResourceTier tier;      ///< Installation/Machine/User tier
    QDateTime lastModified; ///< File modification time
    qint64 size;            ///< File size in bytes
    
    DiscoveredResource() 
        : type(ResourceType::Unknown)
        , tier(ResourceTier::User)
        , size(0) 
    {}
};

/**
 * @brief QDirListing-based resource scanner
 * 
 * Modern scanner implementation using Qt 6.8's QDirListing for
 * efficient streaming directory iteration. Key differences from
 * the original ResourceScanner:
 * 
 * - **Streaming**: Results delivered via callback as found (memory efficient)
 * - **Non-recursive by default**: Uses QDirListing's built-in recursion when needed
 * - **Simpler categorization**: Type detection from path patterns
 * - **Lightweight results**: Returns DiscoveredResource structs
 * 
 * @par Example Usage:
 * @code
 * ResourceScannerDirListing scanner;
 * QVector<DiscoveredResource> results;
 * 
 * scanner.scanLocation("/path/to/resources",
 *                      ResourceTier::Installation,
 *                      "install-loc",
 *                      [&results](const DiscoveredResource& res) {
 *                          results.append(res);
 *                      });
 * @endcode
 */
class PLATFORMINFO_API ResourceScannerDirListing : public QObject {
    Q_OBJECT

public:
    /// Callback type for streaming results
    using ScanCallback = std::function<void(const DiscoveredResource&)>;

    explicit ResourceScannerDirListing(QObject* parent = nullptr);
    ~ResourceScannerDirListing() override = default;
    
    /**
     * @brief Scan a single location for all resource types
     * 
     * Scans the given base path, looking for known resource subfolders
     * (color-schemes, fonts, libraries, etc.) and streaming discovered
     * resources through the callback.
     * 
     * @param basePath Base directory to scan
     * @param tier Resource tier (Installation, Machine, User)
     * @param locationKey Display name for this location
     * @param callback Function called for each discovered resource
     * @return Number of resources discovered
     */
    int scanLocation(const QString& basePath,
                     ResourceTier tier,
                     const QString& locationKey,
                     const ScanCallback& callback);
    
    /**
     * @brief Scan a location for a specific resource type
     * 
     * Scans only the subfolder for the specified resource type.
     * 
     * @param basePath Base directory containing resource subfolders
     * @param type Specific resource type to scan for
     * @param tier Resource tier
     * @param locationKey Display name for this location
     * @param callback Function called for each discovered resource
     * @return Number of resources discovered
     */
    int scanLocationForType(const QString& basePath,
                            ResourceType type,
                            ResourceTier tier,
                            const QString& locationKey,
                            const ScanCallback& callback);
    
    /**
     * @brief Collect all resources into a vector (convenience method)
     * 
     * Wraps scanLocation() to return a vector instead of using callbacks.
     * 
     * @param basePath Base directory to scan
     * @param tier Resource tier
     * @param locationKey Display name for this location
     * @return Vector of all discovered resources
     */
    QVector<DiscoveredResource> collectAll(const QString& basePath,
                                           ResourceTier tier,
                                           const QString& locationKey);
    
    /**
     * @brief Get subfolder name for a resource type
     * @param type The resource type
     * @return Folder name (e.g., "color-schemes", "libraries")
     */
    static QString resourceSubfolder(ResourceType type);
    
    /**
     * @brief Get file extensions for a resource type
     * @param type The resource type
     * @return List of extensions (e.g., {"*.json"}, {"*.scad"})
     */
    static QStringList resourceFilters(ResourceType type);
    
    /**
     * @brief Determine resource type from a file path
     * 
     * Analyzes the path to determine which resource type it belongs to
     * based on parent folder names and file extension.
     * 
     * @param filePath Absolute or relative file path
     * @return ResourceType, or Unknown if not recognized
     */
    static ResourceType categorizeByPath(const QString& filePath);

signals:
    void scanStarted(const QString& path, ResourceType type);
    void resourceFound(const DiscoveredResource& resource);
    void scanCompleted(const QString& path, int count);
    void scanError(const QString& path, const QString& error);

private:
    /**
     * @brief Internal scan using QDirListing
     * 
     * @param scanPath Path to scan (already includes resource subfolder)
     * @param type Resource type being scanned
     * @param tier Resource tier
     * @param locationKey Location identifier
     * @param category Current category (for recursive scans)
     * @param callback Result callback
     * @param recursive Whether to recurse into subfolders
     * @return Number of resources found
     */
    int scanWithDirListing(const QString& scanPath,
                           ResourceType type,
                           ResourceTier tier,
                           const QString& locationKey,
                           const QString& category,
                           const ScanCallback& callback,
                           bool recursive);
};

} // namespace resInventory

#endif // RESOURCESCANNER_DIRLISTING_H
