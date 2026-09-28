#include "idoscasetreemenuprovider.h"
#include "idoscasetreeview.h"
#include "idoscasetreemodel.h"
#include "idoscaseobject.h"
#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idosproject.h"
#include "idosdataobjecthandling.h"
#include "idostreegroupnode.h"
#include "idostreereferencenode.h"
#include "idosobjecttreenode.h"
#include <QAction>
#include <QStyle>
#include <QSet>

IDOSCaseTreeMenuProvider::IDOSCaseTreeMenuProvider(IDOSCaseTreeView* view)
    : IDOSTreeMenuProvider(view)
    , m_view(view)
{
}

QMenu* IDOSCaseTreeMenuProvider::createContextMenu()
{
    IDOSCaseTreeModel* model = qobject_cast<IDOSCaseTreeModel*>(m_view->model());
    if (model == nullptr || model->project() == nullptr)
    {
        return nullptr;
    }
    QMenu* menu = new QMenu(m_view);
    const QModelIndex index = m_view->currentIndex();
    const QIcon trashIcon = m_view->style()->standardIcon(QStyle::SP_TrashIcon);

    // 按当前选中节点类型分派菜单
    if (index.isValid())
    {
        IDOSDataObject* obj = model->objectFromIndex(index);
        // 属性分组节点（Static/Regions/Dynamic）：导入属性
        IDOSTreeGroupNode* group = dynamic_cast<IDOSTreeGroupNode*>(model->nodeFromIndex(index));
        if (group != nullptr && group->groupKey() == QStringLiteral("case.grids"))
        {
            // 网格分组节点：导入网格到所属工况
            m_pendingTargetCaseId = resolveCaseIdFromGroup(group);
            if (!m_pendingTargetCaseId.isEmpty())
            {
                QAction* importAction =
                    menu->addAction(QIcon(QStringLiteral(":/images/gui-grid.svg")), tr("Import Grid..."));
                importAction->setObjectName(QStringLiteral("importGridAction"));
                connect(importAction, &QAction::triggered, this, &IDOSCaseTreeMenuProvider::onImportGrid);
                return menu;
            }
            return menu;
        }
        if (group != nullptr && isPropertyGroupKey(group->groupKey()))
        {
            const QPair<QString, QString> ctx = resolvePropertyImportContext(group);
            m_pendingGridId = ctx.first;
            m_pendingTargetCaseId = ctx.second;
            if (!m_pendingGridId.isEmpty())
            {
                QAction* importAction = menu->addAction(
                    QIcon(QStringLiteral(":/images/gui-grid-static-properties.svg")), tr("Import Property..."));
                importAction->setObjectName(QStringLiteral("importPropertyAction"));
                connect(importAction, &QAction::triggered, this, &IDOSCaseTreeMenuProvider::onImportProperty);
                return menu;
            }
            return menu;
        }
        // 网格引用节点：删除网格（连带删除其属性）
        if (qobject_cast<IDOSGrid*>(obj) != nullptr)
        {
            m_pendingGridId = obj->objectId();
            QAction* deleteAction = menu->addAction(trashIcon, tr("Delete Grid..."));
            deleteAction->setObjectName(QStringLiteral("deleteGridAction"));
            connect(deleteAction, &QAction::triggered, this, &IDOSCaseTreeMenuProvider::onDeleteGrid);
            return menu;
        }
        // 工况根节点：删除工况
        if (qobject_cast<IDOSCaseObject*>(obj) != nullptr)
        {
            m_pendingCaseId = obj->objectId();
            QAction* deleteAction = menu->addAction(trashIcon, tr("Delete Case..."));
            deleteAction->setObjectName(QStringLiteral("deleteCaseAction"));
            connect(deleteAction, &QAction::triggered, this, &IDOSCaseTreeMenuProvider::onDeleteCase);
            return menu;
        }
        // 属性叶子节点：删除属性
        if (qobject_cast<IDOSGridProperty*>(obj) != nullptr)
        {
            m_pendingPropertyId = obj->objectId();
            QAction* deleteAction = menu->addAction(trashIcon, tr("Delete Property..."));
            deleteAction->setObjectName(QStringLiteral("deletePropertyAction"));
            connect(deleteAction, &QAction::triggered, this, &IDOSCaseTreeMenuProvider::onDeleteProperty);
            return menu;
        }
    }

    // 空白处或未识别节点：新建/导入工况
    QAction* createAction = menu->addAction(QIcon(QStringLiteral(":/images/gui-case.svg")), tr("New Case..."));
    createAction->setObjectName(QStringLiteral("newCaseAction"));
    connect(createAction, &QAction::triggered, this, &IDOSCaseTreeMenuProvider::onNewCase);
    QAction* importAction =
        menu->addAction(QIcon(QStringLiteral(":/images/gui-case-inputs.svg")), tr("Import Case..."));
    importAction->setObjectName(QStringLiteral("importCaseAction"));
    connect(importAction, &QAction::triggered, this, &IDOSCaseTreeMenuProvider::onImportCase);
    return menu;
}

void IDOSCaseTreeMenuProvider::onNewCase()
{
    IDOSCaseTreeModel* model = qobject_cast<IDOSCaseTreeModel*>(m_view->model());
    if (model != nullptr && IDOSDataObjectHandling::newCase(model->project(), m_view))
    {
        m_view->expandToDepth(0);
    }
}

