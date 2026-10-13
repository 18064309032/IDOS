#include <cmath>
#include <limits>
#include <memory>
#include <type_traits>

#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QDir>
#include <QDoubleSpinBox>
#include <QDialog>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMenu>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPixmap>
#include <QPushButton>
#include <QScopedPointer>
#include <QSettings>
#include <QSignalSpy>
#include <QSize>
#include <QSlider>
#include <QTemporaryDir>
#include <QTableWidget>
#include <QTimer>
#include <QtTest>
#include <QVariantMap>
#include <QVector3D>
#include <QWidget>

#include <vtkActor.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkLookupTable.h>
#include <vtkMapper.h>
#include <vtkProperty.h>

#include <SARibbonBar.h>
#include <SARibbonCategory.h>
#include <SARibbonPanel.h>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>

#include "command/idoscommandmanager.h"
#include "command/idoscommandregistry.h"
#include "idosapplication.h"
#include "idoscasedialog.h"
#include "idoscaseitemref.h"
#include "idosdatametadata.h"
#include "idosdataprovider.h"
#include "idosdataproviderregistry.h"
#include "idosdataregistry.h"
#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idosgridrendermetadata.h"
#include "idosgridrenderprovider.h"
#include "idosimportrestartresultscommand.h"
#include "idosmainwindow.h"
#include "idospluginregistry.h"
#include "idosproject.h"
#include "idosrenderobject.h"
#include "idosrenderprovider.h"
#include "idosrenderregistry.h"
#include "idosrenderscene.h"
#include "idosrenderserver.h"
#include "idosrenderview.h"
#include "idosruntimeinfowidget.h"
#include "idossimulationcaseobject.h"
#include "idossimulationcasetreeguiprovider.h"
#include "idossimulationinitializewidget.h"
#include "idossimulationresultplaybackwidget.h"
#include "idossimulationrunstatuswidget.h"
#include "idostreemodel.h"
#include "idostreeregistry.h"
#include "idostreeview.h"
#include "idoswell.h"
#include "idoswellhead.h"
#include "idoswellpath.h"
#include "idoswellpathpoint.h"
#include "idoswellrendermetadata.h"
#include "idoswellrenderprovider.h"
#include "log/idoslogger.h"

#include "case_workflow_test.h"

static_assert(!std::is_abstract<IDOSRenderObject>::value, "RenderObject must be concrete.");
static_assert(std::is_abstract<IDOSRenderProvider>::value, "RenderProvider must remain abstract.");

void CaseWorkflowTest::onFlowExecution()
{
    const QString source = qEnvironmentVariable("IDOS_FLOW_SMOKE_DATA");
    if (source.isEmpty())
    {
        QSKIP("Set IDOS_FLOW_SMOKE_DATA to the SPE1 deck to test the installed WSL Flow.");
    }
    QTemporaryDir directory(QFileInfo(source).absolutePath() + QStringLiteral("/flow test XXXXXX"));
    QVERIFY(directory.isValid());
    QVERIFY(QFile::copy(source, directory.path() + QStringLiteral("/SPE1.DATA")));
    IDOSProject project;
    IDOSCommandRegistry registry;
    QVERIFY(registry.add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportRestartResultsCommandMetadata())));
    project.commandManager()->setRegistry(&registry);
    std::unique_ptr<IDOSDataProvider> reader = IDOSDataProviderRegistry::instance().create(QStringLiteral("idos.case.simulation.eclipse"));
    QVERIFY(reader != nullptr);
    const QList<IDOSDataObject*> objects = reader->read(directory.path() + QStringLiteral("/SPE1.DATA"));
    QVERIFY2(reader->lastError().isEmpty(), qPrintable(reader->lastError()));
    IDOSSimulationCaseObject* simulationCase = nullptr;
    for (IDOSDataObject* object : objects)
    {
        if (qobject_cast<IDOSSimulationCaseObject*>(object) != nullptr)
        {
            simulationCase = qobject_cast<IDOSSimulationCaseObject*>(object);
        }
        project.add(object);
    }
    QVERIFY(simulationCase != nullptr);
    const int initialCount = project.objects().size();
    IDOSSimulationInitializeWidget initialize(simulationCase);
    QLineEdit* runDirectory = initialize.findChild<QLineEdit*>();
    QVERIFY(runDirectory != nullptr);
    runDirectory->setText(directory.path() + QStringLiteral("/prepared run"));
    QVERIFY(QMetaObject::invokeMethod(&initialize, "onInitializeTriggered", Qt::DirectConnection));
    if (simulationCase->runInfo().isEmpty())
    {
        qInfo().noquote() << initialize.findChild<QPlainTextEdit*>()->toPlainText();
    }
    QCOMPARE(simulationCase->runInfo().value(QStringLiteral("status")).toString(), QStringLiteral("initialized"));
    IDOSSimulationRunStatusWidget window(simulationCase);
    QLineEdit* distribution = window.findChild<QLineEdit*>(QStringLiteral("flowDistributionEdit"));
    QLineEdit* executable = window.findChild<QLineEdit*>(QStringLiteral("flowExecutableEdit"));
    QPushButton* start = window.findChild<QPushButton*>(QStringLiteral("flowStartButton"));
    QVERIFY(distribution != nullptr && executable != nullptr && start != nullptr);
    distribution->setText(QStringLiteral("Ubuntu-24.04"));
    executable->setText(QStringLiteral("/usr/bin/flow"));
    start->click();
    QTRY_VERIFY_WITH_TIMEOUT(!window.isRunning(), 120000);
    qInfo().noquote() << simulationCase->runInfo().value(QStringLiteral("message")).toString();
    QCOMPARE(simulationCase->runInfo().value(QStringLiteral("status")).toString(), QStringLiteral("succeeded"));
    const QString screenshotRoot = qEnvironmentVariable("IDOS_TEST_SCREENSHOT_DIR");
    if (!screenshotRoot.isEmpty())
    {
        window.grab().save(screenshotRoot + QStringLiteral("/flow-run.png"));
    }
    QVERIFY(project.objects().size() > initialCount);
    const int importedCount = project.objects().size();
    QVERIFY(project.commandManager()->canUndo());
    project.commandManager()->undo();
    QCOMPARE(project.objects().size(), initialCount);
    project.commandManager()->redo();
    QCOMPARE(project.objects().size(), importedCount);

    executable->setText(QStringLiteral("/bin/true"));
    start->click();
    QTRY_VERIFY_WITH_TIMEOUT(!window.isRunning(), 15000);
    QCOMPARE(simulationCase->runInfo().value(QStringLiteral("status")).toString(), QStringLiteral("importFailed"));
    QCOMPARE(project.objects().size(), importedCount);
    executable->setText(QStringLiteral("/no-such-flow"));
    start->click();
    QTRY_VERIFY_WITH_TIMEOUT(!window.isRunning(), 15000);
    QCOMPARE(simulationCase->runInfo().value(QStringLiteral("status")).toString(), QStringLiteral("failed"));

    QFile sleeper(directory.path() + QStringLiteral("/sleeper.sh"));
    QVERIFY(sleeper.open(QIODevice::WriteOnly));
    sleeper.write("#!/bin/sh\nprintf 'stage-'\nsleep 0.1\nprintf 'one\\r\\nstage-two\\rtail'\nexec /bin/sleep 60\n");
    sleeper.close();
    const QString linuxScript = QStringLiteral("/mnt/") + directory.path().left(1).toLower() + directory.path().mid(2) +
                                QStringLiteral("/sleeper.sh");
    QCOMPARE(QProcess::execute(QStringLiteral("wsl.exe"), QStringList()
                                                              << QStringLiteral("-d") << QStringLiteral("Ubuntu-24.04")
                                                              << QStringLiteral("--exec") << QStringLiteral("chmod")
                                                              << QStringLiteral("+x") << linuxScript),
             0);
    executable->setText(linuxScript);
    QSignalSpy logSpy(&IDOSLogger::instance(), &IDOSLogger::messagePosted);
    start->click();
    QVERIFY(window.isRunning());
    QTRY_VERIFY_WITH_TIMEOUT(window.findChild<QPlainTextEdit*>()->toPlainText().contains(QStringLiteral("stage-two")), 10000);
    window.close();
    QVERIFY(window.isRunning());
    window.cancel();
    QTRY_VERIFY_WITH_TIMEOUT(!window.isRunning(), 20000);
    QCOMPARE(simulationCase->runInfo().value(QStringLiteral("status")).toString(), QStringLiteral("cancelled"));
    QCOMPARE(project.objects().size(), importedCount);
    QVERIFY(!start->isEnabled());
    int firstLineCount = 0;
    int secondLineCount = 0;
    int tailCount = 0;
    for (const QList<QVariant>& arguments : logSpy)
    {
        const QString message = arguments.first().toString();
        if (message == QStringLiteral("stage-one"))
        {
            ++firstLineCount;
        }
        if (message == QStringLiteral("stage-two"))
        {
            ++secondLineCount;
        }
        if (message.startsWith(QStringLiteral("tail")))
        {
            ++tailCount;
        }
    }
    QCOMPARE(firstLineCount, 1);
    QCOMPARE(secondLineCount, 1);
    QCOMPARE(tailCount, 1);
    const QString outputDirectory = simulationCase->runInfo().value(QStringLiteral("outputDirectory")).toString();
    QFile pidFile(outputDirectory + QStringLiteral("/flow.pid"));
    QVERIFY(pidFile.open(QIODevice::ReadOnly));
    const qlonglong pid = pidFile.readAll().trimmed().toLongLong();
    QVERIFY(pid > 1);
    QVERIFY(QProcess::execute(QStringLiteral("wsl.exe"), QStringList()
                                                             << QStringLiteral("-d") << QStringLiteral("Ubuntu-24.04")
                                                             << QStringLiteral("--exec") << QStringLiteral("/bin/kill")
                                                             << QStringLiteral("-0") << QStringLiteral("--")
                                                             << QStringLiteral("-%1").arg(pid)) != 0);
}

