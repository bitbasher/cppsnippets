/**
 * @file preferencesdialog.cpp
 * @brief Implementation of PreferencesDialog
 * 
 * Follows Qt TabDialog pattern where each tab is its own QWidget subclass.
 */

#include "gui/preferencesdialog.h"
#include "gui/platformInfoWidget.hpp"
#include "gui/dialogButtonBar.hpp"
#include "gui/installationTab.hpp"
#include "gui/machineTab.hpp"
#include "gui/userTab.hpp"
#include "gui/envVarsTab.h"
#include "gui/resourceLocationWidget.hpp"
#include "platformInfo/resourceLocationManager.h"
#include "resInventory/ResourceLocation.h"
#include "platformInfo/platformInfo.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QMessageBox>

// ============================================================================
// PreferencesDialog
// ============================================================================

PreferencesDialog::PreferencesDialog(platformInfo::ResourceLocationManager* manager,
                                     QWidget* parent)
    : QDialog(parent)
    , m_manager(manager)
{
    setWindowTitle(tr("Preferences"));
    setMinimumSize(700, 500);
    
    // Create platform info widget
    m_platformInfoWidget = new PlatformInfoWidget;
    
    // Create tab widgets
    m_tabWidget = new QTabWidget;
    
    m_installationTab = new InstallationTab;
    connect(m_installationTab, &InstallationTab::locationsChanged,
            this, &PreferencesDialog::onLocationsChanged);
    m_tabWidget->addTab(m_installationTab, tr("Installation"));
    
    m_machineTab = new MachineTab;
    connect(m_machineTab, &MachineTab::locationsChanged,
            this, &PreferencesDialog::onLocationsChanged);
    m_tabWidget->addTab(m_machineTab, tr("Machine (All Users)"));
    
    m_userTab = new UserTab;
    connect(m_userTab, &UserTab::locationsChanged,
            this, &PreferencesDialog::onLocationsChanged);
    m_tabWidget->addTab(m_userTab, tr("User (Personal)"));
    
    m_envVarsTab = new EnvVarsTab(m_manager);
    connect(m_envVarsTab, &EnvVarsTab::envVarsChanged,
            this, &PreferencesDialog::onLocationsChanged);
    m_tabWidget->addTab(m_envVarsTab, tr("Environment Variables"));
    
    // Create button bar
    m_buttonBar = new DialogButtonBar;
    connect(m_buttonBar, &DialogButtonBar::restoreDefaultsClicked,
            this, &PreferencesDialog::onRestoreDefaults);
    connect(m_buttonBar, &DialogButtonBar::accepted,
            this, &PreferencesDialog::accept);
    connect(m_buttonBar, &DialogButtonBar::rejected,
            this, &PreferencesDialog::reject);
    
    // Main layout - apply at the end
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_platformInfoWidget);
    mainLayout->addWidget(m_tabWidget);
    mainLayout->addWidget(m_buttonBar);
    setLayout(mainLayout);
    
    // Load data
    m_platformInfoWidget->updateFromManager(m_manager);
    loadSettings();
}

PreferencesDialog::~PreferencesDialog() = default;

void PreferencesDialog::onLocationsChanged()
{
    // Changes are now applied immediately via Add buttons
}

