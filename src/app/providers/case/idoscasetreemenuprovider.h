#ifndef IDOS_CASE_TREE_MENU_PROVIDER_H
#define IDOS_CASE_TREE_MENU_PROVIDER_H

#include <QJsonObject>
#include <QList>
#include <QString>

#include "idos_app.h"
#include "tree/idostreemenuprovider.h"

class IDOSDataObject;
class QAction;
class QMenu;
class QMimeData;

class APP_EXPORT IDOSCaseTreeMenuProvider : public IDOSTreeMenuProvider
{
    Q_OBJECT
public:
    explicit IDOSCaseTreeMenuProvider(IDOSTreeView* view);
    ~IDOSCaseTreeMenuProvider() override;
    QMenu* createContextMenu(const QList<IDOSTreeItem*>& selectedItems,
                             IDOSTreeItem* contextItem) override;
    QMimeData* createMimeData(const QList<IDOSTreeItem*>& items) const override;
    bool canCut(const QMimeData* data) const override;
    bool canPaste(const QMimeData* data, const IDOSTreeItem* target, Qt::DropAction action) const override;
    bool paste(const QMimeData* data, IDOSTreeItem* target, Qt::DropAction action) override;
private:
    QAction* addAction(QMenu* menu, IDOSTreeView* view, const QString& text);
    void addCaseActions(QMenu* menu, IDOSTreeView* view);
    void addTransferActions(QMenu* menu);
    QJsonObject readTransfer(const QMimeData* data) const;
    QJsonObject transferArguments(const QMimeData* data, const IDOSTreeItem* target) const;
};

#endif
