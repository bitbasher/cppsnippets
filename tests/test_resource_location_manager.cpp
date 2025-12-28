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

TEST(ResourceLocationManagerEnabled, DiscoveryFindsAllTiers) {
    TestFixture fx;

    // Test the new unified discovery function
    auto allLocations = fx.manager.discoverAllLocations();

    // Should find locations from all three tiers
    bool hasInstall = false;
    bool hasMachine = false;
    bool hasUser = false;

    for (const auto& loc : allLocations) {
        if (loc.tier == ResourceTier::Installation) hasInstall = true;
        if (loc.tier == ResourceTier::Machine) hasMachine = true;
        if (loc.tier == ResourceTier::User) hasUser = true;
    }

    EXPECT_TRUE(hasInstall) << "Should discover installation locations";
    EXPECT_TRUE(hasMachine) << "Should discover machine locations";
    EXPECT_TRUE(hasUser) << "Should discover user locations";

    // All discovered locations should default to enabled
    for (const auto& loc : allLocations) {
        EXPECT_TRUE(loc.isEnabled) << "Location should default to enabled: " << loc.path.toStdString();
    }
}

TEST(ResourceLocationManagerEnabled, TierTagsAreCorrect) {
    TestFixture fx;

    auto allLocations = fx.manager.discoverAllLocations();
    ASSERT_FALSE(allLocations.isEmpty());

    // Verify each location has a valid tier tag
    for (const auto& loc : allLocations) {
        EXPECT_TRUE(loc.tier == ResourceTier::Installation ||
                   loc.tier == ResourceTier::Machine ||
                   loc.tier == ResourceTier::User)
            << "Invalid tier for location: " << loc.path.toStdString();
    }

    // Verify installation locations contain expected paths
    auto tiered = fx.manager.enabledLocationsByTier();
    ASSERT_FALSE(tiered.installation.isEmpty());
    
    for (const auto& loc : tiered.installation) {
        EXPECT_EQ(loc.tier, ResourceTier::Installation);
    }

    // Verify machine locations
    ASSERT_FALSE(tiered.machine.isEmpty());
    for (const auto& loc : tiered.machine) {
        EXPECT_EQ(loc.tier, ResourceTier::Machine);
        EXPECT_TRUE(loc.path.contains("ScadTemplates") || 
                   loc.path.contains("ProgramData"))
            << "Machine location has unexpected path: " << loc.path.toStdString();
    }

    // Verify user locations
    ASSERT_FALSE(tiered.user.isEmpty());
    for (const auto& loc : tiered.user) {
        EXPECT_EQ(loc.tier, ResourceTier::User);
    }
}
