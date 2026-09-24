#ifndef IDOS_TREE_MODEL_H
#define IDOS_TREE_MODEL_H

#include "idos_gui.h"
#include "tree/idostreenode.h"

#include <QAbstractItemModel>

class IDOSDataObject;
class IDOSProject;

/**
 * @brief 通用树模型基类。
 *
 * 只负责 Qt Model/View 适配、工程对象变化监听和根节点生命周期。
 */
class GUI_EXPORT IDOSTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit IDOSTreeModel(QObject* parent = nullptr);
    ~IDOSTreeModel() override;

    void setProject(IDOSProject* project);
    IDOSProject* project() const;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    IDOSTreeNode* nodeFromIndex(const QModelIndex& index) const;
    IDOSDataObject* objectFromIndex(const QModelIndex& index) const;

protected:
    virtual bool shouldShowObject(const IDOSDataObject* object) const = 0;
    virtual void buildObjectTree(IDOSTreeNode* parentNode, const IDOSDataObject* object) = 0;

private slots:
    void onObjectAdded(const QString& objectId);
    void onObjectRemoved(const QString& objectId);
    void onObjectChanged(const QString& objectId);

private:
    void rebuildTree();

    IDOSProject* m_project;
    IDOSTreeNode* m_rootNode;
};

#endif // IDOS_TREE_MODEL_H
