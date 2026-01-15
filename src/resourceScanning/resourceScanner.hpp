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
     * @brief Scan all locations and populate QStandardItemModel
     * 
     * Phase 3-4: Scans examples/ and templates/ folders
     * Future: Will scan all resource types
     * 
     * @param model Qt model to populate with discovered resources
     * @param locations List of discovered resource locations
     * @return true on success, false on error
     */
    bool scanToModel(QStandardItemModel* model, const QList<platformInfo::ResourceLocation>& locations);
    
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
     * @brief Populate QStandardItemModel from inventories
     * 
     * Phase 3-4: Populates from examples and templates
     * Future: Will populate from all resource inventories
     * 
     * @param model Qt model to populate
     */
    void populateModel(QStandardItemModel* model);
    
    // Inventory storage - one per resource type
    resourceInventory::ExamplesInventory m_examplesInventory;
    resourceInventory::TemplatesInventory m_templatesInventory;
    // Future: LibrariesInventory, FontsInventory, etc.
};

} // namespace resourceScanning
