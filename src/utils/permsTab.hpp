#ifndef PERMSTAB_HPP
#define PERMSTAB_HPP

#include <QWidget>
#include <QFileInfo>

// ============================================================================
// PermissionsTab - displays file permissions
// ============================================================================
class PermissionsTab : public QWidget {
    Q_OBJECT
public:
    explicit PermissionsTab(const QFileInfo& fileInfo, QWidget* parent = nullptr);
};

#endif // PERMSTAB_HPP