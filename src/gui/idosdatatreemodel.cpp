#include "idosdatatreemodel.h"
#include "idoscaseobject.h"
#include "idosgrid.h"
#include "idostreebuilder.h"
#include "idostreegroupnode.h"
#include "idosdatatreeprovider.h"
#include "idostreeproviderregistry.h"

IDOSDataTreeModel::IDOSDataTreeModel(QObject* parent)
    : IDOSTreeModel(parent)
    , m_treeProviderRegistry(nullptr)
{
    connect(this, &IDOSTreeModel::checkStateChanged, this, &IDOSDataTreeModel::onCheckStateChanged);
}

IDOSDataTreeModel::~IDOSDataTreeModel()
{
}

void IDOSDataTreeModel::setTreeProviderRegistry(IDOSTreeProviderRegistry* registry)
{
    m_treeProviderRegistry = registry;
}

bool IDOSDataTreeModel::shouldShowObject(const IDOSDataObject* object) const
{
    // 网格及其属性归工况树管理，不作为输入数据展示
    return object != nullptr && qobject_cast<const IDOSCaseObject*>(object) == nullptr &&
           qobject_cast<const IDOSGrid*>(object) == nullptr &&
           object->containerId().isEmpty();
}

void IDOSDataTreeModel::buildObjectTree(IDOSTreeNode* parentNode, const IDOSDataObject* object)
{
    if (parentNode == nullptr || object == nullptr || m_treeProviderRegistry == nullptr)
    {
        return;
    }

    // 父节点（业务分组或根）统一由 parentNodeForNewObject 决定，此处只负责构建
    IDOSDataTreeProvider* provider = m_treeProviderRegistry->dataProviderFor(object->typeId());
    if (provider != nullptr)
    {
        IDOSTreeBuilder builder(parentNode);
        provider->buildTree(builder, object);
    }
}

IDOSTreeNode* IDOSDataTreeModel::parentNodeForNewObject(const IDOSDataObject* object) const
{
    if (object == nullptr || m_treeProviderRegistry == nullptr)
    {
        return nullptr;
    }
    IDOSDataTreeProvider* provider = m_treeProviderRegistry->dataProviderFor(object->typeId());
    if (provider == nullptr)
    {
        return nullptr;
    }
    // provider 无分组 key（如网格）时返回 nullptr，基类回退挂根节点
    return m_groups.value(provider->groupKey(), nullptr);
}

IDOSTreeGroupNode* IDOSDataTreeModel::addGroupNode(IDOSTreeBuilder& builder,
                                                   const QString& key,
                                                   const QString& title,
                                                   const QString& iconPath)
{
    IDOSTreeGroupNode* node = builder.addGroup(key, title);
    node->setIcon(QIcon(iconPath));
    m_groups.insert(key, node);
    return node;
}

void IDOSDataTreeModel::onCheckStateChanged(const QModelIndex& index, bool checked)
{
    IDOSDataObject* object = objectFromIndex(index);
    if (object != nullptr)
    {
        emit itemCheckedChanged(object->objectId(), checked);
    }
}

void IDOSDataTreeModel::buildDefaultTree(IDOSTreeNode* rootNode)
{
    m_groups.clear();
    IDOSTreeBuilder builder(rootNode);

    // 井组及其子分类
    IDOSTreeGroupNode* wellGroup =
        addGroupNode(builder, QStringLiteral("data.wellGroup"), tr("Well Group"),
                     QStringLiteral(":/images/gui-well-group.svg"));
    IDOSTreeBuilder wellBuilder = builder.childBuilder(wellGroup);
    addGroupNode(wellBuilder, QStringLiteral("data.globalLogs"), tr("Global Well Logs"),
                 QStringLiteral(":/images/gui-global-logs.svg"));
    addGroupNode(wellBuilder, QStringLiteral("data.globalCompletions"), tr("Global Completions"),
                 QStringLiteral(":/images/gui-global-completions.svg"));
    addGroupNode(wellBuilder, QStringLiteral("data.globalObservations"), tr("Global Observations"),
                 QStringLiteral(":/images/gui-global-observations.svg"));
    addGroupNode(wellBuilder, QStringLiteral("data.wellProperties"), tr("Well Properties"),
                 QStringLiteral(":/images/gui-well-properties.svg"));
    addGroupNode(wellBuilder, QStringLiteral("data.wells"), tr("Wells"),
                 QStringLiteral(":/images/gui-wells.svg"));

    // 顶层数据分类
    addGroupNode(builder, QStringLiteral("data.seismic"), tr("Seismic"),
                 QStringLiteral(":/images/gui-seismic.svg"));
    addGroupNode(builder, QStringLiteral("data.surfaceProperties"), tr("Surface Properties"),
                 QStringLiteral(":/images/gui-surface-properties.svg"));
    addGroupNode(builder, QStringLiteral("data.faults"), tr("Faults"),
                 QStringLiteral(":/images/gui-faults.svg"));
    addGroupNode(builder, QStringLiteral("data.boundaries"), tr("Polygons / Boundaries"),
                 QStringLiteral(":/images/gui-boundaries.svg"));
    addGroupNode(builder, QStringLiteral("data.naturalFractures"), tr("Natural Fractures"),
                 QStringLiteral(":/images/gui-natural-fractures.svg"));
    addGroupNode(builder, QStringLiteral("data.hydraulicFractures"), tr("Hydraulic Fracturing"),
                 QStringLiteral(":/images/gui-hydraulic-fractures.svg"));
    addGroupNode(builder, QStringLiteral("data.fluids"), tr("Fluids"),
                 QStringLiteral(":/images/gui-fluids.svg"));
    addGroupNode(builder, QStringLiteral("data.rockPhysics"), tr("Rock Physics Functions"),
                 QStringLiteral(":/images/gui-rock-physics.svg"));
    addGroupNode(builder, QStringLiteral("data.initialConditions"), tr("Initial Conditions"),
                 QStringLiteral(":/images/gui-initial-conditions.svg"));
    addGroupNode(builder, QStringLiteral("data.microseismic"), tr("Microseismic"),
                 QStringLiteral(":/images/gui-microseismic.svg"));
    addGroupNode(builder, QStringLiteral("data.production"), tr("Production Summary"),
                 QStringLiteral(":/images/gui-production.svg"));
    addGroupNode(builder, QStringLiteral("data.streamlines"), tr("Flow Diagnostics"),
                 QStringLiteral(":/images/gui-flow-diagnostics.svg"));
    addGroupNode(builder, QStringLiteral("data.management"), tr("Field Management Strategies"),
                 QStringLiteral(":/images/gui-field-management.svg"));
}