void CaseWorkflowTest::onFlowOutputFraming()
{
    const QString source = qEnvironmentVariable("IDOS_FLOW_SMOKE_DATA");
    if (source.isEmpty())
    {
        QSKIP("Set IDOS_FLOW_SMOKE_DATA to test WSL output framing.");
    }
    QTemporaryDir directory(QFileInfo(source).absolutePath() + QStringLiteral("/output test XXXXXX"));
    QVERIFY(directory.isValid());
    QVERIFY(QFile::copy(source, directory.path() + QStringLiteral("/input.DATA")));
    QFile script(directory.path() + QStringLiteral("/output.sh"));
    QVERIFY(script.open(QIODevice::WriteOnly));
    script.write("#!/bin/sh\nprintf 'stage-'\nsleep 0.1\nprintf 'one\\r\\nstage-two\\rtail'\n");
    script.close();
    const QString linuxScript = QStringLiteral("/mnt/") + directory.path().left(1).toLower()
        + directory.path().mid(2) + QStringLiteral("/output.sh");
    QCOMPARE(QProcess::execute(QStringLiteral("wsl.exe"), QStringList() << QStringLiteral("-d")
        << QStringLiteral("Ubuntu-24.04") << QStringLiteral("--exec") << QStringLiteral("chmod")
        << QStringLiteral("+x") << linuxScript), 0);
    IDOSProject project;
    IDOSGrid* grid = new IDOSGrid();
    grid->setDimensions(1, 1, 1);
    project.add(grid);
    IDOSSimulationCaseObject* simulationCase = new IDOSSimulationCaseObject();
    simulationCase->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), grid->objectId()));
    QVariantMap info;
    info.insert(QStringLiteral("runDirectory"), directory.path());
    info.insert(QStringLiteral("dataFile"), QStringLiteral("input.DATA"));
    simulationCase->setRunInfo(info);
    project.add(simulationCase);
    IDOSSimulationRunStatusWidget window(simulationCase);
    window.findChild<QLineEdit*>(QStringLiteral("flowDistributionEdit"))->setText(QStringLiteral("Ubuntu-24.04"));
    window.findChild<QLineEdit*>(QStringLiteral("flowExecutableEdit"))->setText(linuxScript);
    QSignalSpy spy(&IDOSLogger::instance(), &IDOSLogger::messagePosted);
    window.findChild<QPushButton*>(QStringLiteral("flowStartButton"))->click();
    QTRY_VERIFY_WITH_TIMEOUT(!window.isRunning(), 15000);
    QStringList lines;
    for (const QList<QVariant>& arguments : spy)
    {
        lines.append(arguments.first().toString());
    }
    QCOMPARE(lines.count(QStringLiteral("stage-one")), 1);
    QCOMPARE(lines.count(QStringLiteral("stage-two")), 1);
    QCOMPARE(lines.count(QStringLiteral("tail")), 1);
    QCOMPARE(simulationCase->runInfo().value(QStringLiteral("status")).toString(), QStringLiteral("importFailed"));
}

void CaseWorkflowTest::onSimulationOutput()
{
    IDOSProject project;
    IDOSGrid* grid = new IDOSGrid();
    grid->setDimensions(1, 1, 1);
    project.add(grid);
    IDOSSimulationCaseObject* simulationCase = new IDOSSimulationCaseObject();
    simulationCase->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), grid->objectId()));
    project.add(simulationCase);
    IDOSGridProperty* property = new IDOSGridProperty();
    property->setGridId(grid->objectId());
    property->setKeyword(QStringLiteral("SGAS"));
    property->setKind(IDOSGridProperty::Kind::Dynamic);
    property->setDimensions(1, 1, 1);
    QVERIFY(property->setReportStepValues(1, QVector<double>{0.1}));
    QVERIFY(property->setReportStepValues(3, QVector<double>{0.3}));
    QVERIFY(property->setReportStepValues(8, QVector<double>{0.8}));
    project.add(property);
    IDOSRuntimeInfoWidget output;
    QPlainTextEdit* text = output.findChild<QPlainTextEdit*>();
    QVERIFY(text != nullptr);
    IDOSSimulationResultPlaybackWidget playback;
    QSignalSpy spy(&IDOSLogger::instance(), &IDOSLogger::messagePosted);
    playback.setCurrentObject(simulationCase);
    QCOMPARE(spy.size(), 1);
    QTRY_VERIFY(text->toPlainText().contains(QStringLiteral("Displaying SGAS at report step 1.")));
    QSlider* slider = playback.findChild<QSlider*>();
    QVERIFY(slider != nullptr);
    slider->setValue(1);
    QCOMPARE(spy.size(), 2);
    QTRY_VERIFY(text->toPlainText().contains(QStringLiteral("Displaying SGAS at report step 3.")));
    QVERIFY(property->setCurrentReportStep(8));
    QCOMPARE(spy.size(), 3);
    QTRY_VERIFY(text->toPlainText().contains(QStringLiteral("Displaying SGAS at report step 8.")));
    spy.clear();
    QVERIFY(QMetaObject::invokeMethod(&playback, "onPlayTriggered", Qt::DirectConnection));
    QTimer* timer = playback.findChild<QTimer*>();
    QVERIFY(timer != nullptr);
    QTRY_VERIFY_WITH_TIMEOUT(!timer->isActive(), 3000);
    QCOMPARE(spy.size(), 5);
    QTRY_VERIFY(text->toPlainText().contains(QStringLiteral("Report-step playback started.")));
    QTRY_VERIFY(text->toPlainText().contains(QStringLiteral("Report-step playback completed.")));
    QCOMPARE(property->currentReportStep(), 8);
}