void IDOSCaseTreeMenuProvider::onImportCase()
{
    IDOSCaseTreeModel* model = qobject_cast<IDOSCaseTreeModel*>(m_view->model());
    if (model != nullptr && IDOSDataObjectHandling::importCase(model->project(), m_view))
    {
        m_view->expandToDepth(1);
    }
}

void IDOSCaseTreeMenuProvider::onDeleteCase()
{
    IDOSCaseTreeModel* model = qobject_cast<IDOSCaseTreeModel*>(m_view->model());
    if (model == nullptr || m_pendingCaseId.isEmpty())
    {
        m_pendingCaseId.clear();
        return;
    }
    if (IDOSDataObjectHandling::deleteCase(model->project(), m_pendingCaseId, m_view))
    {
        m_view->collapseAll();
        m_view->expandToDepth(0);
    }
    m_pendingCaseId.clear();
}

void IDOSCaseTreeMenuProvider::onDeleteProperty()
{
    IDOSCaseTreeModel* model = qobject_cast<IDOSCaseTreeModel*>(m_view->model());
    if (model == nullptr || m_pendingPropertyId.isEmpty())
    {
        m_pendingPropertyId.clear();
        return;
    }
    IDOSDataObjectHandling::deleteProperty(model->project(), m_pendingPropertyId, m_view);
    m_pendingPropertyId.clear();
}

void IDOSCaseTreeMenuProvider::onImportProperty()
{
    IDOSCaseTreeModel* model = qobject_cast<IDOSCaseTreeModel*>(m_view->model());
    if (model == nullptr || m_pendingGridId.isEmpty())
    {
        m_pendingGridId.clear();
        m_pendingTargetCaseId.clear();
        return;
    }
    const QString gridId = m_pendingGridId;
    const QString caseId = m_pendingTargetCaseId;
    m_pendingGridId.clear();
    m_pendingTargetCaseId.clear();
    if (IDOSDataObjectHandling::importProperty(model->project(), gridId, caseId, m_view))
    {
        m_view->expandToDepth(2);
    }
}

void IDOSCaseTreeMenuProvider::onImportGrid()
{
    IDOSCaseTreeModel* model = qobject_cast<IDOSCaseTreeModel*>(m_view->model());
    const QString caseId = m_pendingTargetCaseId;
    m_pendingTargetCaseId.clear();
    if (model != nullptr && IDOSDataObjectHandling::importGrid(model->project(), caseId, m_view))
    {
        m_view->expandToDepth(2);
    }
}

void IDOSCaseTreeMenuProvider::onDeleteGrid()
{
    IDOSCaseTreeModel* model = qobject_cast<IDOSCaseTreeModel*>(m_view->model());
    const QString gridId = m_pendingGridId;
    m_pendingGridId.clear();
    if (model != nullptr)
    {
        IDOSDataObjectHandling::deleteGrid(model->project(), gridId, m_view);
    }
}

bool IDOSCaseTreeMenuProvider::isPropertyGroupKey(const QString& groupKey) const
{
    static const QSet<QString> propertyGroupKeys = {
        QStringLiteral("grid.staticProperties"),
        QStringLiteral("grid.regions"),
        QStringLiteral("grid.dynamicProperties")};
    return propertyGroupKeys.contains(groupKey);
}

QPair<QString, QString> IDOSCaseTreeMenuProvider::resolvePropertyImportContext(IDOSTreeGroupNode* group) const
{
    QString gridId;
    QString caseId;
    if (group == nullptr)
    {
        return qMakePair(gridId, caseId);
    }
    // 向上遍历父节点：首个 IDOSTreeReferenceNode 为网格引用（取 objectId 作为 gridId），
    // 首个 IDOSObjectTreeNode 为工况根（取 objectId 作为 caseId）
    for (IDOSTreeNode* node = group->parent(); node != nullptr; node = node->parent())
    {
        if (gridId.isEmpty())
        {
            IDOSTreeReferenceNode* refNode = dynamic_cast<IDOSTreeReferenceNode*>(node);
            if (refNode != nullptr)
            {
                gridId = refNode->itemRef().objectId();
                continue;
            }
        }
        if (caseId.isEmpty())
        {
            IDOSObjectTreeNode* objNode = dynamic_cast<IDOSObjectTreeNode*>(node);
            if (objNode != nullptr && !objNode->objectId().isEmpty())
            {
                caseId = objNode->objectId();
            }
        }
        if (!gridId.isEmpty() && !caseId.isEmpty())
        {
            break;
        }
    }
    return qMakePair(gridId, caseId);
}

QString IDOSCaseTreeMenuProvider::resolveCaseIdFromGroup(IDOSTreeGroupNode* group) const
{
    if (group == nullptr)
    {
        return QString();
    }
    for (IDOSTreeNode* node = group->parent(); node != nullptr; node = node->parent())
    {
        IDOSObjectTreeNode* objNode = dynamic_cast<IDOSObjectTreeNode*>(node);
        if (objNode != nullptr && !objNode->objectId().isEmpty())
        {
            return objNode->objectId();
        }
    }
    return QString();
}
