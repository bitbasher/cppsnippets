/**
 * @file TestsInventory.cpp
 * @brief Implementation of TestsInventory
 */

#include "TestsInventory.hpp"
#include "../resourceMetadata/ResourceTier.hpp"

#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QDebug>

namespace resourceInventory {

bool TestsInventory::addTest(const QString& scriptPath, const QString& tier) {
    if (scriptPath.isEmpty()) {
        return false;
    }
    
    QFileInfo scriptInfo(scriptPath);
    if (!scriptInfo.exists() || !scriptInfo.isFile()) {
        qWarning() << "TestsInventory: Test file does not exist:" << scriptPath;
        return false;
    }
    
    // Verify it's a .scad file
    if (scriptInfo.suffix().toLower() != "scad") {
        qWarning() << "TestsInventory: Not a .scad file:" << scriptPath;
        return false;
    }
    
    // Use absolute path as key
    QString key = scriptInfo.absoluteFilePath();
    
    // Skip if already added
    if (m_tests.contains(key)) {
        return false;
    }
    
    // Create ResourceScript
    ResourceScript test(scriptPath);
    
    // Set tier
    resourceMetadata::ResourceTier testTier = resourceMetadata::stringToTier(tier);
    test.setTier(testTier);
    
    // No category for tests (flat structure)
    test.setCategory("");
    
    // Find attachments in same directory
    QDir testDir = scriptInfo.dir();
    QString baseName = scriptInfo.completeBaseName(); // filename without extension
    
    QStringList attachments;
    QStringList filters;
    filters << (baseName + ".json") << (baseName + ".txt") << (baseName + ".dat")
            << (baseName + ".png") << (baseName + ".stl") << (baseName + ".dxf");
    
    for (const QString& filter : filters) {
        QString attachmentPath = testDir.absoluteFilePath(filter);
        if (QFileInfo::exists(attachmentPath)) {
            attachments.append(attachmentPath);
        }
    }
    
    if (!attachments.isEmpty()) {
        test.setAttachments(attachments);
    }
    
    // Store in QVariant
    m_tests.insert(key, QVariant::fromValue(test));
    
    return true;
}

QList<QVariant> TestsInventory::getAll() const {
    return m_tests.values();
}

} // namespace resourceInventory
