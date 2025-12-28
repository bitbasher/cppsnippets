/**
 * @file preferencesdialog.cpp
 * @brief Implementation of PreferencesDialog
 * 
 * Follows Qt TabDialog pattern where each tab is its own QWidget subclass.
 */

#include "gui/preferencesdialog.h"
#include "resInventory/resourceScanner.h"
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
void PreferencesDialog::setInventoryManager(resInventory::ResourceInventoryManager* inventory)
{
    m_inventoryManager = inventory;
}


void PreferencesDialog::onLocationsChanged()
{
    // Changes are now applied immediately via Add buttons
}

void PreferencesDialog::loadSettings() {
    // ========== Installation Tab ==========
    QString effectiveInstallPath = m_manager->effectiveInstallationPath();
    bool hasEffectiveInstall = !effectiveInstallPath.isEmpty();
    QString userInstallPath = m_manager->userSpecifiedInstallationPath();
    bool hasUserInstallPath = !userInstallPath.isEmpty() && 
                               platformInfo::ResourceLocationManager::isValidInstallation(userInstallPath);
    
    QList<platformInfo::ResourceLocation> installLocations;
    ResourceLocationWidget* installWidget = m_installationTab->locationWidget();
    
    if (hasEffectiveInstall || hasUserInstallPath) {
        // We have a valid installation - show it and any siblings
        QString installDir = m_manager->findInstallationResourceDir();

        auto canonical = [](const QString& path) {
            QFileInfo fi(path);
            QString canon = fi.canonicalFilePath();
            return canon.isEmpty() ? fi.absoluteFilePath() : canon;
        };

        const QSet<QString> enabledSiblings = QSet<QString>(m_manager->enabledSiblingPaths().cbegin(), m_manager->enabledSiblingPaths().cend());
        const QSet<QString> disabledInstall = QSet<QString>(m_manager->disabledInstallationPaths().cbegin(), m_manager->disabledInstallationPaths().cend());

        if (!installDir.isEmpty()) {
            platformInfo::ResourceLocation loc(installDir, 
                tr("Application Resources"), 
                tr("Built-in resources from this installation"));
            loc.isEnabled = !disabledInstall.contains(canonical(installDir));
            loc.exists = true;
            loc.isWritable = false;
            installLocations.append(loc);
        }

        // Add sibling installations (no deduplication; user may disable any)
        QVector<platformInfo::ResourceLocation> siblings = m_manager->findSiblingInstallations();
        for (auto& sibling : siblings) {
            const QString key = canonical(sibling.path);
            if (disabledInstall.contains(key)) {
                sibling.isEnabled = false;
            } else if (enabledSiblings.contains(sibling.path)) {
                sibling.isEnabled = true;
            } // else keep default (typically true)
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
    // Disable locations that don't exist; keep checkbox enabled otherwise
    for (auto& loc : machineLocations) {
        if (!loc.exists) {
            loc.isEnabled = false;
        }
    }
    
    // Add XDG_DATA_DIRS environment variable entries
    // On Windows: only shown if defined; On POSIX/Mac: shown as placeholder if not defined
    machineLocations.append(MachineTab::xdgDataDirsLocations());
    
    m_machineTab->locationWidget()->setLocations(machineLocations.toList());
    
    // ========== User Tab ==========
    QVector<platformInfo::ResourceLocation> userLocations = m_manager->availableUserLocations();
    // Disable locations that don't exist; keep checkbox enabled otherwise
    for (auto& loc : userLocations) {
        if (!loc.exists) {
            loc.isEnabled = false;
        }
    }
    
    // Add XDG_DATA_HOME environment variable entries
    // On Windows: only shown if defined; On POSIX/Mac: shown as placeholder if not defined
    userLocations.append(UserTab::xdgDataHomeLocations());
    
    m_userTab->locationWidget()->setLocations(userLocations.toList());
}

void PreferencesDialog::saveSettings() {
    // Save installation tier (siblings only)
    {
        ResourceLocationWidget* installWidget = m_installationTab->locationWidget();
        const auto installLocs = installWidget->locations();
        m_manager->saveMachineLocationsConfig(installLocs);
        
        QStringList enabledSiblings;
        QStringList disabledInstall;
        QString currentInstallDir = m_manager->findInstallationResourceDir();
        QString userInstallPath = m_manager->userSpecifiedInstallationPath();
        
        for (const auto& loc : installLocs) {
            if (!loc.path.isEmpty()) {
                if (loc.path == currentInstallDir || loc.path == userInstallPath) {
                    if (!loc.isEnabled) {
                        disabledInstall.append(loc.path);
                    }
                } else {
                    if (loc.isEnabled) {
                        enabledSiblings.append(loc.path);
                    } else {
                        disabledInstall.append(loc.path);
                    }
                }
            }
        }
        m_manager->setEnabledSiblingPaths(enabledSiblings);
        m_manager->setDisabledInstallationPaths(disabledInstall);
    }
    
    // Save machine locations config and enabled state
    {
        const auto machineLocs = m_machineTab->locationWidget()->locations();
        m_manager->saveMachineLocationsConfig(machineLocs);
        m_manager->setEnabledMachineLocations(m_machineTab->locationWidget()->enabledPaths());
        QStringList disabled;
        for (const auto& loc : machineLocs) {
            if (!loc.isEnabled && !loc.path.isEmpty()) {
                disabled.append(loc.path);
            }
        }
        m_manager->setDisabledMachineLocations(disabled);
    }
    
    // Save user locations config and enabled state
    {
        const auto userLocs = m_userTab->locationWidget()->locations();
        m_manager->saveUserLocationsConfig(userLocs);
        m_manager->setEnabledUserLocations(m_userTab->locationWidget()->enabledPaths());
        QStringList disabled;
        for (const auto& loc : userLocs) {
            if (!loc.isEnabled && !loc.path.isEmpty()) {
                disabled.append(loc.path);
            }
        }
        m_manager->setDisabledUserLocations(disabled);
    }
}

void PreferencesDialog::accept() {
    saveSettings();
    
    // Save env vars if there are changes
    if (m_envVarsTab->hasUnsavedChanges()) {
        m_envVarsTab->saveEnvVars();
    }
    
    // Trigger inventory rescan if manager set
    if (m_inventoryManager && m_manager) {
        m_inventoryManager->buildInventory(*m_manager);
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
        
        // Reset UI display
        m_machineTab->locationWidget()->setLocations(m_manager->defaultMachineLocations());
        m_userTab->locationWidget()->setLocations(m_manager->defaultUserLocations());
        
        // Reset enabled state to defaults (install + machine + user + siblings)
        m_manager->restoreEnabledPathsToDefaults();
        
        // Trigger rescan to apply changes
        if (m_inventoryManager) {
            m_inventoryManager->buildInventory(*m_manager);
        }
    }
}
