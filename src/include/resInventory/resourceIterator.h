#ifndef RESOURCEITERATOR_H
#define RESOURCEITERATOR_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <memory>

#include "platformInfo/resourcePaths.h"
#include "resInventory/resLocMap.h"
#include "resInventory/resLocTree.h"

namespace resInventory {

/**
 * @brief Enumeration of resource tiers
 * 
 * Resources are organized into three tiers based on their scope and accessibility:
 * - Installation: Built-in resources from the application installation
 * - Machine: System-wide resources available to all users
 * - User: Personal resources in the user's home directory
 */
enum class ResourceTier {
    Installation,   ///< Built-in resources from application installation
    Machine,        ///< System-wide resources for all users
    User            ///< Personal user resources
};

/**
 * @brief Abstract base class for resource iterators
 * 
 * Provides the interface for iterating over resource locations and
 * collecting found resources into appropriate storage structures.
 * 
 * Subclasses implement either flat (ResLocMap) or hierarchical (ResLocTree)
 * iteration strategies depending on the resource type.
 * 
 * @see ResourceIteratorFlat for simple resources (fonts, color-schemes, etc.)
 * @see ResourceIteratorTree for hierarchical resources (libraries with examples/tests)
 */
class ResourceIteratorBase {
public:
    /**
     * @brief Virtual destructor for proper cleanup of derived classes
     */
    virtual ~ResourceIteratorBase() = default;
    
    /**
     * @brief Scan the configured folders for resources
     * 
     * Iterates through all folder locations, searching for subdirectories
     * matching the configured resource types. Found resources are collected
     * into the appropriate storage structure.
     * 
     * @return true if at least one resource was found, false otherwise
     */
    virtual bool scan() = 0;
    
    /**
     * @brief Check if this iterator produces flat (map) results
     * @return true if results are stored in a ResLocMap
     */
    virtual bool isFlat() const = 0;
    
    /**
     * @brief Check if this iterator produces hierarchical (tree) results
     * @return true if results are stored in a ResLocTree
     */
    virtual bool isHierarchical() const = 0;
    
    /**
     * @brief Get the resource tier this iterator is scanning
     * @return The ResourceTier (Installation, Machine, or User)
     */
    ResourceTier tier() const { return m_tier; }
    
    /**
     * @brief Get the folder locations being scanned
     * @return List of absolute folder paths to scan
     */
    QStringList folderLocations() const { return m_folderLocations; }
    
    /**
     * @brief Get the resource types being searched for
     * @return Vector of ResourceType enums to look for
     */
    QVector<platformInfo::ResourceType> resourceTypes() const { return m_resourceTypes; }

protected:
    /**
     * @brief Protected constructor for use by derived classes
     * 
     * @param tier The resource tier (Installation, Machine, User)
     * @param folderLocations List of absolute folder paths to scan
     * @param resourceTypes Vector of resource types to search for
     */
    ResourceIteratorBase(ResourceTier tier,
                         const QStringList& folderLocations,
                         const QVector<platformInfo::ResourceType>& resourceTypes);
    
    ResourceTier m_tier;                                    ///< The resource tier being scanned
    QStringList m_folderLocations;                          ///< Folder paths to scan
    QVector<platformInfo::ResourceType> m_resourceTypes;    ///< Resource types to find
};

/**
 * @brief Iterator for flat/simple resources
 * 
 * Scans folder locations for resources that have a simple flat structure,
 * such as fonts, color schemes, shaders, templates, and translations.
 * 
 * Results are collected into a ResLocMap organized by tier.
 * 
 * @par Example Usage:
 * @code
 * QStringList folders = {"/usr/share/openscad", "/opt/openscad"};
 * QVector<ResourceType> types = {ResourceType::Fonts, ResourceType::ColorSchemes};
 * 
 * ResourceIteratorFlat iterator(ResourceTier::Machine, folders, types);
 * if (iterator.scan()) {
 *     ResLocMap& results = iterator.results();
 *     // Process found resources...
 * }
 * @endcode
 */
class ResourceIteratorFlat : public ResourceIteratorBase {
public:
    /**
     * @brief Construct a flat resource iterator
     * 
     * @param tier The resource tier (Installation, Machine, User)
     * @param folderLocations List of absolute folder paths to scan
     * @param resourceTypes Vector of resource types to search for
     * 
     * @note Resource types that are hierarchical (e.g., Libraries) will
     *       still be scanned but only top-level entries will be collected.
     */
    ResourceIteratorFlat(ResourceTier tier,
                         const QStringList& folderLocations,
                         const QVector<platformInfo::ResourceType>& resourceTypes);
    
