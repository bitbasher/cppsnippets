#ifndef TABDIALOG_HPP
#define TABDIALOG_HPP

#include <QDialog>

class QTabWidget;
class QDialogButtonBox;

// ============================================================================
// TabDialog - assembles the tabs
// ============================================================================
class TabDialog : public QDialog {
    Q_OBJECT
public:
    explicit TabDialog(const QString& fileName, QWidget* parent = nullptr);
private:
    QTabWidget* tabWidget;
    QDialogButtonBox* buttonBox;
};

#endif // TABDIALOG_HPP