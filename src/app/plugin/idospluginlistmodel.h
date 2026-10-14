#ifndef IDOS_PLUGIN_LIST_MODEL_H
#define IDOS_PLUGIN_LIST_MODEL_H

#include <QAbstractItemModel>
#include <QIcon>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class IDOSPluginRegistry;

class IDOSPluginListModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    enum PluginDataRole
    {
        PluginKeyRole = Qt::UserRole + 1,
        PluginIconRole,
        PluginDescriptionRole,
        PluginCategoryRole,
        PluginVersionRole,
        PluginErrorRole,
        PluginLoadedRole,
        SearchTextRole
    };

    explicit IDOSPluginListModel(IDOSPluginRegistry* registry, QObject* parent = nullptr);

    QModelIndex index(int row,
                      int column,
                      const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index,
                 const QVariant& value,
                 int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    void refresh();
    int rowForKey(const QString& key) const;

  private:
    QString statusText(const QVariantMap& plugin) const;

    IDOSPluginRegistry* m_registry;
    QVariantList m_plugins;
    QIcon m_pluginIcon;
};

#endif // IDOS_PLUGIN_LIST_MODEL_H
