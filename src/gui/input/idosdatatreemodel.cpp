#include "input/idosdatatreemodel.h"
#include "case/idoscaseobject.h"
#include "tree/idostreebuilder.h"
#include "tree/idosdatatreeprovider.h"
#include "tree/idostreeproviderregistry.h"

IDOSDataTreeModel::IDOSDataTreeModel(QObject* parent)
    : IDOSTreeModel(parent)
    , m_treeProviderRegistry(nullptr)
{
}

IDOSDataTreeModel::~IDOSDataTreeModel() = default;

void IDOSDataTreeModel::setTreeProviderRegistry(IDOSTreeProviderRegistry* registry)
{
    m_treeProviderRegistry = registry;
}

bool IDOSDataTreeModel::shouldShowObject(const IDOSDataObject* object) const
{
    return object != nullptr && qobject_cast<const IDOSCaseObject*>(object) == nullptr && object->containerId().isEmpty();
}

void IDOSDataTreeModel::buildObjectTree(IDOSTreeNode* parentNode, const IDOSDataObject* object)
{
    if (parentNode == nullptr || object == nullptr || m_treeProviderRegistry == nullptr) return;

    IDOSTreeBuilder builder(parentNode);
    IDOSDataTreeProvider* provider = m_treeProviderRegistry->dataProviderFor(object->typeId());
    if (provider != nullptr) provider->buildTree(builder, object);
}
