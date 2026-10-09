#include <QAction>
#include <QFile>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScopedPointer>
#include <QTemporaryDir>
#include <QTimer>
#include <QVector3D>
#include <QtTest>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>

#include "idoscasedialog.h"
#include "idoscasetreemenuprovider.h"
#include "idoscasetreemodel.h"
#include "idoscasetreeview.h"
#include "idosdataobjecthandling.h"
#include "idosdataprovider.h"
#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idosgridrenderobjectprovider.h"
#include "idosmainwindow.h"
#include "idosproject.h"
#include "idosproviderregistry.h"
#include "idosrendermesh.h"
#include "idosrenderview.h"
#include "idossimulationcaseobject.h"
#include "idoswell.h"

#include "case_workflow_test.h"

void CaseWorkflowTest::onViewDecorations()
{
    IDOSMainWindow window;
    QAction* orientationAction = window.findChild<QAction*>(QStringLiteral("orientationMarkerAction"));
    QAction* legendAction = window.findChild<QAction*>(QStringLiteral("legendAction"));
    IDOSRenderView* view = window.findChild<IDOSRenderView*>();
    QVERIFY(orientationAction != nullptr);
    QVERIFY(legendAction != nullptr);
    QVERIFY(view != nullptr);
    QVERIFY(orientationAction->isCheckable());
    QVERIFY(orientationAction->isChecked());
    QVERIFY(orientationAction->isEnabled());
    QVERIFY(legendAction->isCheckable());
    QVERIFY(!legendAction->isEnabled());
    orientationAction->trigger();
    QVERIFY(!view->orientationMarkerVisible());
    view->setOrientationMarkerVisible(true);
    QVERIFY(orientationAction->isChecked());

    IDOSRenderMesh* mesh = new IDOSRenderMesh();
    for (int index = 0; index < 8; ++index)
    {
        mesh->addPoint(QVector3D(index & 1, (index >> 1) & 1, (index >> 2) & 1));
    }
    mesh->addHexahedron(0, 1, 3, 2, 4, 5, 7, 6);
    mesh->setCellScalars(QStringLiteral("Porosity"), QVector<double>{0.2});
    view->setObject(mesh);
    QVERIFY(view->legendAvailable());
    QVERIFY(legendAction->isEnabled());
    QVERIFY(legendAction->isChecked());
    legendAction->trigger();
    QVERIFY(!view->legendVisible());
    view->refresh();
    QVERIFY(!view->legendVisible());
    QVERIFY(!legendAction->isChecked());
    mesh->clearCellScalars();
    view->refresh();
    QVERIFY(!view->legendAvailable());
    QVERIFY(!legendAction->isEnabled());
    QVERIFY(view->setCellScalars(QStringLiteral("Permeability"), QVector<double>{12.0}));
    QVERIFY(legendAction->isEnabled());
    QVERIFY(!legendAction->isChecked());
    legendAction->trigger();
    QVERIFY(view->legendVisible());
    view->clear();
    QVERIFY(!view->legendAvailable());
    QVERIFY(!legendAction->isEnabled());
}

