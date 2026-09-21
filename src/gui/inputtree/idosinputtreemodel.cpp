#include "idosinputtreemodel.h"
#include "idoswellitemprovider.h"

IDOSInputTreeModel::IDOSInputTreeModel(QObject* parent)
    : IDOSTreeModel(parent)
{
    setTreeScope(TreeScope::Input);
    registerItemProvider(new IDOSWellItemProvider());
}

IDOSInputTreeModel::~IDOSInputTreeModel() = default;
