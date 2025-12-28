#ifndef RESOURCELOCATIONWIDGET_HPP
#define RESOURCELOCATIONWIDGET_HPP

#include <QWidget>
#include <QList>

#include <resInventory/ResourceLocation.h>

class QListView;
class QPushButton;
class QGroupBox;
class LocationInputWidget;
class ResourceLocationModel;

/**
 * @brief Widget for displaying and editing resource locations for a single tier
 * 
 * This widget provides a list view of resource locations with controls for
 * adding, removing, and browsing for new locations. Each location can be
 * enabled/disabled via checkboxes.
 * 
 * Uses model/view architecture with ResourceLocationModel for proper
 * separation of data management from presentation.
 */
class ResourceLocationWidget : public QWidget {
    Q_OBJECT

public:
    explicit ResourceLocationWidget(const QString& title, 
                                     bool allowAdd = true,
                                     bool allowRemove = true,
                                     QWidget* parent = nullptr);
    
    void setLocations(const QList<platformInfo::ResourceLocation>& locations);
    QList<platformInfo::ResourceLocation> locations() const;
    QStringList enabledPaths() const;
    
    ResourceLocationModel* model() const;
    
    void setReadOnly(bool readOnly);
    
    /**
     * @brief Show or hide the input widget for adding new locations
     */
    void setInputVisible(bool visible);
    bool isInputVisible() const;

signals:
    void locationsChanged();
    void locationAdded();
    void rescanLocationsClicked();

private slots:
    void onAddLocation();
    void onRemoveLocation();
    void onSelectionChanged();

private:
    void setupUi(const QString& title, bool allowAdd, bool allowRemove);
    void updateButtons();
    
    QGroupBox* m_groupBox;
    QListView* m_listView;
    ResourceLocationModel* m_model;
    LocationInputWidget* m_inputWidget;
    QPushButton* m_removeButton;
    QPushButton* m_rescanButton;
    bool m_readOnly = false;
    bool m_allowAdd = true;
};

#endif // RESOURCELOCATIONWIDGET_HPP
