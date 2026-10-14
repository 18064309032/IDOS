#ifndef IDOS_FLUID_ROCK_TABLE_TREE_GUI_PROVIDER_H
#define IDOS_FLUID_ROCK_TABLE_TREE_GUI_PROVIDER_H

#include "idos_app.h"
#include "idostreeguiprovider.h"

class QAction;

class APP_EXPORT IDOSFluidRockTableTreeGuiProvider : public IDOSTreeGuiProvider
{
    Q_OBJECT

  public:
    QString name() const override;
    bool supports(const IDOSTreeItem* item) const override;
    void populateMenu(QMenu* menu, IDOSTreeView* view) override;

  private slots:
    void onImportTables();
    void onAddTableToCase();

  private:
    QAction* addAction(QMenu* menu, IDOSTreeView* view, const QString& text);
    IDOSTreeView* actionView() const;
};

#endif
