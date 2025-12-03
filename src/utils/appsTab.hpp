#ifndef APPSTAB_HPP
#define APPSTAB_HPP

#include <QWidget>
#include <QFileInfo>

// ============================================================================
// ApplicationsTab - displays application associations
// ============================================================================
class ApplicationsTab : public QWidget {
    Q_OBJECT
public:
    explicit ApplicationsTab(const QFileInfo& fileInfo, QWidget* parent = nullptr);
};

#endif // APPSTAB_HPP