#include <QSignalSpy>
#include <QStringList>
#include <QtTest>

#include "idoscaseitemref.h"
#include "idoscasetreemodel.h"
#include "idoscasetreeview.h"
#include "idosdatatreemodel.h"
#include "idosdatatreeview.h"
#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idosmainwindow.h"
#include "idosproject.h"
#include "idossimulationcaseobject.h"
#include "idoswell.h"

#include "tree_incremental_test.h"

// 测试替身：objectId 正常构造后不可外部修改（protected，仅供恢复加载），
// 子类开放一个赋值入口以构造 Project::addObject 的同 id 替换场景
class IDOSTestWell : public IDOSWell
{
  public:
    void assignObjectId(const QString& id) { setObjectId(id); }
};

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
    IDOSDataTreeModel* model = window.dataTreeModel();
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
    project->addObject(well);
    QVERIFY(!m_dataTreeReset);
    QCOMPARE(m_dataRowsInserted, 1);
    QCOMPARE(m_dataRowsRemoved, 0);
    QCOMPARE(model->rowCount(wells), 1);
    QCOMPARE(model->data(model->index(0, 0, wells), Qt::DisplayRole).toString(),
             QStringLiteral("A10"));

    // 改名：分支原地重建，行位置不变，仍然没有整树 reset
    resetSignalCounters();
    well->setName(QStringLiteral("A11"));
    QVERIFY(!m_dataTreeReset);
    QCOMPARE(model->rowCount(wells), 1);
    QCOMPARE(model->data(model->index(0, 0, wells), Qt::DisplayRole).toString(),
             QStringLiteral("A11"));

    // 增量删除：只发删除信号，不触发整树 reset
    resetSignalCounters();
    QVERIFY(project->removeObject(well->objectId()));
    QVERIFY(!m_dataTreeReset);
    QCOMPARE(m_dataRowsRemoved, 1);
    QCOMPARE(m_dataRowsInserted, 0);
    QCOMPARE(model->rowCount(wells), 0);
}

void TreeIncrementalTest::onSameIdReplacementDoesNotDuplicate()
{
    IDOSMainWindow window;
    IDOSProject* project = new IDOSProject(&window);
    IDOSDataTreeModel* model = window.dataTreeModel();
    connect(model, &QAbstractItemModel::modelAboutToBeReset, this, &TreeIncrementalTest::onDataTreeReset);
    window.setProject(project);

    const QModelIndex wells = findIndexByName(model, QModelIndex(), QStringLiteral("Wells"));
    QVERIFY(wells.isValid());

    IDOSTestWell* well = new IDOSTestWell();
    well->setName(QStringLiteral("A10"));
    const QString fixedId = QStringLiteral("test-well-id");
    well->assignObjectId(fixedId);
    project->addObject(well);
    QCOMPARE(model->rowCount(wells), 1);

    // 同 objectId 替换：Project 删旧对象后只发 objectAdded，树上不得出现重复节点
    resetSignalCounters();
    IDOSTestWell* replacement = new IDOSTestWell();
    replacement->setName(QStringLiteral("A10-Rebuilt"));
    replacement->assignObjectId(fixedId);
    project->addObject(replacement);
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
    IDOSDataTreeModel* dataModel = window.dataTreeModel();
    IDOSDataTreeView* dataView = window.dataTreeView();
    IDOSCaseTreeModel* caseModel = window.caseTreeModel();
    IDOSCaseTreeView* caseView = window.caseTreeView();

    // 数据树：井改名触发分支重建，井节点展开态必须保留
    IDOSWell* well = new IDOSWell();
    well->setName(QStringLiteral("A10"));
    project->addObject(well);
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
    project->addObject(grid);
    IDOSSimulationCaseObject* caseObject = new IDOSSimulationCaseObject();
    caseObject->setName(QStringLiteral("Base Case"));
    caseObject->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), grid->objectId()));
    project->addObject(caseObject);

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
    project->addObject(poro);

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
    IDOSCaseTreeModel* caseModel = window.caseTreeModel();
    connect(caseModel, &QAbstractItemModel::modelAboutToBeReset, this, &TreeIncrementalTest::onCaseTreeReset);
    connect(caseModel, &QAbstractItemModel::rowsInserted, this, &TreeIncrementalTest::onCaseRowsInserted);
    connect(caseModel, &QAbstractItemModel::rowsRemoved, this, &TreeIncrementalTest::onCaseRowsRemoved);
    window.setProject(project);

    IDOSGrid* grid = new IDOSGrid();
    grid->setName(QStringLiteral("Main Grid"));
    grid->setDimensions(1, 1, 1);
    project->addObject(grid);

    IDOSSimulationCaseObject* caseObject = new IDOSSimulationCaseObject();
    caseObject->setName(QStringLiteral("Base Case"));
    caseObject->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), grid->objectId()));
    resetSignalCounters();
    project->addObject(caseObject);
    QCOMPARE(m_caseRowsInserted, 1);   // 工况分支插入
    QCOMPARE(m_caseRowsRemoved, 0);

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
        project->addObject(property);
    }
    project->endUpdate();
    QVERIFY(allPropertiesValid);

    QVERIFY(!m_caseTreeReset);
    // 关键断言：5 个属性只让工况分支摘除/重建一次，而不是 5 次（修复 O(N²)）
    QCOMPARE(m_caseRowsRemoved, 1);
    QCOMPARE(m_caseRowsInserted, 1);

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
    project.addObject(well1);
    project.addObject(well2);
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
    project.addObject(well3);
    project.endUpdate();
    QCOMPARE(batchAddedSpy.count(), 0);
    project.endUpdate();
    QCOMPARE(batchAddedSpy.count(), 1);
    QCOMPARE(batchAddedSpy.takeFirst().at(0).toStringList().count(), 1);

    // 归并：事务内把刚加入的对象删掉，最终只算 removed，不进 added
    IDOSWell* well4 = new IDOSWell();
    well4->setName(QStringLiteral("W4"));
    project.addObject(well4);
    project.beginUpdate();
    const bool wellRemoved = project.removeObject(well4->objectId());
    project.endUpdate();
    QVERIFY(wellRemoved);
    QCOMPARE(batchRemovedSpy.count(), 1);
    QCOMPARE(batchRemovedSpy.takeFirst().at(0).toStringList(), QStringList(well4->objectId()));

    // 非事务路径行为不变：仍发单个信号
    singleAddedSpy.clear();
    IDOSWell* well5 = new IDOSWell();
    well5->setName(QStringLiteral("W5"));
    project.addObject(well5);
    QCOMPARE(singleAddedSpy.count(), 1);
}

