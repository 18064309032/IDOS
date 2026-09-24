#ifndef IDOS_TREE_BUILDER_H
#define IDOS_TREE_BUILDER_H

#include "idos_gui.h"
#include "tree/idostreepartkey.h"
#include "case/idoscaseitemref.h"

#include <QString>

class IDOSDataObject;
class IDOSTreeGroupNode;
class IDOSTreeNode;
class IDOSObjectTreeNode;
class IDOSTreePartNode;
class IDOSTreeReferenceNode;

/**
 * @brief 通用树节点构建器。
 *
 * Provider 通过构建器创建节点，避免二次开发直接维护 nodeId、domain、
 * objectId 等内部字段。
 */
class GUI_EXPORT IDOSTreeBuilder
{
public:
    explicit IDOSTreeBuilder(IDOSTreeNode* parentNode = nullptr);

    IDOSTreeNode* parentNode() const;
    void setParentNode(IDOSTreeNode* parentNode);

    IDOSTreeGroupNode* addGroup(const QString& groupKey, const QString& title);
    IDOSObjectTreeNode* addObject(const IDOSDataObject* object, const QString& title = QString());
    IDOSTreePartNode* addPart(const IDOSDataObject* owner,
                              const IDOSTreePartKey& partKey,
                              const QString& title,
                              const QString& itemKey = QString());
    IDOSTreeReferenceNode* addReference(const IDOSCaseItemRef& ref, const QString& title);

    IDOSTreeBuilder childBuilder(IDOSTreeNode* parentNode) const;

private:
    IDOSTreeNode* m_parentNode;
};

#endif // IDOS_TREE_BUILDER_H
