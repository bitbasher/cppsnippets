#ifndef GENERALTAB_HPP
#define GENERALTAB_HPP

#include <QWidget>
#include <QFileInfo>

// ============================================================================
// GeneralTab - displays general file information
// ============================================================================
class GeneralTab : public QWidget {
    Q_OBJECT
public:
    explicit GeneralTab(const QFileInfo& fileInfo, QWidget* parent = nullptr);
};

#endif // GENERALTAB_HPP