/**
 * @file resourceStore.h
 * @brief Typed storage for discovered resources
 * 
 * ResourceStore provides organized storage for resources discovered by
 * ResourceScannerDirListing. Resources are stored in typed vectors,
 * one per ResourceType, for efficient access and iteration.
 * 
 * This replaces the old ResLocTree approach with a simpler, more
 * Qt-idiomatic design that works well with Model/View architecture.
 * 
 * @see ResourceScannerDirListing for discovering resources
 * @see ResourceModel (future) for Qt Model/View integration
 */

#ifndef RESOURCESTORE_H
#define RESOURCESTORE_H

#include <QObject>
#include <QVector>
#include <QHash>
#include <QReadWriteLock>

#include "resInventory/resourceScannerDirListing.h"

namespace resInventory {

/**
 * @brief Typed storage container for discovered resources
 * 
 * Organizes discovered resources by type for efficient retrieval.
 * Thread-safe for concurrent read access (uses QReadWriteLock).
 * 
 * @par Design Goals:
 * - Simple typed vectors instead of complex tree structures
 * - Fast lookup by ResourceType
 * - Filter support by tier, category, location
 * - Ready for Qt Model/View integration
 * 
 * @par Example Usage:
 * @code
 * ResourceStore store;
 * ResourceScannerDirListing scanner;
 * 
 * // Populate from scanner
 * scanner.scanLocation("/path/to/resources",
 *                      ResourceTier::Installation,
 *                      "install",
 *                      [&store](const DiscoveredResource& res) {
 *                          store.addResource(res);
 *                      });
 * 
 * // Query resources
 * auto templates = store.resourcesOfType(ResourceType::Template);
 * auto userFonts = store.resourcesOfType(ResourceType::Font, ResourceTier::User);
 * @endcode
 */
class ResourceStore : public QObject
{
    Q_OBJECT

public:
    explicit ResourceStore(QObject* parent = nullptr);
    ~ResourceStore() override = default;
    
    // ========================================================================
    // Adding resources
    // ========================================================================
    
    /**
     * @brief Add a discovered resource to the store
     * @param resource The resource to add
     */
    void addResource(const DiscoveredResource& resource);
    
    /**
     * @brief Add multiple resources at once
     * @param resources Vector of resources to add
     */
    void addResources(const QVector<DiscoveredResource>& resources);
    
    /**
     * @brief Convenience method to scan and store in one call
     * 
     * Uses ResourceScannerDirListing to scan the path and stores
     * all discovered resources.
     * 
     * @param scanner The scanner to use
     * @param basePath Base directory to scan
     * @param tier Resource tier
     * @param locationKey Location identifier
     * @return Number of resources added
     */
    int scanAndStore(ResourceScannerDirListing& scanner,
                     const QString& basePath,
                     ResourceTier tier,
                     const QString& locationKey);
    
    /**
     * @brief Scan a specific resource type and store results
     * 
     * @param scanner The scanner to use
     * @param basePath Base directory to scan
     * @param type Resource type to scan for
     * @param tier Resource tier
     * @param locationKey Location identifier
     * @return Number of resources added
     */
    int scanTypeAndStore(ResourceScannerDirListing& scanner,
                         const QString& basePath,
                         ResourceType type,
                         ResourceTier tier,
                         const QString& locationKey);
    
    // ========================================================================
    // Querying resources
    // ========================================================================
    
    /**
     * @brief Get all resources of a specific type
     * @param type The resource type to retrieve
     * @return Vector of resources (empty if none found)
     */
    QVector<DiscoveredResource> resourcesOfType(ResourceType type) const;
    
    /**
     * @brief Get resources filtered by type and tier
     * @param type The resource type
     * @param tier The resource tier to filter by
     * @return Filtered vector of resources
     */
    QVector<DiscoveredResource> resourcesOfType(ResourceType type, 
                                                 ResourceTier tier) const;
    
    /**
     * @brief Get resources filtered by type and location
     * @param type The resource type
     * @param locationKey The location key to filter by
     * @return Filtered vector of resources
     */
    QVector<DiscoveredResource> resourcesByLocation(ResourceType type,
                                                     const QString& locationKey) const;
    
    /**
     * @brief Get resources filtered by type and category
     * @param type The resource type
     * @param category The category to filter by
     * @return Filtered vector of resources
     */
    QVector<DiscoveredResource> resourcesByCategory(ResourceType type,
                                                     const QString& category) const;
    
    /**
     * @brief Get all resources (all types combined)
     * @return Vector of all stored resources
     */
    QVector<DiscoveredResource> allResources() const;
    
    /**
     * @brief Find a resource by its path
     * @param path Absolute file path
     * @return Pointer to resource if found, nullptr otherwise
     */
    const DiscoveredResource* findByPath(const QString& path) const;
    
    // ========================================================================
    // Counts and status
    // ========================================================================
    
    /**
     * @brief Get count of resources by type
     * @param type The resource type
     * @return Number of resources of that type
     */
    int countByType(ResourceType type) const;
    
    /**
     * @brief Get count of resources by type and tier
     * @param type The resource type
     * @param tier The resource tier
     * @return Number of matching resources
     */
    int countByTypeAndTier(ResourceType type, ResourceTier tier) const;
    
    /**
     * @brief Get total count of all resources
     * @return Total number of stored resources
     */
    int totalCount() const;
    
    /**
     * @brief Check if store is empty
     * @return true if no resources stored
     */
    bool isEmpty() const;
    
    /**
     * @brief Check if store has resources of a specific type
     * @param type The resource type
     * @return true if at least one resource of that type exists
     */
    bool hasType(ResourceType type) const;
    
    /**
     * @brief Get list of resource types that have content
     * @return Vector of ResourceType enums that have at least one resource
     */
    QVector<ResourceType> availableTypes() const;
    
    /**
     * @brief Get unique categories for a resource type
     * @param type The resource type
     * @return List of unique category strings
     */
    QStringList categoriesForType(ResourceType type) const;
    
    /**
     * @brief Get unique location keys for a resource type
     * @param type The resource type
     * @return List of unique location key strings
     */
    QStringList locationsForType(ResourceType type) const;
    
    // ========================================================================
    // Modification
    // ========================================================================
    
    /**
     * @brief Clear all stored resources
     */
    void clear();
    
    /**
     * @brief Clear resources of a specific type
     * @param type The resource type to clear
     */
    void clearType(ResourceType type);
    
    /**
     * @brief Clear resources from a specific tier
     * @param tier The resource tier to clear
     */
    void clearTier(ResourceTier tier);
    
    /**
     * @brief Remove a specific resource by path
     * @param path Absolute file path
     * @return true if resource was found and removed
     */
    bool removeByPath(const QString& path);

signals:
    /**
     * @brief Emitted when a resource is added
     * @param resource The added resource
     */
    void resourceAdded(const DiscoveredResource& resource);
    
    /**
     * @brief Emitted when resources are cleared
     * @param type The type that was cleared, or Unknown if all were cleared
     */
    void resourcesCleared(ResourceType type);
    
    /**
     * @brief Emitted when a resource is removed
     * @param path The path of the removed resource
     */
    void resourceRemoved(const QString& path);

private:
    /// Storage vectors indexed by ResourceType
    QHash<ResourceType, QVector<DiscoveredResource>> m_resources;
    
    /// Path to resource lookup for findByPath
    QHash<QString, ResourceType> m_pathIndex;
    
    /// Thread safety lock
    mutable QReadWriteLock m_lock;
};

} // namespace resInventory

#endif // RESOURCESTORE_H
