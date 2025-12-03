/**
 * @file preferencesdialog.cpp
 * @brief Implementation of PreferencesDialog
 */

#include "preferencesdialog.h"
#include "platformInfo/resourceLocationManager.h"
#include "platformInfo/ResourceLocation.h"
#include "platformInfo/platformInfo.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>

// ============================================================================
// PreferencesDialog
// ============================================================================

PreferencesDialog::PreferencesDialog(platformInfo::ResourceLocationManager* manager,
                                     QWidget* parent)
    : QDialog(parent)
    , m_manager(manager)
    , m_platformLabel(nullptr)
    , m_installDirLabel(nullptr)
    , m_folderNameLabel(nullptr)
    , m_applyButton(nullptr)
    , m_restoreDefaultsButton(nullptr)
    , m_tabWidget(nullptr)
    , m_installationWidget(nullptr)
    , m_machineWidget(nullptr)
    , m_userWidget(nullptr)
{
    setWindowTitle(tr("Preferences"));
    setMinimumSize(700, 500);
    
    setupUi();
    loadSettings();
}

PreferencesDialog::~PreferencesDialog() = default;

void PreferencesDialog::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Platform info section
    QGroupBox* platformGroup = new QGroupBox(tr("Platform Information"), this);
    QFormLayout* platformLayout = new QFormLayout(platformGroup);
    
    m_platformLabel = new QLabel(this);
    platformLayout->addRow(tr("Detected Platform:"), m_platformLabel);
    
    m_folderNameLabel = new QLabel(this);
    platformLayout->addRow(tr("Folder Name:"), m_folderNameLabel);
    
    m_installDirLabel = new QLabel(this);
    m_installDirLabel->setWordWrap(true);
    platformLayout->addRow(tr("Installation Resources:"), m_installDirLabel);
    
    mainLayout->addWidget(platformGroup);
    
    // Tabs for each tier
    m_tabWidget = new QTabWidget(this);

    // Installation tab (no add/remove, but allow enabling/disabling siblings)
    QWidget* installationTab = new QWidget(this);
    m_installationWidget = new ResourceLocationWidget(tr("Installation Resources"), false, false, installationTab);
    // Note: NOT read-only - allows toggling sibling installations on/off
    connect(m_installationWidget, &ResourceLocationWidget::locationsChanged,
            this, [this]() { m_applyButton->setEnabled(true); });
    QVBoxLayout* installationLayout = new QVBoxLayout(installationTab);
    installationLayout->addWidget(m_installationWidget);
    installationTab->setLayout(installationLayout);
    m_tabWidget->addTab(installationTab, tr("Installation (Built-in)"));

    // Machine tab
    QWidget* machineTab = new QWidget(this);
    m_machineWidget = new ResourceLocationWidget(tr("Machine Resources"), true, true, machineTab);
    connect(m_machineWidget, &ResourceLocationWidget::locationsChanged,
            this, [this]() { m_applyButton->setEnabled(true); });
    QVBoxLayout* machineLayout = new QVBoxLayout(machineTab);
    machineLayout->addWidget(m_machineWidget);
    machineTab->setLayout(machineLayout);
    m_tabWidget->addTab(machineTab, tr("Machine (All Users)"));

    // User tab
    QWidget* userTab = new QWidget(this);
    m_userWidget = new ResourceLocationWidget(tr("User Resources"), true, true, userTab);
    connect(m_userWidget, &ResourceLocationWidget::locationsChanged,
            this, [this]() { m_applyButton->setEnabled(true); });
    QVBoxLayout* userLayout = new QVBoxLayout(userTab);
    userLayout->addWidget(m_userWidget);
    userTab->setLayout(userLayout);
    m_tabWidget->addTab(userTab, tr("User (Personal)"));
    
    mainLayout->addWidget(m_tabWidget);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_restoreDefaultsButton = new QPushButton(tr("Restore Defaults"), this);
    connect(m_restoreDefaultsButton, &QPushButton::clicked, 
            this, &PreferencesDialog::onRestoreDefaults);
    buttonLayout->addWidget(m_restoreDefaultsButton);
    
    buttonLayout->addStretch();
    
    m_applyButton = new QPushButton(tr("Apply"), this);
    m_applyButton->setEnabled(false);
    connect(m_applyButton, &QPushButton::clicked, this, &PreferencesDialog::onApply);
    buttonLayout->addWidget(m_applyButton);
    
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &PreferencesDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &PreferencesDialog::reject);
    buttonLayout->addWidget(buttonBox);
    
    mainLayout->addLayout(buttonLayout);
    
    updatePlatformInfo();
}

