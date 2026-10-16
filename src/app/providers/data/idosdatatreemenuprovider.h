#ifndef IDOS_DATA_TREE_MENU_PROVIDER_H
#define IDOS_DATA_TREE_MENU_PROVIDER_H

#include <QJsonObject>
#include <QList>
#include <QString>

#include "idos_app.h"
#include "tree/idostreemenuprovider.h"

class IDOSDataObject;
class QAction;
class QMenu;
class QMimeData;

class APP_EXPORT IDOSDataTreeMenuProvider : public IDOSTreeMenuProvider
{
    Q_OBJECT
public:
    explicit IDOSDataTreeMenuProvider(IDOSTreeView* view);
    ~IDOSDataTreeMenuProvider() override;
    QMenu* createContextMenu(const QList<IDOSTreeItem*>& selectedItems,
                             IDOSTreeItem* contextItem) override;
    QMimeData* createMimeData(const QList<IDOSTreeItem*>& items) const override;
    bool canCut(const QMimeData* data) const override;
    bool canPaste(const QMimeData* data, const IDOSTreeItem* target, Qt::DropAction action) const override;
    bool paste(const QMimeData* data, IDOSTreeItem* target, Qt::DropAction action) override;
private:
    void addDeleteActions(QMenu* menu, IDOSTreeView* view, const QList<IDOSTreeItem*>& selectedItems);
    void addTransferActions(QMenu* menu);
    QAction* addTransferAction(QMenu* menu, const QString& text);
    QAction* addGridAction(QMenu* menu, IDOSTreeView* view, const QString& text);
    void addGridActions(QMenu* menu, IDOSTreeView* view);
    void addGridDisplayActions(QMenu* menu, IDOSTreeView* view);
    QAction* addGridDisplayAction(QMenu* menu, IDOSTreeView* view, const QString& text, const QString& gridId, const QString& propertyId);
    void addWellActions(QMenu* menu, IDOSTreeView* view);
    bool supportsWellDisplayItem(const IDOSTreeItem* item) const;
    void addWellDisplayActions(QMenu* menu, IDOSTreeView* view);
    QJsonObject readTransfer(const QMimeData* data) const;
    QJsonObject transferArguments(const QMimeData* data, const IDOSTreeItem* target) const;
    bool supportsDelete(const IDOSDataObject* data) const;
};

#endif