void CaseWorkflowTest::onProvider()
{
    std::unique_ptr<IDOSDataProvider> reader =
        IDOSProviderRegistry::instance().createProvider(QStringLiteral("idos.case.simulation.eclipse"));
    QVERIFY(reader != nullptr);
    QList<IDOSDataObject*> objects = reader->read(QFINDTESTDATA("data/case.DATA"));
    QVERIFY2(reader->lastError().isEmpty(), qPrintable(reader->lastError()));
    QCOMPARE(objects.size(), 1);
    IDOSSimulationCaseObject* simulationCase = qobject_cast<IDOSSimulationCaseObject*>(objects.first());
    QVERIFY(simulationCase != nullptr);
    QCOMPARE(simulationCase->name(), QStringLiteral("Sample Case"));
    QCOMPARE(simulationCase->pendingWellNames(), QStringList({QStringLiteral("A10"), QStringLiteral("B1")}));
    QVERIFY(!simulationCase->sourceFile().isEmpty());
    IDOSProject project;
    IDOSGrid* grid = new IDOSGrid();
    grid->setName(QStringLiteral("B1"));
    project.addObject(grid);
    IDOSWell* well = new IDOSWell();
    well->setName(QStringLiteral("a10"));
    project.addObject(well);
    simulationCase->resolveReferences(&project);
        QCOMPARE(simulationCase->itemRefs().size(), 1);
    QCOMPARE(simulationCase->itemRefs().first().objectId(), well->objectId());
    QCOMPARE(simulationCase->pendingWellNames(), QStringList{QStringLiteral("B1")});
    simulationCase->resolveReferences(&project);
        QCOMPARE(simulationCase->itemRefs().size(), 1);
    qDeleteAll(objects);
    QTemporaryDir temporary;
    const QString broken = temporary.filePath(QStringLiteral("broken.DATA"));
    QFile file(broken);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("RUNSPEC\nSCHEDULE\nINCLUDE\n'missing.inc' /\n");
    file.close();
    objects = reader->read(broken);
    QVERIFY(objects.isEmpty());
    QVERIFY(!reader->lastError().isEmpty());
    const QString actual = qEnvironmentVariable("IDOS_TEST_CASE_FILE");
    if (!actual.isEmpty())
    {
        objects = reader->read(actual);
        QVERIFY2(reader->lastError().isEmpty(), qPrintable(reader->lastError()));
        QVERIFY(objects.size() > 1);
        simulationCase = qobject_cast<IDOSSimulationCaseObject*>(objects.first());
        QVERIFY(simulationCase->name().contains(QStringLiteral("KES24_COMP")));
        QVERIFY(simulationCase->pendingWellNames().isEmpty());
        IDOSGrid* grid = nullptr;
        int staticProperties = 0;
        int regionProperties = 0;
        for (IDOSDataObject* object : objects)
        {
            IDOSGrid* candidateGrid = qobject_cast<IDOSGrid*>(object);
            if (candidateGrid != nullptr)
            {
                grid = candidateGrid;
                continue;
            }
            IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(object);
            if (property == nullptr)
            {
                continue;
            }
            if (property->keyword() == QStringLiteral("SATNUM") || property->keyword() == QStringLiteral("ROCKNUM") ||
                property->keyword() == QStringLiteral("PVTNUM") || property->keyword() == QStringLiteral("EQLNUM") ||
                property->keyword() == QStringLiteral("FIPNUM"))
            {
                ++regionProperties;
            }
            else
            {
                ++staticProperties;
            }
        }
        QVERIFY(grid != nullptr);
        QCOMPARE(grid->nx(), 125);
        QCOMPARE(grid->ny(), 21);
        QCOMPARE(grid->nz(), 50);
        Opm::Parser parser;
        const Opm::Deck deck = parser.parseFile(actual.toStdString());
        const Opm::EclipseState state(deck);
        const Opm::EclipseGrid& reference = state.getInputGrid();
        IDOSGridRenderObjectProvider renderProvider;
        QScopedPointer<IDOSRenderMesh> mesh(dynamic_cast<IDOSRenderMesh*>(renderProvider.createObject(static_cast<const IDOSDataObject*>(grid))));
        QVERIFY(mesh != nullptr);
        QCOMPARE(mesh->hexahedronCount(), static_cast<int>(reference.getNumActive()));
        const int opmCorners[8] = {0, 1, 3, 2, 4, 5, 7, 6};
        int renderedCell = 0;
        for (int k = 0; k < grid->nz(); ++k)
        {
            for (int j = 0; j < grid->ny(); ++j)
            {
                for (int i = 0; i < grid->nx(); ++i)
                {
                    for (int corner = 0; corner < 8; ++corner)
                    {
                        const std::array<double, 3> expected = reference.getCornerPos(i, j, k, opmCorners[corner]);
                        const QVector3D expectedPoint(expected[0], expected[1], expected[2]);
                        QCOMPARE(grid->cornerPosition(i, j, k, corner), expectedPoint);
                        if (grid->isActive(i, j, k))
                        {
                            const int pointId = mesh->hexahedra().at(renderedCell * 8 + corner);
                            QCOMPARE(mesh->points().at(pointId), expectedPoint);
                        }
                    }
                    if (grid->isActive(i, j, k))
                    {
                        QCOMPARE(mesh->cellGlobalIndices().at(renderedCell), i + j * grid->nx() + k * grid->nx() * grid->ny());
                        ++renderedCell;
                    }
                }
            }
        }
        qInfo("Compared all 1050000 grid corners and all active mesh corners against OPM getCornerPos.");
        IDOSRenderView renderView;
        renderView.resize(1200, 700);
        renderView.show();
        renderView.setObject(mesh.take());
        QTest::qWait(1000);
        const QString screenshotRoot = qEnvironmentVariable("IDOS_TEST_SCREENSHOT_DIR");
        if (!screenshotRoot.isEmpty())
        {
            QVERIFY(renderView.grab().save(screenshotRoot + QStringLiteral("/grid-opm-verified.png")));
        }
        QCOMPARE(simulationCase->itemRefs().size(), 1);
        QCOMPARE(simulationCase->itemRefs().first().objectId(), grid->objectId());
        QVERIFY(staticProperties >= 9);
        QVERIFY(regionProperties >= 4);
        qDeleteAll(objects);
    }
}