void CaseWorkflowTest::onProviderDimensions()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QFile input(directory.path() + QStringLiteral("/injection.DATA"));
    QVERIFY(input.open(QIODevice::WriteOnly));
    input.write("RUNSPEC\nTITLE\nInjection test\nFIELD\nGRID\nPROPS\nSOLUTION\nSCHEDULE\n"
                "WCONINJE\n'INJ' 'GAS' 'OPEN' 'RATE' 100000 1* 9014 /\n/\nEND\n");
    input.close();
    std::unique_ptr<IDOSDataProvider> reader =
        IDOSDataProviderRegistry::instance().create(QStringLiteral("idos.case.simulation.eclipse"));
    QVERIFY(reader != nullptr);
    const QList<IDOSDataObject*> objects = reader->read(input.fileName());
    QVERIFY2(reader->lastError().isEmpty(), qPrintable(reader->lastError()));
    QCOMPARE(objects.size(), 1);
    IDOSProject project;
    project.add(objects.first());
    IDOSSimulationCaseObject* simulationCase = qobject_cast<IDOSSimulationCaseObject*>(objects.first());
    QVERIFY(simulationCase != nullptr);
    QCOMPARE(simulationCase->wellControlRows().size(), 1);
    const QVariantList keywordIndex = simulationCase->parsedSourceData().value(QStringLiteral("keywordIndex")).toList();
    QVERIFY(!keywordIndex.isEmpty());
    for (const QVariant& keyword : keywordIndex)
    {
        QCOMPARE(keyword.toMap().value(QStringLiteral("file")).toString(), QFileInfo(input.fileName()).absoluteFilePath());
    }
    const QVariantMap fields = simulationCase->wellControlRows().first().toMap().value(QStringLiteral("fields")).toMap();
    QVERIFY(fields.contains(QStringLiteral("GAS_STEAM_RATIO")));
    QVERIFY(fields.value(QStringLiteral("BHP")).toDouble() > 9014.0);
}

void CaseWorkflowTest::onExamplePlugin()
{
    QSettings settings(QStringLiteral("pluginRegistry"), QStringLiteral("idos"));
    const QString settingKey = QStringLiteral("/plugins/idosexampleplugin");
    const bool hadSetting = settings.contains(settingKey);
    const QVariant previousSetting = settings.value(settingKey);
    IDOSProject project;
    IDOSProject otherProject;
    IDOSMainWindow window;
    window.setProject(&project);
    const QString path = QCoreApplication::applicationDirPath() + QStringLiteral("/plugins/idosexampleplugin.dll");
    IDOSPluginRegistry* plugins = IDOSPluginRegistry::instance();
    plugins->loadCppPlugin(path);
    QVERIFY2(plugins->isLoaded(QStringLiteral("idosexampleplugin")),
             qPrintable(plugins->pluginError(QStringLiteral("idosexampleplugin"))));
    const QString typeId = QStringLiteral("idos.example.pointset");
    QVERIFY(window.dataRegistry()->metadata(typeId) != nullptr);
    QVERIFY(window.treeRegistry()->metadata(typeId) != nullptr);
    QVERIFY(window.renderRegistry()->metadata(typeId) != nullptr);
    QVERIFY(window.commandRegistry()->find(QStringLiteral("pointset.create")) != nullptr);
    QScopedPointer<QMenu> blankMenu(window.dataTreeView()->createMenu(QModelIndex()));
    QVERIFY(blankMenu->actions().isEmpty());
    QJsonObject invalid;
    QVERIFY(!project.commandManager()->execute(QStringLiteral("pointset.create"), invalid)
                 .value(QStringLiteral("success")).toBool());
    QVERIFY(!project.commandManager()->canUndo());

    QJsonObject arguments;
    arguments.insert(QStringLiteral("name"), QStringLiteral("Extension sample"));
    IDOSCommandManager* manager = project.commandManager();
    QJsonObject response = manager->execute(QStringLiteral("pointset.create"), arguments);
    QVERIFY2(response.value(QStringLiteral("success")).toBool(), qPrintable(manager->error()));
    const QString id = response.value(QStringLiteral("result")).toObject().value(QStringLiteral("objectId")).toString();
    QVERIFY(project.object(id) != nullptr);
    QCOMPARE(project.object(id)->typeId(), typeId);
    const QModelIndex index = window.dataTreeModel()->indexFromDataId(id);
    QVERIFY(index.isValid());
    QCOMPARE(window.dataTreeModel()->rowCount(index), 8);
    IDOSSimulationCaseObject* caseObject = new IDOSSimulationCaseObject();
    caseObject->setName(QStringLiteral("Point set reference"));
    caseObject->addItemRef(IDOSCaseItemRef(QStringLiteral("case.input"), id));
    project.add(caseObject);
    const QModelIndex referenceIndex = window.caseTreeModel()->indexFromDataId(id);
    QVERIFY(referenceIndex.isValid());
    QCOMPARE(window.caseTreeModel()->rowCount(referenceIndex), 8);
    const QList<IDOSRenderView*> views = window.findChildren<IDOSRenderView*>();
    QVERIFY(!views.isEmpty());
    IDOSRenderView* firstView = views.first();
    IDOSRenderObject* firstObject = findRenderObject(firstView, id);
    QVERIFY(firstObject != nullptr);
    QCOMPARE(firstObject->provider()->actors().first()->GetMapper()->GetInput()->GetNumberOfPoints(), vtkIdType(8));
    QPersistentModelIndex persistentIndex(index);
    project.object(id)->setName(QStringLiteral("Updated sample"));
    QVERIFY(persistentIndex.isValid());
    QCOMPARE(persistentIndex.data().toString(), QStringLiteral("Updated sample"));
    QCOMPARE(findRenderObject(firstView, id), firstObject);

    IDOSRenderView secondView;
    IDOSRenderServer server(window.renderRegistry());
    server.addView(&secondView);
    server.setProject(&project);
    IDOSRenderObject* secondObject = findRenderObject(&secondView, id);
    QVERIFY(secondObject != nullptr);
    QVERIFY(firstObject->id() != secondObject->id());
    const double originalSize = firstObject->provider()->actors().first()->GetProperty()->GetPointSize();
    secondObject->setOpacity(0.5);
    QCOMPARE(secondObject->provider()->actors().first()->GetProperty()->GetOpacity(), 0.5);
    QCOMPARE(firstObject->opacity(), 1.0);
    QCOMPARE(firstObject->provider()->actors().first()->GetProperty()->GetPointSize(), originalSize);
    QCOMPARE(manager->undoText(), QObject::tr("Create Point Set"));
    secondView.resize(900, 600);
    secondView.show();
    secondView.resetCamera();
    secondView.render();
    QTest::qWait(150);
    const QImage image = secondView.captureImage();
    QVERIFY(!image.isNull());
    const QString screenshotRoot = qEnvironmentVariable("IDOS_TEST_SCREENSHOT_DIR");
    if (!screenshotRoot.isEmpty())
    {
        QVERIFY(image.save(screenshotRoot + QStringLiteral("/example-pointset.png")));
    }

    QScopedPointer<QMenu> treeMenu(window.dataTreeView()->createMenu(index));
    QAction* deleteAction = treeMenu->findChild<QAction*>(QStringLiteral("deletePointSetAction"));
    QVERIFY(deleteAction != nullptr);
    deleteAction->trigger();
    QVERIFY(project.object(id) == nullptr);
    QVERIFY(findRenderObject(firstView, id) == nullptr);
    QVERIFY(findRenderObject(&secondView, id) == nullptr);
    QVERIFY(!window.dataTreeModel()->indexFromDataId(id).isValid());
    manager->undo();
    QVERIFY(project.object(id) != nullptr);
    QVERIFY(findRenderObject(firstView, id) != nullptr);
    manager->undo();
    QVERIFY(project.object(id) == nullptr);
    plugins->unloadCppPlugin(path);
    QVERIFY(plugins->isLoaded(QStringLiteral("idosexampleplugin")));
    QVERIFY(!plugins->pluginError(QStringLiteral("idosexampleplugin")).isEmpty());
    manager->redo();
    QVERIFY(project.object(id) != nullptr);

    window.setProject(&otherProject);
    deleteAction->trigger();
    QVERIFY(project.object(id) != nullptr);
    QVERIFY(otherProject.objects().isEmpty());
    plugins->setPluginEnabled(path, false);
    QVERIFY(plugins->isLoaded(QStringLiteral("idosexampleplugin")));
    manager->clear();
    QVERIFY(project.remove(id));
    plugins->unloadCppPlugin(path);
    QVERIFY(plugins->isLoaded(QStringLiteral("idosexampleplugin")));
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    plugins->unloadCppPlugin(path);
    QVERIFY(!plugins->isLoaded(QStringLiteral("idosexampleplugin")));
    QVERIFY(window.dataRegistry()->metadata(typeId) == nullptr);
    QVERIFY(treeMenu->findChild<QAction*>(QStringLiteral("deletePointSetAction")) == nullptr);
    QVERIFY(window.treeRegistry()->metadata(typeId) == nullptr);
    QVERIFY(window.renderRegistry()->metadata(typeId) == nullptr);
    QVERIFY(window.commandRegistry()->find(QStringLiteral("pointset.create")) == nullptr);
    plugins->loadCppPlugin(path);
    QVERIFY(plugins->isLoaded(QStringLiteral("idosexampleplugin")));
    QScopedPointer<IDOSDataObject> detached(window.dataRegistry()->metadata(typeId)->create());
    plugins->unloadCppPlugin(path);
    QVERIFY(plugins->isLoaded(QStringLiteral("idosexampleplugin")));
    detached.reset();
    plugins->unloadCppPlugin(path);
    QVERIFY(!plugins->isLoaded(QStringLiteral("idosexampleplugin")));
    if (hadSetting)
    {
        settings.setValue(settingKey, previousSetting);
    }
    else
    {
        settings.remove(settingKey);
    }
}

