/**
 * @file test_env_expansion.cpp
 * @brief Test utility for environment variable expansion in ResourcePaths
 * 
 * Demonstrates how the ResourcePaths expansion works across platforms including:
 * - Environment variable expansion (${VAR} and %VAR%)
 * - Folder name appending rules (paths ending with "/" get folder name added)
 * - Installation tier suffix handling (e.g., "ScadTemplates (Nightly)")
 * 
 * Usage: test_env_expansion [verbose]
 */

#include <QCoreApplication>
#include <QProcessEnvironment>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <iostream>
#include <iomanip>

// Forward declaration - implementation will link to libscadtemplates
namespace platformInfo {
    class ResourcePaths;
}

/**
 * Minimal standalone expansion function for testing
 * Mirrors ResourcePaths::expandEnvVars() exactly
 */
QString expandEnvVars(const QString& path) {
    if (path.isEmpty()) {
        return path;
    }
    
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    
    // Pattern matches both ${VAR} and %VAR% style environment variable references
    const QRegularExpression pattern(QStringLiteral(R"(\$\{([^}]+)\}|%([^%]+)%)"));
    
    QString result;
    result.reserve(path.size());
    int lastIndex = 0;
    
    // Iterate through all matches and build the expanded string
    for (auto match = pattern.globalMatch(path); match.hasNext(); ) {
        auto m = match.next();
        
        // Append text before the match
        result.append(path.mid(lastIndex, m.capturedStart() - lastIndex));
        
        // Extract variable name from either ${VAR} or %VAR% capture group
        const QString varName = m.captured(1).isEmpty() ? m.captured(2) : m.captured(1);
        
        // Append the expanded value (or empty string if undefined)
        result.append(env.value(varName, QString()));
        
        lastIndex = m.capturedEnd();
    }
    
    // Append any remaining text after the last match
    result.append(path.mid(lastIndex));
    
    // Normalize path separators to forward slashes for consistency
    // This ensures mixed ${VAR}/../relative paths work correctly
    result.replace(QLatin1Char('\\'), QLatin1Char('/'));
    
    // Note: Do NOT cleanPath here - we need to preserve trailing slashes
    // which indicate that folder name should be appended
    return result;
}

/**
 * Apply folder name appending rules
 * Paths ending with "/" get folder name appended, others used as-is
 */
QString applyFolderRules(const QString& path, const QString& folderName, bool applyInstallSuffix) {
    QString expanded = expandEnvVars(path);
    const QString suffix = applyInstallSuffix ? QStringLiteral(" (Nightly)") : QString();
    
    if (expanded.endsWith(QStringLiteral("/"))) {
        expanded += folderName + suffix;
    }
    
    // Clean the path to resolve . and .. components
    expanded = QDir::cleanPath(expanded);
    
    // Convert to absolute path and clean again
    return QDir::cleanPath(QDir(expanded).absolutePath());
}

void printHeader(const QString& title) {
    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << title.toStdString() << "\n";
    std::cout << std::string(70, '=') << "\n\n";
}

void printRow(const QString& label, const QString& template_str, const QString& expanded) {
    std::cout << "Template:  " << template_str.toStdString() << "\n";
    std::cout << "Expanded:  " << expanded.toStdString() << "\n";
    std::cout << "\n";
}

void testPlatformPaths() {
    printHeader("Platform-Specific Default Search Paths");
    
#if defined(Q_OS_WIN) || defined(_WIN32) || defined(_WIN64)
    std::cout << "Platform: Windows\n\n";
    
    QStringList windowsPaths = {
        QStringLiteral("%PROGRAMFILES%/"),
        QStringLiteral("%APPDATA%/"),
        QStringLiteral("%LOCALAPPDATA%/"),
        QStringLiteral("%PROGRAMDATA%/")
    };
    
    for (const QString& path : windowsPaths) {
        printRow("", path, expandEnvVars(path));
    }
    
#elif defined(Q_OS_MACOS) || defined(__APPLE__)
    std::cout << "Platform: macOS\n\n";
    
    QStringList macPaths = {
        QStringLiteral("${HOME}/Library/Application Support/"),
        QStringLiteral("/Library/Application Support/"),
        QStringLiteral("${HOME}/.config/")
    };
    
    for (const QString& path : macPaths) {
        printRow("", path, expandEnvVars(path));
    }
    
#else
    std::cout << "Platform: Linux/BSD/POSIX\n\n";
    
    QStringList linuxPaths = {
        QStringLiteral("${HOME}/.config/"),
        QStringLiteral("${XDG_CONFIG_HOME}/"),
        QStringLiteral("${HOME}/.local/share/"),
        QStringLiteral("/usr/share/"),
        QStringLiteral("/usr/local/share/")
    };
    
    for (const QString& path : linuxPaths) {
        printRow("", path, expandEnvVars(path));
    }
#endif
}

