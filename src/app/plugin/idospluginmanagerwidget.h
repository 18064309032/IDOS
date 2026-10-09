#ifndef IDOS_PLUGIN_MANAGER_WIDGET_H
#define IDOS_PLUGIN_MANAGER_WIDGET_H

#include <QString>
#include <QVector>
#include <QWidget>

class IDOSPluginListModel;
class IDOSPluginRegistry;
class QLabel;
class QLineEdit;
class QModelIndex;
class QSortFilterProxyModel;
class QTreeView;
class IDOSPluginManagerWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSPluginManagerWidget(IDOSPluginRegistry* registry,
                                     QWidget* parent = nullptr);
    void refresh();

  private slots:
    void onCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
    void onPluginDataChanged(const QModelIndex& topLeft,
                             const QModelIndex& bottomRight,
                             const QVector<int>& roles);
    void onSearchTextChanged(const QString& text);
    void onRefreshClicked();

  private:
    void refreshPluginList(const QString& selectedKey = QString());
    void updatePluginDetails(const QModelIndex& proxyIndex);
    void updatePluginSummary();

    IDOSPluginListModel* m_pluginModel;
    QSortFilterProxyModel* m_filterProxy;
    QLabel* m_pageTitle;
    QLabel* m_pageSubtitle;
    QLabel* m_pluginSummary;
    QLineEdit* m_searchEdit;
    QWidget* m_pluginPane;
    QTreeView* m_pluginList;
    QWidget* m_detailsPane;
    QLabel* m_detailsTitle;
    QLabel* m_nameLabel;
    QLineEdit* m_pathEdit;
    QLabel* m_categoryLabel;
    QLabel* m_versionLabel;
    QLabel* m_statusLabel;
    QLabel* m_descriptionLabel;
    QLabel* m_errorLabel;
};

#endif // IDOS_PLUGIN_MANAGER_WIDGET_H
