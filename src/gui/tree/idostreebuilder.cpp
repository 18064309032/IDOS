#include "tree/idostreebuilder.h"
#include "tree/idostreegroupnode.h"
#include "tree/idosobjecttreenode.h"
#include "tree/idostreepartnode.h"
#include "tree/idostreereferencenode.h"
#include "data/idosdataobject.h"

IDOSTreeBuilder::IDOSTreeBuilder(IDOSTreeNode* parentNode)
    : m_parentNode(parentNode)
{
}

IDOSTreeNode* IDOSTreeBuilder::parentNode() const
{
    return m_parentNode;
}

void IDOSTreeBuilder::setParentNode(IDOSTreeNode* parentNode)
{
    m_parentNode = parentNode;
}

IDOSTreeGroupNode* IDOSTreeBuilder::addGroup(const QString& groupKey, const QString& title)
{
    if (m_parentNode == nullptr)
    {
        return nullptr;
    }

    auto* node = new IDOSTreeGroupNode(m_parentNode);
    node->setGroupKey(groupKey);
    node->setName(title);
    m_parentNode->appendChild(node);
    return node;
}

IDOSObjectTreeNode* IDOSTreeBuilder::addObject(const IDOSDataObject* object, const QString& title)
{
    if (m_parentNode == nullptr || object == nullptr)
    {
        return nullptr;
    }

    auto* node = new IDOSObjectTreeNode(m_parentNode);
    node->setObjectId(object->objectId());
    node->setName(title.isEmpty() ? object->name() : title);
    m_parentNode->appendChild(node);
    return node;
}

IDOSTreePartNode* IDOSTreeBuilder::addPart(const IDOSDataObject* owner,
                                           const IDOSTreePartKey& partKey,
                                           const QString& title,
                                           const QString& itemKey)
{
    if (m_parentNode == nullptr || owner == nullptr)
    {
        return nullptr;
    }

    auto* node = new IDOSTreePartNode(m_parentNode);
    node->setOwnerObjectId(owner->objectId());
    node->setPartKey(partKey);
    node->setItemKey(itemKey);
    node->setName(title);
    m_parentNode->appendChild(node);
    return node;
}

IDOSTreeReferenceNode* IDOSTreeBuilder::addReference(const IDOSCaseItemRef& ref, const QString& title)
{
    if (m_parentNode == nullptr)
    {
        return nullptr;
    }

    auto* node = new IDOSTreeReferenceNode(m_parentNode);
    node->setItemRef(ref);
    node->setName(title);
    m_parentNode->appendChild(node);
    return node;
}

IDOSTreeBuilder IDOSTreeBuilder::childBuilder(IDOSTreeNode* parentNode) const
{
    return IDOSTreeBuilder(parentNode);
}
