#include <gtest/gtest.h>
#include <QDir>
#include <QFileInfo>
#include <QTemporaryDir>

#include "platformInfo/resourceLocationManager.h"

using platformInfo::ResourceLocationManager;
using platformInfo::ResourcePaths;
using resourceInfo::ResourceTier;

namespace {

struct TestFixture {
    QTemporaryDir temp;
    QSettings settings;
    ResourceLocationManager manager;

    TestFixture()
        : settings(settingsPath(), QSettings::IniFormat)
        , manager(&settings, QString()) {
        setupFilesystem();
        setupEnvOverrides();
        manager.setApplicationPath(appBinPath());
    }

    QString settingsPath() const { return QDir(temp.path()).filePath("settings.ini"); }

    QString base(const QString &sub = QString()) const {
        return sub.isEmpty() ? temp.path() : QDir(temp.path()).absoluteFilePath(sub);
    }

    QString programFiles() const { return base("ProgramFiles"); }
    QString programData() const { return base("ProgramData"); }
    QString appDataRoaming() const { return base("AppData/Roaming"); }
    QString appDataLocal() const { return base("AppData/Local"); }
    QString appRoot() const { return programFiles() + QLatin1String("/openscad"); }
    QString appBinPath() const { return appRoot() + QLatin1String("/bin"); }

    void setupFilesystem() {
        // Installation tree
        QDir().mkpath(appBinPath());
        QDir().mkpath(appRoot() + QLatin1String("/examples"));

        // Machine tier
        QDir().mkpath(programData() + QLatin1String("/ScadTemplates"));

        // User tier
        QDir().mkpath(appDataRoaming() + QLatin1String("/ScadTemplates"));
        QDir().mkpath(appDataLocal() + QLatin1String("/ScadTemplates"));
    }

    void setupEnvOverrides() {
        auto &paths = manager.resourcePaths();
        paths.addEnvVar(QStringLiteral("PROGRAMFILES"), programFiles());
        paths.addEnvVar(QStringLiteral("PROGRAMDATA"), programData());
        paths.addEnvVar(QStringLiteral("APPDATA"), appDataRoaming());
        paths.addEnvVar(QStringLiteral("LOCALAPPDATA"), appDataLocal());
        paths.addEnvVar(QStringLiteral("HOME"), base());
        paths.addEnvVar(QStringLiteral("XDG_CONFIG_HOME"), base(".config"));
        paths.addEnvVar(QStringLiteral("XDG_DATA_HOME"), base(".local/share"));
    }
};

} // namespace

TEST(ResourceLocationManagerEnabled, DefaultsIncludeInstallAndUser) {
    TestFixture fx;

    auto tiered = fx.manager.enabledLocationsByTier();

    // Install should exist and be enabled
    ASSERT_FALSE(tiered.installation.isEmpty());
    EXPECT_TRUE(tiered.installation.first().isEnabled);
    EXPECT_TRUE(tiered.installation.first().exists);

    // Machine uses %PROGRAMDATA%/ScadTemplates and should exist
    ASSERT_FALSE(tiered.machine.isEmpty());
    EXPECT_TRUE(tiered.machine.first().path.contains("ScadTemplates"));
    EXPECT_TRUE(tiered.machine.first().exists);

    // User entries should include both roaming/local; at least one present
    ASSERT_FALSE(tiered.user.isEmpty());
    bool hasRoaming = false;
    bool hasLocal = false;
    for (const auto &loc : tiered.user) {
        if (loc.path.contains("Roaming")) hasRoaming = true;
        if (loc.path.contains("Local")) hasLocal = true;
    }
    EXPECT_TRUE(hasRoaming || hasLocal);
}

