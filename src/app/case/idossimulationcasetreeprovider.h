#ifndef IDOS_SIMULATIONCASE_TREE_PROVIDER_H
#define IDOS_SIMULATIONCASE_TREE_PROVIDER_H

#include "idos_app.h"
#include "idostreeprovider.h"

class IDOSTreeRegistry;

class APP_EXPORT IDOSSimulationCaseTreeProvider : public IDOSTreeProvider
{
  public:
    IDOSSimulationCaseTreeProvider(IDOSDataObject* data, IDOSTreeRegistry* registry);
    void populate(IDOSTreeItem* item) override;
    bool dependsOn(const QStringList& dataIds) const override;

  private:
    IDOSTreeItem* addItem(IDOSTreeItem* parent, const QString& key, const QString& title);
    IDOSTreeRegistry* m_registry;
};

#endif
