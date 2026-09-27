#ifndef IDOS_CASE_TREE_MENU_PROVIDER_H
#define IDOS_CASE_TREE_MENU_PROVIDER_H
#include "idos_app.h"
#include "idostreemenuprovider.h"
class IDOSCaseTreeView;
class APP_EXPORT IDOSCaseTreeMenuProvider : public IDOSTreeMenuProvider
{
    Q_OBJECT
  public:
    explicit IDOSCaseTreeMenuProvider(IDOSCaseTreeView* view);
    QMenu* createContextMenu() override;
  private slots:
    void onNewCase();
    void onImportCase();

  private:
    IDOSCaseTreeView* m_view;
};
#endif