void testQualifiedPaths() {
    printHeader("Qualified Paths (Env Vars + Folder Names)");
    
    const QString folderName = QStringLiteral("ScadTemplates");
    
    std::cout << "Shows how paths become fully qualified for resource discovery:\n";
    std::cout << "  1. Expand environment variables\n";
    std::cout << "  2. Append folder name to paths ending with '/'\n";
    std::cout << "  3. Installation tier adds suffix (e.g., ' (Nightly)')\n\n";
    
#if defined(Q_OS_WIN) || defined(_WIN32) || defined(_WIN64)
    std::cout << "Platform: Windows\n\n";
    std::cout << "=== Installation Tier (with suffix) ===\n";
    
    QStringList installPaths = {
        QStringLiteral("%PROGRAMFILES%/"),
        QStringLiteral("."),
        QStringLiteral("../share/"),
        QStringLiteral(".."),
        QStringLiteral("${HOME}/../..")
    };
    
    for (const QString& path : installPaths) {
        std::cout << "Template:  " << path.toStdString() << "\n";
        std::cout << "Qualified: " << applyFolderRules(path, folderName, true).toStdString() << "\n\n";
    }
    
    std::cout << "=== User Tier (no suffix) ===\n";
    
    QStringList userPaths = {
        QStringLiteral("%APPDATA%/"),
        QStringLiteral("%LOCALAPPDATA%/"),
        QStringLiteral(".")
    };
    
    for (const QString& path : userPaths) {
        std::cout << "Template:  " << path.toStdString() << "\n";
        std::cout << "Qualified: " << applyFolderRules(path, folderName, false).toStdString() << "\n\n";
    }
    
#elif defined(Q_OS_MACOS) || defined(__APPLE__)
    std::cout << "Platform: macOS\n\n";
    std::cout << "=== Installation Tier (with suffix) ===\n";
    
    QStringList installPaths = {
        QStringLiteral("../Resources"),
        QStringLiteral("../share/"),
        QStringLiteral("..")
    };
    
    for (const QString& path : installPaths) {
        std::cout << "Template:  " << path.toStdString() << "\n";
        std::cout << "Qualified: " << applyFolderRules(path, folderName, true).toStdString() << "\n\n";
    }
    
    std::cout << "=== User Tier (no suffix) ===\n";
    
    QStringList userPaths = {
        QStringLiteral("${HOME}/Library/Application Support/"),
        QStringLiteral("../../Documents/")
    };
    
    for (const QString& path : userPaths) {
        std::cout << "Template:  " << path.toStdString() << "\n";
        std::cout << "Qualified: " << applyFolderRules(path, folderName, false).toStdString() << "\n\n";
    }
    
#else
    std::cout << "Platform: Linux/BSD/POSIX\n\n";
    std::cout << "=== Installation Tier (with suffix) ===\n";
    
    QStringList installPaths = {
        QStringLiteral("../share/"),
        QStringLiteral("../../share/"),
        QStringLiteral("."),
        QStringLiteral("..")
    };
    
    for (const QString& path : installPaths) {
        std::cout << "Template:  " << path.toStdString() << "\n";
        std::cout << "Qualified: " << applyFolderRules(path, folderName, true).toStdString() << "\n\n";
    }
    
    std::cout << "=== User Tier (no suffix) ===\n";
    
    QStringList userPaths = {
        QStringLiteral("${XDG_CONFIG_HOME}/"),
        QStringLiteral("${HOME}/.config/"),
        QStringLiteral("${HOME}/.local/share/")
    };
    
    for (const QString& path : userPaths) {
        std::cout << "Template:  " << path.toStdString() << "\n";
        std::cout << "Qualified: " << applyFolderRules(path, folderName, false).toStdString() << "\n\n";
    }
#endif
}

