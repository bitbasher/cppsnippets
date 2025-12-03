#pragma once
#include <QDialog>
#include <QString>

class AboutDialog : public QDialog {
    Q_OBJECT
public:
    explicit AboutDialog(QWidget* parent, const QString& version, const QString& platform, const QString& resourceDir);
};
