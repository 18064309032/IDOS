#ifndef IDOS_TREE_MENU_PROVIDER_H
#define IDOS_TREE_MENU_PROVIDER_H

#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QtCore/qnamespace.h>

#include "idos_gui.h"

class IDOSTreeItem;
class IDOSTreeView;
class QMenu;
class QMimeData;

class GUI_EXPORT IDOSTreeMenuProvider : public QObject
{
    Q_OBJECT

  public:
    explicit IDOSTreeMenuProvider(IDOSTreeView* view);
    ~IDOSTreeMenuProvider() override;
    virtual QMenu* createContextMenu(const QList<IDOSTreeItem*>& selectedItems,
                                     IDOSTreeItem* contextItem) = 0;
    virtual QMimeData* createMimeData(const QList<IDOSTreeItem*>& items) const;
    virtual bool canCut(const QMimeData* data) const;
    virtual bool canPaste(const QMimeData* data, const IDOSTreeItem* target,
                          Qt::DropAction action) const;
    virtual bool paste(const QMimeData* data, IDOSTreeItem* target,
                       Qt::DropAction action);
    IDOSTreeView* view() const;

private slots:
    void onContextMenuAboutToShow(QMenu* menu);

private:
    QPointer<IDOSTreeView> m_view;
};

#endif