void TreeIncrementalTest::onCaseTreeReferenceRefreshAndCheckPreserved()
{
    IDOSMainWindow window;
    IDOSProject* project = new IDOSProject(&window);
    IDOSCaseTreeModel* model = window.caseTreeModel();
    connect(model, &QAbstractItemModel::modelAboutToBeReset, this, &TreeIncrementalTest::onCaseTreeReset);
    connect(model, &QAbstractItemModel::rowsInserted, this, &TreeIncrementalTest::onCaseRowsInserted);
    connect(model, &QAbstractItemModel::rowsRemoved, this, &TreeIncrementalTest::onCaseRowsRemoved);
    window.setProject(project);

    IDOSGrid* grid = new IDOSGrid();
    grid->setName(QStringLiteral("Main Grid"));
    grid->setDimensions(1, 1, 1);
    project->addObject(grid);

    IDOSSimulationCaseObject* caseObject = new IDOSSimulationCaseObject();
    caseObject->setName(QStringLiteral("Base Case"));
    caseObject->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), grid->objectId()));
    project->addObject(caseObject);
    QCOMPARE(model->rowCount(QModelIndex()), 1);

    // 属性在工况之后加入：通过 refreshReferencingBranches 增量进入工况子树
    resetSignalCounters();
    IDOSGridProperty* poro = new IDOSGridProperty();
    poro->setName(QStringLiteral("PORO"));
    poro->setKeyword(QStringLiteral("PORO"));
    poro->setGridId(grid->objectId());
    poro->setDimensions(1, 1, 1);
    QVERIFY(poro->setValues(QVector<double>{0.2}));
    project->addObject(poro);
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
    permx->setKeyword(QStringLiteral("PERMX"));
    permx->setGridId(grid->objectId());
    permx->setDimensions(1, 1, 1);
    QVERIFY(permx->setValues(QVector<double>{100.0}));
    project->addObject(permx);
    QVERIFY(!m_caseTreeReset);

    poroIndex = findIndexByName(model, QModelIndex(), QStringLiteral("PORO"));
    QVERIFY(poroIndex.isValid());
    QCOMPARE(model->data(poroIndex, Qt::CheckStateRole).toInt(), int(Qt::Checked));
    const QModelIndex permxIndex = findIndexByName(model, QModelIndex(), QStringLiteral("PERMX"));
    QVERIFY(permxIndex.isValid());
    QCOMPARE(model->data(permxIndex, Qt::CheckStateRole).toInt(), int(Qt::Unchecked));

    // 网格被删：工况分支刷新为 Missing object，引用节点不可勾选，属性子树消失
    resetSignalCounters();
    QVERIFY(project->removeObject(grid->objectId()));
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

QTEST_MAIN(TreeIncrementalTest)
#include "tree_incremental_test.moc"
