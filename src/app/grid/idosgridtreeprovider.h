#ifndef IDOS_GRID_TREE_PROVIDER_H
#define IDOS_GRID_TREE_PROVIDER_H

#include <QtCore/qnamespace.h>

#include "idos_app.h"
#include "tree/idostreeitemprovider.h"

class QAction;
class IDOSGridFilterObject;
class IDOSGridPropertyObject;

class APP_EXPORT IDOSGridTreeProvider : public IDOSTreeItemProvider
{
    Q_OBJECT

  public:
    IDOSGridTreeProvider(IDOSDataObject* data);
    bool dependsOn(const QStringList& dataIds) const override;
    void refresh() override;
    QMenu* createContextMenu(const QList<IDOSTreeItem*>& selectedItems = QList<IDOSTreeItem*>()) override;

  private slots:
    void onGridSettingsTriggered();
    void onGridRenameFilter();
    void onGridDeleteFilter();
    void onGridImportDynamicProperties();
    void onGridImportProperty();
    void onGridDeleteProperty();
    void onGridDelete();
    void onGridDisplayTriggered();
    void onRenameObject();

  private:
    IDOSTreeItem* addItem(IDOSTreeItem* parent, const QString& key, const QString& title,
                          IDOSDataObject* data = nullptr);
};

#endif
