#ifndef IDOS_WELL_TREE_PROVIDER_H
#define IDOS_WELL_TREE_PROVIDER_H

#include "idos_app.h"
#include "tree/idostreeitemprovider.h"

class APP_EXPORT IDOSWellTreeProvider : public IDOSTreeItemProvider
{
    Q_OBJECT

  public:
    IDOSWellTreeProvider(IDOSDataObject* data);
    void refresh() override;
    QMenu* createContextMenu(const QList<IDOSTreeItem*>& selectedItems = QList<IDOSTreeItem*>()) override;

  private slots:
    void onAddToCase();
    void onNewWell();
    void onImportWellHeader();
    void onImportWellTrajectory();
    void onImportWellCurves();
    void onImportWellMarkers();
    void onImportWellCompletions();
    void onImportWellCheckshots();
    void onImportWell();
    void onViewWellData();
    void onDeleteWell();
    void onWellSettingsTriggered();
    void onRenameObject();

  private:
    IDOSTreeItem* addItem(IDOSTreeItem* parent, const QString& key, const QString& title);
};

#endif
