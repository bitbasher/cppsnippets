/**
 * @file findexecutable_demo.cpp
 * @brief Demonstrates QStandardPaths::findExecutable() method
 * 
 * Note: findExecutable() is for EXECUTABLES (.exe files on Windows),
 * NOT for directories. To find directories, use QDir or QDirListing.
 * 
 * This demo shows:
 * 1. Finding appname in system PATH
 * 2. Searching for directories named appname using QDirListing
 */

#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QDirListing>
#include <QFileInfo>
#include <iostream>

const auto appname = QString("OpenSCAD");

void findAppExecutable()
{
    std::cout << "\n========================================\n";
    std::cout << "Finding appname in System PATH\n";
    std::cout << "========================================\n";
    
    // findExecutable() looks for .exe files in PATH
    QString result = QStandardPaths::findExecutable(appname);
    
    if (result.isEmpty()) {
        std::cout << "❌ " << appname.toStdString() << " not found in system PATH\n";
    } else {
        std::cout << "✅ Found: " << result.toStdString() << "\n";
        
        QFileInfo info(result);
        std::cout << "   Size: " << info.size() << " bytes\n";
        std::cout << "   Executable: " << (info.isExecutable() ? "Yes" : "No") << "\n";
    }
}

void findAppExecutableInPaths(const QStringList& paths)
{
    std::cout << "\n========================================\n";
    std::cout << "Finding an executable in Specific Paths\n";
    std::cout << "========================================\n";
    
    std::cout << "Searching in:\n";
    for (const QString& path : paths) {
        std::cout << "  - " << path.toStdString() << "\n";
    }
    std::cout << "\n";
    
    QString result = QStandardPaths::findExecutable(appname, paths);
    
    if (result.isEmpty()) {
        std::cout << "❌ " << appname.toStdString() << ".exe not found in specified paths\n";
    } else {
        std::cout << "✅ Found: " << result.toStdString() << "\n";
    }
}

void findOpenSCADDirectories(const QString& rootPath)
{
    std::cout << "\n========================================\n";
    std::cout << "Finding Directories Named appname\n";
    std::cout << "========================================\n";
    std::cout << "Searching in: " << rootPath.toStdString() << "\n";
    std::cout << "(This may take a while...)\n\n";
    
    if (!QDir(rootPath).exists()) {
        std::cout << "❌ Root path does not exist\n";
        return;
    }
    
    QStringList foundDirs;
    
    // Use QDirListing to recursively search (modern Qt 6 API)
    // DirsOnly excludes files and other entries automatically
    using F = QDirListing::IteratorFlag;
    int scanned = 0;
    for (const auto& dirEntry : QDirListing(rootPath, F::DirsOnly | F::Recursive)) {
        scanned++;
        if (scanned % 100 == 0) {
            std::cout << "\rScanned " << scanned << " directories..." << std::flush;
        }
        
        QFileInfo info(dirEntry.filePath());
        
        // Check if directory name matches appname (case-insensitive)
        if (info.fileName().compare(appname, Qt::CaseInsensitive) == 0) {
            foundDirs.append(dirEntry.filePath());
        }
    }
    
    std::cout << "\rScanned " << scanned << " directories total.\n\n";
    
    if (foundDirs.isEmpty()) {
        std::cout << "❌ No '" << appname.toStdString() << "' directories found\n";
    } else {
        std::cout << "✅ Found " << foundDirs.size() << " '" << appname.toStdString() << "' director(ies):\n\n";
        
        for (const QString& dir : foundDirs) {
            QFileInfo info(dir);
            std::cout << "  - " << dir.toStdString() 
                      << " [R:" << (info.isReadable() ? "Y" : "N") 
                      << " W:" << (info.isWritable() ? "Y" : "N") << "]\n";
        }
    }
}

void findBinFoldersInRepositories(const QString& reposPath)
{
    std::cout << "\n========================================\n";
    std::cout << "Finding build/bin/* Folders\n";
    std::cout << "========================================\n";
    std::cout << "Searching in: " << reposPath.toStdString() << "\n\n";
    
    if (!QDir(reposPath).exists()) {
        std::cout << "❌ Repositories path does not exist\n";
        return;
    }
    
    QStringList binPaths;
    
    // Look for build/bin/Debug and build/bin/Release patterns using QDirListing
    // DirsOnly excludes files and other entries automatically
    using F = QDirListing::IteratorFlag;
    int scanned = 0;
    for (const auto& dirEntry : QDirListing(reposPath, F::DirsOnly | F::Recursive)) {
        QString path = QDir::toNativeSeparators(dirEntry.filePath());
        scanned++;
        
        if (scanned % 100 == 0) {
            std::cout << "\rScanned " << scanned << " directories..." << std::flush;
        }
        
        // Check if path matches build\bin\Debug or build\bin\Release (Windows native separators)
        if (path.contains("\\build\\bin\\Debug", Qt::CaseInsensitive) ||
            path.contains("\\build\\bin\\Release", Qt::CaseInsensitive)) {
            binPaths.append(path);
        }
    }
    
    std::cout << "\rScanned " << scanned << " directories total.\n\n";
    
    if (binPaths.isEmpty()) {
        std::cout << "❌ No build/bin/* folders found\n";
    } else {
        std::cout << "✅ Found " << binPaths.size() << " build/bin/* folder(s):\n\n";
        
        for (const QString& binPath : binPaths) {
            std::cout << "  - " << binPath.toStdString() << "\n";
        }
    }
    
    // Now try finding appname in these paths
    std::cout << "\n----------------------------------------\n";
    std::cout << "Searching forappname in found paths...\n";
    std::cout << "----------------------------------------\n";
    
    if (!binPaths.isEmpty()) {
        QString result = QStandardPaths::findExecutable(appname, binPaths);
        if (!result.isEmpty()) {
            std::cout << "✅ Found: " << result.toStdString() << "\n";
        } else {
            std::cout << "❌ " << appname.toStdString() << ".exe not found in any bin folders\n";
        }
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    std::cout << "===========================================\n";
    std::cout << "QStandardPaths::findExecutable() Demo\n";
    std::cout << "===========================================\n\n";
    
    // 1. Find appname in system PATH
    findAppExecutable();
    
    // 2. Find in Program Files (common installation location)
    QStringList programFilesPaths = {
        "C:/Program Files",
        "C:/bin"
    };
    findAppExecutableInPaths(programFilesPaths);
    
    // 3. Find directories named appname in D:/repositories
    QString reposPath = "D:/repositories";
    findOpenSCADDirectories(reposPath);
    
    // 4. Find build/bin folders in D:/repositories
    findBinFoldersInRepositories(reposPath);
    
    std::cout << "\n===========================================\n";
    std::cout << "Demo Complete\n";
    std::cout << "===========================================\n\n";
    
    return 0;
}