TEST(ResourceLocationManagerEnabled, EnableMaskFiltersTiers) {
    TestFixture fx;

    // Build tiered once to get canonical paths
    auto tiered = fx.manager.enabledLocationsByTier();
    ASSERT_FALSE(tiered.installation.isEmpty());
    ASSERT_FALSE(tiered.user.isEmpty());

    QString installPath = tiered.installation.first().path;
    QString userPath = tiered.user.first().path;

    // Persist enabled set with only install path -> user should be filtered out
    QStringList enabledOnlyInstall;
    enabledOnlyInstall << installPath;
    fx.settings.setValue(QLatin1String("Resources/EnabledPaths"), enabledOnlyInstall);
    fx.settings.sync();

    auto filtered = fx.manager.enabledLocationsByTier();
    EXPECT_FALSE(filtered.installation.isEmpty());
    EXPECT_TRUE(filtered.user.isEmpty());

    // Now enable user only; installation stays protected/auto-added
    QStringList enabledUserOnly;
    enabledUserOnly << userPath;
    fx.settings.setValue(QLatin1String("Resources/EnabledPaths"), enabledUserOnly);
    fx.settings.sync();

    auto restored = fx.manager.enabledLocationsByTier();
    EXPECT_FALSE(restored.installation.isEmpty()); // protected install added back
    ASSERT_FALSE(restored.user.isEmpty());
    EXPECT_EQ(restored.user.first().path, userPath);
}

TEST(ResourceLocationManagerEnabled, StalePathsAreDetectedAndRemoved) {
    TestFixture fx;

    // Build tiered once to get a valid installation path
    auto tiered = fx.manager.enabledLocationsByTier();
    ASSERT_FALSE(tiered.installation.isEmpty());
    QString validInstallPath = tiered.installation.first().path;

    // Inject fake/non-existent paths into QSettings
    QStringList pathsWithStale;
    pathsWithStale << validInstallPath;                              // Valid path
    pathsWithStale << "C:/FakePath/DoesNotExist/Resources";          // Stale path 1
    pathsWithStale << "D:/NonExistent/OpenSCAD/share";               // Stale path 2
    pathsWithStale << fx.programData() + "/ScadTemplates";           // Valid machine path

    fx.settings.setValue(QLatin1String("Resources/EnabledPaths"), pathsWithStale);
    fx.settings.sync();

    // Capture qInfo output by redirecting to a test message handler
    QStringList logMessages;
    static thread_local QStringList* messageSink = nullptr;
    auto handler = [](QtMsgType type, const QMessageLogContext&, const QString& msg) {
        if (type == QtInfoMsg && messageSink) {
            messageSink->append(msg);
        }
    };
    auto oldHandler = qInstallMessageHandler(handler);
    messageSink = &logMessages;

    // Trigger loading which should detect and remove stale paths
    auto cleaned = fx.manager.enabledLocationsByTier();

    // Restore original message handler
    messageSink = nullptr;
    qInstallMessageHandler(oldHandler);

    // Verify the stale paths were removed from settings
    QStringList persistedPaths = fx.settings.value(QLatin1String("Resources/EnabledPaths")).toStringList();
    
    // Should only contain valid paths now (stale ones removed)
    EXPECT_FALSE(persistedPaths.contains("C:/FakePath/DoesNotExist/Resources"));
    EXPECT_FALSE(persistedPaths.contains("D:/NonExistent/OpenSCAD/share"));
    
    // Valid paths should still be present
    EXPECT_TRUE(persistedPaths.contains(validInstallPath));
    EXPECT_TRUE(persistedPaths.contains(fx.programData() + "/ScadTemplates"));

    // Verify we have fewer paths after cleanup (2 stale removed)
    EXPECT_EQ(persistedPaths.size(), 2);

    // Validate qInfo messages were emitted about the stale removals
    const QString joined = logMessages.join("\n");
    ASSERT_FALSE(logMessages.isEmpty());
    EXPECT_TRUE(joined.contains("Removed"));
    EXPECT_TRUE(joined.contains("missing path"));
    EXPECT_TRUE(joined.contains("FakePath"));
    EXPECT_TRUE(joined.contains("NonExistent"));
}
