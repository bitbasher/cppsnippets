/**
 * @file TestsInventory.hpp
 * @brief Test scripts inventory using QVariant storage
 * 
 * Similar to Examples but without category folders
 * Test files: .scad with potential attachments (.json, .txt, .dat)
 */

#pragma once

#include "../platformInfo/export.hpp"
#include "resourceItem.hpp"

#include <QHash>
#include <QString>
#include <QVariant>
#include <QList>

namespace resourceInventory {

/**
 * @brief Inventory for test script resources
 * 
 * Stores tests as ResourceScript in QVariant containers.
 * Tests are flat (no categories) but may have attachments.
 * Key: Absolute file path (unique identifier)
 */
class PLATFORMINFO_API TestsInventory {
public:
    TestsInventory() = default;
    ~TestsInventory() = default;
    
    /**
     * @brief Add a test script to inventory
     * @param scriptPath Absolute path to .scad test file
     * @param tier Resource tier string ("Installation", "Machine", "User")
     * @return true if added, false if already exists or invalid
     */
    bool addTest(const QString& scriptPath, const QString& tier);
    
    /**
     * @brief Get all tests as QVariant list
     * @return List of QVariants containing ResourceScript objects
     */
    QList<QVariant> getAll() const;
    
    /**
     * @brief Get total count of tests
     * @return Number of tests in inventory
     */
    int count() const { return m_tests.size(); }
    
    /**
     * @brief Clear all tests from inventory
     */
    void clear() { m_tests.clear(); }
    
private:
    // Test storage: Key = absolute path, Value = QVariant(ResourceScript)
    QHash<QString, QVariant> m_tests;
};

} // namespace resourceInventory
