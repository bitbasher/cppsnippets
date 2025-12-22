#ifndef RESOURCELOCATIONWIDGET_HPP
#define RESOURCELOCATIONWIDGET_HPP

#include <QWidget>
#include <QVector>

#include <resInventory/ResourceLocation.h>

class QListWidget;
class QPushButton;
class QGroupBox;
class LocationInputWidget;

/**
 * @brief Widget for displaying and editing resource locations for a single tier
 * 
 * This widget provides a list view of resource locations with controls for
 * adding, removing, and browsing for new locations. Each location can be
 * enabled/disabled via checkboxes.
 */
class ResourceLocationWidget : public QWidget {
    Q_OBJECT

public:
    explicit ResourceLocationWidget(const QString& title, 
                                     bool allowAdd = true,
                                     bool allowRemove = true,
                                     QWidget* parent = nullptr);
    
    void setLocations(const QVector<platformInfo::ResourceLocation>& locations);
    QVector<platformInfo::ResourceLocation> locations() const;
    QStringList enabledPaths() const;
    
    void setReadOnly(bool readOnly);
    
    /**
     * @brief Show or hide the input widget for adding new locations
     */
    void setInputVisible(bool visible);
    bool isInputVisible() const;

signals:
    void locationsChanged();
    void locationAdded();

private slots:
    void onAddLocation();
    void onRemoveLocation();
    void onItemChanged();
    void onSelectionChanged();

private:
    void setupUi(const QString& title, bool allowAdd, bool allowRemove);
    void updateButtons();
    
    QGroupBox* m_groupBox;
    QListWidget* m_listWidget;
    LocationInputWidget* m_inputWidget;
    QPushButton* m_removeButton;
    bool m_readOnly = false;
    bool m_allowAdd = true;
    
    QVector<platformInfo::ResourceLocation> m_locations;
};

#endif // RESOURCELOCATIONWIDGET_HPP
