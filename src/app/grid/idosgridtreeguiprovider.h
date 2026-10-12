#ifndef IDOS_GRID_TREE_GUI_PROVIDER_H
#define IDOS_GRID_TREE_GUI_PROVIDER_H

#include "idos_app.h"
#include "idostreeguiprovider.h"

class QAction;
class IDOSRenderObject;
class IDOSRenderView;

class APP_EXPORT IDOSGridTreeGuiProvider : public IDOSTreeGuiProvider
{
    Q_OBJECT

  public:
    QString name() const override;
    bool supports(const IDOSTreeItem* item) const override;
    void populateMenu(QMenu* menu, IDOSTreeView* view) override;
    bool isChecked(const IDOSTreeItem* item, const IDOSTreeView* view) const override;
    bool setChecked(IDOSTreeItem* item, bool checked, IDOSTreeView* view) override;

  private slots:
    void onImportProperty();
    void onImportDynamicProperties();
    void onDeleteProperty();
    void onDeleteGrid();

  private:
    int gridPropertyCount(const IDOSTreeView* view, const QString& gridId) const;
    bool deleteProperty(IDOSTreeView* view, const QString& propertyId);
    bool importProperty(IDOSTreeView* view, const QString& targetGridId, const QString& targetCaseId);
    bool importDynamicProperties(IDOSTreeView* view, const QString& targetGridId);
    bool deleteGrid(IDOSTreeView* view, const QString& gridId);
    IDOSRenderObject* gridRenderObject(const QString& gridId, IDOSRenderView* view) const;
    QAction* addAction(QMenu* menu, IDOSTreeView* view, const QString& text);
    IDOSTreeView* actionView() const;
};

#endif