void CaseWorkflowTest::onViewDecorations()
{
    IDOSMainWindow window;
    IDOSRenderView* view = window.findChild<IDOSRenderView*>();
    QVERIFY(view != nullptr);
    QAction* orientationAction = view->findChild<QAction*>(QStringLiteral("viewToolAxesAction"));
    QAction* legendAction = view->findChild<QAction*>(QStringLiteral("viewToolLegendAction"));
    QVERIFY(orientationAction != nullptr);
    QVERIFY(legendAction != nullptr);
    QVERIFY(view != nullptr);
    QVERIFY(orientationAction->isCheckable());
    QVERIFY(orientationAction->isChecked());
    QVERIFY(orientationAction->isEnabled());
    QVERIFY(legendAction->isCheckable());
    QVERIFY(!legendAction->isEnabled());
    orientationAction->trigger();
    QVERIFY(!view->axesVisible());
    view->setAxesVisible(true);
    QVERIFY(orientationAction->isChecked());

    IDOSGrid grid;
    grid.setDimensions(1, 1, 1);
    IDOSGridProperty property;
    property.setDimensions(1, 1, 1);
    property.setName(QStringLiteral("Porosity"));
    property.setValues(QVector<double>{0.2});
    IDOSGridRenderProvider* provider = new IDOSGridRenderProvider(&grid);
    IDOSRenderObject* object = new IDOSRenderObject(provider);

    object->setVisible(true);
    provider->setProperty(&property);
    view->scene()->clear();
    object->update();
    QVERIFY(view->scene()->add(object));
    QVERIFY(view->legendAvailable());
    QVERIFY(legendAction->isEnabled());
    QVERIFY(legendAction->isChecked());
    legendAction->trigger();
    QVERIFY(!view->legendVisible());
    QCOMPARE(provider->legends().first()->GetVisibility(), 1);
    view->scene()->notifyChanged(object->id());
    QVERIFY(!view->legendVisible());
    QVERIFY(!legendAction->isChecked());
    provider->setProperty(nullptr);
    view->scene()->notifyChanged(object->id());
    QVERIFY(!view->legendAvailable());
    QVERIFY(!legendAction->isEnabled());
    property.setName(QStringLiteral("Permeability"));
    property.setValues(QVector<double>{12.0});
    provider->setProperty(&property);
    view->scene()->notifyChanged(object->id());
    QVERIFY(legendAction->isEnabled());
    QVERIFY(!legendAction->isChecked());
    legendAction->trigger();
    QVERIFY(view->legendVisible());
    view->scene()->clear();
    QVERIFY(!view->legendAvailable());
    QVERIFY(!legendAction->isEnabled());
}

void CaseWorkflowTest::onRequiredSimulationPlugin()
{
    IDOSPluginRegistry* registry = IDOSPluginRegistry::instance();
    QSettings settings(QStringLiteral("pluginRegistry"), QStringLiteral("idos"));
    const QString settingKey = QStringLiteral("/plugins/idossimulationplugin");
    const bool hadSetting = settings.contains(settingKey);
    const QVariant previousSetting = settings.value(settingKey);
    settings.setValue(settingKey, false);
    {
        IDOSMainWindow window;
        QVERIFY(registry->isLoaded(QStringLiteral("idossimulationplugin")));
        SARibbonCategory* home = window.ribbonBar()->categoryByObjectName(QStringLiteral("idosHomeCategory"));
        QVERIFY(home != nullptr);
        QVERIFY(window.ribbonBar()->categoryByObjectName(QStringLiteral("idosSimulationCategory")) == nullptr);
        SARibbonPanel* simulation = home->panelByObjectName(QStringLiteral("idosHomePanel_simulation"));
        SARibbonPanel* results = home->panelByObjectName(QStringLiteral("idosHomePanel_results"));
        SARibbonPanel* output = home->panelByObjectName(QStringLiteral("idosHomePanel_output"));
        QVERIFY(simulation != nullptr);
        QVERIFY(results != nullptr);
        QVERIFY(output != nullptr);
        QVERIFY(simulation->actions().contains(window.findChild<QAction*>(QStringLiteral("simulationRunPreflightAction"))));
        QVERIFY(results->actions().contains(window.findChild<QAction*>(QStringLiteral("simulationCurveComparisonAction"))));
        QVERIFY(output->actions().contains(window.findChild<QAction*>(QStringLiteral("simulationCaseInventoryReportAction"))));
        QString path;
        const QVariantList catalog = registry->pluginCatalog(registry->libraryDir().path());
        for (const QVariant& entry : catalog)
        {
            const QVariantMap plugin = entry.toMap();
            if (plugin.value(QStringLiteral("key")).toString() == QStringLiteral("idossimulationplugin"))
            {
                QVERIFY(plugin.value(QStringLiteral("required")).toBool());
                QVERIFY(plugin.value(QStringLiteral("enabled")).toBool());
                path = plugin.value(QStringLiteral("path")).toString();
            }
        }
        QVERIFY(!path.isEmpty());
        registry->setPluginEnabled(path, false);
        QVERIFY(registry->isLoaded(QStringLiteral("idossimulationplugin")));
        registry->unloadCppPlugin(path);
        QVERIFY(registry->isLoaded(QStringLiteral("idossimulationplugin")));
    }
    QVERIFY(!registry->isLoaded(QStringLiteral("idossimulationplugin")));
    if (hadSetting)
    {
        settings.setValue(settingKey, previousSetting);
    }
    else
    {
        settings.remove(settingKey);
    }
}

