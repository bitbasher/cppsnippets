/**
 * @file ResourceScanner.hpp
 * @brief Top-level resource scanner that orchestrates inventory population
 * 
 * Consumes list of discovered ResourceLocations, checks what resource folders
 * exist at each location, and delegates to inventory classes for population.
 * 
 * Phase 3 implementation - Examples only
 * Future: Templates, Libraries, Fonts, etc.
 */

#pragma once

#include "../platformInfo/export.hpp"
#include "../platformInfo/ResourceLocation.hpp"
#include "../resourceInventory/ExamplesInventory.hpp"
#include "../resourceInventory/TemplatesInventory.hpp"
#include "../resourceInventory/FontsInventory.hpp"
#include "../resourceInventory/ShadersInventory.hpp"
#include "../resourceInventory/TranslationsInventory.hpp"
#include "../resourceInventory/TestsInventory.hpp"

#include <QStandardItemModel>
#include <QList>

namespace resourceScanning {

/**
 * @brief Orchestrates resource scanning across all discovered locations
 * 
 * Takes list of ResourceLocations from path discovery.
 * For each location, checks what resource folders exist (examples/, templates/, etc.)
 * Delegates to specialized inventory classes to populate storage.
 * 
 * Phase 3: Examples scanning
 * Phase 4: Templates scanning
 * Future phases: Libraries, Fonts, ColorSchemes, etc.
 */
class PLATFORMINFO_API ResourceScanner {
public:
    ResourceScanner() = default;
    ~ResourceScanner() = default;
    
    /**
     * @brief Scan all locations and create populated QStandardItemModel
     * 
     * Phase 3-4: Scans examples/ and templates/ folders
     * Future: Will scan all resource types
     * 
     * @param locations List of discovered resource locations
     * @return Populated model with discovered resources (caller owns)
     */
    QStandardItemModel* scanToModel(const QList<platformInfo::ResourceLocation>& locations);
    
    /**
     * @brief Get the examples inventory (read-only access)
     * @return Reference to populated examples inventory
     */
    const resourceInventory::ExamplesInventory& examplesInventory() const { return m_examplesInventory; }
    
    /**
     * @brief Get the templates inventory (read-only access)
     * @return Reference to populated templates inventory
     */
    const resourceInventory::TemplatesInventory& templatesInventory() const { return m_templatesInventory; }
    
    /**
     * @brief Get count of examples found across all locations
     * @return Total number of examples
     */
    int examplesCount() const { return m_examplesInventory.count(); }
    
    /**
     * @brief Get count of templates found across all locations
     * @return Total number of templates
     */
    int templatesCount() const { return m_templatesInventory.count(); }
    
    /**
     * @brief Get the fonts inventory (read-only access)
     * @return Reference to populated fonts inventory
     */
    const resourceInventory::FontsInventory& fontsInventory() const { return m_fontsInventory; }
    
    /**
     * @brief Get count of fonts found across all locations
     * @return Total number of fonts
     */
    int fontsCount() const { return m_fontsInventory.count(); }
    
    /**
     * @brief Get the shaders inventory (read-only access)
     * @return Reference to populated shaders inventory
     */
    const resourceInventory::ShadersInventory& shadersInventory() const { return m_shadersInventory; }
    
    /**
     * @brief Get count of shaders found across all locations
     * @return Total number of shaders
     */
    int shadersCount() const { return m_shadersInventory.count(); }
    
    /**
     * @brief Get the translations inventory (read-only access)
     * @return Reference to populated translations inventory
     */
    const resourceInventory::TranslationsInventory& translationsInventory() const { return m_translationsInventory; }
    
    /**
     * @brief Get count of translations found across all locations
     * @return Total number of translations
     */
    int translationsCount() const { return m_translationsInventory.count(); }
    
    /**
     * @brief Get the tests inventory (read-only access)
     * @return Reference to populated tests inventory
     */
    const resourceInventory::TestsInventory& testsInventory() const { return m_testsInventory; }
    
    /**
     * @brief Get count of tests found across all locations
     * @return Total number of tests
     */
    int testsCount() const { return m_testsInventory.count(); }

private:
    /**
     * @brief Scan examples folder at a single location
     * 
     * Uses QDirListing to traverse examples/ folder.
     * Calls inventory.addFolder() for directories (categories).
     * Calls inventory.addExample() for loose .scad files.
     * 
     * @param location Resource location to scan
     * @return Number of examples added, or -1 on error
     */
    int scanExamplesAt(const platformInfo::ResourceLocation& location);
    
