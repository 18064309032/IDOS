#include <QStringList>
#include <QVariantMap>
#include <QVector>

#include "log/idoslogger.h"

#include "idospluginregistry.h"

#include "idospluginlistmodel.h"

IDOSPluginListModel::IDOSPluginListModel(IDOSPluginRegistry* registry, QObject* parent)
    : QAbstractItemModel(parent)
    , m_registry(registry)
    , m_pluginIcon(QStringLiteral(":/images/app-plugin.svg"))
{
}

QModelIndex IDOSPluginListModel::index(int row,
                                       int column,
                                       const QModelIndex& parentIndex) const
{
    if (parentIndex.isValid() || row < 0 || row >= m_plugins.size() || column < 0
        || column >= columnCount())
    {
        return QModelIndex();
    }
    return createIndex(row, column);
}

QModelIndex IDOSPluginListModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

int IDOSPluginListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_plugins.size();
}

int IDOSPluginListModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return 2;
}

QVariant IDOSPluginListModel::data(const QModelIndex& modelIndex, int role) const
{
    if (!modelIndex.isValid() || modelIndex.row() < 0 || modelIndex.row() >= m_plugins.size()
        || modelIndex.column() < 0 || modelIndex.column() >= columnCount())
    {
        return QVariant();
    }

    const QVariantMap plugin = m_plugins.at(modelIndex.row()).toMap();
    if (role == Qt::DisplayRole)
    {
        if (modelIndex.column() == 0)
        {
            return plugin.value(QStringLiteral("name"));
        }
        return statusText(plugin);
    }
    if (role == Qt::DecorationRole && modelIndex.column() == 0)
    {
        return m_pluginIcon;
    }
    if (role == Qt::CheckStateRole && modelIndex.column() == 0)
    {
        return plugin.value(QStringLiteral("enabled")).toBool() ? Qt::Checked : Qt::Unchecked;
    }
    if (role == PluginKeyRole)
    {
        return plugin.value(QStringLiteral("key"));
    }
    if (role == PluginPathRole)
    {
        return plugin.value(QStringLiteral("path"));
    }
    if (role == PluginDescriptionRole)
    {
        return plugin.value(QStringLiteral("description"));
    }
    if (role == PluginCategoryRole)
    {
        return plugin.value(QStringLiteral("category"));
    }
    if (role == PluginVersionRole)
    {
        return plugin.value(QStringLiteral("version"));
    }
    if (role == PluginErrorRole)
    {
        return plugin.value(QStringLiteral("error"));
    }
    if (role == PluginLoadedRole)
    {
        return plugin.value(QStringLiteral("loaded"));
    }
    if (role == SearchTextRole)
    {
        const QStringList searchFields = QStringList()
                                             << plugin.value(QStringLiteral("name")).toString()
                                             << plugin.value(QStringLiteral("description")).toString()
                                             << plugin.value(QStringLiteral("category")).toString();
        return searchFields.join(QLatin1Char(' '));
    }
    return QVariant();
}

bool IDOSPluginListModel::setData(const QModelIndex& modelIndex,
                                  const QVariant& value,
                                  int role)
{
    if (!modelIndex.isValid() || modelIndex.row() < 0 || modelIndex.row() >= m_plugins.size()
        || modelIndex.column() != 0 || role != Qt::CheckStateRole || m_registry == nullptr)
    {
        return false;
    }

    QVariantMap plugin = m_plugins.at(modelIndex.row()).toMap();
    const QString pluginPath = plugin.value(QStringLiteral("path")).toString();
    const QString pluginKey = plugin.value(QStringLiteral("key")).toString();
    const bool enabled = value.toInt() == Qt::Checked;
    IDOS_DEBUG(tr("Plugin manager checkbox changed: key=%1, enabled=%2, path=%3")
                   .arg(pluginKey)
                   .arg(enabled)
                   .arg(pluginPath));
    m_registry->setPluginEnabled(pluginPath, enabled);

    plugin.insert(QStringLiteral("enabled"), enabled);
    plugin.insert(QStringLiteral("loaded"), m_registry->isLoaded(pluginKey));
    plugin.insert(QStringLiteral("error"), m_registry->pluginError(pluginKey));
    m_plugins.replace(modelIndex.row(), plugin);
    IDOS_DEBUG(tr("Plugin manager checkbox handling finished: key=%1, enabled=%2, loaded=%3, error=%4")
                   .arg(pluginKey)
                   .arg(enabled)
                   .arg(m_registry->isLoaded(pluginKey))
                   .arg(m_registry->pluginError(pluginKey)));

    QVector<int> changedRoles;
    changedRoles << Qt::CheckStateRole << Qt::DisplayRole << PluginErrorRole
                 << PluginLoadedRole;
    emit dataChanged(index(modelIndex.row(), 0), index(modelIndex.row(), 1), changedRoles);
    return true;
}

Qt::ItemFlags IDOSPluginListModel::flags(const QModelIndex& modelIndex) const
{
    Qt::ItemFlags itemFlags = QAbstractItemModel::flags(modelIndex);
    if (modelIndex.isValid() && modelIndex.column() == 0)
    {
        itemFlags |= Qt::ItemIsUserCheckable;
    }
    return itemFlags;
}

QVariant IDOSPluginListModel::headerData(int section,
                                         Qt::Orientation orientation,
                                         int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        if (section == 0)
        {
            return tr("Plugin");
        }
        if (section == 1)
        {
            return tr("Status");
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

void IDOSPluginListModel::refresh()
{
    beginResetModel();
    if (m_registry == nullptr)
    {
        m_plugins.clear();
    }
    else
    {
        m_plugins = m_registry->pluginCatalog(m_registry->libraryDir().path());
    }
    endResetModel();
}

int IDOSPluginListModel::rowForKey(const QString& key) const
{
    for (int row = 0; row < m_plugins.size(); ++row)
    {
        const QVariantMap plugin = m_plugins.at(row).toMap();
        if (plugin.value(QStringLiteral("key")).toString() == key)
        {
            return row;
        }
    }
    return -1;
}

QString IDOSPluginListModel::statusText(const QVariantMap& plugin) const
{
    if (!plugin.value(QStringLiteral("error")).toString().isEmpty())
    {
        return tr("Load error");
    }
    if (plugin.value(QStringLiteral("loaded")).toBool())
    {
        return tr("Loaded");
    }
    if (plugin.value(QStringLiteral("enabled")).toBool())
    {
        return tr("Enabled");
    }
    return tr("Disabled");
}