void CaseWorkflowTest::onWellInputMenus()
{
    IDOSProject project;
    IDOSMainWindow window;
    window.setProject(&project);
    IDOSWell* well = new IDOSWell();
    well->setName(QStringLiteral("Input Well"));
    IDOSWellMarkerSet markers;
    markers.addMarker(IDOSWellMarker(QStringLiteral("Top Sand"), 100.0));
    well->setMarkers(markers);
    well->setCompletions({IDOSWellCompletion(0, true, 150.0, 110.0)});
    IDOSWellCheckshot checkshot;
    QVERIFY(checkshot.setSamples(QVector<double>{0.0, 100.0}, QVector<double>{0.0, 80.0}));
    checkshot.setUnits(QStringLiteral("M"), QStringLiteral("MS"));
    checkshot.setTimeType(QStringLiteral("TWT"));
    well->setCheckshot(checkshot);
    project.add(well);
    IDOSTreeModel* model = window.dataTreeModel();
    const QModelIndex root = model->indexFromDataId(well->objectId());
    QVERIFY(root.isValid());
    QCOMPARE(model->rowCount(root), 3);
    for (int row = 0; row < model->rowCount(root); ++row)
    {
        QScopedPointer<QMenu> menu(window.dataTreeView()->createMenu(model->index(row, 0, root)));
        QAction* viewData = menu->findChild<QAction*>(QStringLiteral("viewWellInputDataAction"));
        QVERIFY(viewData != nullptr);
        viewData->trigger();
        QDialog* dialog = window.dataTreeView()->findChild<QDialog*>(QStringLiteral("wellInputDataWindow"));
        QVERIFY(dialog != nullptr);
        QTableWidget* table = dialog->findChild<QTableWidget*>();
        QVERIFY(table != nullptr);
        QCOMPARE(table->rowCount(), row == 2 ? 2 : 1);
        QVERIFY(!table->item(0, 0)->text().isEmpty());
        if (row == 0)
        {
            QCOMPARE(table->item(0, 2)->text(), QStringLiteral("Not mapped"));
        }
        delete dialog;
    }
    window.setProject(nullptr);
}

void CaseWorkflowTest::onGridColoring()
{
    IDOSProject project;
    IDOSMainWindow window;
    window.setProject(&project);
    IDOSGrid* grid = new IDOSGrid();
    grid->setDimensions(1, 1, 1);
    grid->setName(QStringLiteral("Coloring Grid"));
    project.add(grid);
    IDOSGridProperty* property = new IDOSGridProperty();
    property->setDimensions(1, 1, 1);
    property->setGridId(grid->objectId());
    property->setName(QStringLiteral("Porosity"));
    QVERIFY(property->setValues(QVector<double>{0.25}));
    project.add(property);
    IDOSSimulationCaseObject* simulationCase = new IDOSSimulationCaseObject();
    simulationCase->setName(QStringLiteral("Coloring Case"));
    simulationCase->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), grid->objectId()));
    project.add(simulationCase);
    IDOSTreeView* tree = window.caseTreeView();
    IDOSRenderView* firstView = qobject_cast<IDOSRenderView*>(tree->targetView());
    QVERIFY(firstView != nullptr);
    IDOSRenderObject* firstObject = findRenderObject(firstView, grid->objectId());
    QVERIFY(firstObject != nullptr);
    IDOSGridRenderProvider* firstProvider = dynamic_cast<IDOSGridRenderProvider*>(firstObject->provider());
    QVERIFY(firstProvider != nullptr);
    IDOSRenderView secondView;
    IDOSGridRenderProvider* secondProvider = new IDOSGridRenderProvider(grid);
    IDOSRenderObject* secondObject = new IDOSRenderObject(secondProvider);
    QVERIFY(secondView.scene()->add(secondObject));
    secondProvider->setProperty(property);
    const QVariantMap originalSecondSettings = secondProvider->coloringSettings();
    const QModelIndex index = window.caseTreeModel()->indexFromDataId(property->objectId());
    QVERIFY(index.isValid());
    QScopedPointer<QMenu> menu(tree->createMenu(index));
    QAction* settings = menu->findChild<QAction*>(QStringLiteral("simulationColoringSettingsAction"));
    QVERIFY(settings != nullptr);
    QVERIFY(settings->isEnabled());
    settings->trigger();
    QPointer<QWidget> widget(window.findChild<QWidget*>(QStringLiteral("coloringSettingsWindow")));
    QVERIFY(!widget.isNull());
    QCOMPARE(firstProvider->property(), property);
    tree->setTargetView(&secondView);
    QComboBox* ramp = widget->findChild<QComboBox*>(QStringLiteral("coloringRampCombo"));
    QVERIFY(ramp != nullptr);
    QLabel* gridName = widget->findChild<QLabel*>(QStringLiteral("coloringGridNameLabel"));
    QLabel* viewName = widget->findChild<QLabel*>(QStringLiteral("coloringViewNameLabel"));
    QVERIFY(gridName != nullptr && viewName != nullptr);
    QApplication::processEvents();
    QCOMPARE(gridName->geometry().left(), viewName->geometry().left());
    QCOMPARE(viewName->text(), firstView->windowTitle());
    QVERIFY(viewName->text() != firstView->property("viewId").toString());
    firstView->setWindowTitle(QStringLiteral("Renamed Target View"));
    QCOMPARE(viewName->text(), QStringLiteral("Renamed Target View"));
    QCOMPARE(ramp->count(), 22);
    for (int rampIndex = 0; rampIndex < ramp->count(); ++rampIndex)
    {
        ramp->setCurrentIndex(rampIndex);
        const QString key = ramp->currentData().toString();
        const QList<QColor> colors = IDOSGridRenderProvider::colorRampColors(key, false);
        QCOMPARE(colors.size(), 256);
        vtkLookupTable* lookup = vtkLookupTable::SafeDownCast(firstProvider->actors().first()->GetMapper()->GetLookupTable());
        QVERIFY(lookup != nullptr);
        for (const int sample : QList<int>{0, 127, 255})
        {
            double rgba[4];
            lookup->GetTableValue(sample, rgba);
            QVERIFY(std::abs(rgba[0] - colors.at(sample).redF()) < 0.005);
            QVERIFY(std::abs(rgba[1] - colors.at(sample).greenF()) < 0.005);
            QVERIFY(std::abs(rgba[2] - colors.at(sample).blueF()) < 0.005);
        }
        const QImage icon = ramp->itemIcon(rampIndex).pixmap(QSize(256, 16)).toImage();
        QVERIFY(!icon.isNull());
        QCOMPARE(icon.pixelColor(0, 0).rgb(), colors.first().rgb());
        QCOMPARE(icon.pixelColor(icon.width() - 1, 0).rgb(), colors.last().rgb());
    }
    const QList<QColor> ocean = IDOSGridRenderProvider::colorRampColors(QStringLiteral("OceanBreeze"), false);
    QVERIFY(std::abs(ocean.first().redF() - 0.15) < 0.0001);
    QVERIFY(std::abs(ocean.last().blueF() - 0.90) < 0.0001);
    QCheckBox* reverse = widget->findChild<QCheckBox*>(QStringLiteral("coloringReverseCheck"));
    QVERIFY(reverse != nullptr);
    ramp->setCurrentIndex(ramp->findData(QStringLiteral("OceanBreeze")));
    reverse->setChecked(true);
    vtkLookupTable* reversedLookup = vtkLookupTable::SafeDownCast(firstProvider->actors().first()->GetMapper()->GetLookupTable());
    QVERIFY(reversedLookup != nullptr);
    double reversedColor[4];
    reversedLookup->GetTableValue(0, reversedColor);
    QVERIFY(std::abs(reversedColor[0] - ocean.last().redF()) < 0.005);
    const QImage reversedIcon = ramp->currentData(Qt::DecorationRole).value<QIcon>().pixmap(QSize(256, 16)).toImage();
    QCOMPARE(reversedIcon.pixelColor(0, 0).rgb(), ocean.last().rgb());
    reverse->setChecked(false);
    ramp->showPopup();
    QApplication::processEvents();
    QVERIFY(ramp->view()->window()->grab().save(QDir(qEnvironmentVariable("IDOS_TEST_SCREENSHOT_DIR", QDir::tempPath()))
                                              .filePath(QStringLiteral("coloring-ramps.png"))));
    ramp->hidePopup();
    ramp->setCurrentIndex(ramp->findData(QStringLiteral("gray")));
    QCOMPARE(firstProvider->coloringSettings().value(QStringLiteral("ramp")).toString(), QStringLiteral("gray"));
    QCOMPARE(secondProvider->coloringSettings(), originalSecondSettings);
    QCheckBox* automatic = widget->findChild<QCheckBox*>(QStringLiteral("coloringAutomaticCheck"));
    QDoubleSpinBox* minimum = widget->findChild<QDoubleSpinBox*>(QStringLiteral("coloringMinimumSpin"));
    QDoubleSpinBox* maximum = widget->findChild<QDoubleSpinBox*>(QStringLiteral("coloringMaximumSpin"));
    QVERIFY(automatic != nullptr && minimum != nullptr && maximum != nullptr);
    automatic->setChecked(false);
    minimum->setValue(0.0);
    maximum->setValue(1.0);
    QCOMPARE(firstProvider->coloringSettings().value(QStringLiteral("minimum")).toDouble(), 0.0);
    QCOMPARE(firstProvider->coloringSettings().value(QStringLiteral("maximum")).toDouble(), 1.0);
    QCOMPARE(firstProvider->actors().first()->GetMapper()->GetScalarRange()[0], 0.0);
    QCOMPARE(firstProvider->actors().first()->GetMapper()->GetScalarRange()[1], 1.0);
    QApplication::processEvents();
    const QString screenshotDirectory = qEnvironmentVariable("IDOS_TEST_SCREENSHOT_DIR", QDir::tempPath());
    QVERIFY(widget->grab().save(QDir(screenshotDirectory).filePath(QStringLiteral("coloring-settings.png"))));
    minimum->setValue(2.0);
    QCOMPARE(firstProvider->coloringSettings().value(QStringLiteral("minimum")).toDouble(), 0.0);
    minimum->setValue(0.0);
    QCheckBox* legend = widget->findChild<QCheckBox*>(QStringLiteral("coloringLegendCheck"));
    QVERIFY(legend != nullptr);
    legend->setChecked(false);
    QVERIFY(firstProvider->legends().isEmpty());
    QVERIFY(property->setValues(QVector<double>{std::numeric_limits<double>::quiet_NaN()}));
    QVERIFY(std::isnan(firstProvider->actors().first()->GetMapper()->GetInput()->GetCellData()->GetScalars()->GetTuple1(0)));
    QVERIFY(project.remove(property->objectId()));
    QVERIFY(firstProvider->property() == nullptr);
    QComboBox* properties = widget->findChild<QComboBox*>(QStringLiteral("coloringPropertyCombo"));
    QVERIFY(properties != nullptr);
    QCOMPARE(properties->currentData().toString(), QString());
    QVERIFY(project.remove(grid->objectId()));
    QTRY_VERIFY(widget.isNull());
    tree->setTargetView(firstView);
    window.setProject(nullptr);
}

