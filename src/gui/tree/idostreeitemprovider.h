#ifndef IDOS_TREE_ITEM_PROVIDER_H
#define IDOS_TREE_ITEM_PROVIDER_H

#include "idos_gui.h"
#include "data/idosdataobject.h"
#include <QMenu>
#include <QString>

class IDOSTreeModel;
class IDOSTreeNode;

/**
 * @brief 按类型注册的树节点构建策略（扩展点）。
 *
 * 每种数据类型注册一个 provider，决定该类型对象在树上的子节点结构与
 * 右键菜单（仿 QGIS QgsDataItemProvider 模式）。树模型本身不认识具体
 * 类型，只按 IDOSDataObject::typeId() 查表分发——插件注册自己的
 * provider 即可让新类型上树。
 *
 * 非 QObject，由注册方（通常为 IDOSTreeModel 子类）管理生命周期。
 */
class GUI_EXPORT IDOSTreeItemProvider
{
public:
    virtual ~IDOSTreeItemProvider() = default;

    /** 本 provider 服务的类型标识（与 IDOSDataObject::typeId() 对应）。 */
    virtual QString typeId() const = 0;

    /**
     * @brief 对象在树节点上的显示名。
     *
     * 显示名的唯一裁决点：构建子树与 objectChanged 刷新都经此取名字，
     * 保证两处一致（如网格属性优先显示 keyword 而非 name）。
     */
    virtual QString displayName(const IDOSDataObject* obj) const
    {
        return obj != nullptr ? obj->name() : QString();
    }

    /**
     * @brief 构建对象的子节点。
     * @param model 关联的模型（可通过 Project 查询数据）。
     * @param parentNode 对象对应的树节点，子节点挂其下。
     * @param obj 对应的领域对象。
     */
    virtual void buildChildren(IDOSTreeModel* model, IDOSTreeNode* parentNode,
                               IDOSDataObject* obj) = 0;

    /**
     * @brief 子节点结构是否需要重建。
     *
     * 模型收到 objectChanged 时调用：返回 false 则只刷新对象节点自身，
     * 返回 true 则整体重建子树。结构静态的类型（如 Grid）用默认实现。
     * 需要 Project 数据做比对时通过 model->project() 获取。
     */
    virtual bool needsRebuild(IDOSTreeModel* model, IDOSTreeNode* parentNode,
                              IDOSDataObject* obj)
    {
        Q_UNUSED(model);
        Q_UNUSED(parentNode);
        Q_UNUSED(obj);
        return false;
    }

    /** 向对象节点的右键菜单追加类型特定动作。默认无。 */
    virtual void addMenuActions(QMenu* menu, IDOSDataObject* obj)
    {
        Q_UNUSED(menu);
        Q_UNUSED(obj);
    }
};

#endif // IDOS_TREE_ITEM_PROVIDER_H
