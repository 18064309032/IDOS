#ifndef IDOS_FLUID_ROCK_TABLE_TREE_PROVIDER_H
#define IDOS_FLUID_ROCK_TABLE_TREE_PROVIDER_H

#include <QString>

#include "idos_app.h"
#include "tree/idostreeitemprovider.h"

class QAction;
class QMenu;

class APP_EXPORT IDOSFluidRockTableTreeProvider : public IDOSTreeItemProvider
{
    Q_OBJECT

  public:
    IDOSFluidRockTableTreeProvider(IDOSDataObject* data);
    QMenu* createContextMenu(const QList<IDOSTreeItem*>& selectedItems = QList<IDOSTreeItem*>()) override;
    void refresh() override;

  private slots:
    void onAddToCase();
    void onImportFluidRockTables();
    void onImportDataset();
    void onAddFluidRockTableToCase();
    void onRenameObject();
    void onDeleteData();

};

#endif
