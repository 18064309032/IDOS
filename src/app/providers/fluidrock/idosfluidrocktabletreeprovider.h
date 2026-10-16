#ifndef IDOS_FLUID_ROCK_TABLE_TREE_PROVIDER_H
#define IDOS_FLUID_ROCK_TABLE_TREE_PROVIDER_H

#include <QObject>
#include <QString>

#include "idos_app.h"
#include "providers/idostreeitemprovider.h"

class QAction;
class QMenu;

class APP_EXPORT IDOSFluidRockTableTreeProvider : public QObject, public IDOSTreeItemProvider
{
    Q_OBJECT

  public:
    IDOSFluidRockTableTreeProvider(IDOSDataObject* data);
    QMenu* createContextMenu() override;
    void refresh() override;

  private:
    QAction* addAction(QMenu* menu, const QString& text);
};

#endif