void CaseWorkflowTest::onRenderLifecycle()
{
    IDOSProject project;
    IDOSRenderRegistry registry;
    IDOSRenderServer server(&registry);
    QScopedPointer<IDOSRenderView> firstView(new IDOSRenderView());
    server.addView(firstView.data());
    registry.add(new IDOSGridRenderMetadata());
    server.setProject(&project);
    IDOSGrid* grid = new IDOSGrid();
    grid->setDimensions(1, 1, 1);
    grid->setVisible(true);
    project.add(grid);
    IDOSRenderObject* object = findRenderObject(firstView.data(), grid->objectId());
    QVERIFY(object != nullptr);
    object->setVisible(true);
    QVERIFY(object->id() != grid->objectId());
    const QString renderId = object->id();
    IDOSGridRenderProvider* provider = dynamic_cast<IDOSGridRenderProvider*>(object->provider());
    QVERIFY(provider != nullptr);
    QCOMPARE(provider->dataSource(), static_cast<IDOSDataObject*>(grid));
    IDOSGridProperty* property = new IDOSGridProperty();
    property->setDimensions(1, 1, 1);
    property->setGridId(grid->objectId());
    property->setName(QStringLiteral("Porosity"));
    property->setValues(QVector<double>{0.25});
    property->setVisible(true);
    project.add(property);
    provider->setProperty(property);
    provider->update();
    QCOMPARE(provider->actors().size(), 1);
    QCOMPARE(provider->legends().size(), 1);
    vtkDataArray* scalars = provider->actors().first()->GetMapper()->GetInput()->GetCellData()->GetScalars();
    QVERIFY(scalars != nullptr);
    QCOMPARE(scalars->GetTuple1(0), 0.25);
    vtkActor* gridActor = provider->actors().first();
    vtkDataSet* gridGeometry = gridActor->GetMapper()->GetInput();
    object->setHighlighted(true);
    QVERIFY(object->highlighted());
    QVERIFY(provider->highlighted());
    QVERIFY(gridActor->GetProperty()->GetEdgeVisibility() != 0);
    object->setOpacity(0.3);
    QCOMPARE(gridActor->GetProperty()->GetOpacity(), 0.3);
    object->setVisible(false);
    QCOMPARE(gridActor->GetVisibility(), 0);
    QVERIFY(provider->legends().isEmpty());
    object->setVisible(true);
    QCOMPARE(gridActor->GetVisibility(), 1);
    QCOMPARE(provider->actors().first(), gridActor);
    QCOMPARE(gridActor->GetMapper()->GetInput(), gridGeometry);
    QScopedPointer<IDOSRenderView> secondViewWidget(new IDOSRenderView());
    IDOSRenderView* secondView = secondViewWidget.data();
    server.addView(secondView);
    QVERIFY(secondView != nullptr);
    IDOSRenderObject* secondGrid = secondView->scene()->objects().first();
    QVERIFY(secondGrid->provider() != provider);
    firstView->setSelection(QList<QString>{object->id()});
    object->setOpacity(0.4);
    provider->setDisplayMode(IDOSDisplayMode::Wireframe);
    QCOMPARE(secondGrid->opacity(), 1.0);
    QVERIFY(!secondGrid->highlighted());
    QCOMPARE(dynamic_cast<IDOSGridRenderProvider*>(secondGrid->provider())->displayMode(), IDOSDisplayMode::Surface);
    grid->setVisible(false);
    QVERIFY(provider->visible());
    grid->setVisible(true);
    QCOMPARE(provider->actors().first(), gridActor);
    QCOMPARE(findRenderObject(firstView.data(), grid->objectId())->id(), renderId);
    QCOMPARE(provider->opacity(), 0.4);
    QCOMPARE(provider->displayMode(), IDOSDisplayMode::Wireframe);
    IDOSGridRenderProvider* secondProvider = dynamic_cast<IDOSGridRenderProvider*>(secondGrid->provider());
    QVERIFY(secondProvider != nullptr);
    secondGrid->setVisible(true);
    secondProvider->setProperty(property);
    property->setValues(QVector<double>{0.5});
    QCOMPARE(provider->actors().first()->GetMapper()->GetInput()->GetCellData()->GetScalars()->GetTuple1(0), 0.5);
    QCOMPARE(secondProvider->actors().first()->GetMapper()->GetInput()->GetCellData()->GetScalars()->GetTuple1(0), 0.5);
    QCOMPARE(provider->opacity(), 0.4);
    QCOMPARE(provider->displayMode(), IDOSDisplayMode::Wireframe);
    QVERIFY(provider->highlighted());
    QCOMPARE(secondProvider->opacity(), 1.0);
    QVERIFY(!secondProvider->highlighted());
    project.beginUpdate();
    property->setValues(QVector<double>{0.75});
    QCOMPARE(provider->actors().first()->GetMapper()->GetInput()->GetCellData()->GetScalars()->GetTuple1(0), 0.5);
    project.endUpdate();
    QCOMPARE(provider->actors().first()->GetMapper()->GetInput()->GetCellData()->GetScalars()->GetTuple1(0), 0.75);
    project.beginUpdate();
    IDOSDataObject* removedProperty = project.take(property->objectId());
    QVERIFY(provider->property() == nullptr);
    QVERIFY(secondProvider->property() == nullptr);
    QVERIFY(provider->legends().isEmpty());
    QVERIFY(secondProvider->legends().isEmpty());
    delete removedProperty;
    project.endUpdate();
    QVERIFY(provider->legends().isEmpty());
    project.beginUpdate();
    const QString gridId = grid->objectId();
    IDOSDataObject* removedGrid = project.take(gridId);
    QVERIFY(findRenderObject(firstView.data(), gridId) == nullptr);
    delete removedGrid;
    project.endUpdate();
    QVERIFY(firstView->scene()->objects().isEmpty());
    QVERIFY(firstView->selection().isEmpty());

    registry.add(new IDOSWellRenderMetadata());
    IDOSWell* well = new IDOSWell();
    IDOSWellHead head;
    head.setSurfaceX(10.0);
    head.setSurfaceY(20.0);
    well->setWellHead(head);
    IDOSWellPath path;
    path.setPoints(QVector<IDOSWellPathPoint>{IDOSWellPathPoint(0.0, 10.0, 20.0, 0.0, 0.0),
                                            IDOSWellPathPoint(100.0, 10.0, 20.0, -100.0, 100.0)});
    well->setPath(path);
    well->setVisible(true);
    project.add(well);
    IDOSRenderObject* wellObject = findRenderObject(firstView.data(), well->objectId());
    QVERIFY(wellObject != nullptr);
    IDOSWellRenderProvider* wellProvider = dynamic_cast<IDOSWellRenderProvider*>(wellObject->provider());
    QVERIFY(wellProvider != nullptr);
    wellProvider->update();
    QCOMPARE(wellProvider->actors().size(), 2);
    vtkActor* headActor = wellProvider->actors().first();
    vtkActor* pathActor = wellProvider->actors().last();
    wellObject->setHighlighted(true);
    QCOMPARE(headActor->GetProperty()->GetPointSize(), 18.0f);
    QCOMPARE(pathActor->GetProperty()->GetLineWidth(), 5.0f);
    wellObject->setOpacity(0.5);
    QCOMPARE(headActor->GetProperty()->GetOpacity(), 0.5);
    QCOMPARE(pathActor->GetProperty()->GetOpacity(), 0.5);
    wellObject->setVisible(false);
    QCOMPARE(headActor->GetVisibility(), 0);
    QCOMPARE(pathActor->GetVisibility(), 0);
    wellObject->setHighlighted(false);
    wellObject->setVisible(true);
    QCOMPARE(headActor->GetProperty()->GetPointSize(), 12.0f);
    QCOMPARE(pathActor->GetProperty()->GetLineWidth(), 2.5f);
    QCOMPARE(wellProvider->actors().first(), headActor);
    QCOMPARE(wellProvider->dataSource(), static_cast<IDOSDataObject*>(well));
    QVERIFY(registry.remove(QStringLiteral("idos.well")));
    QVERIFY(firstView->scene()->objects().isEmpty());
}

