/**
 * @file settings_generator.cpp
 * @brief Utility to populate QSettings with test user-designated resource paths
 * 
 * This tool writes test paths to QSettings that the main application can read.
 * Used for testing sibling installation and user-designated path discovery.
 */

#include <QCoreApplication>
#include <QSettings>
#include <QStringList>
#include <QString>
#include <iostream>

void printUsage() {
    std::cout << "\n=== Settings Generator for Resource Paths ===\n\n";
    std::cout << "Usage:\n";
    std::cout << "  settings-generator [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --add <path>     Add a user-designated path\n";
    std::cout << "  --clear          Clear all user-designated paths\n";
    std::cout << "  --list           List current user-designated paths\n";
    std::cout << "  --default        Add some default test paths\n";
    std::cout << "  --help           Show this help message\n\n";
    std::cout << "Examples:\n";
    std::cout << "  settings-generator --add \"C:/CustomOpenSCAD\"\n";
    std::cout << "  settings-generator --add \"%USERPROFILE%/MyTemplates\"\n";
    std::cout << "  settings-generator --default\n";
    std::cout << "  settings-generator --list\n";
    std::cout << "  settings-generator --clear\n\n";
}

void listPaths() {
    QSettings settings(QStringLiteral("ScadTemplates"), QStringLiteral("ResourcePaths"));
    QStringList paths = settings.value(QStringLiteral("user_designated_paths"), QStringList()).toStringList();
    
    std::cout << "\n=== Current User-Designated Paths ===\n\n";
    
    if (paths.isEmpty()) {
        std::cout << "(No paths configured)\n\n";
    } else {
        for (int i = 0; i < paths.size(); ++i) {
            std::cout << "  [" << (i + 1) << "] " << paths[i].toStdString() << "\n";
        }
        std::cout << "\nTotal: " << paths.size() << " path(s)\n\n";
    }
    
    std::cout << "Settings stored at: " << settings.fileName().toStdString() << "\n\n";
}

void clearPaths() {
    QSettings settings(QStringLiteral("ScadTemplates"), QStringLiteral("ResourcePaths"));
    settings.remove(QStringLiteral("user_designated_paths"));
    settings.sync();
    
    std::cout << "\n✓ All user-designated paths cleared\n\n";
}

void addPath(const QString& path) {
    QSettings settings(QStringLiteral("ScadTemplates"), QStringLiteral("ResourcePaths"));
    QStringList paths = settings.value(QStringLiteral("user_designated_paths"), QStringList()).toStringList();
    
    if (!paths.contains(path)) {
        paths.append(path);
        settings.setValue(QStringLiteral("user_designated_paths"), paths);
        settings.sync();
        
        std::cout << "\n✓ Added path: " << path.toStdString() << "\n\n";
    } else {
        std::cout << "\n⚠ Path already exists: " << path.toStdString() << "\n\n";
    }
}

void addDefaultPaths() {
    std::cout << "\n=== Adding Default Test Paths ===\n\n";
    
#if defined(Q_OS_WIN) || defined(_WIN32) || defined(_WIN64)
    QStringList defaults = {
        QStringLiteral("C:/CustomScad"),
        QStringLiteral("%USERPROFILE%/Documents/MyTemplates"),
        QStringLiteral("D:/ProjectResources/ScadLibs")
    };
    std::cout << "Platform: Windows\n\n";
#elif defined(Q_OS_MACOS) || defined(__APPLE__)
    QStringList defaults = {
        QStringLiteral("/Applications/CustomSCAD"),
        QStringLiteral("${HOME}/Documents/MyTemplates"),
        QStringLiteral("/Volumes/External/ScadLibs")
    };
    std::cout << "Platform: macOS\n\n";
#else // Linux
    QStringList defaults = {
        QStringLiteral("/opt/customscad"),
        QStringLiteral("${HOME}/scad-templates"),
        QStringLiteral("/usr/local/custom-scad")
    };
    std::cout << "Platform: Linux\n\n";
#endif
    
    for (const QString& path : defaults) {
        addPath(path);
    }
    
    std::cout << "Default test paths added. Use --list to see them.\n\n";
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    QStringList args = QCoreApplication::arguments();
    
    // Remove program name
    if (!args.isEmpty()) {
        args.removeFirst();
    }
    
    // No arguments - show help
    if (args.isEmpty()) {
        printUsage();
        return 0;
    }
    
    // Process arguments
    for (int i = 0; i < args.size(); ++i) {
        const QString& arg = args[i];
        
        if (arg == QStringLiteral("--help") || arg == QStringLiteral("-h")) {
            printUsage();
            return 0;
        }
        else if (arg == QStringLiteral("--list") || arg == QStringLiteral("-l")) {
            listPaths();
        }
        else if (arg == QStringLiteral("--clear") || arg == QStringLiteral("-c")) {
            clearPaths();
        }
        else if (arg == QStringLiteral("--default") || arg == QStringLiteral("-d")) {
            addDefaultPaths();
        }
        else if (arg == QStringLiteral("--add") || arg == QStringLiteral("-a")) {
            if (i + 1 < args.size()) {
                addPath(args[++i]);
            } else {
                std::cerr << "Error: --add requires a path argument\n";
                return 1;
            }
        }
        else {
            std::cerr << "Unknown option: " << arg.toStdString() << "\n";
            std::cerr << "Use --help for usage information\n";
            return 1;
        }
    }
    
    return 0;
}
