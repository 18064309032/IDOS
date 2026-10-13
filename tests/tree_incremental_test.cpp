#include <memory>

#include <QAction>
#include <QItemSelectionModel>
#include <QKeyEvent>
#include <QMenu>
#include <QPersistentModelIndex>
#include <QScopedPointer>
#include <QSignalSpy>
#include <QStringList>
#include <QtTest>

#include <QVTKOpenGLNativeWidget.h>
#include <vtkCommand.h>
#include <vtkObject.h>
#include <vtkSmartPointer.h>

#include "idoscaseitemref.h"
#include "idosdatametadata.h"
#include "idosdataregistry.h"
#include "idosgrid.h"
#include "idosgridmetadata.h"
#include "idosgridproperty.h"
#include "idosmainwindow.h"
#include "idosproject.h"
#include "idosrenderview.h"
#include "idossimulationcasemetadata.h"
#include "idossimulationcaseobject.h"
#include "idostoolbar.h"
#include "idostoolregistry.h"
#include "idostree.h"
#include "idostreeitem.h"
#include "idostreemodel.h"
#include "idostreeregistry.h"
#include "idostreeserver.h"
#include "idostreeview.h"
#include "idoswell.h"
#include "idoswellmetadata.h"
#include "idoswelltreeguiprovider.h"
#include "idoswelltreemetadata.h"

#include "tree_incremental_test.h"

// 测试替身：objectId 正常构造后不可外部修改（protected，仅供恢复加载），
// 子类开放一个赋值入口以构造 Project::addObject 的同 id 替换场景
class IDOSTestWell : public IDOSWell
{
  public:
    void assignObjectId(const QString& id);
};

void IDOSTestWell::assignObjectId(const QString& id)
{
    setObjectId(id);
}

void TreeIncrementalTest::resetSignalCounters()
{
    m_dataTreeReset = false;
    m_caseTreeReset = false;
    m_dataRowsInserted = 0;
    m_dataRowsRemoved = 0;
    m_caseRowsInserted = 0;
    m_caseRowsRemoved = 0;
}

QModelIndex TreeIncrementalTest::findIndexByName(const QAbstractItemModel* model,
                                                 const QModelIndex& parent,
                                                 const QString& name)
{
    for (int row = 0; row < model->rowCount(parent); ++row)
    {
        const QModelIndex child = model->index(row, 0, parent);
        if (model->data(child, Qt::DisplayRole).toString() == name)
        {
            return child;
        }
        const QModelIndex nested = findIndexByName(model, child, name);
        if (nested.isValid())
        {
            return nested;
        }
    }
    return QModelIndex();
}

void TreeIncrementalTest::onDataTreeReset()
{
    m_dataTreeReset = true;
}

void TreeIncrementalTest::onDataRowsInserted()
{
    ++m_dataRowsInserted;
}

void TreeIncrementalTest::onDataRowsRemoved()
{
    ++m_dataRowsRemoved;
}

void TreeIncrementalTest::onCaseTreeReset()
{
    m_caseTreeReset = true;
}

void TreeIncrementalTest::onCaseRowsInserted()
{
    ++m_caseRowsInserted;
}

void TreeIncrementalTest::onCaseRowsRemoved()
{
    ++m_caseRowsRemoved;
}

