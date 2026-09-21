#ifndef IDOS_MODEL_TREE_MODEL_H
#define IDOS_MODEL_TREE_MODEL_H

#include "../tree/idostreenode.h"
#include "idosmodeltreenode.h"
#include "idos_core.h"
#include <QAbstractItemModel>

class IDOSProject;
class IDOSDataObject;

/**
 * @brief Model Tree 的 QAbstractItemModel。
 *
 * 只显示 Project 中的网格模型（IDOSGrid）。
 * 节点存 objectId，通过 Project 查询实际对象。
 */
class GUI_EXPORT IDOSModelTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit IDOSModelTreeModel(QObject* parent = nullptr);
    ~IDOSModelTreeModel() override;

    /** 设置工程，开始监听。 */
    void setProject(IDOSProject* project);

    // ===== QAbstractItemModel 接口 =====

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    /** 从 QModelIndex 取出对应的树节点。 */
    IDOSModelTreeNode* nodeFromIndex(const QModelIndex& index) const;

    /** 从 QModelIndex 取出对应的领域对象（通过 Project 查）。 */
    IDOSDataObject* objectFromIndex(const QModelIndex& index) const;

private slots:
    void onObjectAdded(const QString& objectId);
    void onObjectRemoved(const QString& objectId);
    void onObjectChanged(const QString& objectId);

private:
    void rebuildTree();
    IDOSModelTreeNode* findNodeByObjectId(const QString& objectId) const;

    IDOSProject* m_project;
    IDOSModelTreeNode* m_rootNode;
};

#endif // IDOS_MODEL_TREE_MODEL_H
