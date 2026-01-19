/**
 * @file preferencesdialog.cpp
 * @brief Implementation of PreferencesDialog
 * 
 * STUBBED: ResourceLocationManager removed - needs reimplementation with new architecture
 */

#include "gui/preferencesdialog.hpp"
#include "gui/platformInfoWidget.hpp"
#include "gui/dialogButtonBar.hpp"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QLabel>

// ============================================================================
// PreferencesDialog - STUBBED
// ============================================================================

PreferencesDialog::PreferencesDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Preferences"));
    setMinimumSize(500, 300);
    
    // STUB: Create minimal UI until ResourceLocationManager replacement is implemented
    QVBoxLayout* mainLayout = new QVBoxLayout;
    
    // Platform info
    m_platformInfoWidget = new PlatformInfoWidget;
    m_platformInfoWidget->updateFromManager();
    mainLayout->addWidget(m_platformInfoWidget);
    
    // Placeholder message
    QLabel* placeholder = new QLabel(tr(
        "Preferences dialog is temporarily disabled.\n\n"
        "ResourceLocationManager was removed during refactoring.\n"
        "This will be reimplemented with the new architecture."));
    placeholder->setWordWrap(true);
    mainLayout->addWidget(placeholder);
    
    mainLayout->addStretch();
    
    // Button bar
    m_buttonBar = new DialogButtonBar;
    connect(m_buttonBar, &DialogButtonBar::accepted, this, &PreferencesDialog::accept);
    connect(m_buttonBar, &DialogButtonBar::rejected, this, &PreferencesDialog::reject);
    mainLayout->addWidget(m_buttonBar);
    
    setLayout(mainLayout);
}

PreferencesDialog::~PreferencesDialog() = default;

void PreferencesDialog::accept() {
    // STUB: Nothing to save yet
    QDialog::accept();
}

void PreferencesDialog::reject() {
    // STUB: Nothing to reload
    QDialog::reject();
}