void TreeIncrementalTest::onDataTreeIncrementalAddRenameRemove()
{
    IDOSMainWindow window;
    IDOSProject* project = new IDOSProject(&window);
    IDOSTreeModel* model = window.dataTreeModel();
    connect(model, &QAbstractItemModel::modelAboutToBeReset, this, &TreeIncrementalTest::onDataTreeReset);
    connect(model, &QAbstractItemModel::rowsInserted, this, &TreeIncrementalTest::onDataRowsInserted);
    connect(model, &QAbstractItemModel::rowsRemoved, this, &TreeIncrementalTest::onDataRowsRemoved);
    window.setProject(project);

    const QModelIndex wells = findIndexByName(model, QModelIndex(), QStringLiteral("Wells"));
    QVERIFY(wells.isValid());
    QCOMPARE(model->rowCount(wells), 0);

    // 增量新增：只发插入信号，不触发整树 reset
    resetSignalCounters();
    IDOSWell* well = new IDOSWell();
    well->setName(QStringLiteral("A10"));
    project->add(well);
    QVERIFY(!m_dataTreeReset);
    QCOMPARE(m_dataRowsInserted, 1);
    QCOMPARE(m_dataRowsRemoved, 0);
    QCOMPARE(model->rowCount(wells), 1);
    QCOMPARE(model->data(model->index(0, 0, wells), Qt::DisplayRole).toString(),
             QStringLiteral("A10"));

    // 改名保留节点身份、Provider 和持久索引。
    const QPersistentModelIndex wellIndex(model->index(0, 0, wells));
    IDOSTreeItem* wellItem = model->nodeFromIndex(wellIndex);
    IDOSTreeProvider* wellProvider = wellItem->provider();
    const QString treeId = wellItem->id();
    resetSignalCounters();
    well->setName(QStringLiteral("A11"));
    QVERIFY(!m_dataTreeReset);
    QVERIFY(wellIndex.isValid());
    QCOMPARE(model->nodeFromIndex(wellIndex), wellItem);
    QCOMPARE(wellItem->provider(), wellProvider);
    QCOMPARE(wellItem->id(), treeId);
    QCOMPARE(m_dataRowsRemoved, 0);
    QCOMPARE(m_dataRowsInserted, 0);
    QCOMPARE(model->rowCount(wells), 1);
    QCOMPARE(model->data(model->index(0, 0, wells), Qt::DisplayRole).toString(),
             QStringLiteral("A11"));

    // 增量删除：只发删除信号，不触发整树 reset
    resetSignalCounters();
    QVERIFY(project->remove(well->objectId()));
    QVERIFY(!m_dataTreeReset);
    QCOMPARE(m_dataRowsRemoved, 1);
    QCOMPARE(m_dataRowsInserted, 0);
    QCOMPARE(model->rowCount(wells), 0);
}

void TreeIncrementalTest::onSameIdReplacementDoesNotDuplicate()
{
    IDOSMainWindow window;
    IDOSProject* project = new IDOSProject(&window);
    IDOSTreeModel* model = window.dataTreeModel();
    connect(model, &QAbstractItemModel::modelAboutToBeReset, this, &TreeIncrementalTest::onDataTreeReset);
    window.setProject(project);

    const QModelIndex wells = findIndexByName(model, QModelIndex(), QStringLiteral("Wells"));
    QVERIFY(wells.isValid());

    IDOSTestWell* well = new IDOSTestWell();
    well->setName(QStringLiteral("A10"));
    const QString fixedId = QStringLiteral("test-well-id");
    well->assignObjectId(fixedId);
    project->add(well);
    QCOMPARE(model->rowCount(wells), 1);

    // 同 objectId 替换：Project 删旧对象后只发 objectAdded，树上不得出现重复节点
    resetSignalCounters();
    IDOSTestWell* replacement = new IDOSTestWell();
    replacement->setName(QStringLiteral("A10-Rebuilt"));
    replacement->assignObjectId(fixedId);
    project->add(replacement);
    QVERIFY(!m_dataTreeReset);
    QCOMPARE(model->rowCount(wells), 1);
    QCOMPARE(model->data(model->index(0, 0, wells), Qt::DisplayRole).toString(),
             QStringLiteral("A10-Rebuilt"));
}

