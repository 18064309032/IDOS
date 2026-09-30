#ifndef IDOS_DATA_TREE_MENU_PROVIDER_H
#define IDOS_DATA_TREE_MENU_PROVIDER_H

#include "idostreemenuprovider.h"
#include "idos_app.h"

class IDOSDataTreeView;

class APP_EXPORT IDOSDataTreeMenuProvider : public IDOSTreeMenuProvider
{
    Q_OBJECT
  public:
    explicit IDOSDataTreeMenuProvider(IDOSDataTreeView* view);
    QMenu* createContextMenu() override;

  private slots:
    void onNewWellTriggered();
    void onImportWellDataTriggered();
    void onImportWellDataSingleTriggered();
    void onDeleteWellTriggered();

  private:
    IDOSDataTreeView* m_view;
};

#endif
