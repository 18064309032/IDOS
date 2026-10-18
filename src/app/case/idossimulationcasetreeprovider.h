#ifndef IDOS_SIMULATIONCASE_TREE_PROVIDER_H
#define IDOS_SIMULATIONCASE_TREE_PROVIDER_H

#include "idos_app.h"
#include "tree/idostreeitemprovider.h"

class IDOSTreeItemProviderRegistry;

class APP_EXPORT IDOSSimulationCaseTreeProvider : public IDOSTreeItemProvider
{
    Q_OBJECT

  public:
    IDOSSimulationCaseTreeProvider(IDOSDataObject* data, IDOSTreeItemProviderRegistry* registry);
    void refresh() override;
    QMenu* createContextMenu(const QList<IDOSTreeItem*>& selectedItems = QList<IDOSTreeItem*>()) override;
    bool dependsOn(const QStringList& dataIds) const override;

  private slots:
    void onImportGrid();
    void onImportSummaryResults();
    void onViewSummaryResults();
    void onViewPropertyCurves();
    void onGenerateInput();
    void onRenameCase();
    void onDeleteCase();
    void onRenameObject();

  private:
    IDOSTreeItem* addItem(IDOSTreeItem* parent, const QString& key, const QString& title);
    IDOSTreeItemProviderRegistry* m_registry;
};

#endif