void TreeIncrementalTest::onExpansionStatePreservedAcrossRebuild()
{
    IDOSMainWindow window;
    IDOSProject* project = new IDOSProject(&window);
    window.setProject(project);
    IDOSTreeModel* dataModel = window.dataTreeModel();
    IDOSTreeView* dataView = window.dataTreeView();
    IDOSTreeModel* caseModel = window.caseTreeModel();
    IDOSTreeView* caseView = window.caseTreeView();

    // 数据树：井改名触发分支重建，井节点展开态必须保留
    IDOSWell* well = new IDOSWell();
    well->setName(QStringLiteral("A10"));
    project->add(well);
    const QModelIndex wellsGroup = findIndexByName(dataModel, QModelIndex(), QStringLiteral("Wells"));
    QVERIFY(wellsGroup.isValid());
    const QModelIndex wellNode = dataModel->index(0, 0, wellsGroup);
    dataView->setExpanded(wellsGroup, true);
    dataView->setExpanded(wellNode, true);
    well->setName(QStringLiteral("A11"));
    const QModelIndex rebuiltWell = dataModel->index(0, 0, wellsGroup);
    QCOMPARE(dataModel->data(rebuiltWell, Qt::DisplayRole).toString(), QStringLiteral("A11"));
    QVERIFY(dataView->isExpanded(wellsGroup));
    QVERIFY(dataView->isExpanded(rebuiltWell));

    // 工况树：属性加入导致工况分支重建，工况根与 Grids 组保持展开，未展开组保持折叠
    IDOSGrid* grid = new IDOSGrid();
    grid->setName(QStringLiteral("Main Grid"));
    grid->setDimensions(1, 1, 1);
    project->add(grid);
    IDOSSimulationCaseObject* caseObject = new IDOSSimulationCaseObject();
    caseObject->setName(QStringLiteral("Base Case"));
    caseObject->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), grid->objectId()));
    project->add(caseObject);

    const QModelIndex caseRoot = caseModel->index(0, 0, QModelIndex());
    const QModelIndex runSettingsGroup = caseModel->index(0, 0, caseRoot);
    const QModelIndex gridsGroup = caseModel->index(1, 0, caseRoot);
    caseView->setExpanded(caseRoot, true);
    caseView->setExpanded(gridsGroup, true);
    QVERIFY(!caseView->isExpanded(runSettingsGroup));

    IDOSGridProperty* poro = new IDOSGridProperty();
    poro->setName(QStringLiteral("PORO"));
    poro->setKeyword(QStringLiteral("PORO"));
    poro->setGridId(grid->objectId());
    poro->setDimensions(1, 1, 1);
    QVERIFY(poro->setValues(QVector<double>{0.2}));
    project->add(poro);

    const QModelIndex rebuiltCaseRoot = caseModel->index(0, 0, QModelIndex());
    QCOMPARE(caseModel->data(rebuiltCaseRoot, Qt::DisplayRole).toString(), QStringLiteral("Base Case"));
    QVERIFY(caseView->isExpanded(rebuiltCaseRoot));
    QVERIFY(caseView->isExpanded(caseModel->index(1, 0, rebuiltCaseRoot)));
    QVERIFY(!caseView->isExpanded(caseModel->index(0, 0, rebuiltCaseRoot)));
}

void TreeIncrementalTest::collectDisplayNames(const QAbstractItemModel* model,
                                              const QModelIndex& parent, QStringList& names)
{
    for (int row = 0; row < model->rowCount(parent); ++row)
    {
        const QModelIndex child = model->index(row, 0, parent);
        names.append(model->data(child, Qt::DisplayRole).toString());
        collectDisplayNames(model, child, names);
    }
}

void TreeIncrementalTest::onBatchImportRebuildsCaseBranchOnce()
{
    IDOSMainWindow window;
    IDOSProject* project = new IDOSProject(&window);
    IDOSTreeModel* caseModel = window.caseTreeModel();
    connect(caseModel, &QAbstractItemModel::modelAboutToBeReset, this, &TreeIncrementalTest::onCaseTreeReset);
    connect(caseModel, &QAbstractItemModel::rowsInserted, this, &TreeIncrementalTest::onCaseRowsInserted);
    connect(caseModel, &QAbstractItemModel::rowsRemoved, this, &TreeIncrementalTest::onCaseRowsRemoved);
    window.setProject(project);

    IDOSGrid* grid = new IDOSGrid();
    grid->setName(QStringLiteral("Main Grid"));
    grid->setDimensions(1, 1, 1);
    project->add(grid);

    IDOSSimulationCaseObject* caseObject = new IDOSSimulationCaseObject();
    caseObject->setName(QStringLiteral("Base Case"));
    caseObject->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), grid->objectId()));
    resetSignalCounters();
    project->add(caseObject);
    QCOMPARE(m_caseRowsInserted, 1);   // 工况分支插入
    QCOMPARE(m_caseRowsRemoved, 0);

    const QPersistentModelIndex persistentCase(caseModel->index(0, 0, QModelIndex()));
    IDOSTreeItem* caseItem = caseModel->nodeFromIndex(persistentCase);
    IDOSTreeProvider* caseProvider = caseItem->provider();
    const int categoryCount = caseModel->rowCount(persistentCase);

    // 批量导入 5 个同网格属性（模拟 Eclipse DATA 导入）
    resetSignalCounters();
    project->beginUpdate();
    bool allPropertiesValid = true;
    for (int i = 0; i < 5; ++i)
    {
        IDOSGridProperty* property = new IDOSGridProperty();
        property->setName(QStringLiteral("P%1").arg(i));
        property->setKeyword(property->name());
        property->setGridId(grid->objectId());
        property->setDimensions(1, 1, 1);
        allPropertiesValid = property->setValues(QVector<double>(1, 0.1 * i)) && allPropertiesValid;
        project->add(property);
    }
    project->endUpdate();
    QVERIFY(allPropertiesValid);

    QVERIFY(!m_caseTreeReset);
    // 批量修改只刷新一次子树，工况节点与 Provider 保持不变。
    QVERIFY(persistentCase.isValid());
    QCOMPARE(caseModel->nodeFromIndex(persistentCase), caseItem);
    QCOMPARE(caseItem->provider(), caseProvider);
    QCOMPARE(m_caseRowsRemoved, categoryCount);

    // 重建后的分支内容完整：网格引用 + 5 个属性引用全部可在树中找到
    const QModelIndex caseRoot = caseModel->index(0, 0, QModelIndex());
    QCOMPARE(caseModel->data(caseRoot, Qt::DisplayRole).toString(), QStringLiteral("Base Case"));
    QStringList displayNames;
    collectDisplayNames(caseModel, caseRoot, displayNames);
    QVERIFY(displayNames.contains(QStringLiteral("Main Grid")));
    for (int i = 0; i < 5; ++i)
    {
        QVERIFY2(displayNames.contains(QStringLiteral("P%1").arg(i)),
                 QStringLiteral("missing property P%1 after batch rebuild").arg(i).toUtf8().constData());
    }
}