void PreferencesDialog::loadSettings() {
    // ========== Installation Tab ==========
    bool isValidInstall = m_manager->isRunningFromValidInstallation();
    QString userInstallPath = m_manager->userSpecifiedInstallationPath();
    bool hasUserInstallPath = !userInstallPath.isEmpty() && 
                               platformInfo::ResourceLocationManager::isValidInstallation(userInstallPath);
    
    QVector<platformInfo::ResourceLocation> installLocations;
    
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
        
        // Disable the path/name/browse since we have a valid installation
        m_installationWidget->setReadOnly(true);
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
        
        // Enable path/name/browse fields so user can specify installation
        m_installationWidget->setReadOnly(false);
    }
    
    m_installationWidget->setLocations(installLocations);
    
    // ========== Machine Tab ==========
    // Use the manager's locations (which already checks existence)
    QVector<platformInfo::ResourceLocation> machineLocations = m_manager->availableMachineLocations();
    // Disable and uncheck locations that don't exist or exist but have no resource folders
    for (auto& loc : machineLocations) {
        if (!loc.exists || !loc.hasResourceFolders) {
            loc.isEnabled = false;
        }
    }
    m_machineWidget->setLocations(machineLocations);
    
    // ========== User Tab ==========
    QVector<platformInfo::ResourceLocation> userLocations = m_manager->availableUserLocations();
    // Disable and uncheck locations that don't exist or exist but have no resource folders
    // Exception: OPENSCADPATH placeholder should remain disabled but visible
    for (auto& loc : userLocations) {
        if (!loc.exists || (!loc.hasResourceFolders && !loc.path.startsWith(QLatin1Char('(')))) {
            loc.isEnabled = false;
        }
    }
    m_userWidget->setLocations(userLocations);
}

void PreferencesDialog::updatePlatformInfo() {
    if (!m_manager) return;
    
    // Platform name
    platformInfo::PlatformInfo info;
    QString platformName;
    switch (m_manager->osType()) {
        case platformInfo::ExtnOSType::Windows:
            platformName = tr("Windows");
            break;
        case platformInfo::ExtnOSType::MacOS:
            platformName = tr("macOS");
            break;
        case platformInfo::ExtnOSType::Linux:
            platformName = tr("Linux");
            break;
        case platformInfo::ExtnOSType::BSD:
            platformName = tr("BSD");
            break;
        default:
            platformName = tr("Unknown");
            break;
    }
    m_platformLabel->setText(platformName + QStringLiteral(" (") + 
                             platformInfo::PlatformInfo::productVersion() + QStringLiteral(")"));
    
    // Folder name
    m_folderNameLabel->setText(m_manager->folderName());
    
    // Installation directory
    QString installDir = m_manager->findInstallationResourceDir();
    if (installDir.isEmpty()) {
        m_installDirLabel->setText(tr("<not found>"));
        m_installDirLabel->setStyleSheet(QStringLiteral("color: red;"));
    } else {
        m_installDirLabel->setText(installDir);
        m_installDirLabel->setStyleSheet(QString());
    }
}

void PreferencesDialog::saveSettings() {
    if (!m_manager) return;
    
    // Save user-specified installation path if not running from valid installation
    if (!m_manager->isRunningFromValidInstallation()) {
        QVector<platformInfo::ResourceLocation> installLocs = m_installationWidget->locations();
        if (!installLocs.isEmpty()) {
            QString userPath = installLocs.first().path;
            if (platformInfo::ResourceLocationManager::isValidInstallation(userPath)) {
                m_manager->setUserSpecifiedInstallationPath(userPath);
            }
        }
    }
    
    // Save sibling installation enabled state
    // Get the installation locations and extract which siblings are enabled
    QVector<platformInfo::ResourceLocation> installLocs = m_installationWidget->locations();
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
    m_manager->saveMachineLocationsConfig(m_machineWidget->locations());
    
    // Save user locations config and enabled state
    m_manager->saveUserLocationsConfig(m_userWidget->locations());
    
    m_applyButton->setEnabled(false);
}

void PreferencesDialog::accept() {
    saveSettings();
    QDialog::accept();
}

void PreferencesDialog::reject() {
    // Reload to discard changes
    m_manager->reloadConfiguration();
    QDialog::reject();
}

void PreferencesDialog::onApply() {
    saveSettings();
}

void PreferencesDialog::onRestoreDefaults() {
    if (QMessageBox::question(this, tr("Restore Defaults"),
            tr("This will reset all machine and user resource locations to defaults.\n\n"
               "Are you sure?"),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        
        m_machineWidget->setLocations(m_manager->defaultMachineLocations());
        m_userWidget->setLocations(m_manager->defaultUserLocations());
        m_applyButton->setEnabled(true);
    }
}
