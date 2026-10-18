#ifndef IDOS_POINT_SET_TREE_PROVIDER_H
#define IDOS_POINT_SET_TREE_PROVIDER_H

#include "tree/idostreeitemprovider.h"

class QMenu;

class IDOSPointSetTreeProvider : public IDOSTreeItemProvider
{
    Q_OBJECT

  public:
    IDOSPointSetTreeProvider(IDOSDataObject* data);
    void refresh() override;
    QMenu* createContextMenu(const QList<IDOSTreeItem*>& selectedItems = QList<IDOSTreeItem*>()) override;

  signals:
    void deletePointSetRequested(const QString& dataId, const QString& session);

  private slots:
    void onDeletePointSet();
};

#endif
