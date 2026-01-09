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
#include <QCommandLineParser>
#include <iostream>

// These will be set from command line arguments
static QString appname = "OpenSCAD";
static QString companyname = "";
static QString startingDir = "D:/repositories";

void findAppExecutable()
{
    std::cout << "\n========================================\n";
    std::cout << "Finding " << appname.toStdString() << " in System PATH\n";
    std::cout << "========================================\n";
    
    // Show PATH environment variable
    QString pathEnv = qEnvironmentVariable("PATH");
    QStringList pathDirs = pathEnv.split(';', Qt::SkipEmptyParts);
    std::cout << "\nPATH contains " << pathDirs.size() << " directories:\n";
    for (int i = 0; i < pathDirs.size() && i < 10; ++i) {
        std::cout << "  [" << (i+1) << "] " << pathDirs[i].toStdString() << "\n";
    }
    if (pathDirs.size() > 10) {
        std::cout << "  ... (" << (pathDirs.size() - 10) << " more)\n";
    }
    std::cout << "\n";
    
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

void findInApplicationsLocation()
{
    std::cout << "\n========================================\n";
    std::cout << "Finding " << appname.toStdString() << " in ApplicationsLocation\n";
    std::cout << "========================================\n";
    
    // Get all ApplicationsLocation paths from QStandardPaths
    QStringList appLocations = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    
    std::cout << "ApplicationsLocation paths (" << appLocations.size() << "):\n";
    for (const QString& path : appLocations) {
        QFileInfo info(path);
        std::cout << "  - " << path.toStdString() 
                  << (info.exists() ? "" : " [NOT FOUND]")
                  << "\n";
    }
    std::cout << "\n";
    
    // Search for executable in ApplicationsLocation paths
    QString result = QStandardPaths::findExecutable(appname, appLocations);
    
    if (result.isEmpty()) {
        std::cout << "❌ " << appname.toStdString() << ".exe not found in ApplicationsLocation paths\n";
        std::cout << "\nNote: ApplicationsLocation is for Start Menu shortcuts on Windows.\n";
        std::cout << "      For actual installations, search Program Files instead.\n";
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
    std::cout << "Finding " << appname.toStdString() << " in Specific Paths (Recursive)\n";
    std::cout << "========================================\n";
    
    std::cout << "Root directories to search:\n";
    for (const QString& path : paths) {
        QFileInfo info(path);
        std::cout << "  - " << path.toStdString() 
                  << (info.exists() ? "" : " [NOT FOUND]")
                  << "\n";
    }
    std::cout << "\n";
    
    // First try non-recursive (QStandardPaths::findExecutable doesn't recurse)
    QString result = QStandardPaths::findExecutable(appname, paths);
    
    if (!result.isEmpty()) {
        std::cout << "✅ Found (direct search): " << result.toStdString() << "\n";
        return;
    }
    
    // Now do recursive search
    std::cout << "Not found in root directories. Searching recursively...\n\n";
    
    QString exeName = appname;
    if (!exeName.endsWith(".exe", Qt::CaseInsensitive)) {
        exeName += ".exe";
    }
    
    using F = QDirListing::IteratorFlag;
    int scanned = 0;
    
    for (const QString& rootPath : paths) {
        if (!QDir(rootPath).exists()) continue;
        
        std::cout << "Scanning " << rootPath.toStdString() << "...\n";
        
        for (const auto& dirEntry : QDirListing(rootPath, F::FilesOnly | F::Recursive)) {
            scanned++;
            if (scanned % 500 == 0) {
                std::cout << "\rScanned " << scanned << " files..." << std::flush;
            }
            
            QFileInfo info(dirEntry.filePath());
            
            // Check if filename matches (case-insensitive)
            if (info.fileName().compare(exeName, Qt::CaseInsensitive) == 0) {
                std::cout << "\r\n✅ Found: " << info.absoluteFilePath().toStdString() << "\n";
                std::cout << "   Size: " << info.size() << " bytes\n";
                std::cout << "   Executable: " << (info.isExecutable() ? "Yes" : "No") << "\n";
                std::cout << "   Total files scanned: " << scanned << "\n";
                return;
            }
        }
    }
    
    std::cout << "\r❌ " << appname.toStdString() << ".exe not found after scanning " 
              << scanned << " files\n";
}

void findOpenSCADDirectories(const QString& rootPath)
{
    std::cout << "\n========================================\n";
    std::cout << "Finding Directories Named " << appname.toStdString() << "\n";
    std::cout << "========================================\n";
    
    QFileInfo rootInfo(rootPath);
    std::cout << "Root directory: " << rootPath.toStdString() << "\n";
    std::cout << "Exists: " << (rootInfo.exists() ? "Yes" : "No") << "\n";
    std::cout << "Readable: " << (rootInfo.isReadable() ? "Yes" : "No") << "\n";
    std::cout << "\n(This may take a while...)\n\n";
    
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
    std::cout << "Searching for " << appname.toStdString() << " in found paths...\n";
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
    app.setApplicationName("FindExecutableDemo");
    app.setApplicationVersion("1.0");
    
    // Setup command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription("Demo for finding executables using QStandardPaths");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // Add options
    QCommandLineOption appNameOption(QStringList() << "a" << "app",
        "Application name to search for (e.g., OpenSCAD, notepad)",
        "appname",
        "OpenSCAD");
    parser.addOption(appNameOption);
    
    QCommandLineOption companyOption(QStringList() << "c" << "company",
        "Company/organization name (optional)",
        "companyname",
        "");
    parser.addOption(companyOption);
    
    QCommandLineOption startDirOption(QStringList() << "d" << "dir",
        "Starting directory for recursive search",
        "directory",
        "D:/repositories");
    parser.addOption(startDirOption);
    
    // Process command line
    parser.process(app);
    
    // Get values from command line
    appname = parser.value(appNameOption);
    companyname = parser.value(companyOption);
    startingDir = parser.value(startDirOption);
    
    // Set organization name if provided
    if (!companyname.isEmpty()) {
        app.setOrganizationName(companyname);
    }
    
    std::cout << "===========================================\n";
    std::cout << "QStandardPaths::findExecutable() Demo\n";
    std::cout << "===========================================\n";
    std::cout << "Application: " << appname.toStdString() << "\n";
    if (!companyname.isEmpty()) {
        std::cout << "Company:     " << companyname.toStdString() << "\n";
    }
    std::cout << "Search Dir:  " << startingDir.toStdString() << "\n";
    std::cout << "\n";
    
    // 1. Find appname in system PATH
    findAppExecutable();
    
    // 2. Find in ApplicationsLocation (Start Menu on Windows)
    findInApplicationsLocation();
    
    // 3. Find in Program Files (common installation location)
    QStringList programFilesPaths = {
        "C:/Program Files",
        "C:/Program Files (x86)",
        "C:/bin"
    };
    findAppExecutableInPaths(programFilesPaths);
    
    // 4. Find directories named appname in starting directory
    findOpenSCADDirectories(startingDir);
    
    // 5. Find build/bin folders in starting directory
    findBinFoldersInRepositories(startingDir);
    
    std::cout << "\n===========================================\n";
    std::cout << "Demo Complete\n";
    std::cout << "===========================================\n\n";
    std::cout << "Usage examples:\n";
    std::cout << "  " << argv[0] << " --app OpenSCAD\n";
    std::cout << "  " << argv[0] << " --app notepad --dir C:/Windows\n";
    std::cout << "  " << argv[0] << " -a myapp -c mycompany -d D:/Projects\n";
    std::cout << "\n";
    
    return 0;
}