void TreeIncrementalTest::onProjectUpdateTransactionBatchesSignals()
{
    IDOSProject project;
    QSignalSpy singleAddedSpy(&project, &IDOSProject::objectAdded);
    QSignalSpy batchAddedSpy(&project, &IDOSProject::objectsAdded);
    QSignalSpy batchRemovedSpy(&project, &IDOSProject::objectsRemoved);

    // 事务期间不发单信号，最外层 endUpdate 时发一次批量信号
    project.beginUpdate();
    IDOSWell* well1 = new IDOSWell();
    well1->setName(QStringLiteral("W1"));
    IDOSWell* well2 = new IDOSWell();
    well2->setName(QStringLiteral("W2"));
    project.add(well1);
    project.add(well2);
    const int singleSignalsDuringUpdate = singleAddedSpy.count();
    const int batchSignalsDuringUpdate = batchAddedSpy.count();
    project.endUpdate();
    QCOMPARE(singleSignalsDuringUpdate, 0);
    QCOMPARE(batchSignalsDuringUpdate, 0);
    QCOMPARE(singleAddedSpy.count(), 0);
    QCOMPARE(batchAddedSpy.count(), 1);
    QCOMPARE(batchAddedSpy.takeFirst().at(0).toStringList().count(), 2);

    // 嵌套事务：内层闭合不发，最外层闭合才发
    project.beginUpdate();
    project.beginUpdate();
    IDOSWell* well3 = new IDOSWell();
    well3->setName(QStringLiteral("W3"));
    project.add(well3);
    project.endUpdate();
    QCOMPARE(batchAddedSpy.count(), 0);
    project.endUpdate();
    QCOMPARE(batchAddedSpy.count(), 1);
    QCOMPARE(batchAddedSpy.takeFirst().at(0).toStringList().count(), 1);

    // 归并：事务内把刚加入的对象删掉，最终只算 removed，不进 added
    IDOSWell* well4 = new IDOSWell();
    well4->setName(QStringLiteral("W4"));
    project.add(well4);
    const QString well4Id = well4->objectId();
    project.beginUpdate();
    const bool wellRemoved = project.remove(well4Id);
    project.endUpdate();
    QVERIFY(wellRemoved);
    QCOMPARE(batchRemovedSpy.count(), 1);
    QCOMPARE(batchRemovedSpy.takeFirst().at(0).toStringList(), QStringList(well4Id));

    // 非事务路径行为不变：仍发单个信号
    singleAddedSpy.clear();
    IDOSWell* well5 = new IDOSWell();
    well5->setName(QStringLiteral("W5"));
    project.add(well5);
    QCOMPARE(singleAddedSpy.count(), 1);
}

