#ifndef IDOS_IMPORTED_DATA_TREE_PROVIDER_H
#define IDOS_IMPORTED_DATA_TREE_PROVIDER_H

#include "idos_app.h"
#include "tree/idostreeitemprovider.h"

class QAction;
class QMenu;

class APP_EXPORT IDOSImportedDataTreeProvider : public IDOSTreeItemProvider
{
    Q_OBJECT

  public:
    IDOSImportedDataTreeProvider(IDOSDataObject* data);
    QMenu* createContextMenu(const QList<IDOSTreeItem*>& selectedItems = QList<IDOSTreeItem*>()) override;
    void refresh() override;

  private slots:
    void onSelectSeismicInline();
    void onImportDataset();
    void onRenameObject();
    void onDeleteData();

};

#endif // IDOS_IMPORTED_DATA_TREE_PROVIDER_H
