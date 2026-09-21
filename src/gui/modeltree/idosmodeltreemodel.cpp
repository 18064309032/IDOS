#include "idosmodeltreemodel.h"
#include "idosgriditemprovider.h"
#include "idosmodelitemprovider.h"

IDOSModelTreeModel::IDOSModelTreeModel(QObject* parent)
    : IDOSTreeModel(parent)
{
    setTreeScope(TreeScope::Model);
    registerItemProvider(new IDOSGridItemProvider());
    registerItemProvider(new IDOSModelItemProvider());
}

IDOSModelTreeModel::~IDOSModelTreeModel() = default;