void TreeIncrementalTest::onCaseTreeReferenceRefreshAndCheckPreserved()
{
    IDOSMainWindow window;
    IDOSProject* project = new IDOSProject(&window);
    IDOSTreeModel* model = window.caseTreeModel();
    connect(model, &QAbstractItemModel::modelAboutToBeReset, this, &TreeIncrementalTest::onCaseTreeReset);
    connect(model, &QAbstractItemModel::rowsInserted, this, &TreeIncrementalTest::onCaseRowsInserted);
    connect(model, &QAbstractItemModel::rowsRemoved, this, &TreeIncrementalTest::onCaseRowsRemoved);
    window.setProject(project);

    IDOSGrid* grid = new IDOSGrid();
    grid->setName(QStringLiteral("Main Grid"));
    grid->setDimensions(1, 1, 1);
    project->add(grid);

    IDOSSimulationCaseObject* caseObject = new IDOSSimulationCaseObject();
    caseObject->setName(QStringLiteral("Base Case"));
    caseObject->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), grid->objectId()));
    project->add(caseObject);
    QCOMPARE(model->rowCount(QModelIndex()), 1);

    // 属性在工况之后加入：通过 refreshReferencingBranches 增量进入工况子树
    resetSignalCounters();
    IDOSGridProperty* poro = new IDOSGridProperty();
    poro->setName(QStringLiteral("PORO"));
    poro->setKeyword(QStringLiteral("PORO"));
    poro->setGridId(grid->objectId());
    poro->setDimensions(1, 1, 1);
    QVERIFY(poro->setValues(QVector<double>{0.2}));
    project->add(poro);
    QVERIFY(!m_caseTreeReset);

    QModelIndex poroIndex = findIndexByName(model, QModelIndex(), QStringLiteral("PORO"));
    QVERIFY(poroIndex.isValid());
    QVERIFY(model->flags(poroIndex) & Qt::ItemIsUserCheckable);
    QVERIFY(model->setData(poroIndex, Qt::Checked, Qt::CheckStateRole));
    QCOMPARE(model->data(poroIndex, Qt::CheckStateRole).toInt(), int(Qt::Checked));

    // 再加入一个属性会重建工况分支，PORO 的勾选必须保留（PERMX 默认不勾）
    resetSignalCounters();
    IDOSGridProperty* permx = new IDOSGridProperty();
    permx->setName(QStringLiteral("PERMX"));
    permx->setVisible(false);
    permx->setKeyword(QStringLiteral("PERMX"));
    permx->setGridId(grid->objectId());
    permx->setDimensions(1, 1, 1);
    QVERIFY(permx->setValues(QVector<double>{100.0}));
    project->add(permx);
    QVERIFY(!m_caseTreeReset);

    poroIndex = findIndexByName(model, QModelIndex(), QStringLiteral("PORO"));
    QVERIFY(poroIndex.isValid());
    QCOMPARE(model->data(poroIndex, Qt::CheckStateRole).toInt(), int(Qt::Checked));
    const QModelIndex permxIndex = findIndexByName(model, QModelIndex(), QStringLiteral("PERMX"));
    QVERIFY(permxIndex.isValid());
    QCOMPARE(model->data(permxIndex, Qt::CheckStateRole).toInt(), int(Qt::Unchecked));

    // 网格被删：工况分支刷新为 Missing object，引用节点不可勾选，属性子树消失
    resetSignalCounters();
    QVERIFY(project->remove(grid->objectId()));
    QVERIFY(!m_caseTreeReset);
    const QModelIndex missingGrid = findIndexByName(model, QModelIndex(), QStringLiteral("Main Grid"));
    QVERIFY(!missingGrid.isValid());
    const QModelIndex caseRoot = model->index(0, 0, QModelIndex());
    const QModelIndex gridsGroup = model->index(1, 0, caseRoot);
    QCOMPARE(model->rowCount(gridsGroup), 1);
    const QModelIndex missingNode = model->index(0, 0, gridsGroup);
    QVERIFY(model->data(missingNode, Qt::DisplayRole).toString().contains(QStringLiteral("Missing")));
    QVERIFY(!(model->flags(missingNode) & Qt::ItemIsUserCheckable));
    QCOMPARE(model->rowCount(missingNode), 0);
}

void TreeIncrementalTest::onRegistryLifecycle()
{
    IDOSProject project;
    IDOSWell* well = new IDOSWell();
    project.add(well);
    const QString id = well->objectId();
    IDOSTreeRegistry registry;
    IDOSTreeServer server(&registry);
    IDOSTree tree;
    tree.setName(QStringLiteral("data"));
    IDOSTreeModel model(&tree);
    IDOSTreeView view;
    view.setModel(&model);
    view.setRegistry(&registry);
    QVERIFY(server.addTree(&tree));
    QVERIFY(!server.addTree(&tree));
    server.setProject(&project);
    QCOMPARE(model.rowCount(), 0);
    QVERIFY(registry.add(new IDOSWellTreeMetadata()));
    QCOMPARE(model.rowCount(), 1);
    IDOSWellTreeMetadata* duplicate = new IDOSWellTreeMetadata();
    QVERIFY(!registry.add(duplicate));
    delete duplicate;
    std::shared_ptr<IDOSWellTreeGuiProvider> gui = std::make_shared<IDOSWellTreeGuiProvider>();
    QVERIFY(registry.addGuiProvider(gui));
    QVERIFY(!registry.addGuiProvider(gui));
    const QModelIndex index = model.indexFromDataId(id);
    QVERIFY(!model.setData(index, Qt::Unchecked, Qt::CheckStateRole));
    QVERIFY(well->isVisible());
    QVERIFY(registry.remove(QStringLiteral("idos.well")));
    QCOMPARE(model.rowCount(), 0);
    QVERIFY(!registry.remove(QStringLiteral("idos.well")));
    QVERIFY(registry.add(new IDOSWellTreeMetadata()));
    QCOMPARE(model.rowCount(), 1);
    QVERIFY(server.removeTree(&tree));
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(tree.project(), nullptr);
}

