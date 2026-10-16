#ifndef IDOS_SIMULATIONCASE_TREE_PROVIDER_H
#define IDOS_SIMULATIONCASE_TREE_PROVIDER_H

#include "idos_app.h"
#include "providers/idostreeitemprovider.h"

class IDOSTreeItemProviderRegistry;

class APP_EXPORT IDOSSimulationCaseTreeProvider : public IDOSTreeItemProvider
{
  public:
    IDOSSimulationCaseTreeProvider(IDOSDataObject* data, IDOSTreeItemProviderRegistry* registry);
    void refresh() override;
    bool dependsOn(const QStringList& dataIds) const override;

  private:
    IDOSTreeItem* addItem(IDOSTreeItem* parent, const QString& key, const QString& title);
    IDOSTreeItemProviderRegistry* m_registry;
};

#endif
