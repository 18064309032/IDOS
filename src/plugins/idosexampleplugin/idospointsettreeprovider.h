#ifndef IDOS_POINT_SET_TREE_PROVIDER_H
#define IDOS_POINT_SET_TREE_PROVIDER_H

#include <QObject>

#include "providers/idostreeitemprovider.h"

class QMenu;

class IDOSPointSetTreeProvider : public QObject, public IDOSTreeItemProvider
{
    Q_OBJECT

  public:
    IDOSPointSetTreeProvider(IDOSDataObject* data);
    void refresh() override;
    QMenu* createContextMenu() override;

};

#endif