void TreeIncrementalTest::onTreeOwnership()
{
    IDOSTree first;
    IDOSTree second;
    IDOSTreeModel model(&first);
    QSignalSpy inserted(&model, &QAbstractItemModel::rowsInserted);
    QSignalSpy removed(&model, &QAbstractItemModel::rowsRemoved);
    IDOSTreeItem* parent = new IDOSTreeItem();
    IDOSTreeItem* child = new IDOSTreeItem();
    QVERIFY(parent->appendChild(child));
    QVERIFY(!child->appendChild(parent));
    QVERIFY(first.insert(first.root(), parent));
    QVERIFY(!second.insert(second.root(), parent));
    QVERIFY(!first.insert(parent, parent));
    QCOMPARE(inserted.count(), 1);
    IDOSTreeItem* leaf = new IDOSTreeItem();
    QVERIFY(child->appendChild(leaf));
    QCOMPARE(inserted.count(), 2);
    QPersistentModelIndex leafIndex(model.indexFromNode(leaf));
    QVERIFY(first.remove(parent));
    QCOMPARE(removed.count(), 1);
    QVERIFY(!leafIndex.isValid());
}

void TreeIncrementalTest::onMenuTargets()
{
    IDOSMainWindow window;
    IDOSProject project;
    window.setProject(&project);
    IDOSWell* first = new IDOSWell();
    first->setName(QStringLiteral("First"));
    project.add(first);
    IDOSWell* second = new IDOSWell();
    second->setName(QStringLiteral("Second"));
    project.add(second);
    IDOSTreeModel* model = window.dataTreeModel();
    IDOSTreeView* view = window.dataTreeView();
    const QModelIndex firstIndex = model->indexFromDataId(first->objectId());
    const QModelIndex secondIndex = model->indexFromDataId(second->objectId());
    view->selectionModel()->select(firstIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    view->selectionModel()->select(secondIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    QCOMPARE(view->selectedItems().size(), 2);
    QScopedPointer<QMenu> blankMenu(view->createMenu(QModelIndex()));
    QVERIFY(blankMenu->actions().isEmpty());
    QVERIFY(view->menuItem() == nullptr);
    QCOMPARE(view->selectedItems().size(), 2);
    QScopedPointer<QMenu> menu(view->createMenu(firstIndex));
    QCOMPARE(view->menuItem()->dataId(), first->objectId());
    QCOMPARE(view->selectedItems().size(), 2);
    QCOMPARE(menu->actions().last()->property("dataId").toString(), first->objectId());
    view->setCurrentIndex(secondIndex);
    QCOMPARE(menu->actions().last()->property("dataId").toString(), first->objectId());
    IDOSProject replacement;
    window.setProject(&replacement);
    menu->actions().last()->trigger();
    QCOMPARE(project.objects().size(), 2);
    QCOMPARE(replacement.objects().size(), 0);
}

IDOSTestInputTool::IDOSTestInputTool(IDOSRenderView* view)
    :
    IDOSTool(view)
    , consumed(false)
    , inputs(0)
    , observations(0)
    , observedValue(0)
    , registry(nullptr)
    , removalAccepted(false)
    , stopOnInput(false)
{
    connect(this, &IDOSTool::callbackFinished, this, &IDOSTestInputTool::onCallbackFinished);
}

void IDOSTestInputTool::onCallbackFinished(bool consumed)
{
    Q_UNUSED(consumed)
    if (registry != nullptr)
    {
        removalAccepted = registry->remove(QStringLiteral("test.input"));
    }
}

QString IDOSTestInputTool::exclusiveGroup() const
{
    return group;
}

bool IDOSTestInputTool::onEventFilter(QObject* watched, QEvent* event)
{
    Q_UNUSED(watched)
    if (event->type() == QEvent::KeyPress)
    {
        ++inputs;
        if (registry != nullptr)
        {
            removalAccepted = registry->remove(QStringLiteral("test.input"));
        }
        if (stopOnInput)
        {
            view()->disableTool(this);
            QCoreApplication::processEvents();
        }
        return consumed;
    }
    return false;
}

void IDOSTestInputTool::observe(vtkObject* source)
{
    observeVtkEvent(source, vtkCommand::ModifiedEvent);
}

void IDOSTestInputTool::onVtkEvent(vtkObject* source, unsigned long eventId, void* callData)
{
    Q_UNUSED(source)
    Q_UNUSED(eventId)
    ++observations;
    if (callData != nullptr)
    {
        observedValue = *static_cast<int*>(callData);
    }
    if (registry != nullptr)
    {
        removalAccepted = registry->remove(QStringLiteral("test.input"));
    }
}

IDOSTestToolProvider::IDOSTestToolProvider(IDOSRenderView* view)
    :
    IDOSToolProvider(view)
    , m_action(new QAction(tr("Test input"), this))
    , m_menu(new QMenu(view))
    , m_tool(new IDOSTestInputTool(view))
{
    m_tool->setParent(this);
    m_action->setCheckable(true);
    QAction* operation = m_menu->addAction(tr("Nested operation"));
    connect(operation, &QAction::triggered, this, &IDOSTestToolProvider::onToggleTool);
    m_action->setMenu(m_menu);
}

IDOSTestToolProvider::~IDOSTestToolProvider()
{
    delete m_menu;
}

QList<QAction*> IDOSTestToolProvider::actions() const
{
    return QList<QAction*>{m_action};
}

QList<IDOSTool*> IDOSTestToolProvider::tools() const
{
    return QList<IDOSTool*>{m_tool};
}

void IDOSTestToolProvider::updateState()
{
    m_action->setChecked(view()->axesVisible());
}

void IDOSTestToolProvider::onToggleTool()
{
    if (view()->isToolEnabled(m_tool))
    {
        view()->disableTool(m_tool);
    }
    else
    {
        view()->enableTool(m_tool);
    }
}

IDOSTestToolMetadata::IDOSTestToolMetadata(const QString& id)
    :
    m_id(id)
{
}

QString IDOSTestToolMetadata::id() const
{
    return m_id;
}

QString IDOSTestToolMetadata::name() const
{
    return QObject::tr("Test input");
}

QString IDOSTestToolMetadata::description() const
{
    return QObject::tr("Test input tool");
}

QString IDOSTestToolMetadata::iconPath() const
{
    return QString();
}

QString IDOSTestToolMetadata::groupId() const
{
    return QStringLiteral("interaction");
}

int IDOSTestToolMetadata::order() const
{
    return 0;
}

bool IDOSTestToolMetadata::supports(const IDOSRenderView* view) const
{
    return !view->isTwoDimensional();
}

IDOSToolProvider* IDOSTestToolMetadata::create(IDOSRenderView* view) const
{
    return new IDOSTestToolProvider(view);
}

void TreeIncrementalTest::onToolRegistryAndWindows()
{
    IDOSToolRegistry registry;
    IDOSRenderView first;
    IDOSRenderView second;
    IDOSToolBar* firstBar = new IDOSToolBar();
    IDOSToolBar* secondBar = new IDOSToolBar();
    QVERIFY(firstBar->setRegistry(&registry));
    QVERIFY(secondBar->setRegistry(&registry));
    const int frameworkActionCount = firstBar->actions().size();
    QVERIFY(firstBar->findChildren<IDOSToolProvider*>().isEmpty());
    QVERIFY(first.setToolBar(firstBar));
    QVERIFY(second.setToolBar(secondBar));
    QVERIFY(firstBar->isHidden());
    QVERIFY(!second.setToolBar(firstBar));
    QVERIFY(registry.add(new IDOSTestToolMetadata()));
    std::unique_ptr<IDOSTestToolMetadata> duplicate(new IDOSTestToolMetadata());
    QVERIFY(!registry.add(duplicate.get()));
    QCOMPARE(firstBar->actions().size(), frameworkActionCount + 1);
    QVERIFY(!firstBar->isHidden());
    QCOMPARE(secondBar->actions().size(), frameworkActionCount + 1);
    QVERIFY(firstBar->actions().last() != secondBar->actions().last());
    firstBar->actions().last()->menu()->actions().first()->trigger();
    QCOMPARE(first.enabledTools().size(), 1);
    QVERIFY(second.enabledTools().isEmpty());
    firstBar->hide();
    first.setAxesVisible(false);
    QVERIFY(firstBar->isHidden());
    QVERIFY(!firstBar->actions().last()->isChecked());
    QVERIFY(secondBar->actions().last()->isChecked());
    QPointer<QAction> oldAction = firstBar->actions().last();
    second.setTwoDimensional(true);
    QCOMPARE(secondBar->actions().size(), frameworkActionCount);
    QVERIFY(secondBar->isHidden());
    second.setTwoDimensional(false);
    QCOMPARE(secondBar->actions().size(), frameworkActionCount + 1);
    QVERIFY(registry.remove(QStringLiteral("test.input")));
    QVERIFY(oldAction == nullptr);
    QVERIFY(firstBar->isHidden());
    QVERIFY(secondBar->isHidden());
    QVERIFY(first.setToolBar(nullptr));
    QVERIFY(first.toolBar() == nullptr);
}

void TreeIncrementalTest::onToolInputLifecycle()
{
    IDOSRenderView view;
    IDOSRenderView other;
    IDOSTestInputTool first(&view);
    IDOSTestInputTool second(&view);
    QVERIFY(!first.setActive(true));
    QVERIFY(!other.enableTool(&first));
    QVERIFY(view.enableTool(&first));
    QVERIFY(view.enableTool(&second));
    QKeyEvent event(QEvent::KeyPress, Qt::Key_F8, Qt::NoModifier);
    QApplication::sendEvent(view.vtkWidget(), &event);
    QCOMPARE(first.inputs, 1);
    QCOMPARE(second.inputs, 1);
    second.consumed = true;
    QApplication::sendEvent(view.vtkWidget(), &event);
    QCOMPARE(first.inputs, 1);
    QCOMPARE(second.inputs, 2);
    QVERIFY(view.captureTool(&first));
    QVERIFY(!view.captureTool(&second));
    view.releaseTool(&first);
    QVERIFY(view.disableTool(&first));
    QVERIFY(view.disableTool(&second));
    first.group = QStringLiteral("test.exclusive");
    second.group = QStringLiteral("test.exclusive");
    QVERIFY(view.enableTool(&second));
    QVERIFY(view.enableTool(&first));
    QVERIFY(!second.isActive());
    first.stopOnInput = true;
    QApplication::sendEvent(view.vtkWidget(), &event);
    QTRY_VERIFY(!first.isActive());
    QVERIFY(view.enabledTools().isEmpty());
    QApplication::sendEvent(view.vtkWidget(), &event);
    QCOMPARE(first.inputs, 2);
}

void TreeIncrementalTest::onToolObserversAndUnregister()
{
    IDOSToolRegistry registry;
    IDOSRenderView view;
    IDOSToolBar* bar = new IDOSToolBar();
    QVERIFY(bar->setRegistry(&registry));
    QVERIFY(view.setToolBar(bar));
    QVERIFY(registry.add(new IDOSTestToolMetadata()));
    const QList<IDOSToolProvider*> providers = bar->findChildren<IDOSToolProvider*>();
    QVERIFY(!providers.isEmpty());
    IDOSTestInputTool* tool = static_cast<IDOSTestInputTool*>(providers.first()->tools().first());
    tool->registry = &registry;
    QVERIFY(view.enableTool(tool));
    vtkSmartPointer<vtkObject> source = vtkSmartPointer<vtkObject>::New();
    tool->observe(source);
    source->Modified();
    QCOMPARE(tool->observations, 1);
    int eventValue = 42;
    source->InvokeEvent(vtkCommand::ModifiedEvent, &eventValue);
    QCOMPARE(tool->observations, 2);
    QCOMPARE(tool->observedValue, eventValue);
    vtkSmartPointer<vtkObject> temporarySource = vtkSmartPointer<vtkObject>::New();
    tool->observe(temporarySource);
    temporarySource = nullptr;
    QVERIFY(!tool->removalAccepted);
    QVERIFY(registry.metadata(QStringLiteral("test.input")) != nullptr);
    QVERIFY(view.disableTool(tool));
    source->Modified();
    QCOMPARE(tool->observations, 2);
    QVERIFY(view.enableTool(tool));
    QKeyEvent event(QEvent::KeyPress, Qt::Key_F8, Qt::NoModifier);
    QApplication::sendEvent(view.vtkWidget(), &event);
    QVERIFY(!tool->removalAccepted);
    QVERIFY(view.captureTool(tool));
    QPointer<IDOSTestInputTool> guard = tool;
    QVERIFY(registry.remove(QStringLiteral("test.input")));
    QVERIFY(guard == nullptr);
    QVERIFY(view.enabledTools().isEmpty());
    QVERIFY(view.capturedTool() == nullptr);
    QVERIFY(!source->HasObserver(vtkCommand::ModifiedEvent));
}

QTEST_MAIN(TreeIncrementalTest)
