#ifndef RESOURCELOCATIONMODEL_H
#define RESOURCELOCATIONMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <resInventory/ResourceLocation.h>

/**
 * @brief Model for resource locations following Qt model/view architecture
 * 
 * Implements QAbstractListModel to manage a list of ResourceLocation items
 * with enabled/disabled state via Qt::CheckStateRole.
 * 
 * This model separates data management from presentation, allowing proper
 * decoupling between the model and any views that display it.
 * 
 * Usage:
 * @code
 *   ResourceLocationModel* model = new ResourceLocationModel;
 *   model->setLocations(locList);
 *   
 *   QListView* view = new QListView;
 *   view->setModel(model);
 *   
 *   // Later, retrieve the state (with enabled flags)
 *   QList<platformInfo::ResourceLocation> currentState = model->locations();
 *   QStringList enabledPaths = model->enabledPaths();
 * @endcode
 */
class ResourceLocationModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit ResourceLocationModel(QObject* parent = nullptr);
    
    // Data management
    void setLocations(const QList<platformInfo::ResourceLocation>& locations);
    QList<platformInfo::ResourceLocation> locations() const;
    QStringList enabledPaths() const;
    
    // QAbstractListModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    QList<platformInfo::ResourceLocation> m_locations;
};

#endif // RESOURCELOCATIONMODEL_H