void PreferencesDialog::loadSettings() {
    // ========== Installation Tab ==========
    bool isValidInstall = m_manager->isRunningFromValidInstallation();
    QString userInstallPath = m_manager->userSpecifiedInstallationPath();
    bool hasUserInstallPath = !userInstallPath.isEmpty() && 
                               platformInfo::ResourceLocationManager::isValidInstallation(userInstallPath);
    
    QVector<platformInfo::ResourceLocation> installLocations;
    ResourceLocationWidget* installWidget = m_installationTab->locationWidget();
    
    if (isValidInstall || hasUserInstallPath) {
        // We have a valid installation - show it and any siblings
        QString installDir = m_manager->findInstallationResourceDir();
        
        if (!installDir.isEmpty()) {
            platformInfo::ResourceLocation loc(installDir, 
                tr("Application Resources"), 
                tr("Built-in resources from this installation"));
            loc.isEnabled = true;
            loc.exists = true;
            loc.isWritable = false;
            installLocations.append(loc);
        }
        
        // Add sibling installations
        QStringList enabledSiblings = m_manager->enabledSiblingPaths();
        QVector<platformInfo::ResourceLocation> siblings = m_manager->findSiblingInstallations();
        for (auto& sibling : siblings) {
            sibling.isEnabled = enabledSiblings.contains(sibling.path);
            installLocations.append(sibling);
        }
        
        // Hide input widget since we have a valid installation
        m_installationTab->setFallbackMode(false);
    } else {
        // Not running from valid installation - allow user to specify one
        // Show helpful message as a pseudo-location
        platformInfo::ResourceLocation placeholder;
        placeholder.path = m_manager->defaultInstallationSearchPath();
        placeholder.displayName = tr("No OpenSCAD installation detected");
        placeholder.description = tr("Use Browse to locate an OpenSCAD installation folder");
        placeholder.isEnabled = false;
        placeholder.exists = false;
        placeholder.isWritable = false;
        installLocations.append(placeholder);
        
        // Show input widget in fallback mode so user can specify installation
        m_installationTab->setFallbackMode(true);
    }
    
    installWidget->setLocations(installLocations);
    
    // ========== Machine Tab ==========
    // Use the manager's locations (which already checks existence)
    QVector<platformInfo::ResourceLocation> machineLocations = m_manager->availableMachineLocations();
    // Disable and uncheck locations that don't exist or exist but have no resource folders
    for (auto& loc : machineLocations) {
        if (!loc.exists || !loc.hasResourceFolders) {
            loc.isEnabled = false;
        }
    }
    
    // Remove OPENSCAD_PATH; manager will handle OPENSCADPATH placeholder if present
    
    // Add XDG_DATA_DIRS environment variable entries
    // On Windows: only shown if defined; On POSIX/Mac: shown as placeholder if not defined
    machineLocations.append(MachineTab::xdgDataDirsLocations());
    
    m_machineTab->locationWidget()->setLocations(machineLocations);
    
    // ========== User Tab ==========
    QVector<platformInfo::ResourceLocation> userLocations = m_manager->availableUserLocations();
    // Disable and uncheck locations that don't exist or exist but have no resource folders
    // Exception: OPENSCADPATH placeholder should remain disabled but visible
    for (auto& loc : userLocations) {
        if (!loc.exists || (!loc.hasResourceFolders && !loc.path.startsWith(QLatin1Char('(')))) {
            loc.isEnabled = false;
        }
    }
    
    // Remove OPENSCAD_PATH from user tier; keep OPENSCADPATH only via manager
    
    // Add XDG_DATA_HOME environment variable entries
    // On Windows: only shown if defined; On POSIX/Mac: shown as placeholder if not defined
    userLocations.append(UserTab::xdgDataHomeLocations());
    
    m_userTab->locationWidget()->setLocations(userLocations);
}

void PreferencesDialog::saveSettings() {
    if (!m_manager) return;
    
    ResourceLocationWidget* installWidget = m_installationTab->locationWidget();
    
    // Save user-specified installation path if not running from valid installation
    if (!m_manager->isRunningFromValidInstallation()) {
        QVector<platformInfo::ResourceLocation> installLocs = installWidget->locations();
        if (!installLocs.isEmpty()) {
            QString userPath = installLocs.first().path;
            if (platformInfo::ResourceLocationManager::isValidInstallation(userPath)) {
                m_manager->setUserSpecifiedInstallationPath(userPath);
            }
        }
    }
    
    // Save sibling installation enabled state
    // Get the installation locations and extract which siblings are enabled
    QVector<platformInfo::ResourceLocation> installLocs = installWidget->locations();
    QStringList enabledSiblings;
    QString currentInstallDir = m_manager->findInstallationResourceDir();
    for (const auto& loc : installLocs) {
        // Skip the current installation (always enabled, not a sibling)
        if (loc.path == currentInstallDir) continue;
        // Skip if this is the user-specified path (not a sibling)
        if (loc.path == m_manager->userSpecifiedInstallationPath()) continue;
        // This is a sibling - check if enabled
        if (loc.isEnabled) {
            enabledSiblings.append(loc.path);
        }
    }
    m_manager->setEnabledSiblingPaths(enabledSiblings);
    
    // Save machine locations config and enabled state
    m_manager->saveMachineLocationsConfig(m_machineTab->locationWidget()->locations());
    
    // Save user locations config and enabled state
    m_manager->saveUserLocationsConfig(m_userTab->locationWidget()->locations());
}

void PreferencesDialog::accept() {
    saveSettings();
    
    // Save env vars if there are changes
    if (m_envVarsTab->hasUnsavedChanges()) {
        m_envVarsTab->saveEnvVars();
    }
    
    QDialog::accept();
}

void PreferencesDialog::reject() {
    // Reload to discard changes
    m_manager->reloadConfiguration();
    QDialog::reject();
}

void PreferencesDialog::onRestoreDefaults() {
    if (QMessageBox::question(this, tr("Restore Defaults"),
            tr("This will reset all machine and user resource locations to defaults.\n\n"
               "Are you sure?"),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        
        m_machineTab->locationWidget()->setLocations(m_manager->defaultMachineLocations());
        m_userTab->locationWidget()->setLocations(m_manager->defaultUserLocations());
    }
}