void CaseWorkflowTest::onRenderArchitecture()
{
    IDOSProject project;
    IDOSRenderRegistry registry;
    IDOSRenderServer server(&registry);
    IDOSRenderView first;
    IDOSRenderView second;
    QCOMPARE(first.scene()->parent(), static_cast<QObject*>(&first));
    QVERIFY(first.scene() != second.scene());
    server.setProject(&project);
    server.addView(&first);
    server.addView(&second);
    server.addView(&first);
    QCOMPARE(server.views().size(), 2);

    IDOSGrid* grid = new IDOSGrid();
    grid->setDimensions(1, 1, 1);
    grid->setVisible(true);
    project.add(grid);
    QVERIFY(first.scene()->objects().isEmpty());
    QVERIFY(registry.add(new IDOSGridRenderMetadata()));
    const QString dataId = grid->objectId();
    const QString firstId = server.objectIds(&first, dataId).first();
    const QString secondId = server.objectIds(&second, dataId).first();
    QVERIFY(firstId != secondId);
    IDOSRenderObject* firstObject = first.scene()->object(firstId);
    IDOSRenderObject* secondObject = second.scene()->object(secondId);
    secondObject->setVisible(true);
    QVERIFY(firstObject != secondObject);
    QVERIFY(firstObject->provider() != secondObject->provider());
    QVERIFY(firstObject->provider()->actors().first() != secondObject->provider()->actors().first());
    QCOMPARE(server.dataSource(&first, firstId), static_cast<IDOSDataObject*>(grid));
    QVERIFY(server.dataSource(&second, firstId) == nullptr);

    QSignalSpy selectionSpy(&first, &IDOSRenderView::selectionChanged);
    first.setSelection(QList<QString>{firstId});
    QCOMPARE(selectionSpy.count(), 1);
    QVERIFY(firstObject->highlighted());
    QVERIFY(!secondObject->highlighted());
    QWidget* vtkWidget = nullptr;
    const QList<QWidget*> childWidgets = first.findChildren<QWidget*>();
    for (QWidget* child : childWidgets)
    {
        if (child->inherits("QVTKOpenGLNativeWidget"))
        {
            vtkWidget = child;
            break;
        }
    }
    QVERIFY(vtkWidget != nullptr);
    QTimer popupGuard;
    popupGuard.setSingleShot(true);
    connect(&popupGuard, &QTimer::timeout, this, &CaseWorkflowTest::onCloseContextMenu);
    popupGuard.start(100);
    QContextMenuEvent mouseContext(QContextMenuEvent::Mouse, QPoint(1, 1), vtkWidget->mapToGlobal(QPoint(1, 1)));
    QCoreApplication::sendEvent(vtkWidget, &mouseContext);
    popupGuard.stop();
    QVERIFY(mouseContext.isAccepted());
    popupGuard.start(100);
    QContextMenuEvent keyboardContext(QContextMenuEvent::Keyboard, QPoint(1, 1), vtkWidget->mapToGlobal(QPoint(1, 1)));
    QCoreApplication::sendEvent(vtkWidget, &keyboardContext);
    popupGuard.stop();
    QVERIFY(keyboardContext.isAccepted());
    QVERIFY(first.findChildren<QMenu*>().isEmpty());
    QCOMPARE(first.selection(), QList<QString>{firstId});
    firstObject->setOpacity(0.25);
    firstObject->setVisible(false);
    first.scene()->notifyChanged(firstId);
    QCOMPARE(secondObject->opacity(), 1.0);
    QVERIFY(secondObject->visible());
    grid->setName(QStringLiteral("Renamed Grid"));
    QCOMPARE(firstObject->name(), QStringLiteral("Renamed Grid"));
    QCOMPARE(secondObject->name(), QStringLiteral("Renamed Grid"));
    QVERIFY(!firstObject->visible());
    QCOMPARE(firstObject->opacity(), 0.25);
    QVERIFY(firstObject->highlighted());

    // Scene batches redraw notifications while lifecycle signals remain immediate.
    QSignalSpy sceneSpy(first.scene(), &IDOSRenderScene::changed);
    QSignalSpy removalSpy(first.scene(), &IDOSRenderScene::objectAboutToBeRemoved);
    first.scene()->beginUpdate();
    first.scene()->beginUpdate();
    first.scene()->notifyChanged(firstId);
    first.scene()->notifyChanged(firstId);
    QCOMPARE(sceneSpy.count(), 0);
    first.scene()->endUpdate();
    QCOMPARE(sceneSpy.count(), 0);
    first.scene()->endUpdate();
    QCOMPARE(sceneSpy.count(), 1);
    QVERIFY(!first.scene()->add(firstObject));
    QVERIFY(!first.scene()->remove(QStringLiteral("missing")));

    project.beginUpdate();
    IDOSDataObject* detached = project.take(dataId);
    QCOMPARE(removalSpy.count(), 1);
    QVERIFY(first.scene()->objects().isEmpty());
    QVERIFY(second.scene()->objects().isEmpty());
    QVERIFY(first.selection().isEmpty());
    QVERIFY(server.objectIds(&first, dataId).isEmpty());
    delete detached;
    project.endUpdate();

    IDOSGrid* replacement = new IDOSGrid();
    replacement->setDimensions(1, 1, 1);
    project.add(replacement);
    QVERIFY(!first.scene()->objects().isEmpty());
    QVERIFY(registry.remove(QStringLiteral("idos.grid")));
    QVERIFY(first.scene()->objects().isEmpty());
    QVERIFY(second.scene()->objects().isEmpty());
    QVERIFY(registry.add(new IDOSGridRenderMetadata()));
    server.removeView(&first);
    QVERIFY(first.scene()->objects().isEmpty());
    QVERIFY(!second.scene()->objects().isEmpty());
    QCOMPARE(server.views().size(), 1);

    IDOSRenderView* temporaryView = new IDOSRenderView();
    server.addView(temporaryView);
    delete temporaryView;
    QCOMPARE(server.views().size(), 1);
    IDOSProject* temporaryProject = new IDOSProject();
    IDOSGrid* temporaryGrid = new IDOSGrid();
    temporaryGrid->setDimensions(1, 1, 1);
    temporaryProject->add(temporaryGrid);
    server.setProject(temporaryProject);
    QVERIFY(!second.scene()->objects().isEmpty());
    delete temporaryProject;
    QVERIFY(server.project() == nullptr);
    QVERIFY(second.scene()->objects().isEmpty());
}

