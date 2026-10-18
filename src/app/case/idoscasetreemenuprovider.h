#ifndef IDOS_CASE_TREE_MENU_PROVIDER_H
#define IDOS_CASE_TREE_MENU_PROVIDER_H

#include <QJsonObject>
#include <QList>
#include <QString>

#include "idos_app.h"
#include "tree/idostreemenuprovider.h"

class IDOSDataObject;
class IDOSCaseTreeView;
class QAction;
class QMenu;
class QMimeData;

class APP_EXPORT IDOSCaseTreeMenuProvider : public IDOSTreeMenuProvider
{
    Q_OBJECT
public:
    explicit IDOSCaseTreeMenuProvider(IDOSCaseTreeView* view);
    ~IDOSCaseTreeMenuProvider() override;
    QMenu* createContextMenu(const QList<IDOSTreeItem*>& selectedItems,
                             IDOSTreeItem* contextItem) override;
    QMimeData* createMimeData(const QList<IDOSTreeItem*>& items) const override;
    bool canCut(const QMimeData* data) const override;
    bool canPaste(const QMimeData* data, const IDOSTreeItem* target, Qt::DropAction action) const override;
    bool paste(const QMimeData* data, IDOSTreeItem* target, Qt::DropAction action) override;
private:
    QJsonObject readTransfer(const QMimeData* data) const;
    QJsonObject transferArguments(const QMimeData* data, const IDOSTreeItem* target) const;
  private slots:
    void onNewCase();
    void onImportCase();
};

#endif