    /**
     * @brief Scan folders and collect resources into the internal ResLocMap
     * 
     * For each folder location, checks for subdirectories matching each
     * resource type's subdirectory name. Found directories are added
     * to the appropriate tier list in the ResLocMap.
     * 
     * @return true if at least one resource location was found
     */
    bool scan() override;
    
    /**
     * @brief This iterator produces flat results
     * @return Always returns true
     */
    bool isFlat() const override { return true; }
    
    /**
     * @brief This iterator does not produce hierarchical results
     * @return Always returns false
     */
    bool isHierarchical() const override { return false; }
    
    /**
     * @brief Get the scan results
     * 
     * Returns a reference to the internal ResLocMap containing all
     * found resource locations organized by tier.
     * 
     * @return Reference to the ResLocMap with scan results
     * 
     * @note Call scan() before accessing results
     */
    ResLocMap& results() { return m_results; }
    
    /**
     * @brief Get the scan results (const version)
     * @return Const reference to the ResLocMap with scan results
     */
    const ResLocMap& results() const { return m_results; }

private:
    ResLocMap m_results;    ///< Storage for found resource locations
};

/**
 * @brief Iterator for hierarchical resources
 * 
 * Scans folder locations for resources that have a hierarchical structure,
 * such as libraries that may contain examples, tests, and sub-libraries.
 * 
 * Results are collected into a ResLocTree with parent-child relationships
 * preserved.
 * 
 * @par Example Usage:
 * @code
 * QStringList folders = {"~/.local/share/OpenSCAD/libraries"};
 * QVector<ResourceType> types = {ResourceType::Libraries};
 * 
 * ResourceIteratorTree iterator(ResourceTier::User, folders, types);
 * if (iterator.scan()) {
 *     ResLocTree* tree = iterator.results();
 *     // Navigate the tree structure...
 * }
 * @endcode
 */
class ResourceIteratorTree : public ResourceIteratorBase {
public:
    /**
     * @brief Construct a hierarchical resource iterator
     * 
     * @param tier The resource tier (Installation, Machine, User)
     * @param folderLocations List of absolute folder paths to scan
     * @param resourceTypes Vector of resource types to search for
     * @param parent Optional parent widget for the internal QTreeWidget
     * 
     * @note This iterator is best suited for Libraries which may contain
     *       nested examples, tests, and documentation.
     */
    ResourceIteratorTree(ResourceTier tier,
                         const QStringList& folderLocations,
                         const QVector<platformInfo::ResourceType>& resourceTypes,
                         QWidget* parent = nullptr);
    
    /**
     * @brief Destructor - cleans up the internal tree widget
     */
    ~ResourceIteratorTree() override;
    
    /**
     * @brief Scan folders and collect resources into the internal ResLocTree
     * 
     * For each folder location, recursively searches for subdirectories
     * matching each resource type. Found directories are added to the
     * tree with their hierarchical relationships preserved.
     * 
     * For Libraries, child resources (examples, tests) are added as
     * children of their parent library node.
     * 
     * @return true if at least one resource location was found
     */
    bool scan() override;
    
    /**
     * @brief This iterator does not produce flat results
     * @return Always returns false
     */
    bool isFlat() const override { return false; }
    
    /**
     * @brief This iterator produces hierarchical results
     * @return Always returns true
     */
    bool isHierarchical() const override { return true; }
    
    /**
     * @brief Get the scan results
     * 
     * Returns a pointer to the internal ResLocTree containing all
     * found resource locations in a hierarchical structure.
     * 
     * @return Pointer to the ResLocTree with scan results
     * 
     * @note Call scan() before accessing results
     * @note The tree is owned by this iterator and will be deleted
     *       when the iterator is destroyed. Use takeResults() to
     *       transfer ownership.
     */
    ResLocTree* results() { return m_results; }
    
    /**
     * @brief Get the scan results (const version)
     * @return Const pointer to the ResLocTree with scan results
     */
    const ResLocTree* results() const { return m_results; }
    
    /**
     * @brief Take ownership of the results tree
     * 
     * Transfers ownership of the internal ResLocTree to the caller.
     * After calling this method, results() will return nullptr.
     * 
     * @return Pointer to the ResLocTree (caller takes ownership)
     */
    ResLocTree* takeResults();

private:
    ResLocTree* m_results;  ///< Storage for found resource locations (owned)
    