void CaseWorkflowTest::onCloseContextMenu()
{
    QMenu* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
    if (menu != nullptr)
    {
        menu->close();
    }
}

void CaseWorkflowTest::onProvider()
{
    std::unique_ptr<IDOSDataProvider> reader =
        IDOSDataProviderRegistry::instance().create(QStringLiteral("idos.case.simulation.eclipse"));
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
    project.add(grid);
    IDOSWell* well = new IDOSWell();
    well->setName(QStringLiteral("a10"));
    project.add(well);
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
        IDOSGridRenderMetadata metadata;
        QScopedPointer<IDOSRenderObject> renderObject(new IDOSRenderObject(metadata.create(grid)));
        QVERIFY(renderObject != nullptr);
        IDOSRenderProvider* provider = renderObject->provider();
        provider->setVisible(true);
        provider->update();
        QCOMPARE(provider->actors().size(), 1);
        vtkDataSet* renderedGrid = provider->actors().first()->GetMapper()->GetInput();
        QCOMPARE(renderedGrid->GetNumberOfCells(), static_cast<vtkIdType>(reference.getNumActive()));
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
                            double renderedPoint[3];
                            renderedGrid->GetPoint(renderedCell * 8 + corner, renderedPoint);
                            QCOMPARE(QVector3D(renderedPoint[0], renderedPoint[1], renderedPoint[2]), expectedPoint);
                        }
                    }
                    if (grid->isActive(i, j, k))
                    {
                        ++renderedCell;
                    }
                }
            }
        }
        qInfo("Compared all 1050000 grid corners and all active mesh corners against OPM getCornerPos.");
        IDOSRenderView renderView;
        renderView.resize(1200, 700);
        renderView.show();
        renderObject->update();
        QVERIFY(renderView.scene()->add(renderObject.take()));
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
    IDOSTreeView* view = window.caseTreeView();
    IDOSTreeModel* model = window.caseTreeModel();
    QScopedPointer<QMenu> emptyMenu(view->createMenu(view->currentIndex()));
    QVERIFY(emptyMenu->actions().isEmpty());
    IDOSProject* project = new IDOSProject(&window);
    window.setProject(project);
    QScopedPointer<QMenu> menu(view->createMenu(view->currentIndex()));
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
    menu->findChild<QAction*>(QStringLiteral("newCaseAction"))->trigger();
    QCOMPARE(project->objects().size(), 1);
    m_name = QStringLiteral("   ");
    menu->findChild<QAction*>(QStringLiteral("newCaseAction"))->trigger();
    QCOMPARE(project->objects().size(), 1);
    m_expectDisabled = false;
    m_cancel = true;
    m_name = QStringLiteral("Cancelled");
    menu->findChild<QAction*>(QStringLiteral("newCaseAction"))->trigger();
    QCOMPARE(project->objects().size(), 1);
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
    menu->findChild<QAction*>(QStringLiteral("importCaseAction"))->trigger();
    QCOMPARE(project->objects().size(), 1);
    IDOSWell* well = new IDOSWell();
    well->setName(QStringLiteral("A10"));
    project->add(well);
    m_cancel = false;
    m_name = QStringLiteral("Imported Case");
    menu->findChild<QAction*>(QStringLiteral("importCaseAction"))->trigger();
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
    menu->findChild<QAction*>(QStringLiteral("importCaseAction"))->trigger();
    QCOMPARE(project->objects().size(), 3);
    timer.stop();

    IDOSGrid* grid = new IDOSGrid();
    grid->setName(QStringLiteral("Main Grid"));
    grid->setDimensions(1, 1, 1);
    project->add(grid);
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
        project->add(property);
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
    QFileDialog* fileDialog = qobject_cast<QFileDialog*>(QApplication::activeModalWidget());
    if (fileDialog != nullptr)
    {
        if (m_cancel)
        {
            fileDialog->reject();
        }
        else
        {
            fileDialog->selectFile(QFINDTESTDATA("data/case.DATA"));
            QMetaObject::invokeMethod(fileDialog, "accept", Qt::QueuedConnection);
        }
        return;
    }
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

IDOSRenderObject* CaseWorkflowTest::findRenderObject(IDOSRenderView* view, const QString& dataId) const
{
    for (IDOSRenderObject* object : view->scene()->objects())
    {
        IDOSDataObject* data = object->provider()->dataSource();
        if (data != nullptr && data->objectId() == dataId)
        {
            return object;
        }
    }
    return nullptr;
}

QTEST_MAIN(CaseWorkflowTest)

CaseWorkflowTest::CaseWorkflowTest()
{
    IDOSApplication::initProviders();
}
