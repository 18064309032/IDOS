#ifndef IDOS_TREE_GUI_PROVIDER_H
#define IDOS_TREE_GUI_PROVIDER_H

#include <QList>
#include <QObject>
#include <QString>

#include "idos_gui.h"

class IDOSTreeItem;
class IDOSTreeView;
class QMenu;
class QMimeData;

class GUI_EXPORT IDOSTreeGuiProvider : public QObject
{
    Q_OBJECT

  public:
    explicit IDOSTreeGuiProvider(QObject* parent = nullptr);
    ~IDOSTreeGuiProvider() override;
    virtual QString name() const = 0;
    virtual bool supports(const IDOSTreeItem* item) const = 0;
    virtual void populateMenu(QMenu* menu, IDOSTreeView* view) = 0;
    virtual QMimeData* createMimeData(const QList<IDOSTreeItem*>& items, const IDOSTreeView* view) const;
    virtual bool canCut(const QMimeData* data, const IDOSTreeView* view) const;
    virtual bool canPaste(const QMimeData* data, const IDOSTreeItem* target,
                          Qt::DropAction action, const IDOSTreeView* view) const;
    virtual bool paste(const QMimeData* data, IDOSTreeItem* target,
                       Qt::DropAction action, IDOSTreeView* view);
    virtual void populateSelectionMenu(QMenu* menu, IDOSTreeView* view);
    virtual bool isCheckable(const IDOSTreeItem* item) const;
    virtual bool isChecked(const IDOSTreeItem* item, const IDOSTreeView* view) const;
    virtual bool setChecked(IDOSTreeItem* item, bool checked, IDOSTreeView* view);

  signals:
    void checkStateChanged();
};

#endif