    /**
     * @brief Recursively scan a directory for child resources
     * 
     * @param parentItem The parent tree item to add children to
     * @param path The directory path to scan
     * @param depth Current recursion depth (to prevent infinite loops)
     */
    void scanDirectory(ResLocTreeItem* parentItem, const QString& path, int depth);
};

/**
 * @brief Factory class for creating appropriate resource iterators
 * 
 * Creates either a flat (ResourceIteratorFlat) or hierarchical
 * (ResourceIteratorTree) iterator based on the resource types requested.
 * 
 * @par Resource Type to Iterator Mapping:
 * 
 * | Resource Type   | Iterator Type |
 * |-----------------|---------------|
 * | Examples        | Flat          |
 * | Tests           | Flat          |
 * | Fonts           | Flat          |
 * | ColorSchemes    | Flat          |
 * | Shaders         | Flat          |
 * | Templates       | Flat          |
 * | Libraries       | Hierarchical  |
 * | Translations    | Flat          |
 * 
 * @par Example Usage:
 * @code
 * // Create iterator for fonts (will be flat)
 * auto fontIter = ResourceIteratorFactory::create(
 *     ResourceTier::User,
 *     userFolders,
 *     {ResourceType::Fonts}
 * );
 * 
 * if (fontIter->isFlat()) {
 *     auto* flatIter = static_cast<ResourceIteratorFlat*>(fontIter.get());
 *     flatIter->scan();
 *     // Use flatIter->results()...
 * }
 * 
 * // Create iterator for libraries (will be hierarchical)
 * auto libIter = ResourceIteratorFactory::create(
 *     ResourceTier::Machine,
 *     machineFolders,
 *     {ResourceType::Libraries}
 * );
 * 
 * if (libIter->isHierarchical()) {
 *     auto* treeIter = static_cast<ResourceIteratorTree*>(libIter.get());
 *     treeIter->scan();
 *     // Use treeIter->results()...
 * }
 * @endcode
 */
class ResourceIteratorFactory {
public:
    /**
     * @brief Create an appropriate iterator for the given resource types
     * 
     * Analyzes the requested resource types and creates either a flat
     * or hierarchical iterator. If the types include Libraries, a
     * hierarchical iterator is created; otherwise, a flat iterator.
     * 
     * @param tier The resource tier (Installation, Machine, User)
     * @param folderLocations List of absolute folder paths to scan
     * @param resourceTypes Vector of resource types to search for
     * @param parent Optional parent widget (used for tree iterators)
     * 
     * @return unique_ptr to the created iterator (ResourceIteratorFlat or ResourceIteratorTree)
     * 
     * @note If resourceTypes contains both flat and hierarchical types,
     *       a hierarchical iterator is created to handle all types.
     */
    static std::unique_ptr<ResourceIteratorBase> create(
        ResourceTier tier,
        const QStringList& folderLocations,
        const QVector<platformInfo::ResourceType>& resourceTypes,
        QWidget* parent = nullptr);
    
    /**
     * @brief Create a flat iterator explicitly
     * 
     * Creates a ResourceIteratorFlat regardless of resource types.
     * Use this when you know you want flat results.
     * 
     * @param tier The resource tier (Installation, Machine, User)
     * @param folderLocations List of absolute folder paths to scan
     * @param resourceTypes Vector of resource types to search for
     * 
     * @return unique_ptr to a ResourceIteratorFlat
     */
    static std::unique_ptr<ResourceIteratorFlat> createFlat(
        ResourceTier tier,
        const QStringList& folderLocations,
        const QVector<platformInfo::ResourceType>& resourceTypes);
    
    /**
     * @brief Create a hierarchical iterator explicitly
     * 
     * Creates a ResourceIteratorTree regardless of resource types.
     * Use this when you need tree structure for any resource type.
     * 
     * @param tier The resource tier (Installation, Machine, User)
     * @param folderLocations List of absolute folder paths to scan
     * @param resourceTypes Vector of resource types to search for
     * @param parent Optional parent widget for the tree
     * 
     * @return unique_ptr to a ResourceIteratorTree
     */
    static std::unique_ptr<ResourceIteratorTree> createTree(
        ResourceTier tier,
        const QStringList& folderLocations,
        const QVector<platformInfo::ResourceType>& resourceTypes,
        QWidget* parent = nullptr);
    
    /**
     * @brief Check if a resource type requires hierarchical iteration
     * 
     * @param type The resource type to check
     * @return true if the type is best represented as a tree (e.g., Libraries)
     */
    static bool isHierarchicalType(platformInfo::ResourceType type);
    
    /**
     * @brief Check if any types in a list require hierarchical iteration
     * 
     * @param types Vector of resource types to check
     * @return true if any type in the list is hierarchical
     */
    static bool containsHierarchicalType(const QVector<platformInfo::ResourceType>& types);

private:
    ResourceIteratorFactory() = delete;  ///< Static factory - no instantiation
};

} // namespace resInventory

#endif // RESOURCEITERATOR_H
