#ifndef LOCATIONINPUTWIDGET_HPP
#define LOCATIONINPUTWIDGET_HPP

#include <QWidget>

class QLineEdit;
class QPushButton;
class QGroupBox;

/**
 * @brief Widget for inputting a new resource location
 * 
 * Contains Path edit, Name edit, Browse button, and Add button
 * in a group box. Can be hidden when not needed.
 */
class LocationInputWidget : public QWidget {
    Q_OBJECT

public:
    explicit LocationInputWidget(QWidget* parent = nullptr);
    
    QString path() const;
    QString name() const;
    
    void setPath(const QString& path);
    void setName(const QString& name);
    
    void clear();
    
    /**
     * @brief Set whether the widget is editable
     */
    void setReadOnly(bool readOnly);
    bool isReadOnly() const { return m_readOnly; }

signals:
    /**
     * @brief Emitted when Add button is clicked
     */
    void addClicked();
    
    /**
     * @brief Emitted when path or name changes
     */
    void inputChanged();

private slots:
    void onBrowse();
    void onPathChanged();

private:
    QGroupBox* m_groupBox;
    QLineEdit* m_pathEdit;
    QLineEdit* m_nameEdit;
    QPushButton* m_browseButton;
    QPushButton* m_addButton;
    bool m_readOnly = false;
};

#endif // LOCATIONINPUTWIDGET_HPP