    /**
     * @brief Scan templates folder at a single location
     * 
     * Uses QDirListing to traverse templates/ folder.
     * Calls inventory.addFolder() for directories (categories).
     * Calls inventory.addTemplate() for loose .scad files.
     * 
     * @param location Resource location to scan
     * @return Number of templates added, or -1 on error
     */
    int scanTemplatesAt(const platformInfo::ResourceLocation& location);
    
    /**
     * @brief Scan fonts folder at a single location
     * 
     * Uses QDirListing to find .ttf and .otf files.
     * Simple file-based scan (no metadata or attachments).
     * 
     * @param location Resource location to scan
     * @return Number of fonts added, or -1 on error
     */
    int scanFontsAt(const platformInfo::ResourceLocation& location);
    
    /**
     * @brief Scan shaders folder at a single location
     * 
     * Uses QDirListing to find .frag and .vert files.
     * Simple file-based scan (no metadata or attachments).
     * 
     * @param location Resource location to scan
     * @return Number of shaders added, or -1 on error
     */
    int scanShadersAt(const platformInfo::ResourceLocation& location);
    
    /**
     * @brief Scan locale folder at a single location
     * 
     * Uses QDirListing to find .qm and .ts files.
     * Simple file-based scan (no metadata or attachments).
     * 
     * @param location Resource location to scan
     * @return Number of translations added, or -1 on error
     */
    int scanTranslationsAt(const platformInfo::ResourceLocation& location);
    
    /**
     * @brief Scan tests folder at a single location
     * 
     * Similar to examples but flat (no categories).
     * Tests are .scad files that may have attachments (.json, .txt, .dat).
     * 
     * @param location Resource location to scan
     * @return Number of tests added, or -1 on error
     */
    int scanTestsAt(const platformInfo::ResourceLocation& location);
    
    /**
     * @brief Populate QStandardItemModel for a specific resource type
     * 
     * Creates and populates model from the specified inventory.
     * Returns nullptr if that inventory is empty.
     * 
     * @param type Resource type to create model for
     * @return Populated model, or nullptr if inventory empty
     */
    QStandardItemModel* populateModel(resourceMetadata::ResourceType type);
    
public:
    // ========================================================================
    // Location Index Management (Shared across all inventories)
    // ========================================================================
    
    /**
     * @brief Normalize path to safe format (forward slashes, no escape issues)
     * 
     * Converts backslashes to forward slashes for storage.
     * Windows handles both / and \ in paths, so this is safe.
     * Avoids escape character problems in C++ string literals.
     * 
     * @param path Path to normalize
     * @return Normalized path with forward slashes
     */
    static QString normalizePath(const QString& path);
    
    /**
     * @brief Get or create location index for a template folder path
     * 
     * Maintains bidirectional mapping between folder paths and short indices.
     * Generates indices like "aaa", "aab", "aac", ... "aaz", "aba", etc.
     * Path is automatically normalized before indexing.
     * Thread-safe for concurrent access during scanning.
     * 
     * @param folderPath Absolute path to templates folder (will be normalized)
     * @return Location index (e.g., "aaa", "aab")
     */
    static QString getOrCreateLocationIndex(const QString& folderPath);
    
    /**
     * @brief Get folder path for a location index
     * @param index Location index (e.g., "aaa", "aab")
     * @return Absolute path or empty if index not found
     */
    static QString getLocationPath(const QString& index);
    
    /**
     * @brief Get count of indexed locations
     * @return Number of unique template folder locations
     */
    static int locationIndexCount();
    
private:
    // Location index storage (shared across all ResourceScanner instances)
    static QHash<QString, QString> s_pathToLocationIndex;   // path -> "aaa"
    static QHash<QString, QString> s_locationIndexToPath;   // "aaa" -> path
    static int s_nextLocationIndex;                         // Counter for next index
    
    /**
     * @brief Generate location index from counter
     * 
     * Converts number to 3-letter index:
     * 1 → "aaa", 2 → "aab", 3 → "aac", ..., 26 → "aaz",
     * 27 → "aba", 28 → "abb", ..., 702 → "azz",
     * 703 → "baa", etc.
     * 
     * Supports up to 17,576 unique locations (26³).
     * 
     * @param index Numeric index (1-based)
     * @return Three-letter location index
     */
    static QString numberToLocationIndex(int index);
    
    // Inventory storage - one per resource type
    resourceInventory::ExamplesInventory m_examplesInventory;
    resourceInventory::TemplatesInventory m_templatesInventory;
    resourceInventory::FontsInventory m_fontsInventory;
    resourceInventory::ShadersInventory m_shadersInventory;
    resourceInventory::TranslationsInventory m_translationsInventory;
    resourceInventory::TestsInventory m_testsInventory;
    // Future: LibrariesInventory, ColorSchemesInventory, etc.
};

} // namespace resourceScanning
