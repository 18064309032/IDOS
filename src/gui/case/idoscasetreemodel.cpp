#include "case/idoscasetreemodel.h"
#include "case/idoscaseobject.h"
#include "tree/idostreebuilder.h"
#include "tree/idoscasetreeprovider.h"
#include "tree/idostreeproviderregistry.h"

IDOSCaseTreeModel::IDOSCaseTreeModel(QObject* parent)
    : IDOSTreeModel(parent)
    , m_treeProviderRegistry(nullptr)
{
}

IDOSCaseTreeModel::~IDOSCaseTreeModel() = default;

void IDOSCaseTreeModel::setTreeProviderRegistry(IDOSTreeProviderRegistry* registry)
{
    m_treeProviderRegistry = registry;
}

bool IDOSCaseTreeModel::shouldShowObject(const IDOSDataObject* object) const
{
    return qobject_cast<const IDOSCaseObject*>(object) != nullptr;
}

void IDOSCaseTreeModel::buildObjectTree(IDOSTreeNode* parentNode, const IDOSDataObject* object)
{
    auto* caseObject = qobject_cast<const IDOSCaseObject*>(object);
    if (parentNode == nullptr || caseObject == nullptr || m_treeProviderRegistry == nullptr) return;

    IDOSTreeBuilder builder(parentNode);
    IDOSCaseTreeProvider* provider = m_treeProviderRegistry->caseProviderFor(caseObject->caseTypeId());
    if (provider != nullptr) provider->buildCaseTree(builder, caseObject);
}