void testVariableStyles() {
    printHeader("Variable Syntax Support");
    
    std::cout << "Both ${VAR} (Unix) and %VAR% (Windows) styles are supported:\n\n";
    
    QStringList testPaths = {
        QStringLiteral("${HOME}/openscad/templates"),
        QStringLiteral("%USERPROFILE%\\openscad\\templates"),
        QStringLiteral("${HOME}/.config/%APPNAME%"),
        QStringLiteral("C:/Users/%USERNAME%/AppData/Roaming/openscad"),
        QStringLiteral("${HOME}/Library/Application Support/${APPNAME}")
    };
    
    for (const QString& path : testPaths) {
        printRow("", path, expandEnvVars(path));
    }
}

void testMixedCases() {
    printHeader("Mixed Cases & Edge Cases");
    
    QStringList testCases = {
        QStringLiteral("prefix_${HOME}_suffix"),
        QStringLiteral("${VAR1}/${VAR2}/${VAR3}"),
        QStringLiteral("${UNDEFINED}/path"),  // Will expand to /path
        QStringLiteral("/absolute/path"),     // No vars - returned unchanged
        QStringLiteral("relative/path"),      // No vars - returned unchanged
        QStringLiteral(""),                   // Empty string
        QStringLiteral("${HOME}/../.."),
        QStringLiteral("%PROGRAMFILES%/openscad (Nightly)")
    };
    
    for (const QString& path : testCases) {
        if (path.isEmpty()) {
            printRow("Empty", path, expandEnvVars(path));
        } else {
            printRow("", path, expandEnvVars(path));
        }
    }
}

void testSystemEnvironment() {
    printHeader("Available System Environment Variables");
    
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    
    // Print common variables
    QStringList commonVars = {
#if defined(Q_OS_WIN)
        "APPDATA", "LOCALAPPDATA", "PROGRAMFILES", "PROGRAMDATA", "USERPROFILE", "USERNAME"
#elif defined(Q_OS_MACOS)
        "HOME", "USER", "TMPDIR", "SHELL"
#else
        "HOME", "USER", "XDG_CONFIG_HOME", "XDG_DATA_HOME", "XDG_CACHE_HOME", "SHELL", "PATH"
#endif
    };
    
    std::cout << "Key variables on this system:\n\n";
    for (const QString& var : commonVars) {
        QString value = env.value(var);
        if (!value.isEmpty()) {
            std::cout << std::setw(20) << std::left << var.toStdString() 
                      << " = " << value.toStdString() << "\n";
        }
    }
    std::cout << "\n";
}

void printUsage() {
    std::cout << "\n"
              << "Test Environment Variable Expansion Utility\n"
              << "============================================\n\n"
              << "Usage: test_env_expansion [options]\n\n"
              << "Options:\n"
              << "  (no args)      Run all tests\n"
              << "  --verbose      Show all details\n"
              << "  --env          Show system environment variables\n"
              << "  --paths        Show platform-specific default paths\n"
              << "  --qualified    Show fully qualified paths (env vars + folder names)\n"
              << "  --styles       Show variable syntax examples\n"
              << "  --mixed        Show mixed/edge cases\n"
              << "  --help         Show this help message\n\n";
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    QStringList args = app.arguments();
    bool verbose = args.contains("--verbose");
    bool showHelp = args.contains("--help");
    
    if (showHelp) {
        printUsage();
        return 0;
    }
    
    if (args.size() == 1 || verbose) {
        // Run all tests
        testSystemEnvironment();
        testPlatformPaths();
        testQualifiedPaths();
        testVariableStyles();
        testMixedCases();
    } else {
        // Run specific test
        if (args.contains("--env")) {
            testSystemEnvironment();
        }
        if (args.contains("--paths")) {
            testPlatformPaths();
        }
        if (args.contains("--qualified")) {
            testQualifiedPaths();
        }
        if (args.contains("--styles")) {
            testVariableStyles();
        }
        if (args.contains("--mixed")) {
            testMixedCases();
        }
    }
    
    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "Test Complete\n";
    std::cout << std::string(70, '=') << "\n\n";
    
    return 0;
}