void CaseWorkflowTest::onCreateAndImport()
{
    IDOSMainWindow window;
    IDOSCaseTreeView* view = window.caseTreeView();
    IDOSCaseTreeModel* model = window.caseTreeModel();
    IDOSCaseTreeMenuProvider* menuProvider = view->findChild<IDOSCaseTreeMenuProvider*>();
    QVERIFY(menuProvider != nullptr);
    QVERIFY(menuProvider->createContextMenu() == nullptr);
    IDOSProject* project = new IDOSProject(&window);
    window.setProject(project);
    QScopedPointer<QMenu> menu(menuProvider->createContextMenu());
    QVERIFY(menu != nullptr);
    QCOMPARE(menu->actions().size(), 2);
    QTimer timer;
    connect(&timer, &QTimer::timeout, this, &CaseWorkflowTest::onDialog);
    timer.start(20);
    m_cancel = false;
    m_expectDisabled = false;
    m_name = QStringLiteral("  Base Case  ");
    menu->actions().first()->trigger();
    QCOMPARE(project->objects().size(), 1);
    QCOMPARE(model->rowCount(QModelIndex()), 1);
    const QModelIndex created = model->index(0, 0, QModelIndex());
    QCOMPARE(model->rowCount(created), 7);
    const QStringList categories = {QStringLiteral("Run Settings"),
                                    QStringLiteral("Grids"),
                                    QStringLiteral("Fluid and Rock Properties"),
                                    QStringLiteral("Initial State"),
                                    QStringLiteral("Wells and Development Plan"),
                                    QStringLiteral("Output Settings"),
                                    QStringLiteral("Summary Results")};
    for (int row = 0; row < categories.size(); ++row)
    {
        const QModelIndex category = model->index(row, 0, created);
        QCOMPARE(model->data(category, Qt::DisplayRole).toString(), categories.at(row));
        QCOMPARE(model->rowCount(category), 0);
        QVERIFY(!(model->flags(category) & Qt::ItemIsUserCheckable));
    }
    QCOMPARE(project->objects().first()->name(), QStringLiteral("Base Case"));
    m_name = QStringLiteral("base case");
    m_expectDisabled = true;
    QVERIFY(!IDOSDataObjectHandling::newCase(project, view));
    m_name = QStringLiteral("   ");
    QVERIFY(!IDOSDataObjectHandling::newCase(project, view));
    m_expectDisabled = false;
    m_cancel = true;
    m_name = QStringLiteral("Cancelled");
    QVERIFY(!IDOSDataObjectHandling::newCase(project, view));
    QCOMPARE(project->objects().size(), 1);
    const QString source = QFINDTESTDATA("data/case.DATA");
    QVERIFY(!IDOSDataObjectHandling::importCase(project, source, view));
    QCOMPARE(project->objects().size(), 1);
    IDOSWell* well = new IDOSWell();
    well->setName(QStringLiteral("A10"));
    project->addObject(well);
    m_cancel = false;
    m_name = QStringLiteral("Imported Case");
    QVERIFY(IDOSDataObjectHandling::importCase(project, source, view));
    QCOMPARE(project->objects().size(), 3);
    QCOMPARE(model->rowCount(QModelIndex()), 2);
    IDOSSimulationCaseObject* imported = qobject_cast<IDOSSimulationCaseObject*>(project->objectByName(m_name));
    QVERIFY(imported != nullptr);
    QCOMPARE(imported->itemRefs().size(), 1);
    for (int row = 0; row < model->rowCount(QModelIndex()); ++row)
    {
        const QModelIndex index = model->index(row, 0, QModelIndex());
        if (model->objectFromIndex(index) == imported)
        {
            const QModelIndex inputs = model->index(4, 0, index);
            QCOMPARE(model->rowCount(inputs), 2);
            QCOMPARE(model->data(model->index(0, 0, inputs), Qt::DisplayRole).toString(), QStringLiteral("A10"));
            QVERIFY(model->data(model->index(1, 0, inputs), Qt::DisplayRole).toString().contains(QStringLiteral("B1")));
        }
    }
    m_expectDisabled = true;
    QVERIFY(!IDOSDataObjectHandling::importCase(project, source, view));
    QCOMPARE(project->objects().size(), 3);
    timer.stop();

    IDOSGrid* grid = new IDOSGrid();
    grid->setName(QStringLiteral("Main Grid"));
    grid->setDimensions(1, 1, 1);
    project->addObject(grid);
    imported->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), grid->objectId()));
    const QStringList keywords = {QStringLiteral("PORO"), QStringLiteral("SATNUM"), QStringLiteral("PRESSURE")};
    for (int index = 0; index < keywords.size(); ++index)
    {
        IDOSGridProperty* property = new IDOSGridProperty();
        property->setName(keywords.at(index));
        property->setKeyword(keywords.at(index));
        property->setGridId(grid->objectId());
        property->setDimensions(1, 1, 1);
        QVERIFY(property->setValues(QVector<double>{0.2}));
        if (index == 2)
        {
            property->setKind(IDOSGridProperty::Kind::Dynamic);
        }
        project->addObject(property);
    }
    for (int row = 0; row < model->rowCount(QModelIndex()); ++row)
    {
        const QModelIndex index = model->index(row, 0, QModelIndex());
        if (model->objectFromIndex(index) == imported)
        {
            const QModelIndex grids = model->index(1, 0, index);
            QCOMPARE(model->rowCount(grids), 1);
            const QModelIndex mainGrid = model->index(0, 0, grids);
            QCOMPARE(model->rowCount(mainGrid), 3);
            for (int category = 0; category < 3; ++category)
            {
                const QModelIndex properties = model->index(category, 0, mainGrid);
                QVERIFY(model->rowCount(properties) >= 1);
                const QModelIndex property = model->index(model->rowCount(properties) - 1, 0, properties);
                QCOMPARE(model->data(property, Qt::DisplayRole).toString(), keywords.at(category));
            }
        }
    }
}

void CaseWorkflowTest::onDialog()
{
    QMessageBox* message = qobject_cast<QMessageBox*>(QApplication::activeModalWidget());
    if (message != nullptr)
    {
        message->accept();
        return;
    }
    IDOSCaseDialog* dialog = qobject_cast<IDOSCaseDialog*>(QApplication::activeModalWidget());
    if (dialog == nullptr)
    {
        return;
    }
    dialog->findChild<QLineEdit*>(QStringLiteral("caseName"))->setText(m_name);
    QPushButton* confirm = dialog->findChild<QPushButton*>(QStringLiteral("confirmCase"));
    if (m_expectDisabled)
    {
        QVERIFY(!confirm->isEnabled());
    }
    const QString screenshotRoot = qEnvironmentVariable("IDOS_TEST_SCREENSHOT_DIR");
    if (!screenshotRoot.isEmpty() && dialog->windowTitle().contains(QStringLiteral("Import")))
    {
        dialog->grab().save(screenshotRoot + QStringLiteral("/case-import.png"));
    }
    if (m_cancel || m_expectDisabled)
    {
        dialog->reject();
    }
    else
    {
        confirm->click();
    }
}
QTEST_MAIN(CaseWorkflowTest)






