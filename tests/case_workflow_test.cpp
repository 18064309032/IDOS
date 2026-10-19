#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#endif

#include <cmath>
#include <limits>
#include <memory>
#include <type_traits>
#include <vector>

#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QColor>
#include <QColorDialog>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QDateTime>
#include <QDialog>
#include <QDir>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QEvent>
#include <QEventLoop>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QImage>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPixmap>
#include <QPointF>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QScopedPointer>
#include <QScrollArea>
#include <QScrollBar>
#include <QSettings>
#include <QSignalSpy>
#include <QSize>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <QToolButton>
#include <QTranslator>
#include <QVariantMap>
#include <QVector3D>
#include <QVTKOpenGLNativeWidget.h>
#include <QWidget>
#include <QtTest>

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkCellPicker.h>
#include <vtkCommand.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkLookupTable.h>
#include <vtkMapper.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkSmartPointer.h>
#include <vtkStaticCellLocator.h>
#include <vtkStaticCleanUnstructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkWeakPointer.h>

#include <SARibbonBar.h>
#include <SARibbonCategory.h>
#include <SARibbonPanel.h>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Grid/NNC.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <opm/io/eclipse/EclOutput.hpp>
#include <opm/io/eclipse/ESmry.hpp>

#include "case/idoscasedialog.h"
#include "case/idossimulationinitializewidget.h"
#include "case/idossimulationresultplaybackwidget.h"
#include "case/idossimulationrunstatuswidget.h"
#include "command/idoscommandmanager.h"
#include "command/idoscommandregistry.h"
#include "command/idoscreategridfiltercommand.h"
#include "command/idosdeletedatacommand.h"
#include "command/idosdeletegridcommand.h"
#include "command/idosdeletegridfiltercommand.h"
#include "command/idosimportcasecommand.h"
#include "command/idosimportrestartresultscommand.h"
#include "command/idosupdategridfiltercommand.h"
#include "data/case/idoscaseitemref.h"
#include "data/case/idoscaseobject.h"
#include "data/case/idossimulationcaseobject.h"
#include "data/case/idossimulationsummaryresultsobject.h"
#include "data/fluidrock/idosfluidrocktableobject.h"
#include "data/fracture/idosnaturalfracturesetobject.h"
#include "data/grid/idosgridboundaryobject.h"
#include "data/grid/idosgridfilterobject.h"
#include "data/grid/idosgridobject.h"
#include "data/grid/idosgridpropertyobject.h"
#include "data/idosdatametadata.h"
#include "data/idosdataregistry.h"
#include "data/idosnaturalfracturerendermetadata.h"
#include "data/idosnaturalfracturerenderprovider.h"
#include "data/well/idoswellhead.h"
#include "data/well/idoswelllogchannel.h"
#include "data/well/idoswelllogset.h"
#include "data/well/idoswellobject.h"
#include "data/well/idoswellpath.h"
#include "data/well/idoswellpathpoint.h"
#include "grid/idosgridfilterwidget.h"
#include "grid/idosgridpropertyeclipseprovider.h"
#include "grid/idosgridrendermetadata.h"
#include "grid/idosgridrenderprovider.h"
#include "grid/idosgridsettingwidget.h"
#include "idosapplication.h"
#include "idosmainwindow.h"
#include "idosnewprojectdialog.h"
#include "idoswaitingdialog.h"
#include "idosproject.h"
#include "idosprojectmetadata.h"
#include "idosrenderobject.h"
#include "idosrenderprovider.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"
#include "idosruntimeinfowidget.h"
#include "logger/idoslogger.h"
#include "plugin/idospluginregistry.h"
#include "providers/idosdataprovider.h"
#include "providers/idosdataproviderregistry.h"
#include "render/idosrenderproviderregistry.h"
#include "render/idosrendertoolmetadata.h"
#include "render/idosrenderserver.h"
#include "tools/idostoolbar.h"
#include "tools/idostoolproviderregistry.h"
#include "tree/idostreeitem.h"
#include "tree/idostreeitemproviderregistry.h"
#include "tree/idostreemodel.h"
#include "tree/idostreeview.h"
#include "well/idoswellrendermetadata.h"
#include "well/idoswellrenderprovider.h"

#include "case_workflow_test.h"

static_assert(!std::is_abstract<IDOSRenderObject>::value, "RenderObject must be concrete.");
static_assert(std::is_abstract<IDOSRenderProvider>::value, "RenderProvider must remain abstract.");

QVariantMap CaseWorkflowTest::fracture(const QVector<double>& coordinates, int id) const
{
    QVariantList vertices;
    for (int index = 0; index + 2 < coordinates.size(); index += 3)
    {
        QVariantMap vertex;
        vertex.insert(QStringLiteral("east"), coordinates.at(index));
        vertex.insert(QStringLiteral("north"), coordinates.at(index + 1));
        vertex.insert(QStringLiteral("down"), coordinates.at(index + 2));
        vertices.append(vertex);
    }
    QVariantMap result;
    result.insert(QStringLiteral("id"), id);
    result.insert(QStringLiteral("setId"), 1);
    result.insert(QStringLiteral("vertices"), vertices);
    result.insert(QStringLiteral("normal"), QVariantList{0.0, 0.0, 1.0});
    result.insert(QStringLiteral("properties"), QVariantMap{{QStringLiteral("Aperture"), 0.001},
                                                          {QStringLiteral("Porosity"), 0.1}});
    return result;
}

QStringList CaseWorkflowTest::fractureIssueCodes(const QVariantMap& report) const
{
    QStringList codes;
    const QVariantList issues = report.value(QStringLiteral("issues")).toList();
    for (const QVariant& issue : issues)
    {
        codes.append(issue.toMap().value(QStringLiteral("code")).toString());
    }
    return codes;
}

void CaseWorkflowTest::onFieldDifference()
{
    QSettings settings(QStringLiteral("pluginRegistry"), QStringLiteral("idos"));
    const QString key = QStringLiteral("/plugins/idosmultiscalefractureplugin");
    const QVariant previous = settings.value(key);
    const bool existed = settings.contains(key);
    IDOSMainWindow window;
    IDOSProject* project = new IDOSProject(&window);
    window.setProject(project);
    IDOSPluginRegistry* plugins = IDOSPluginRegistry::instance();
    const QString path = QCoreApplication::applicationDirPath() + QStringLiteral("/plugins/idosmultiscalefractureplugin.dll");
    plugins->loadCppPlugin(path);
    QVERIFY(plugins->isLoaded(QStringLiteral("idosmultiscalefractureplugin")));
    IDOSGridObject* grid = new IDOSGridObject();
    QVector<double> coord;
    for (int j = 0; j <= 1; ++j)
    {
        for (int i = 0; i <= 2; ++i)
        {
            coord << i * 10.0 << j * 10.0 << 0.0 << i * 10.0 << j * 10.0 << 10.0;
        }
    }
    grid->setName(QStringLiteral("Difference grid"));
    grid->setCornerPointData(2, 1, 1, coord, {0, 0, 0, 0, 10, 10, 10, 10, 0, 0, 0, 0, 10, 10, 10, 10}, {1, 0});
    project->add(grid);
    IDOSSimulationCaseObject* simulationCase = new IDOSSimulationCaseObject();
    simulationCase->setName(QStringLiteral("Difference case"));
    simulationCase->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), grid->objectId()));
    project->add(simulationCase);
    IDOSGridPropertyObject* field = new IDOSGridPropertyObject();
    field->setName(QStringLiteral("Pressure"));
    field->setKeyword(QStringLiteral("PRESSURE"));
    field->setValueUnits(IDOSGridPropertyObject::ValueUnits::SI);
    field->setGridId(grid->objectId());
    field->setDimensions(2, 1, 1);
    field->setKind(IDOSGridPropertyObject::Kind::Dynamic);
    QVERIFY(field->setReportStepValues(1, {10.0, 20.0}));
    QVERIFY(field->setReportStepValues(2, {14.0, std::numeric_limits<double>::quiet_NaN()}));
    QVERIFY(field->setCurrentReportStep(1));
    project->add(field);
    QCOMPARE(field->reportStepValues(2).first(), 14.0);
    QCOMPARE(field->currentReportStep(), 1);
    QVERIFY(field->reportStepValues(99).isEmpty());
    QJsonObject arguments{{QStringLiteral("gridId"), grid->objectId()},
                          {QStringLiteral("beforeId"), field->objectId()},
                          {QStringLiteral("afterId"), field->objectId()},
                          {QStringLiteral("beforeStep"), 1}, {QStringLiteral("afterStep"), 2},
                          {QStringLiteral("name"), QStringLiteral("Pressure change")}};
    const QString command = QStringLiteral("analysis.field.difference");
    QJsonObject response = project->commandManager()->execute(command, arguments);
    QVERIFY2(response.value(QStringLiteral("success")).toBool(), qPrintable(project->commandManager()->error()));
    const QString outputId = project->commandManager()->result().value(QStringLiteral("objectId")).toString();
    IDOSGridPropertyObject* output = qobject_cast<IDOSGridPropertyObject*>(project->object(outputId));
    QVERIFY(output != nullptr);
    QCOMPARE(output->values().first(), 4.0);
    QVERIFY(std::isnan(output->values().last()));
    QCOMPARE(field->currentReportStep(), 1);
    QVERIFY(window.caseTreeModel()->index(outputId).isValid());
    project->commandManager()->undo();
    QVERIFY(project->object(outputId) == nullptr);
    QJsonObject invalid = arguments;
    invalid.insert(QStringLiteral("afterStep"), 99);
    QVERIFY(!project->commandManager()->execute(command, invalid).value(QStringLiteral("success")).toBool());
    QVERIFY(project->commandManager()->canRedo());
    QVERIFY(field->setReportStepValues(2, {50.0, 0.0}));
    project->commandManager()->redo();
    QCOMPARE(output->values().first(), 4.0);
    field->setValueUnits(IDOSGridPropertyObject::ValueUnits::Unknown);
    QVERIFY(!project->commandManager()->execute(command, arguments).value(QStringLiteral("success")).toBool());
    invalid = arguments;
    invalid.insert(QStringLiteral("confirmUnits"), true);
    QVERIFY(project->commandManager()->execute(command, invalid).value(QStringLiteral("success")).toBool());
    QVERIFY(field->setReportStepValues(2, {std::numeric_limits<double>::infinity(), 0.0}));
    QVERIFY(!project->commandManager()->execute(command, invalid).value(QStringLiteral("success")).toBool());
    QVERIFY(field->setReportStepValues(2, {14.0, 0.0}));
    QAction* action = window.findChild<QAction*>(QStringLiteral("idosFieldDifferenceAction"));
    QVERIFY(action != nullptr);
    action->trigger();
    QPointer<QWidget> widget = window.findChild<QWidget*>(QStringLiteral("idosFieldDifferenceWidget"));
    QVERIFY(!widget.isNull());
    QComboBox* afterStep = widget->findChild<QComboBox*>(QStringLiteral("differenceAfterStep"));
    afterStep->setCurrentIndex(afterStep->findData(2));
    widget->findChild<QCheckBox*>(QStringLiteral("differenceConfirmUnits"))->setChecked(true);
    widget->findChild<QPushButton*>(QStringLiteral("differenceCalculate"))->click();
    QVERIFY(!widget->findChild<QLabel*>(QStringLiteral("differenceResult"))->text().isEmpty());
    QCOMPARE(field->currentReportStep(), 1);
    const QString screenshots = qEnvironmentVariable("IDOS_TEST_SCREENSHOT_DIR");
    if (!screenshots.isEmpty())
    {
        QVERIFY(widget->grab().save(QDir(screenshots).filePath(QStringLiteral("field-difference.png"))));
    }
    QTemporaryDir saved;
    QString error;
    QVERIFY2(project->save(saved.filePath(QStringLiteral("difference.idos")), window.dataRegistry(), error), qPrintable(error));
    QScopedPointer<IDOSProject> restored(IDOSProject::load(saved.filePath(QStringLiteral("difference.idos")), window.dataRegistry(), error));
    QVERIFY2(!restored.isNull(), qPrintable(error));
    IDOSGridPropertyObject* loaded = qobject_cast<IDOSGridPropertyObject*>(restored->object(outputId));
    QVERIFY(loaded != nullptr);
    QCOMPARE(loaded->values().first(), 4.0);
    QVERIFY(std::isnan(loaded->values().last()));
    plugins->unloadCppPlugin(path);
    QVERIFY(plugins->isLoaded(QStringLiteral("idosmultiscalefractureplugin")));
    project->commandManager()->clear();
    plugins->unloadCppPlugin(path);
    QVERIFY(!plugins->isLoaded(QStringLiteral("idosmultiscalefractureplugin")));
    QVERIFY(widget.isNull());
    QCOMPARE(output->values().first(), 4.0);
    if (existed)
    {
        settings.setValue(key, previous);
    }
    else
    {
        settings.remove(key);
    }
}

void CaseWorkflowTest::onCurveEvaluation()
{
    QSettings settings(QStringLiteral("pluginRegistry"), QStringLiteral("idos"));
    const QString key = QStringLiteral("/plugins/idosmultiscalefractureplugin");
    const QVariant previous = settings.value(key);
    const bool existed = settings.contains(key);
    IDOSMainWindow window;
    IDOSProject* project = new IDOSProject(&window);
    window.setProject(project);
    IDOSPluginRegistry* plugins = IDOSPluginRegistry::instance();
    const QString path = QCoreApplication::applicationDirPath() + QStringLiteral("/plugins/idosmultiscalefractureplugin.dll");
    plugins->loadCppPlugin(path);
    QVERIFY(plugins->isLoaded(QStringLiteral("idosmultiscalefractureplugin")));
    QTemporaryDir directory;
    QFile simulated(directory.filePath(QStringLiteral("simulated.csv")));
    QVERIFY(simulated.open(QIODevice::WriteOnly));
    simulated.write("days,value\n2,4\n0,0\n");
    simulated.close();
    QFile observed(directory.filePath(QStringLiteral("observed.csv")));
    QVERIFY(observed.open(QIODevice::WriteOnly));
    observed.write("days,value\n-1,99\n0,0\n1,1\n2,2\n3,99\n");
    observed.close();
    const QString command = QStringLiteral("analysis.curve.evaluate");
    QJsonObject arguments{{QStringLiteral("simulatedFile"), simulated.fileName()},
                          {QStringLiteral("observedFile"), observed.fileName()},
                          {QStringLiteral("simulatedTimeColumn"), 1}, {QStringLiteral("simulatedValueColumn"), 2},
                          {QStringLiteral("observedTimeColumn"), 1}, {QStringLiteral("observedValueColumn"), 2},
                          {QStringLiteral("timeMode"), QStringLiteral("days")},
                          {QStringLiteral("origin"), QStringLiteral("2020-01-01T00:00:00Z")},
                          {QStringLiteral("unit"), QStringLiteral("SM3/DAY")}, {QStringLiteral("confirmUnits"), true}};
    QJsonObject response = project->commandManager()->execute(command, arguments);
    QVERIFY2(response.value(QStringLiteral("success")).toBool(), qPrintable(project->commandManager()->error()));
    QJsonObject result = project->commandManager()->result();
    QCOMPARE(result.value(QStringLiteral("samples")).toInt(), 3);
    QCOMPARE(result.value(QStringLiteral("outsideOverlap")).toInt(), 2);
    QVERIFY(std::abs(result.value(QStringLiteral("rmse")).toDouble() - std::sqrt(5.0 / 3.0)) < 1e-10);
    QCOMPARE(result.value(QStringLiteral("mae")).toDouble(), 1.0);
    QCOMPARE(result.value(QStringLiteral("mape")).toDouble(), 100.0);
    QCOMPARE(result.value(QStringLiteral("mapeSamples")).toInt(), 2);
    QVERIFY(std::abs(result.value(QStringLiteral("r2")).toDouble() + 1.5) < 1e-10);
    QVERIFY(!project->commandManager()->canUndo());
    QJsonObject invalid = arguments;
    invalid.insert(QStringLiteral("confirmUnits"), false);
    QVERIFY(!project->commandManager()->execute(command, invalid).value(QStringLiteral("success")).toBool());
    QVERIFY(observed.open(QIODevice::WriteOnly | QIODevice::Truncate));
    observed.write("days,value\n10,1\n11,2\n");
    observed.close();
    QVERIFY(!project->commandManager()->execute(command, arguments).value(QStringLiteral("success")).toBool());
    QVERIFY(observed.open(QIODevice::WriteOnly | QIODevice::Truncate));
    observed.write("days,value\n1,1\n1,2\n");
    observed.close();
    QVERIFY(!project->commandManager()->execute(command, arguments).value(QStringLiteral("success")).toBool());
    QVERIFY(observed.open(QIODevice::WriteOnly | QIODevice::Truncate));
    observed.write("days,value\n0,0\n1,0\n");
    observed.close();
    QVERIFY(project->commandManager()->execute(command, arguments).value(QStringLiteral("success")).toBool());
    result = project->commandManager()->result();
    QVERIFY(result.value(QStringLiteral("r2")).isNull());
    QVERIFY(result.value(QStringLiteral("mape")).isNull());
    QVERIFY(observed.open(QIODevice::WriteOnly | QIODevice::Truncate));
    observed.write("0,0\n1,2\n");
    observed.close();
    arguments.insert(QStringLiteral("header"), false);
    QVERIFY(!project->commandManager()->execute(command, arguments).value(QStringLiteral("success")).toBool());
    QVERIFY(simulated.open(QIODevice::WriteOnly | QIODevice::Truncate));
    simulated.write("2,4\n0,0\n");
    simulated.close();
    QVERIFY(project->commandManager()->execute(command, arguments).value(QStringLiteral("success")).toBool());
    QCOMPARE(project->commandManager()->result().value(QStringLiteral("rmse")).toDouble(), 0.0);
    QVERIFY(simulated.open(QIODevice::WriteOnly | QIODevice::Truncate));
    simulated.write("days,value\n2,4\n0,0\n");
    simulated.close();
    arguments.insert(QStringLiteral("simulatedId"), QStringLiteral("missing"));
    QVERIFY(!project->commandManager()->execute(command, arguments).value(QStringLiteral("success")).toBool());
    arguments.remove(QStringLiteral("simulatedId"));
    arguments.insert(QStringLiteral("header"), true);
    QVERIFY(observed.open(QIODevice::WriteOnly | QIODevice::Truncate));
    observed.write("days,value\n0,\"unterminated\n");
    observed.close();
    QVERIFY(!project->commandManager()->execute(command, arguments).value(QStringLiteral("success")).toBool());
    QVERIFY(observed.open(QIODevice::WriteOnly | QIODevice::Truncate));
    observed.write("days,value\n0,0\n1,0\n");
    observed.close();
    IDOSSimulationSummaryResultsObject* summary = new IDOSSimulationSummaryResultsObject();
    summary->setName(QStringLiteral("Simulation"));
    const qint64 start = QDateTime::fromString(QStringLiteral("2020-01-01T00:00:00Z"), Qt::ISODate).toMSecsSinceEpoch();
    summary->setTimeValues({start, start + 2 * 86400000LL});
    summary->setSeries({QVariantMap{{QStringLiteral("key"), QStringLiteral("FOPR")},
                                   {QStringLiteral("unit"), QStringLiteral("SM3/DAY")},
                                   {QStringLiteral("values"), QVariant::fromValue(QVector<double>{0.0, 4.0})}}});
    project->add(summary);
    arguments.insert(QStringLiteral("simulatedId"), summary->objectId());
    arguments.insert(QStringLiteral("simulatedSeries"), 0);
    QVERIFY(project->commandManager()->execute(command, arguments).value(QStringLiteral("success")).toBool());
    invalid = arguments;
    invalid.insert(QStringLiteral("unit"), QStringLiteral("STB/DAY"));
    QVERIFY(!project->commandManager()->execute(command, invalid).value(QStringLiteral("success")).toBool());
    QVERIFY(observed.open(QIODevice::WriteOnly | QIODevice::Truncate));
    observed.write("date,value\n2020-01-01T00:00:00Z,0\n2020-01-02T00:00:00Z,2\n");
    observed.close();
    arguments.insert(QStringLiteral("timeMode"), QStringLiteral("calendar"));
    QVERIFY(project->commandManager()->execute(command, arguments).value(QStringLiteral("success")).toBool());
    QCOMPARE(project->commandManager()->result().value(QStringLiteral("rmse")).toDouble(), 0.0);
    QAction* action = window.findChild<QAction*>(QStringLiteral("idosCurveEvaluationAction"));
    QVERIFY(action != nullptr);
    action->trigger();
    QPointer<QWidget> widget = window.findChild<QWidget*>(QStringLiteral("idosCurveEvaluationWidget"));
    QVERIFY(!widget.isNull());
    QComboBox* source = widget->findChild<QComboBox*>(QStringLiteral("simulatedSource"));
    source->setCurrentIndex(source->findData(summary->objectId()));
    widget->findChild<QLineEdit*>(QStringLiteral("observedFile"))->setText(observed.fileName());
    QComboBox* mode = widget->findChild<QComboBox*>(QStringLiteral("curveTimeMode"));
    mode->setCurrentIndex(mode->findData(QStringLiteral("calendar")));
    widget->findChild<QCheckBox*>(QStringLiteral("curveConfirmUnits"))->setChecked(true);
    widget->findChild<QPushButton*>(QStringLiteral("curveEvaluate"))->click();
    QCOMPARE(widget->findChild<QTableWidget*>(QStringLiteral("curveSamples"))->rowCount(), 2);
    const QString screenshots = qEnvironmentVariable("IDOS_TEST_SCREENSHOT_DIR");
    if (!screenshots.isEmpty())
    {
        QVERIFY(widget->grab().save(QDir(screenshots).filePath(QStringLiteral("curve-evaluation.png"))));
    }
    m_name = directory.filePath(QStringLiteral("evaluation.csv"));
    const bool nativeDisabled = QApplication::testAttribute(Qt::AA_DontUseNativeDialogs);
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs, true);
    m_cancel = false;
    QTimer::singleShot(200, this, &CaseWorkflowTest::onExportCurveDialog);
    QVERIFY(QMetaObject::invokeMethod(widget.data(), "onExport", Qt::DirectConnection));
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs, nativeDisabled);
    QFile report(m_name);
    QVERIFY2(report.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(widget->findChild<QLabel*>(QStringLiteral("curveStatus"))->text()));
    const QByteArray csv = report.readAll();
    QVERIFY(csv.contains("metric,value\n"));
    QVERIFY(csv.contains("time_utc,simulated,observed,error"));
    QVERIFY(csv.contains("2020-01-02T00:00:00.000Z,2,2,0"));
    QVERIFY(plugins->isLoaded(QStringLiteral("idosmultiscalefractureplugin")));
    widget->findChild<QLineEdit*>(QStringLiteral("curveUnit"))->setText(QStringLiteral("Changed"));
    QCOMPARE(widget->findChild<QTableWidget*>(QStringLiteral("curveSamples"))->rowCount(), 0);
    std::unique_ptr<IDOSCommand> pending = window.commandRegistry()->create(command, arguments, project);
    plugins->unloadCppPlugin(path);
    QVERIFY(plugins->isLoaded(QStringLiteral("idosmultiscalefractureplugin")));
    pending.reset();
    QVERIFY(!project->commandManager()->canUndo());
    plugins->unloadCppPlugin(path);
    QVERIFY(!plugins->isLoaded(QStringLiteral("idosmultiscalefractureplugin")));
    QVERIFY(widget.isNull());
    if (existed)
    {
        settings.setValue(key, previous);
    }
    else
    {
        settings.remove(key);
    }
}

void CaseWorkflowTest::onExportCurveDialog()
{
    QFileDialog* dialog = qobject_cast<QFileDialog*>(QApplication::activeModalWidget());
    if (dialog == nullptr)
    {
        QTimer::singleShot(100, this, &CaseWorkflowTest::onExportCurveDialog);
        return;
    }
    const QString path = QCoreApplication::applicationDirPath() + QStringLiteral("/plugins/idosmultiscalefractureplugin.dll");
    IDOSPluginRegistry::instance()->unloadCppPlugin(path);
    QVERIFY(IDOSPluginRegistry::instance()->isLoaded(QStringLiteral("idosmultiscalefractureplugin")));
    if (!m_cancel)
    {
        m_cancel = true;
        dialog->setDirectory(QFileInfo(m_name).absolutePath());
        dialog->selectFile(QFileInfo(m_name).fileName());
        QTimer::singleShot(300, this, &CaseWorkflowTest::onExportCurveDialog);
        QTimer::singleShot(5000, dialog, &QDialog::reject);
        return;
    }
    QLineEdit* fileName = dialog->findChild<QLineEdit*>(QStringLiteral("fileNameEdit"));
    QVERIFY(fileName != nullptr);
    fileName->setText(m_name);
    QMetaObject::invokeMethod(dialog, "accept", Qt::DirectConnection);
}

void CaseWorkflowTest::onDfnGeneration()
{
    QSettings settings(QStringLiteral("pluginRegistry"), QStringLiteral("idos"));
    const QString settingKey = QStringLiteral("/plugins/idosfracturemodelingplugin");
    const bool hadSetting = settings.contains(settingKey);
    const QVariant previousSetting = settings.value(settingKey);
    IDOSMainWindow window;
    IDOSProject* project = new IDOSProject(&window);
    window.setProject(project);
    const QString pluginPath = QCoreApplication::applicationDirPath() + QStringLiteral("/plugins/idosfracturemodelingplugin.dll");
    IDOSPluginRegistry* plugins = IDOSPluginRegistry::instance();
    plugins->loadCppPlugin(pluginPath);
    QVERIFY(plugins->isLoaded(QStringLiteral("idosfracturemodelingplugin")));
    QAction* action = window.findChild<QAction*>(QStringLiteral("idosGenerateDfnAction"));
    QVERIFY(action != nullptr);
    QVERIFY(!action->isEnabled());
    QVERIFY(window.commandRegistry()->metadata(QStringLiteral("fracture.dfn.generate")) != nullptr);
    QVector<double> coord;
    for (int j = 0; j <= 1; ++j)
    {
        for (int i = 0; i <= 2; ++i)
        {
            coord << 5000000.0 + i * 10.0 << 7000000.0 + j * 10.0 << 0.0
                  << 5000000.0 + i * 10.0 << 7000000.0 + j * 10.0 << 20.0;
        }
    }
    QVector<double> zcorn;
    for (int k = 0; k < 2; ++k)
    {
        for (int cell = 0; cell < 2; ++cell)
        {
            for (int corner = 0; corner < 8; ++corner)
            {
                zcorn.append(k * 10.0 + (corner < 4 ? 0.0 : 10.0));
            }
        }
    }
    IDOSGridObject* grid = new IDOSGridObject();
    grid->setName(QStringLiteral("DFN source grid"));
    grid->setCornerPointData(2, 1, 2, coord, zcorn, {1, 0, 1, 1});
    project->add(grid);
    IDOSGridPropertyObject* density = new IDOSGridPropertyObject();
    density->setName(QStringLiteral("Sampling weights"));
    density->setGridId(grid->objectId());
    density->setDimensions(2, 1, 2);
    QVERIFY(density->setValues({0.0, 1000.0, 1.0, 0.0}));
    project->add(density);
    QVERIFY(action->isEnabled());
    action->trigger();
    QPointer<QWidget> widget = window.findChild<QWidget*>(QStringLiteral("idosDfnGenerationWidget"));
    QVERIFY(!widget.isNull());
    widget->findChild<QSpinBox*>(QStringLiteral("dfnCount"))->setValue(12);
    widget->findChild<QSpinBox*>(QStringLiteral("dfnSeed"))->setValue(42);
    QCOMPARE(widget->findChild<QSpinBox*>(QStringLiteral("dfnLastLayer"))->value(), 2);
    widget->findChild<QDoubleSpinBox*>(QStringLiteral("dfnMinimumLength"))->setValue(2.0);
    widget->findChild<QDoubleSpinBox*>(QStringLiteral("dfnMaximumLength"))->setValue(4.0);
    widget->findChild<QDoubleSpinBox*>(QStringLiteral("dfnStrikeSpread"))->setValue(0.0);
    widget->findChild<QDoubleSpinBox*>(QStringLiteral("dfnDipSpread"))->setValue(0.0);
    QComboBox* weights = widget->findChild<QComboBox*>(QStringLiteral("dfnDensity"));
    weights->setCurrentIndex(weights->findData(density->objectId()));
    QVERIFY(weights->currentIndex() > 0);
    widget->findChild<QPushButton*>(QStringLiteral("dfnGenerate"))->click();
    const QString outputId = project->commandManager()->result().value(QStringLiteral("objectId")).toString();
    IDOSNaturalFractureSetObject* output = qobject_cast<IDOSNaturalFractureSetObject*>(project->object(outputId));
    QVERIFY2(output != nullptr, qPrintable(project->commandManager()->error()));
    QCOMPARE(output->fractures().size(), 12);
    const QVariantList original = output->fractures();
    const QJsonObject parameters = QJsonObject::fromVariantMap(original.first().toMap().value(QStringLiteral("generation")).toMap());
    for (const QVariant& value : output->geometryReport())
    {
        const QVariantMap report = value.toMap();
        QVERIFY(report.value(QStringLiteral("valid")).toBool());
        QVERIFY(fractureIssueCodes(report).isEmpty());
        const QVariantList center = report.value(QStringLiteral("center")).toList();
        QVERIFY(center.at(0).toDouble() > 5000000.0 && center.at(0).toDouble() < 5000010.0);
        QVERIFY(center.at(1).toDouble() > 7000000.0 && center.at(1).toDouble() < 7000010.0);
        QVERIFY(center.at(2).toDouble() > 10.0 && center.at(2).toDouble() < 20.0);
    }
    const QVariantList normal = original.first().toMap().value(QStringLiteral("normal")).toList();
    QVERIFY(std::abs(normal.at(0).toDouble() - std::sqrt(3.0) * 0.5) < 1.0e-10);
    QVERIFY(std::abs(normal.at(1).toDouble()) < 1.0e-10);
    QVERIFY(std::abs(normal.at(2).toDouble() - 0.5) < 1.0e-10);
    const QString screenshotRoot = qEnvironmentVariable("IDOS_TEST_SCREENSHOT_DIR");
    if (!screenshotRoot.isEmpty())
    {
        QVERIFY(widget->grab().save(QDir(screenshotRoot).filePath(QStringLiteral("dfn-generation.png"))));
    }
    const QModelIndex node = window.dataTreeModel()->index(outputId);
    QVERIFY(node.isValid());
    QVERIFY(window.dataTreeModel()->setData(node, Qt::Checked, Qt::CheckStateRole));
    QVERIFY(findRenderObject(window.renderServer(), window.renderServer()->currentView(), outputId)->isVisible());
    QTemporaryDir saved;
    QString error;
    QVERIFY2(project->save(saved.filePath(QStringLiteral("dfn.idos")), window.dataRegistry(), error), qPrintable(error));
    QScopedPointer<IDOSProject> loaded(IDOSProject::load(saved.filePath(QStringLiteral("dfn.idos")), window.dataRegistry(), error));
    QVERIFY2(!loaded.isNull(), qPrintable(error));
    IDOSNaturalFractureSetObject* restored = qobject_cast<IDOSNaturalFractureSetObject*>(loaded->object(outputId));
    QVERIFY(restored != nullptr);
    QCOMPARE(restored->fractures(), original);
    plugins->unloadCppPlugin(pluginPath);
    QVERIFY(plugins->isLoaded(QStringLiteral("idosfracturemodelingplugin")));
    project->commandManager()->undo();
    QVERIFY(project->object(outputId) == nullptr);
    QVERIFY(findRenderObject(window.renderServer(), window.renderServer()->currentView(), outputId) == nullptr);
    QVERIFY(density->setValues({0.0, 0.0, 0.0, 0.0}));
    QJsonObject failure = project->commandManager()->execute(QStringLiteral("fracture.dfn.generate"), parameters);
    QVERIFY(!failure.value(QStringLiteral("success")).toBool());
    QVERIFY(project->commandManager()->canRedo());
    project->commandManager()->redo();
    QCOMPARE(project->object(outputId), static_cast<IDOSDataObject*>(output));
    QCOMPARE(output->fractures(), original);
    QVERIFY(density->setValues({0.0, 1000.0, 1.0, 0.0}));
    QJsonObject response = project->commandManager()->execute(QStringLiteral("fracture.dfn.generate"), parameters);
    QVERIFY(response.value(QStringLiteral("success")).toBool());
    IDOSNaturalFractureSetObject* repeated = qobject_cast<IDOSNaturalFractureSetObject*>(project->object(response.value(QStringLiteral("result")).toObject().value(QStringLiteral("objectId")).toString()));
    QVERIFY(repeated != nullptr);
    QVERIFY(repeated->objectId() != outputId);
    QCOMPARE(repeated->fractures(), original);
    project->commandManager()->undo();
    QJsonObject invalid = parameters;
    invalid.insert(QStringLiteral("count"), 0);
    QVERIFY(!project->commandManager()->execute(QStringLiteral("fracture.dfn.generate"), invalid).value(QStringLiteral("success")).toBool());
    QVERIFY(project->commandManager()->canRedo());
    invalid = parameters;
    invalid.remove(QStringLiteral("seed"));
    QVERIFY(!project->commandManager()->execute(QStringLiteral("fracture.dfn.generate"), invalid).value(QStringLiteral("success")).toBool());
    invalid = parameters;
    invalid.insert(QStringLiteral("lastLayer"), 3);
    QVERIFY(!project->commandManager()->execute(QStringLiteral("fracture.dfn.generate"), invalid).value(QStringLiteral("success")).toBool());
    invalid = parameters;
    invalid.insert(QStringLiteral("densityId"), QStringLiteral("missing"));
    QVERIFY(!project->commandManager()->execute(QStringLiteral("fracture.dfn.generate"), invalid).value(QStringLiteral("success")).toBool());
    QVERIFY(project->commandManager()->canRedo());
    invalid = parameters;
    invalid.insert(QStringLiteral("densityId"), 123);
    QVERIFY(!project->commandManager()->execute(QStringLiteral("fracture.dfn.generate"), invalid).value(QStringLiteral("success")).toBool());
    invalid = parameters;
    invalid.insert(QStringLiteral("minimumLength"), -1.0);
    QVERIFY(!project->commandManager()->execute(QStringLiteral("fracture.dfn.generate"), invalid).value(QStringLiteral("success")).toBool());
    density->setGridId(QStringLiteral("unrelated"));
    QVERIFY(!project->commandManager()->execute(QStringLiteral("fracture.dfn.generate"), parameters).value(QStringLiteral("success")).toBool());
    density->setGridId(grid->objectId());
    QVERIFY(project->commandManager()->canRedo());
    QJsonObject changedSeed = parameters;
    changedSeed.insert(QStringLiteral("seed"), 43);
    response = project->commandManager()->execute(QStringLiteral("fracture.dfn.generate"), changedSeed);
    QVERIFY(response.value(QStringLiteral("success")).toBool());
    IDOSNaturalFractureSetObject* different = qobject_cast<IDOSNaturalFractureSetObject*>(project->object(response.value(QStringLiteral("result")).toObject().value(QStringLiteral("objectId")).toString()));
    QVERIFY(different != nullptr);
    QVERIFY(different->fractures().first().toMap().value(QStringLiteral("vertices"))
        != original.first().toMap().value(QStringLiteral("vertices")));
    const int objectCount = project->objects().size();
    project->commandManager()->clear();
    QCOMPARE(project->objects().size(), objectCount);
    IDOSProject* otherProject = new IDOSProject(&window);
    window.setProject(otherProject);
    QVERIFY(widget.isNull());
    window.setProject(project);
    action->trigger();
    widget = window.findChild<QWidget*>(QStringLiteral("idosDfnGenerationWidget"));
    QVERIFY(!widget.isNull());
    std::unique_ptr<IDOSCommand> pending = window.commandRegistry()->create(QStringLiteral("fracture.dfn.generate"), parameters, project);
    plugins->unloadCppPlugin(pluginPath);
    QVERIFY(plugins->isLoaded(QStringLiteral("idosfracturemodelingplugin")));
    pending.reset();
    plugins->unloadCppPlugin(pluginPath);
    QVERIFY(!plugins->isLoaded(QStringLiteral("idosfracturemodelingplugin")));
    QVERIFY(widget.isNull());
    QVERIFY(window.commandRegistry()->metadata(QStringLiteral("fracture.dfn.generate")) == nullptr);
    QVERIFY(findRenderObject(window.renderServer(), window.renderServer()->currentView(), outputId) != nullptr);
    QCOMPARE(output->fractures(), original);
    for (const QVariant& value : output->geometryReport())
    {
        QVERIFY(value.toMap().value(QStringLiteral("valid")).toBool());
    }
    QVERIFY2(project->save(saved.filePath(QStringLiteral("dfn-after-unload.idos")), window.dataRegistry(), error), qPrintable(error));
    if (hadSetting)
    {
        settings.setValue(settingKey, previousSetting);
    }
    else
    {
        settings.remove(settingKey);
    }
}

void CaseWorkflowTest::onFractureGeometry()
{
    IDOSNaturalFractureSetObject data;
    QSignalSpy changes(&data, &IDOSDataObject::dataChanged);
    const QVector<double> square = {5000000.0, 7000000.0, 3000.0,
                                   5000002.0, 7000000.0, 3000.0,
                                   5000002.0, 7000002.0, 3000.0,
                                   5000000.0, 7000002.0, 3000.0};
    data.setFractures({fracture(square, 1)});
    changes.clear();
    QVariantMap report = data.geometryReport().first().toMap();
    QVERIFY(report.value(QStringLiteral("valid")).toBool());
    QVERIFY(fractureIssueCodes(report).isEmpty());
    QCOMPARE(report.value(QStringLiteral("area")).toDouble(), 4.0);
    QCOMPARE(report.value(QStringLiteral("coordinates")).value<QVector<double>>().first(), 5000000.0);
    QCOMPARE(changes.count(), 0);

    QVariantMap record = fracture(square, 1);
    record.insert(QStringLiteral("normal"), QVariantList{0.0, 0.0, -1.0});
    data.setFractures({record});
    QVERIFY(fractureIssueCodes(data.geometryReport().first().toMap()).isEmpty());
    record.insert(QStringLiteral("normal"), QVariantList{0.0, 1.0, 0.0});
    data.setFractures({record});
    report = data.geometryReport().first().toMap();
    QVERIFY(report.value(QStringLiteral("valid")).toBool());
    QVERIFY(fractureIssueCodes(report).contains(QStringLiteral("normal_alignment")));
    record.insert(QStringLiteral("normal"), QVariantList{0.0, 0.0, 0.0});
    data.setFractures({record});
    QVERIFY(fractureIssueCodes(data.geometryReport().first().toMap()).contains(QStringLiteral("normal")));

    QVector<double> modified = square;
    modified[9] = modified[0];
    modified[10] = modified[1];
    data.setFractures({fracture(modified, 1)});
    report = data.geometryReport().first().toMap();
    QVERIFY(!report.value(QStringLiteral("valid")).toBool());
    QVERIFY(fractureIssueCodes(report).contains(QStringLiteral("duplicate_vertex")));
    modified = square;
    modified[11] += 0.1;
    data.setFractures({fracture(modified, 1)});
    report = data.geometryReport().first().toMap();
    QVERIFY(!report.value(QStringLiteral("valid")).toBool());
    QVERIFY(fractureIssueCodes(report).contains(QStringLiteral("nonplanar")));
    // Millimetre-scale rounding at large map coordinates is not a warped face.
    modified = square;
    modified[11] += 0.0001;
    data.setFractures({fracture(modified, 1)});
    report = data.geometryReport().first().toMap();
    QVERIFY(report.value(QStringLiteral("valid")).toBool());
    QVERIFY(!fractureIssueCodes(report).contains(QStringLiteral("nonplanar")));
    QVERIFY(report.value(QStringLiteral("maximumPlaneDistance")).toDouble()
        < report.value(QStringLiteral("planeTolerance")).toDouble());
    const QVector<double> roundedHexagon = {
        335755.539, 4262856.044, -579.9964502,
        335661.1314, 4262871.899, -635.4221704,
        335663.2144, 4262887.341, -744.9365137,
        335759.705, 4262886.928, -799.0251367,
        335854.1127, 4262871.073, -743.5994165,
        335852.0297, 4262855.631, -634.0850733};
    data.setFractures({fracture(roundedHexagon, 1)});
    report = data.geometryReport().first().toMap();
    QVERIFY(report.value(QStringLiteral("valid")).toBool());
    QVERIFY(!fractureIssueCodes(report).contains(QStringLiteral("nonplanar")));
    modified = roundedHexagon;
    modified[17] += 1.0;
    data.setFractures({fracture(modified, 1)});
    report = data.geometryReport().first().toMap();
    QVERIFY(!report.value(QStringLiteral("valid")).toBool());
    QVERIFY(fractureIssueCodes(report).contains(QStringLiteral("nonplanar")));
    modified = square;
    modified[0] = std::numeric_limits<double>::quiet_NaN();
    data.setFractures({fracture(modified, 1)});
    QVERIFY(fractureIssueCodes(data.geometryReport().first().toMap()).contains(QStringLiteral("coordinate")));
    data.setFractures({fracture({0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0}, 1)});
    report = data.geometryReport().first().toMap();
    QVERIFY(!report.value(QStringLiteral("valid")).toBool());
    QVERIFY(fractureIssueCodes(report).contains(QStringLiteral("self_intersection")));
    data.setFractures({fracture({0, 0, 0, 1, 0, 0, 2, 0, 0}, 1)});
    QVERIFY(fractureIssueCodes(data.geometryReport().first().toMap()).contains(QStringLiteral("degenerate")));
    data.setFractures({fracture({0, 0, 0, 1, 0, 0}, 1)});
    QVERIFY(fractureIssueCodes(data.geometryReport().first().toMap()).contains(QStringLiteral("vertex_count")));
    record = fracture(square, 1);
    record.insert(QStringLiteral("properties"), QVariantMap{{QStringLiteral("Aperture"), -0.01},
                                                           {QStringLiteral("Porosity"), 1.2}});
    data.setFractures({record, fracture(square, 1)});
    const QVariantList reports = data.geometryReport();
    QCOMPARE(fractureIssueCodes(reports.first().toMap()).count(QStringLiteral("property")), 2);
    QVERIFY(fractureIssueCodes(reports.last().toMap()).contains(QStringLiteral("fracture_id")));
    data.setFractures({fracture({0, 0, 0, 2, 0, 0, 2, 1, 0, 1, 1, 0, 1, 2, 0, 0, 2, 0}, 1)});
    report = data.geometryReport().first().toMap();
    QVERIFY(report.value(QStringLiteral("valid")).toBool());
    QCOMPARE(report.value(QStringLiteral("area")).toDouble(), 3.0);
    const QString fieldDirectory = qEnvironmentVariable("IDOS_FRACTURE_TEST_DIR");
    if (!fieldDirectory.isEmpty())
    {
        const QDir directory(fieldDirectory);
        const QStringList files = directory.entryList({QStringLiteral("*.fab")}, QDir::Files);
        QVERIFY(!files.isEmpty());
        int total = 0;
        for (const QString& file : files)
        {
            std::unique_ptr<IDOSDataProvider> provider = IDOSDataProviderRegistry::instance().create(
                QStringLiteral("idos.naturalFracture.ascii"));
            QVERIFY(provider != nullptr);
            const QList<IDOSDataObject*> objects = provider->read(directory.filePath(file));
            QVERIFY2(objects.size() == 1, qPrintable(provider->lastError()));
            QScopedPointer<IDOSDataObject> owner(objects.first());
            IDOSNaturalFractureSetObject* field = qobject_cast<IDOSNaturalFractureSetObject*>(owner.data());
            QVERIFY(field != nullptr);
            const QVariantList fieldReports = field->geometryReport();
            for (const QVariant& fieldValue : fieldReports)
            {
                const QVariantMap fieldReport = fieldValue.toMap();
                QVERIFY2(fieldReport.value(QStringLiteral("valid")).toBool(),
                    qPrintable(file + QStringLiteral(": ")
                        + fieldReport.value(QStringLiteral("fractureId")).toString()));
            }
            total += fieldReports.size();
        }
        qInfo() << "Field fracture files:" << files.size() << "valid polygons:" << total;
    }
}

void CaseWorkflowTest::onDataGroupChecks()
{
    IDOSMainWindow window;
    IDOSProject* project = new IDOSProject(&window);
    window.setProject(project);
    QList<QString> firstIds;
    QList<QString> secondIds;
    const QStringList groups = {QStringLiteral("data.naturalFractures"),
                                QStringLiteral("data.wells"), QStringLiteral("data.boundaries")};
    for (int category = 0; category < groups.size(); ++category)
    {
        for (int index = 0; index < 2; ++index)
        {
            IDOSDataObject* data = nullptr;
            if (category == 0)
            {
                IDOSNaturalFractureSetObject* fractures = new IDOSNaturalFractureSetObject();
                fractures->setFractures({fracture({0, 0, 0, 2, 0, 0, 2, 2, 0, 0, 2, 0}, 1)});
                data = fractures;
            }
            else if (category == 1)
            {
                data = new IDOSWellObject();
            }
            else
            {
                IDOSGridBoundaryObject* boundary = new IDOSGridBoundaryObject();
                boundary->setXCoordinates({0, 2, 2, 0});
                boundary->setYCoordinates({0, 0, 2, 2});
                boundary->setZCoordinates({0, 0, 0, 0});
                boundary->setGroupIds({1, 1, 1, 1});
                data = boundary;
            }
            data->setName(QStringLiteral("Group test %1 %2").arg(category).arg(index));
            project->add(data);
            if (index == 0)
            {
                firstIds.append(data->objectId());
            }
            else
            {
                secondIds.append(data->objectId());
            }
        }
    }
    IDOSRenderView* primary = window.renderServer()->currentView();
    IDOSRenderView other;
    window.renderServer()->addView(&other);
    IDOSTreeView* view = window.dataTreeView();
    QVERIFY(window.renderServer()->setCurrentView(primary));

    IDOSTreeModel* model = window.dataTreeModel();
    const QModelIndex wellParent = model->index(model->find(QStringLiteral("data.wellGroup")));
    for (int category = 0; category < groups.size(); ++category)
    {
        const QModelIndex group = model->index(model->find(groups.at(category)));
        QVERIFY(group.isValid());
        QVERIFY(model->flags(group).testFlag(Qt::ItemIsUserCheckable));
        QCOMPARE(model->data(group, Qt::CheckStateRole).toInt(), int(Qt::Unchecked));
        const QModelIndex first = model->index(firstIds.at(category));
        QVERIFY(model->setData(first, Qt::Checked, Qt::CheckStateRole));
        QCOMPARE(model->data(group, Qt::CheckStateRole).toInt(), int(Qt::PartiallyChecked));
        if (category == 1)
        {
            QCOMPARE(model->data(wellParent, Qt::CheckStateRole).toInt(), int(Qt::PartiallyChecked));
        }
        QSignalSpy changes(primary->scene(), &IDOSRenderScene::changed);
        QVERIFY(model->setData(group, Qt::Checked, Qt::CheckStateRole));
        QCOMPARE(changes.count(), 1);
        QCOMPARE(model->data(group, Qt::CheckStateRole).toInt(), int(Qt::Checked));
        QVERIFY(findRenderObject(window.renderServer(), primary, secondIds.at(category))->isVisible());
        QVERIFY(findRenderObject(window.renderServer(), &other, firstIds.at(category)) == nullptr);
        findRenderObject(window.renderServer(), primary, secondIds.at(category))->setVisible(false);
        primary->scene()->notifyChanged(findRenderObject(window.renderServer(), primary, secondIds.at(category))->id());
        QCOMPARE(model->data(group, Qt::CheckStateRole).toInt(), int(Qt::PartiallyChecked));
        QVERIFY(window.renderServer()->setCurrentView(&other));

        QCOMPARE(model->data(group, Qt::CheckStateRole).toInt(), int(Qt::Unchecked));
        QVERIFY(window.renderServer()->setCurrentView(primary));

        QVERIFY(model->setData(group, Qt::Unchecked, Qt::CheckStateRole));
        QVERIFY(!findRenderObject(window.renderServer(), primary, firstIds.at(category))->isVisible());
        QCOMPARE(model->data(group, Qt::CheckStateRole).toInt(), int(Qt::Unchecked));
    }
    QVERIFY(model->setData(wellParent, Qt::Checked, Qt::CheckStateRole));
    QVERIFY(findRenderObject(window.renderServer(), primary, firstIds.at(1))->isVisible());
    QVERIFY(findRenderObject(window.renderServer(), primary, secondIds.at(1))->isVisible());
    QVERIFY(model->setData(wellParent, Qt::Unchecked, Qt::CheckStateRole));
    IDOSTreeModel foreignModel(nullptr);
    foreignModel.setName(QStringLiteral("case"));
    IDOSTreeItem* foreignGroup = new IDOSTreeItem(new IDOSTreeItemProvider());
    foreignGroup->provider()->setGroup(true);
    foreignGroup->provider()->setKey(QStringLiteral("data.naturalFractures"));
    QVERIFY(foreignModel.insertItem(foreignModel.root(), foreignGroup));
    const QModelIndex foreignIndex = foreignModel.index(foreignGroup);
    QVERIFY(!foreignModel.flags(foreignIndex).testFlag(Qt::ItemIsUserCheckable));
    QVERIFY(!foreignModel.setData(foreignIndex, Qt::Checked, Qt::CheckStateRole));
    const QModelIndex fracturesGroup = model->index(model->find(groups.first()));
    QVERIFY(project->remove(firstIds.first()));
    QVERIFY(project->remove(secondIds.first()));
    QVERIFY(!model->flags(fracturesGroup).testFlag(Qt::ItemIsUserCheckable));
    QVERIFY(!model->data(fracturesGroup, Qt::CheckStateRole).isValid());
    QVERIFY(window.renderServer()->setCurrentView(nullptr));

    QVERIFY(!model->setData(wellParent, Qt::Checked, Qt::CheckStateRole));
    QVERIFY(window.renderServer()->setCurrentView(primary));

}

void CaseWorkflowTest::onConfirmDataDeletion()
{
    QMessageBox* message = qobject_cast<QMessageBox*>(QApplication::activeModalWidget());
    if (message != nullptr)
    {
        message->button(m_cancel ? QMessageBox::No : QMessageBox::Yes)->click();
    }
}

void CaseWorkflowTest::onDeleteImportedData()
{
    IDOSMainWindow window;
    IDOSProject* project = new IDOSProject(&window);
    window.setProject(project);
    IDOSNaturalFractureSetObject* first = new IDOSNaturalFractureSetObject();
    first->setName(QStringLiteral("First fractures"));
    first->setFractures({fracture({0, 0, 0, 2, 0, 0, 2, 2, 0, 0, 2, 0}, 1)});
    IDOSNaturalFractureSetObject* second = new IDOSNaturalFractureSetObject();
    second->setName(QStringLiteral("Second fractures"));
    second->setFractures(first->fractures());
    IDOSGridObject* grid = new IDOSGridObject();
    project->add(first);
    project->add(second);
    project->add(grid);
    const QString firstId = first->objectId();
    const QString secondId = second->objectId();
    IDOSSimulationCaseObject* caseObject = new IDOSSimulationCaseObject();
    caseObject->setItemRefs({IDOSCaseItemRef(QStringLiteral("case.fractures"), firstId),
                            IDOSCaseItemRef(QStringLiteral("case.fractures"), secondId),
                            IDOSCaseItemRef(QStringLiteral("case.grid"), grid->objectId())});
    project->add(caseObject);
    const QString referenceId = caseObject->itemRefs().first().refId();
    IDOSRenderView* primary = window.renderServer()->currentView();
    IDOSRenderView additional;
    window.renderServer()->addView(&additional);
    window.renderServer()->setChecked(firstId, Qt::Checked, primary);
    window.renderServer()->setChecked(secondId, Qt::Checked, &additional);
    QVERIFY(findRenderObject(window.renderServer(), primary, firstId) != nullptr);
    QVERIFY(findRenderObject(window.renderServer(), &additional, secondId) != nullptr);
    IDOSTreeView* view = window.dataTreeView();
    const QModelIndex firstIndex = window.dataTreeModel()->index(firstId);
    const QModelIndex secondIndex = window.dataTreeModel()->index(secondId);
    QVERIFY(firstIndex.isValid() && secondIndex.isValid());
    QScopedPointer<QMenu> single(view->createContextMenu(firstIndex));
    QVERIFY(single->findChild<QAction*>(QStringLiteral("deleteDataAction")) != nullptr);
    view->selectionModel()->select(secondIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    QScopedPointer<QMenu> multiple(view->createContextMenu(firstIndex));
    QAction* remove = multiple->findChild<QAction*>(QStringLiteral("deleteDataAction"));
    QVERIFY(remove != nullptr);
    QCOMPARE(remove->property("objectIds").toStringList().size(), 2);
    QTimer timer;
    connect(&timer, &QTimer::timeout, this, &CaseWorkflowTest::onConfirmDataDeletion);
    timer.start(10);
    m_cancel = true;
    remove->trigger();
    QCOMPARE(project->object(firstId), static_cast<IDOSDataObject*>(first));
    QVERIFY(!project->commandManager()->canUndo());
    m_cancel = false;
    remove->trigger();
    timer.stop();
    QVERIFY(project->object(firstId) == nullptr);
    QVERIFY(project->object(secondId) == nullptr);
    QVERIFY(!window.dataTreeModel()->index(firstId).isValid());
    QVERIFY(findRenderObject(window.renderServer(), primary, firstId) == nullptr);
    QVERIFY(findRenderObject(window.renderServer(), &additional, secondId) == nullptr);
    QCOMPARE(caseObject->itemRefs().size(), 1);
    QCOMPARE(caseObject->itemRefs().first().objectId(), grid->objectId());
    project->commandManager()->undo();
    QCOMPARE(project->object(firstId), static_cast<IDOSDataObject*>(first));
    QCOMPARE(project->object(secondId), static_cast<IDOSDataObject*>(second));
    QCOMPARE(caseObject->itemRefs().size(), 3);
    QCOMPARE(caseObject->itemRefs().first().refId(), referenceId);
    QVERIFY(window.dataTreeModel()->index(firstId).isValid());
    QVERIFY(findRenderObject(window.renderServer(), primary, firstId) != nullptr);
    QVERIFY(findRenderObject(window.renderServer(), &additional, secondId) != nullptr);
    QVERIFY(project->commandManager()->canRedo());
    QVERIFY(!project->commandManager()->execute(new IDOSDeleteDataCommand(project, {firstId, grid->objectId()})));
    QCOMPARE(project->object(firstId), static_cast<IDOSDataObject*>(first));
    QCOMPARE(caseObject->itemRefs().size(), 3);
    QVERIFY(project->commandManager()->canRedo());
    QVERIFY(!project->commandManager()->execute(new IDOSDeleteDataCommand(project, {firstId, firstId})));
    QVERIFY(project->commandManager()->canRedo());
    project->commandManager()->redo();
    QVERIFY(project->object(firstId) == nullptr);
    QCOMPARE(caseObject->itemRefs().size(), 1);
    QPointer<IDOSNaturalFractureSetObject> lifetime(first);
    project->commandManager()->clear();
    QVERIFY(lifetime.isNull());
    QVERIFY(project->object(grid->objectId()) != nullptr);
}

void CaseWorkflowTest::onFractureDisplay()
{
    QTranslator translator;
    QVERIFY(translator.load(QCoreApplication::applicationDirPath() + QStringLiteral("/i18n/idos_zh_CN.qm")));
    QVERIFY(QApplication::installTranslator(&translator));
    QSettings settings(QStringLiteral("pluginRegistry"), QStringLiteral("idos"));
    const QString settingKey = QStringLiteral("/plugins/idosfracturemodelingplugin");
    const bool hadSetting = settings.contains(settingKey);
    const QVariant previousSetting = settings.value(settingKey);
    IDOSMainWindow window;
    IDOSProject* project = new IDOSProject(&window);
    window.setProject(project);
    const QString pluginPath = QCoreApplication::applicationDirPath()
        + QStringLiteral("/plugins/idosfracturemodelingplugin.dll");
    IDOSPluginRegistry* plugins = IDOSPluginRegistry::instance();
    plugins->loadCppPlugin(pluginPath);
    QVERIFY(plugins->isLoaded(QStringLiteral("idosfracturemodelingplugin")));
    QAction* check = window.findChild<QAction*>(QStringLiteral("idosCheckFracturesAction"));
    QVERIFY(check != nullptr);
    QVERIFY(!check->isEnabled());
    IDOSNaturalFractureSetObject* data = new IDOSNaturalFractureSetObject();
    data->setName(QStringLiteral("Sample fractures"));
    data->setFractures({fracture({0, 0, 0, 2, 0, 0, 2, 2, 0, 0, 2, 0}, 1),
                        fracture({0, 0, 1, 1, 0, 1, 2, 0, 1}, 2)});
    project->add(data);
    project->setCurrentObject(data);
    QVERIFY(check->isEnabled());
    const QString dataId = data->objectId();
    IDOSRenderServer* server = window.renderServer();
    IDOSRenderView* firstView = server->currentView();
    QVERIFY(firstView != nullptr);
    IDOSRenderView secondView;
    server->addView(&secondView);
    server->setChecked(dataId, Qt::Checked, firstView);
    server->setChecked(dataId, Qt::Checked, &secondView);
    server->setChecked(dataId, Qt::Unchecked, &secondView);
    IDOSRenderObject* first = findRenderObject(window.renderServer(), firstView, dataId);
    IDOSRenderObject* second = findRenderObject(window.renderServer(), &secondView, dataId);
    QVERIFY(first != nullptr && second != nullptr);
    QVERIFY(first->provider() != second->provider());
    QCOMPARE(actors(first->provider()).size(), 1);
    vtkActor* actor = actors(first->provider()).first();
    vtkPolyDataMapper* mapper = vtkPolyDataMapper::SafeDownCast(actor->GetMapper());
    QVERIFY(mapper != nullptr);
    mapper->Update();
    QCOMPARE(mapper->GetInput()->GetNumberOfCells(), vtkIdType(2));
    QVERIFY(mapper->GetInput()->GetCellData()->GetArray("FractureId") != nullptr);
    QCOMPARE(mapper->GetInput()->GetCellData()->GetArray("FractureId")->GetComponent(0, 0), 1.0);
    const QModelIndex node = window.dataTreeModel()->index(dataId);
    QVERIFY(node.isValid());
    QVERIFY(window.dataTreeModel()->flags(node).testFlag(Qt::ItemIsUserCheckable));
    QVERIFY(window.dataTreeModel()->setData(node, Qt::Checked, Qt::CheckStateRole));
    QVERIFY(first->isVisible());
    QVERIFY(!second->isVisible());
    first->setOpacity(0.4);
    firstView->setSelection({first->id()});
    QVERIFY(first->isSelected());
    QVERIFY(!second->isSelected());
    QCOMPARE(second->opacity(), 1.0);
    const QVariantMap appearance = first->provider()->saveState();
    first->setOpacity(0.9);
    QVERIFY(first->provider()->restoreState(appearance));
    QCOMPARE(first->opacity(), 0.4);

    window.resize(1100, 800);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    firstView->resetCamera();
    firstView->render();
    const QString screenshots = qEnvironmentVariable("IDOS_TEST_SCREENSHOT_DIR");
    if (!screenshots.isEmpty())
    {
        QVERIFY(firstView->grab().save(QDir(screenshots).filePath(QStringLiteral("fracture-display.png"))));
    }
    check->trigger();
    QPointer<QWidget> dialog = window.findChild<QWidget*>(QStringLiteral("idosFractureGeometryReport"));
    QVERIFY(!dialog.isNull());
    QTableWidget* table = dialog->findChild<QTableWidget*>(QStringLiteral("idosFractureGeometryIssues"));
    QVERIFY(table != nullptr);
    QVERIFY(table->rowCount() >= 2);
    if (!screenshots.isEmpty())
    {
        QVERIFY(dialog->grab().save(QDir(screenshots).filePath(QStringLiteral("fracture-geometry-report.png"))));
    }
    QVERIFY(QMetaObject::invokeMethod(table, "cellDoubleClicked", Qt::DirectConnection,
                                     Q_ARG(int, 0), Q_ARG(int, 0)));
    QCOMPARE(firstView->selection().first(), first->id());
    QVERIFY(!second->isVisible());
    server->setCurrentView(&secondView);
    QVERIFY(QMetaObject::invokeMethod(table, "cellDoubleClicked", Qt::DirectConnection,
                                     Q_ARG(int, 0), Q_ARG(int, 0)));
    QVERIFY(!second->isVisible());
    data->setFractures({fracture({0, 0, 0, 3, 0, 0, 3, 3, 0, 0, 3, 0}, 3)});
    QTRY_VERIFY(dialog.isNull());
    QCOMPARE(first->opacity(), 0.4);
    QVERIFY(first->isVisible());
    QCOMPARE(actors(first->provider()).size(), 1);

    QTemporaryDir saved;
    QString error;
    QVERIFY2(project->save(saved.filePath(QStringLiteral("fractures.idos")), window.dataRegistry(), error), qPrintable(error));
    QScopedPointer<IDOSProject> loaded(IDOSProject::load(saved.filePath(QStringLiteral("fractures.idos")),
                                                       window.dataRegistry(), error));
    QVERIFY2(!loaded.isNull(), qPrintable(error));
    IDOSNaturalFractureSetObject* restored = qobject_cast<IDOSNaturalFractureSetObject*>(loaded->object(dataId));
    QVERIFY(restored != nullptr);
    IDOSNaturalFractureRenderProvider restoredProvider(restored);
    QCOMPARE(actors(&restoredProvider).size(), 1);

    check->trigger();
    dialog = window.findChild<QWidget*>(QStringLiteral("idosFractureGeometryReport"));
    QVERIFY(!dialog.isNull());
    IDOSDataObject* detached = project->take(dataId);
    QCOMPARE(detached, static_cast<IDOSDataObject*>(data));
    QTRY_VERIFY(dialog.isNull());
    QVERIFY(findRenderObject(window.renderServer(), firstView, dataId) == nullptr);
    QVERIFY(!check->isEnabled());
    project->add(detached);
    project->setCurrentObject(data);
    QVERIFY(findRenderObject(window.renderServer(), firstView, dataId) != nullptr);
    QVERIFY(check->isEnabled());

    server->setCurrentView(firstView);
    check->trigger();
    dialog = window.findChild<QWidget*>(QStringLiteral("idosFractureGeometryReport"));
    QVERIFY(!dialog.isNull());
    IDOSProject* nextProject = new IDOSProject(&window);
    window.setProject(nextProject);
    QTRY_VERIFY(dialog.isNull());
    QVERIFY(!check->isEnabled());
    window.setProject(project);
    project->setCurrentObject(data);
    check->trigger();
    dialog = window.findChild<QWidget*>(QStringLiteral("idosFractureGeometryReport"));
    QVERIFY(!dialog.isNull());
    plugins->unloadCppPlugin(pluginPath);
    QVERIFY(dialog.isNull());
    QVERIFY(!plugins->isLoaded(QStringLiteral("idosfracturemodelingplugin")));
    QVERIFY(findRenderObject(window.renderServer(), firstView, dataId) != nullptr);
    if (hadSetting)
    {
        settings.setValue(settingKey, previousSetting);
    }
    else
    {
        settings.remove(settingKey);
    }
    QApplication::removeTranslator(&translator);
}

void CaseWorkflowTest::onGridCompanionFiles()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString base = directory.filePath(QStringLiteral("Bundle"));
    const QString gridPath = base + QStringLiteral(".EGRID");
    Opm::EclipseGrid sourceGrid(2, 1, 1);
    sourceGrid.resetACTNUM(std::vector<int>{1, 0});
    sourceGrid.save(gridPath.toStdString(), false, {}, Opm::UnitSystem::newMETRIC());
    std::vector<int> header(100, 0);
    header[8] = 2;
    header[9] = 1;
    header[10] = 1;
    header[11] = 1;
    {
        Opm::EclIO::EclOutput output((base + QStringLiteral(".INIT")).toStdString(), false);
        output.write("INTEHEAD", header);
        output.write("PORO", std::vector<float>{0.25f});
    }
    {
        Opm::EclIO::EclOutput output((base + QStringLiteral(".UNRST")).toStdString(), false);
        output.write("SEQNUM", std::vector<int>{1});
        output.write("INTEHEAD", header);
        output.write("PRESSURE", std::vector<float>{100.0f});
        output.write("SEQNUM", std::vector<int>{2});
        output.write("INTEHEAD", header);
        output.write("PRESSURE", std::vector<float>{200.0f});
    }
    {
        Opm::EclIO::EclOutput output((base + QStringLiteral(".SMSPEC")).toStdString(), false);
        output.write("DIMENS", std::vector<int>{1, 2, 1, 1, 0, 0});
        output.write("RESTART", std::vector<std::string>{""}, 8);
        output.write("KEYWORDS", std::vector<std::string>{"TIME"}, 8);
        output.write("WGNAMES", std::vector<std::string>{""}, 8);
        output.write("NUMS", std::vector<int>{0});
        output.write("UNITS", std::vector<std::string>{"DAYS"}, 8);
        output.write("STARTDAT", std::vector<int>{1, 1, 2020});
    }
    {
        Opm::EclIO::EclOutput output((base + QStringLiteral(".UNSMRY")).toStdString(), false);
        output.write("SEQHDR", std::vector<int>{1});
        output.write("MINISTEP", std::vector<int>{0});
        output.write("PARAMS", std::vector<float>{1.0f});
    }
    QFile unrelated(directory.filePath(QStringLiteral("BundleOther.INIT")));
    QVERIFY(unrelated.open(QIODevice::WriteOnly));
    unrelated.write("invalid unrelated file");
    unrelated.close();
    QFile log(base + QStringLiteral(".PRT"));
    QVERIFY(log.open(QIODevice::WriteOnly));
    log.write("simulator log");
    log.close();

    IDOSMainWindow window;
    IDOSProject* project = new IDOSProject(&window);
    window.setProject(project);
    IDOSSimulationCaseObject* caseObject = new IDOSSimulationCaseObject();
    project->add(caseObject);
    IDOSCommandManager* manager = project->commandManager();
    QJsonObject arguments;
    arguments.insert(QStringLiteral("filePath"), gridPath);
    arguments.insert(QStringLiteral("targetCaseId"), caseObject->objectId());
    QVERIFY2(manager->execute(QStringLiteral("grid.import"), arguments)
                 .value(QStringLiteral("success")).toBool(), qPrintable(manager->error()));
    QCOMPARE(project->objects().size(), 5);
    QCOMPARE(caseObject->itemRefs().size(), 4);
    QStringList importedIds;
    QString gridId;
    for (IDOSDataObject* object : project->objects())
    {
        if (qobject_cast<IDOSGridObject*>(object) != nullptr)
        {
            gridId = object->objectId();
        }
        if (object != caseObject)
        {
            importedIds.append(object->objectId());
        }
    }
    QVERIFY(!gridId.isEmpty());
    int properties = 0;
    int summaries = 0;
    for (IDOSDataObject* object : project->objects())
    {
        IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(object);
        if (property != nullptr)
        {
            ++properties;
            QCOMPARE(property->gridId(), gridId);
            QCOMPARE(property->totalCellCount(), 2);
            QCOMPARE(property->values().at(1), 0.0);
            if (property->kind() == IDOSGridPropertyObject::Kind::Dynamic)
            {
                QCOMPARE(property->reportSteps().size(), 2);
                QVERIFY(property->setCurrentReportStep(2));
                QCOMPARE(property->values().at(0), 200.0);
            }
            else
            {
                QCOMPARE(property->values().at(0), 0.25);
            }
        }
        IDOSSimulationSummaryResultsObject* summary = qobject_cast<IDOSSimulationSummaryResultsObject*>(object);
        if (summary != nullptr)
        {
            ++summaries;
            QCOMPARE(summary->timeValues().size(), 1);
        }
    }
    QCOMPARE(properties, 2);
    QCOMPARE(summaries, 1);
    manager->undo();
    QCOMPARE(project->objects().size(), 1);
    QVERIFY(caseObject->itemRefs().isEmpty());
    QVERIFY(QFile::remove(base + QStringLiteral(".INIT")));
    QVERIFY(QFile::remove(base + QStringLiteral(".UNRST")));
    QVERIFY(QFile::remove(base + QStringLiteral(".SMSPEC")));
    QVERIFY(QFile::remove(base + QStringLiteral(".UNSMRY")));
    manager->redo();
    QCOMPARE(project->objects().size(), 5);
    for (const QString& id : importedIds)
    {
        QVERIFY(project->object(id) != nullptr);
    }
    manager->undo();
    QFile broken(base + QStringLiteral(".INIT"));
    QVERIFY(broken.open(QIODevice::WriteOnly));
    broken.write("invalid companion");
    broken.close();
    QVERIFY(!manager->execute(QStringLiteral("grid.import"), arguments)
                 .value(QStringLiteral("success")).toBool());
    QCOMPARE(project->objects().size(), 1);
    QVERIFY(caseObject->itemRefs().isEmpty());
    QVERIFY(manager->canRedo());
    QVERIFY(QFile::remove(broken.fileName()));
    {
        Opm::EclIO::EclOutput output((base + QStringLiteral(".INIT")).toStdString(), false);
        std::vector<int> wrongHeader = header;
        wrongHeader[8] = 1;
        wrongHeader[9] = 2;
        output.write("INTEHEAD", wrongHeader);
        output.write("PORO", std::vector<float>{0.25f});
    }
    // Incompatible optional INIT data is skipped while the valid grid is imported.
    QVERIFY(manager->execute(QStringLiteral("grid.import"), arguments)
                .value(QStringLiteral("success")).toBool());
    QCOMPARE(project->objects().size(), 2);
    manager->undo();
    QCOMPARE(project->objects().size(), 1);
    QVERIFY(manager->canRedo());
    QVERIFY(QFile::remove(base + QStringLiteral(".INIT")));
    {
        Opm::EclIO::EclOutput output((base + QStringLiteral(".INIT")).toStdString(), false);
        output.write("INTEHEAD", header);
        output.write("PORO", std::vector<float>{0.25f});
    }
    QFile brokenRestart(base + QStringLiteral(".UNRST"));
    QVERIFY(brokenRestart.open(QIODevice::WriteOnly));
    brokenRestart.write("invalid restart after valid INIT");
    brokenRestart.close();
    // An unreadable optional restart does not discard the grid and valid INIT property.
    QVERIFY(manager->execute(QStringLiteral("grid.import"), arguments)
                .value(QStringLiteral("success")).toBool());
    QCOMPARE(project->objects().size(), 3);
    manager->undo();
    QCOMPARE(project->objects().size(), 1);
    QVERIFY(caseObject->itemRefs().isEmpty());
    QVERIFY(manager->canRedo());
    QVERIFY(QFile::remove(base + QStringLiteral(".INIT")));
    QVERIFY(QFile::remove(base + QStringLiteral(".UNRST")));
    QFile brokenSummary(base + QStringLiteral(".UNSMRY"));
    QVERIFY(brokenSummary.open(QIODevice::WriteOnly));
    brokenSummary.write("missing specification");
    brokenSummary.close();
    // Orphan UNSMRY cannot be read without SMSPEC and is skipped.
    QVERIFY(manager->execute(QStringLiteral("grid.import"), arguments)
                .value(QStringLiteral("success")).toBool());
    QCOMPARE(project->objects().size(), 2);
    manager->undo();
    QCOMPARE(project->objects().size(), 1);
    QVERIFY(manager->canRedo());
    QVERIFY(QFile::remove(brokenSummary.fileName()));
    QVERIFY2(manager->execute(QStringLiteral("grid.import"), arguments)
                 .value(QStringLiteral("success")).toBool(), qPrintable(manager->error()));
    QCOMPARE(project->objects().size(), 2);
    for (IDOSDataObject* object : project->objects())
    {
        if (qobject_cast<IDOSGridObject*>(object) != nullptr)
        {
            gridId = object->objectId();
        }
    }
    {
        Opm::EclIO::EclOutput output((base + QStringLiteral(".UNRST")).toStdString(), false);
        output.write("SEQNUM", std::vector<int>{3});
        output.write("INTEHEAD", header);
        output.write("PRESSURE", std::vector<float>{300.0f});
    }
    QJsonObject restartArguments;
    restartArguments.insert(QStringLiteral("gridId"), gridId);
    restartArguments.insert(QStringLiteral("filePath"), base + QStringLiteral(".UNRST"));
    QVERIFY2(manager->execute(QStringLiteral("simulation.results.import"), restartArguments)
                 .value(QStringLiteral("success")).toBool(), qPrintable(manager->error()));
    QCOMPARE(project->objects().size(), 3);
    QCOMPARE(manager->result().value(QStringLiteral("reportStepCount")).toInt(), 1);
    QVERIFY(!manager->execute(QStringLiteral("simulation.results.import"), restartArguments)
                 .value(QStringLiteral("success")).toBool());
    QCOMPARE(project->objects().size(), 3);
    manager->undo();
    QCOMPARE(project->objects().size(), 2);
    QVERIFY(QFile::remove(base + QStringLiteral(".UNRST")));
    manager->redo();
    QCOMPARE(project->objects().size(), 3);
}

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
    QVERIFY(registry.registerMetadata(new IDOSImportRestartResultsCommandMetadata()));
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
    IDOSGridObject* grid = new IDOSGridObject();
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
    IDOSGridObject* grid = new IDOSGridObject();
    grid->setDimensions(1, 1, 1);
    project.add(grid);
    IDOSSimulationCaseObject* simulationCase = new IDOSSimulationCaseObject();
    simulationCase->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), grid->objectId()));
    project.add(simulationCase);
    IDOSGridObject* otherGrid = new IDOSGridObject();
    otherGrid->setDimensions(1, 1, 1);
    project.add(otherGrid);
    IDOSSimulationCaseObject* otherCase = new IDOSSimulationCaseObject();
    otherCase->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), otherGrid->objectId()));
    project.add(otherCase);
    IDOSGridPropertyObject* property = new IDOSGridPropertyObject();
    property->setGridId(grid->objectId());
    property->setKeyword(QStringLiteral("SGAS"));
    property->setKind(IDOSGridPropertyObject::Kind::Dynamic);
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
    QVERIFY(project.setCurrentCaseId(otherCase->objectId()));
    playback.setCurrentObject(grid);
    QCOMPARE(spy.size(), 0);
    QVERIFY(project.setCurrentCaseId(simulationCase->objectId()));
    playback.setCurrentObject(grid);
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
    QVERIFY(window.commandRegistry()->metadata(QStringLiteral("pointset.create")) != nullptr);
    QScopedPointer<QMenu> blankMenu(window.dataTreeView()->createContextMenu(QModelIndex()));
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
    const QModelIndex index = window.dataTreeModel()->index(id);
    QVERIFY(index.isValid());
    QCOMPARE(window.dataTreeModel()->rowCount(index), 8);
    IDOSSimulationCaseObject* caseObject = new IDOSSimulationCaseObject();
    caseObject->setName(QStringLiteral("Point set reference"));
    caseObject->addItemRef(IDOSCaseItemRef(QStringLiteral("case.input"), id));
    project.add(caseObject);
    const QModelIndex referenceIndex = window.caseTreeModel()->index(id);
    QVERIFY(referenceIndex.isValid());
    QCOMPARE(window.caseTreeModel()->rowCount(referenceIndex), 8);
    const QList<IDOSRenderView*> views = window.findChildren<IDOSRenderView*>();
    QVERIFY(!views.isEmpty());
    IDOSRenderView* firstView = views.first();
    window.renderServer()->setChecked(id, Qt::Checked, firstView);
    IDOSRenderObject* firstObject = findRenderObject(window.renderServer(), firstView, id);
    QVERIFY(firstObject != nullptr);
    QCOMPARE(actors(firstObject->provider()).first()->GetMapper()->GetInput()->GetNumberOfPoints(), vtkIdType(8));
    QPersistentModelIndex persistentIndex(index);
    project.object(id)->setName(QStringLiteral("Updated sample"));
    QVERIFY(persistentIndex.isValid());
    QCOMPARE(persistentIndex.data().toString(), QStringLiteral("Updated sample"));
    QCOMPARE(findRenderObject(window.renderServer(), firstView, id), firstObject);

    IDOSRenderView secondView;
    IDOSRenderServer server(window.renderRegistry());
    server.addView(&secondView);
    server.setProject(&project);
    server.setChecked(id, Qt::Checked, &secondView);
    IDOSRenderObject* secondObject = findRenderObject(&server, &secondView, id);
    QVERIFY(secondObject != nullptr);
    QVERIFY(firstObject->id() != secondObject->id());
    const double originalSize = actors(firstObject->provider()).first()->GetProperty()->GetPointSize();
    secondObject->setOpacity(0.5);
    QCOMPARE(actors(secondObject->provider()).first()->GetProperty()->GetOpacity(), 0.5);
    QCOMPARE(firstObject->opacity(), 1.0);
    QCOMPARE(actors(firstObject->provider()).first()->GetProperty()->GetPointSize(), originalSize);
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

    QScopedPointer<QMenu> treeMenu(window.dataTreeView()->createContextMenu(index));
    QAction* deleteAction = treeMenu->findChild<QAction*>(QStringLiteral("deletePointSetAction"));
    QVERIFY(deleteAction != nullptr);
    deleteAction->trigger();
    QVERIFY(project.object(id) == nullptr);
    QVERIFY(findRenderObject(window.renderServer(), firstView, id) == nullptr);
    QVERIFY(findRenderObject(&server, &secondView, id) == nullptr);
    QVERIFY(!window.dataTreeModel()->index(id).isValid());
    manager->undo();
    QVERIFY(project.object(id) != nullptr);
    QVERIFY(findRenderObject(window.renderServer(), firstView, id) != nullptr);
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
    QVERIFY(window.commandRegistry()->metadata(QStringLiteral("pointset.create")) == nullptr);
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

    IDOSGridObject grid;
    grid.setDimensions(1, 1, 1);
    IDOSGridPropertyObject property;
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
    QCOMPARE(legends(provider).first()->GetVisibility(), 1);
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

void CaseWorkflowTest::onPlatformSimulation()
{
    IDOSPluginRegistry* registry = IDOSPluginRegistry::instance();
    QSettings settings(QStringLiteral("pluginRegistry"), QStringLiteral("idos"));
    const QString settingKey = QStringLiteral("/plugins/idossimulationplugin");
    const bool hadSetting = settings.contains(settingKey);
    const QVariant previousSetting = settings.value(settingKey);
    settings.setValue(settingKey, false);
    {
        IDOSMainWindow window;
        QVERIFY(!registry->isLoaded(QStringLiteral("idossimulationplugin")));
        SARibbonCategory* home = window.ribbonBar()->categoryByObjectName(QStringLiteral("idosHomeCategory"));
        QVERIFY(home != nullptr);
        QVERIFY(window.ribbonBar()->categoryByObjectName(QStringLiteral("idosSimulationCategory")) == nullptr);
        SARibbonPanel* simulation = home->panelByObjectName(QStringLiteral("idosHomePanel_simulation"));
        SARibbonPanel* results = home->panelByObjectName(QStringLiteral("idosHomePanel_results"));
        SARibbonPanel* output = home->panelByObjectName(QStringLiteral("idosHomePanel_output"));
        QVERIFY(simulation != nullptr);
        QVERIFY(results != nullptr);
        QVERIFY(output != nullptr);
        const QList<SARibbonPanel*> panels{simulation, results, output};
        const QStringList actionNames{QStringLiteral("simulationRunPreflightAction"),
            QStringLiteral("simulationCurveComparisonAction"), QStringLiteral("simulationCaseInventoryReportAction")};
        for (int index = 0; index < panels.size(); ++index)
        {
            bool found = false;
            for (QAction* action : panels.at(index)->actions())
            {
                if (action->objectName() == actionNames.at(index))
                {
                    found = true;
                    break;
                }
            }
            QVERIFY(found);
        }
        const QVariantList catalog = registry->pluginCatalog(registry->libraryDir().path());
        for (const QVariant& entry : catalog)
        {
            QVERIFY(entry.toMap().value(QStringLiteral("key")).toString() != QStringLiteral("idossimulationplugin"));
        }
        const QList<IDOSRenderView*> views = window.findChildren<IDOSRenderView*>();
        QVERIFY(!views.isEmpty());
        QVERIFY(window.toolRegistry()->metadata(QStringLiteral("idos.view.navigation")) != nullptr);
        registry->unloadAll();
        QVERIFY(window.findChild<QAction*>(QStringLiteral("simulationRunPreflightAction")) != nullptr);
        QVERIFY(window.toolRegistry()->metadata(QStringLiteral("idos.view.navigation")) != nullptr);
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

void CaseWorkflowTest::onDismissImportDialog()
{
    for (QWidget* widget : QApplication::topLevelWidgets())
    {
        QFileDialog* dialog = qobject_cast<QFileDialog*>(widget);
        if (dialog != nullptr && dialog->isVisible())
        {
            setProperty("importDialogs", property("importDialogs").toInt() + 1);
            dialog->reject();
        }
    }
}

void CaseWorkflowTest::onChooseImportMenu()
{
    QMenu* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
    if (menu != nullptr)
    {
        QAction* action = qobject_cast<QAction*>(menu->property("testImportAction").value<QObject*>());
        if (action != nullptr)
        {
            QTest::mouseClick(menu, Qt::LeftButton, Qt::NoModifier, menu->actionGeometry(action).center());
        }
    }
}

void CaseWorkflowTest::onDataTreeImportMenus()
{
    const bool nativeDisabled = QApplication::testAttribute(Qt::AA_DontUseNativeDialogs);
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs, !qEnvironmentVariableIsSet("IDOS_TEST_NATIVE_DIALOGS"));
    IDOSProject project;
    IDOSMainWindow window;
    window.setProject(&project);
    const QString savedProject = qEnvironmentVariable("IDOS_TEST_MENU_PROJECT");
    if (!savedProject.isEmpty())
    {
        QVERIFY(window.openProject(savedProject, false));
    }
    window.show();
    QTest::qWait(50);
    IDOSDataTreeItemModel* model = window.dataTreeModel();
    const QStringList keys{QStringLiteral("data.wells"), QStringLiteral("data.microseismic"),
        QStringLiteral("data.seismic"), QStringLiteral("data.surfaceProperties"),
        QStringLiteral("data.boundaries"), QStringLiteral("data.naturalFractures"),
        QStringLiteral("data.hydraulicFractures"), QStringLiteral("data.fluids"),
        QStringLiteral("data.rockPhysics"), QStringLiteral("data.production")};
    QTimer dismiss;
    dismiss.setInterval(20);
    connect(&dismiss, &QTimer::timeout, this, &CaseWorkflowTest::onDismissImportDialog);
    int tested = 0;
    for (const QString& key : keys)
    {
        IDOSTreeItem* item = model->find(key);
        QVERIFY(item != nullptr);
        QScopedPointer<QMenu> menu(window.dataTreeView()->createContextMenu(model->index(item)));
        QVERIFY(menu != nullptr);
        for (QAction* action : menu->actions())
        {
            if (!action->objectName().startsWith(QStringLiteral("import")) &&
                action->property("fileFilter").toString().isEmpty())
            {
                continue;
            }
            setProperty("importDialogs", 0);
            dismiss.start();
            menu->setProperty("testImportAction", QVariant::fromValue(static_cast<QObject*>(action)));
            QTimer::singleShot(100, this, &CaseWorkflowTest::onChooseImportMenu);
            QTimer::singleShot(5000, menu.data(), &QMenu::close);
            menu->exec(window.mapToGlobal(QPoint(100, 100)));
            dismiss.stop();
            qInfo("IMPORT_MENU key=%s action=%s dialogs=%d", qPrintable(key), qPrintable(action->text()),
                  property("importDialogs").toInt());
            QCOMPARE(property("importDialogs").toInt(), 1);
            ++tested;
        }
    }
    QVERIFY(tested >= 12);
    window.setProject(nullptr);
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs, nativeDisabled);
}

void CaseWorkflowTest::onWellInputMenus()
{
    IDOSProject project;
    IDOSMainWindow window;
    window.setProject(&project);
    IDOSWellObject* well = new IDOSWellObject();
    well->setName(QStringLiteral("Input Well"));
    IDOSWellPath path;
    path.setPoints(QVector<IDOSWellPathPoint>{IDOSWellPathPoint(0.0, 10.0, 20.0, 0.0, 0.0),
        IDOSWellPathPoint(100.0, 11.0, 21.0, -100.0, 100.0)});
    well->setPath(path);
    IDOSWellLogChannel channel(QStringLiteral("GR"));
    channel.setUnit(QStringLiteral("API"));
    channel.setData(QVector<double>{0.0, 100.0}, QVector<double>{45.0, 50.0});
    IDOSWellLogSet logs;
    logs.addChannel(channel);
    well->setLogs(logs);
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
    const QModelIndex root = model->index(well->objectId());
    QVERIFY(root.isValid());
    QCOMPARE(model->rowCount(root), 5);
    QScopedPointer<QMenu> summaryMenu(window.dataTreeView()->createContextMenu(root));
    QAction* summary = summaryMenu->findChild<QAction*>(QStringLiteral("viewWellSummaryAction"));
    QVERIFY(summary != nullptr);
    summary->trigger();
    QDialog* summaryDialog = window.dataTreeView()->findChild<QDialog*>(QStringLiteral("wellInputDataWindow"));
    QVERIFY(summaryDialog != nullptr);
    QCOMPARE(summaryDialog->findChild<QTableWidget*>()->rowCount(), 5);
    delete summaryDialog;

    QScopedPointer<QMenu> trajectoryMenu(window.dataTreeView()->createContextMenu(model->index(0, 0, root)));
    QAction* trajectoryView = trajectoryMenu->findChild<QAction*>(QStringLiteral("viewWellPathAction"));
    QVERIFY(trajectoryView != nullptr);
    QVERIFY(trajectoryMenu->findChild<QAction*>(QStringLiteral("importWellPathAction")) != nullptr);
    trajectoryView->trigger();
    QDialog* trajectoryDialog = window.dataTreeView()->findChild<QDialog*>(QStringLiteral("wellInputDataWindow"));
    QVERIFY(trajectoryDialog != nullptr);
    QCOMPARE(trajectoryDialog->findChild<QTableWidget*>()->rowCount(), 2);
    QCOMPARE(trajectoryDialog->findChild<QTableWidget*>()->columnCount(), 5);
    delete trajectoryDialog;

    QScopedPointer<QMenu> curvesMenu(window.dataTreeView()->createContextMenu(model->index(1, 0, root)));
    QAction* curvesView = curvesMenu->findChild<QAction*>(QStringLiteral("viewWellInputDataAction"));
    QVERIFY(curvesView != nullptr);
    QVERIFY(curvesMenu->findChild<QAction*>(QStringLiteral("importWellCurvesAction")) != nullptr);
    curvesView->trigger();
    QDialog* curvesDialog = window.dataTreeView()->findChild<QDialog*>(QStringLiteral("wellInputDataWindow"));
    QVERIFY(curvesDialog != nullptr);
    QCOMPARE(curvesDialog->findChild<QTableWidget*>()->rowCount(), 2);
    QCOMPARE(curvesDialog->findChild<QTableWidget*>()->columnCount(), 2);
    delete curvesDialog;

    for (int row = 2; row < model->rowCount(root); ++row)
    {
        QScopedPointer<QMenu> menu(window.dataTreeView()->createContextMenu(model->index(row, 0, root)));
        QAction* viewData = menu->findChild<QAction*>(QStringLiteral("viewWellInputDataAction"));
        QVERIFY(viewData != nullptr);
        viewData->trigger();
        QDialog* dialog = window.dataTreeView()->findChild<QDialog*>(QStringLiteral("wellInputDataWindow"));
        QVERIFY(dialog != nullptr);
        QTableWidget* table = dialog->findChild<QTableWidget*>();
        QVERIFY(table != nullptr);
        QCOMPARE(table->rowCount(), row == 4 ? 2 : 1);
        QVERIFY(!table->item(0, 0)->text().isEmpty());
        if (row == 2)
        {
            QCOMPARE(table->item(0, 2)->text(), QStringLiteral("Not mapped"));
        }
        delete dialog;
    }
    const QModelIndex markersIndex = model->index(3, 0, root);
    const QModelIndex markerIndex = model->index(0, 0, markersIndex);
    QScopedPointer<QMenu> markerMenu(window.dataTreeView()->createContextMenu(markerIndex));
    QAction* markerView = markerMenu->findChild<QAction*>(QStringLiteral("viewWellInputDataAction"));
    QVERIFY(markerView != nullptr);
    markerView->trigger();
    QDialog* markerDialog = window.dataTreeView()->findChild<QDialog*>(QStringLiteral("wellInputDataWindow"));
    QVERIFY(markerDialog != nullptr);
    QCOMPARE(markerDialog->findChild<QTableWidget*>()->rowCount(), 1);
    QCOMPARE(markerDialog->findChild<QTableWidget*>()->item(0, 0)->text(), QStringLiteral("Top Sand"));
    delete markerDialog;
    window.setProject(nullptr);
}

void CaseWorkflowTest::onWellSymbolsAndStatusColors()
{
    IDOSWellObject well;
    well.setName(QStringLiteral("Symbol Well"));
    IDOSWellPath path;
    path.setPoints(QVector<IDOSWellPathPoint>{IDOSWellPathPoint(0.0, 0.0, 0.0, 0.0, 0.0),
        IDOSWellPathPoint(200.0, 20.0, 0.0, -200.0, 200.0)});
    well.setPath(path);
    well.setCompletions({IDOSWellCompletion(0, true, 100.0, 50.0), IDOSWellCompletion(0, false, 180.0, 150.0)});
    IDOSWellRenderProvider provider(&well);
    provider.setVisible(true);
    provider.update();
    QCOMPARE(provider.effectiveSymbol(), 3);
    QVariantMap settings = provider.displaySettings();
    settings.insert(QStringLiteral("completionVisible"), true);
    QVERIFY(provider.setDisplaySettings(settings));
    const int lineCounts[28] = {3, 25, 4, 34, 28, 4, 4, 26, 6, 40, 40, 62, 34, 32,
        34, 25, 5, 19, 26, 28, 26, 5, 25, 36, 8, 22, 9, 39};
    const QColor producer(QStringLiteral("#eb1f0d"));
    const QColor injector(QStringLiteral("#2e0ff2"));
    const QColor closed(QStringLiteral("#61ff2e"));
    for (int symbol = 0; symbol < 28; ++symbol)
    {
        settings.insert(QStringLiteral("symbol"), symbol);
        QVERIFY(provider.setDisplaySettings(settings));
        QCOMPARE(provider.effectiveSymbol(), symbol);
        QCOMPARE(actors(&provider).size(), 4);
        vtkPolyDataMapper* mapper = vtkPolyDataMapper::SafeDownCast(actors(&provider).first()->GetMapper());
        QVERIFY(mapper != nullptr);
        mapper->Update();
        QCOMPARE(mapper->GetInput()->GetNumberOfLines(), static_cast<vtkIdType>(lineCounts[symbol]));
        QVERIFY(!IDOSWellRenderProvider::symbolIcon(symbol).isNull());
        const QColor expected = symbol == 15 || symbol == 16 ? injector
            : symbol == 2 || (symbol >= 9 && symbol <= 14) || (symbol >= 19 && symbol <= 22) || symbol >= 26
                ? closed : producer;
        const double* color = actors(&provider).at(1)->GetProperty()->GetColor();
        QVERIFY(std::fabs(color[0] - expected.redF()) < 1.0e-8);
        QVERIFY(std::fabs(color[1] - expected.greenF()) < 1.0e-8);
        QVERIFY(std::fabs(color[2] - expected.blueF()) < 1.0e-8);
        QVERIFY(actors(&provider).at(1)->GetProperty()->GetRenderLinesAsTubes());
    }
    const QStringList statuses{QStringLiteral("producer"), QStringLiteral("injector"),
        QStringLiteral("closed"), QStringLiteral("stopped")};
    const int statusSymbols[4] = {3, 15, 22, 21};
    const QStringList colors{QStringLiteral("#112233"), QStringLiteral("#223344"),
        QStringLiteral("#334455"), QStringLiteral("#445566")};
    const QStringList completionColors{QStringLiteral("#abcdef"), QStringLiteral("#123456"),
        QStringLiteral("#654321"), QStringLiteral("#fedcba")};
    for (int index = 0; index < 4; ++index)
    {
        settings.insert(statuses.at(index) + QStringLiteral("Color"), colors.at(index));
        settings.insert(statuses.at(index) + QStringLiteral("CompletionColor"), completionColors.at(index));
    }
    settings.insert(QStringLiteral("sameCompletionColors"), false);
    for (int index = 0; index < 4; ++index)
    {
        settings.insert(QStringLiteral("symbol"), statusSymbols[index]);
        QVERIFY(provider.setDisplaySettings(settings));
        const QColor expected(colors.at(index));
        const double* color = actors(&provider).at(1)->GetProperty()->GetColor();
        QVERIFY(std::fabs(color[0] - expected.redF()) < 1.0e-8);
        QVERIFY(std::fabs(color[1] - expected.greenF()) < 1.0e-8);
        QVERIFY(std::fabs(color[2] - expected.blueF()) < 1.0e-8);
        const QColor expectedCompletion(completionColors.at(index));
        color = actors(&provider).at(2)->GetProperty()->GetColor();
        QVERIFY(std::fabs(color[0] - expectedCompletion.redF()) < 1.0e-8);
        QVERIFY(std::fabs(color[1] - expectedCompletion.greenF()) < 1.0e-8);
        QVERIFY(std::fabs(color[2] - expectedCompletion.blueF()) < 1.0e-8);
        const QColor expectedClosed(completionColors.at(index == 3 ? 3 : 2));
        color = actors(&provider).at(3)->GetProperty()->GetColor();
        QVERIFY(std::fabs(color[0] - expectedClosed.redF()) < 1.0e-8);
        QVERIFY(std::fabs(color[1] - expectedClosed.greenF()) < 1.0e-8);
        QVERIFY(std::fabs(color[2] - expectedClosed.blueF()) < 1.0e-8);
    }
    const QVariantMap saved = provider.displaySettings();
    settings.insert(QStringLiteral("symbol"), 28);
    QVERIFY(!provider.setDisplaySettings(settings));
    QCOMPARE(provider.displaySettings(), saved);
    settings = saved;
    settings.insert(QStringLiteral("symbol"), -1);
    settings.insert(QStringLiteral("tubes"), false);
    QVERIFY(provider.setDisplaySettings(settings));
    QVERIFY(!actors(&provider).at(1)->GetProperty()->GetRenderLinesAsTubes());
    well.setType(IDOSWellObject::Type::Injector);
    provider.update();
    QCOMPARE(provider.effectiveSymbol(), 15);
    well.setOpen(false);
    provider.update();
    QCOMPARE(provider.effectiveSymbol(), 22);
    IDOSWellHead head;
    head.setSurfaceX(0.0);
    head.setSurfaceY(0.0);
    head.setSymbol(5);
    well.setWellHead(head);
    provider.update();
    QCOMPARE(provider.effectiveSymbol(), 5);
    QCOMPARE(well.wellHead().symbol(), 5);
    settings.insert(QStringLiteral("symbol"), 16);
    QVERIFY(provider.setDisplaySettings(settings));
    QCOMPARE(provider.effectiveSymbol(), 16);
    QCOMPARE(well.wellHead().symbol(), 5);
    IDOSWellRenderProvider other(&well);
    QCOMPARE(other.displaySettings().value(QStringLiteral("symbol")).toInt(), -1);
    QCOMPARE(other.effectiveSymbol(), 5);
}
void CaseWorkflowTest::onWellDisplaySettings()
{
    IDOSProject project;
    IDOSMainWindow window;
    window.setProject(&project);
    IDOSWellObject* well = new IDOSWellObject();
    well->setName(QStringLiteral("Display Well"));
    IDOSWellPath path;
    path.setPoints(QVector<IDOSWellPathPoint>{IDOSWellPathPoint(0.0, 0.0, 0.0, 0.0, 0.0),
        IDOSWellPathPoint(100.0, 0.0, 0.0, -100.0, 100.0),
        IDOSWellPathPoint(200.0, 20.0, 0.0, -200.0, 200.0)});
    well->setPath(path);
    well->setCompletions({IDOSWellCompletion(0, true, 150.0, 110.0),
                         IDOSWellCompletion(1, true, 0.0, 0.0)});
    project.add(well);
    IDOSTreeView* tree = window.dataTreeView();
    IDOSRenderView* firstView = window.renderServer()->currentView();
    QVERIFY(firstView != nullptr);
    window.renderServer()->setChecked(well->objectId(), Qt::Checked, firstView);
    IDOSRenderObject* firstObject = findRenderObject(window.renderServer(), firstView, well->objectId());
    QVERIFY(firstObject != nullptr);
    IDOSWellRenderProvider* firstProvider = dynamic_cast<IDOSWellRenderProvider*>(firstObject->provider());
    QVERIFY(firstProvider != nullptr);
    firstObject->setVisible(true);
    firstView->scene()->notifyChanged(firstObject->id());
    IDOSRenderView secondView;
    IDOSWellRenderProvider* secondProvider = new IDOSWellRenderProvider(well);
    IDOSRenderObject* secondObject = new IDOSRenderObject(secondProvider);
    secondProvider->update();
    QVERIFY(secondView.scene()->add(secondObject));
    const QVariantMap secondSettings = secondProvider->displaySettings();
    IDOSTreeModel* model = window.dataTreeModel();
    const QModelIndex root = model->index(well->objectId());
    QVERIFY(root.isValid());
    QScopedPointer<QMenu> menu(tree->createContextMenu(root));
    QAction* action = menu->findChild<QAction*>(QStringLiteral("wellDisplaySettingsAction"));
    QVERIFY(action != nullptr);
    QVERIFY(action->isEnabled());
    action->trigger();
    QPointer<QWidget> widget(window.findChild<QWidget*>(QStringLiteral("wellDisplaySettingsWindow")));
    QVERIFY(!widget.isNull());
    QComboBox* symbols = widget->findChild<QComboBox*>(QStringLiteral("wellDisplaySymbol"));
    QVERIFY(symbols != nullptr);
    QCOMPARE(symbols->count(), 29);
    for (int index = 0; index < symbols->count(); ++index)
    {
        QVERIFY(!symbols->itemIcon(index).isNull());
    }
    QDoubleSpinBox* width = widget->findChild<QDoubleSpinBox*>(QStringLiteral("wellDisplay_trajectoryWidth"));
    QVERIFY(width != nullptr);
    width->setValue(8.0);
    QCOMPARE(firstProvider->displaySettings().value(QStringLiteral("trajectoryWidth")).toDouble(), 8.0);
    QCOMPARE(secondProvider->displaySettings(), secondSettings);
    QCOMPARE(actors(firstProvider).at(1)->GetProperty()->GetLineWidth(), 8.0f);
    QPushButton* trajectoryColor = widget->findChild<QPushButton*>(QStringLiteral("wellDisplay_trajectoryColor"));
    QVERIFY(trajectoryColor != nullptr);
    QCheckBox* statusColors = widget->findChild<QCheckBox*>(QStringLiteral("wellDisplay_statusColors"));
    QVERIFY(statusColors != nullptr);
    statusColors->setChecked(false);
    trajectoryColor->click();
    QColorDialog* colorDialog = widget->findChild<QColorDialog*>();
    QVERIFY(colorDialog != nullptr);
    colorDialog->setCurrentColor(QColor(QStringLiteral("#123456")));
    colorDialog->accept();
    QCOMPARE(firstProvider->displaySettings().value(QStringLiteral("trajectoryColor")).toString(), QStringLiteral("#123456"));
    QCOMPARE(secondProvider->displaySettings(), secondSettings);
    QCheckBox* clip = widget->findChild<QCheckBox*>(QStringLiteral("wellDisplay_clip"));
    QDoubleSpinBox* minimum = widget->findChild<QDoubleSpinBox*>(QStringLiteral("wellDisplay_minimum"));
    QDoubleSpinBox* maximum = widget->findChild<QDoubleSpinBox*>(QStringLiteral("wellDisplay_maximum"));
    QVERIFY(clip != nullptr && minimum != nullptr && maximum != nullptr);
    clip->setChecked(true);
    minimum->setValue(50.0);
    maximum->setValue(150.0);
    vtkPolyDataMapper* mapper = vtkPolyDataMapper::SafeDownCast(actors(firstProvider).at(1)->GetMapper());
    QVERIFY(mapper != nullptr);
    mapper->Update();
    double bounds[6];
    mapper->GetInput()->GetBounds(bounds);
    QCOMPARE(bounds[4], -150.0);
    QCOMPARE(bounds[5], -50.0);
    const QVariantMap validSettings = firstProvider->displaySettings();
    minimum->setValue(160.0);
    QCOMPARE(firstProvider->displaySettings(), validSettings);
    minimum->setValue(50.0);
    QCheckBox* completions = widget->findChild<QCheckBox*>(QStringLiteral("wellDisplay_completionVisible"));
    QVERIFY(completions != nullptr);
    completions->setChecked(true);
    QCOMPARE(actors(firstProvider).size(), 3);
    QVERIFY(actors(firstProvider).at(2)->GetVisibility() != 0);
    QCheckBox* label = widget->findChild<QCheckBox*>(QStringLiteral("wellDisplay_labelVisible"));
    QVERIFY(label != nullptr);
    label->setChecked(true);
    QCOMPARE(legends(firstProvider).size(), 1);
    QComboBox* shape = widget->findChild<QComboBox*>(QStringLiteral("wellDisplayHeadShape"));
    QVERIFY(shape != nullptr);
    shape->setCurrentIndex(shape->findData(QStringLiteral("sphere")));
    vtkPolyDataMapper* headMapper = vtkPolyDataMapper::SafeDownCast(actors(firstProvider).at(0)->GetMapper());
    QVERIFY(headMapper != nullptr);
    headMapper->Update();
    QVERIFY(headMapper->GetInput()->GetNumberOfPolys() > 0);
    QCheckBox* smooth = widget->findChild<QCheckBox*>(QStringLiteral("wellDisplay_smooth"));
    QVERIFY(smooth != nullptr);
    clip->setChecked(false);
    smooth->setChecked(true);
    mapper = vtkPolyDataMapper::SafeDownCast(actors(firstProvider).at(1)->GetMapper());
    mapper->Update();
    QVERIFY(mapper->GetInput()->GetNumberOfPoints() > path.pointCount());
    const QVariantMap saved = firstProvider->displaySettings();
    firstProvider->setSelected(true);
    firstProvider->setSelected(false);
    firstProvider->update();
    QCOMPARE(firstProvider->displaySettings(), saved);
    QCOMPARE(well->path().pointCount(), 3);
    QCheckBox* visibility = widget->findChild<QCheckBox*>(QStringLiteral("wellDisplay_visible"));
    QDoubleSpinBox* opacity = widget->findChild<QDoubleSpinBox*>(QStringLiteral("wellDisplay_opacity"));
    QVERIFY(visibility != nullptr && opacity != nullptr);
    opacity->setValue(40.0);
    QCOMPARE(firstProvider->opacity(), 0.4);
    visibility->setChecked(false);
    QVERIFY(!firstProvider->isVisible());
    visibility->setChecked(true);
    QVERIFY(firstProvider->isVisible());
    const QString screenshotDirectory = qEnvironmentVariable("IDOS_TEST_SCREENSHOT_DIR");
    if (!screenshotDirectory.isEmpty())
    {
        QApplication::processEvents();
        QVERIFY(widget->grab().save(QDir(screenshotDirectory).filePath(QStringLiteral("well-display-settings.png"))));
        QTabWidget* tabs = widget->findChild<QTabWidget*>();
        tabs->setCurrentIndex(4);
        QApplication::processEvents();
        QVERIFY(widget->grab().save(QDir(screenshotDirectory).filePath(QStringLiteral("well-status-colors.png"))));
        tabs->setCurrentIndex(0);
        shape->setCurrentIndex(shape->findData(QStringLiteral("symbol")));
        symbols->showPopup();
        QApplication::processEvents();
        QWidget* popup = symbols->view()->window();
        QVERIFY(popup != nullptr);
        QVERIFY(popup->grab().save(QDir(screenshotDirectory).filePath(QStringLiteral("well-symbols.png"))));
        symbols->hidePopup();
    }
    QVERIFY(window.renderServer()->setCurrentView(firstView));
    const QModelIndex trajectory = model->index(0, 0, root);
    QScopedPointer<QMenu> pathMenu(tree->createContextMenu(trajectory));
    QAction* pathAction = pathMenu->findChild<QAction*>(QStringLiteral("wellDisplaySettingsAction"));
    QVERIFY(pathAction != nullptr);
    pathAction->trigger();
    widget = window.findChild<QWidget*>(QStringLiteral("wellDisplaySettingsWindow"));
    QVERIFY(!widget.isNull());
    QCOMPARE(widget->findChild<QTabWidget*>()->currentIndex(), 1);
    const QModelIndex completion = model->index(1, 0, root);
    QScopedPointer<QMenu> completionMenu(tree->createContextMenu(completion));
    QVERIFY(completionMenu->findChild<QAction*>(QStringLiteral("wellDisplaySettingsAction")) == nullptr);
    QVERIFY(window.renderServer()->setCurrentView(nullptr));
    QScopedPointer<QMenu> disabledMenu(tree->createContextMenu(root));
    QAction* disabled = disabledMenu->findChild<QAction*>(QStringLiteral("wellDisplaySettingsAction"));
    QVERIFY(disabled != nullptr && !disabled->isEnabled());
    project.remove(well->objectId());
    QTRY_VERIFY(widget.isNull());
    action->trigger();
    QVERIFY(window.findChild<QWidget*>(QStringLiteral("wellDisplaySettingsWindow")) == nullptr);
    IDOSWellObject* nextWell = new IDOSWellObject();
    nextWell->setName(QStringLiteral("Next Well"));
    nextWell->setPath(path);
    project.add(nextWell);
    QVERIFY(window.renderServer()->setCurrentView(firstView));
    QScopedPointer<QMenu> nextMenu(tree->createContextMenu(model->index(nextWell->objectId())));
    QAction* nextAction = nextMenu->findChild<QAction*>(QStringLiteral("wellDisplaySettingsAction"));
    QVERIFY(nextAction != nullptr);
    nextAction->trigger();
    widget = window.findChild<QWidget*>(QStringLiteral("wellDisplaySettingsWindow"));
    QVERIFY(!widget.isNull());
    IDOSRenderObject* nextObject = findRenderObject(window.renderServer(), firstView, nextWell->objectId());
    QVERIFY(nextObject != nullptr);
    firstView->scene()->remove(nextObject->id());
    QTRY_VERIFY(widget.isNull());
    window.renderServer()->addView(&secondView);
    QVERIFY(window.renderServer()->setCurrentView(&secondView));
    IDOSWellRenderProvider* nextProvider = new IDOSWellRenderProvider(nextWell);
    nextProvider->update();
    QVERIFY(secondView.scene()->add(new IDOSRenderObject(nextProvider)));
    QScopedPointer<QMenu> switchMenu(tree->createContextMenu(model->index(nextWell->objectId())));
    QAction* switchAction = switchMenu->findChild<QAction*>(QStringLiteral("wellDisplaySettingsAction"));
    QVERIFY(switchAction != nullptr);
    switchAction->trigger();
    widget = window.findChild<QWidget*>(QStringLiteral("wellDisplaySettingsWindow"));
    QVERIFY(!widget.isNull());
    window.setProject(nullptr);
    QVERIFY(widget.isNull());
}
void CaseWorkflowTest::onGridSettings()
{
    QTranslator translator;
    QVERIFY(translator.load(QCoreApplication::applicationDirPath() + QStringLiteral("/i18n/idos_zh_CN.qm")));
    QVERIFY(QApplication::installTranslator(&translator));
    IDOSProject project;
    IDOSMainWindow window;
    window.setProject(&project);
    IDOSGridObject* grid = new IDOSGridObject();
    grid->setDimensions(1, 1, 1);
    grid->setName(QStringLiteral("Coloring Grid"));
    project.add(grid);
    IDOSGridPropertyObject* property = new IDOSGridPropertyObject();
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
    IDOSRenderView* firstView = window.renderServer()->currentView();
    QVERIFY(firstView != nullptr);
    window.renderServer()->setChecked(grid->objectId(), Qt::Checked, firstView);
    IDOSRenderObject* firstObject = findRenderObject(window.renderServer(), firstView, grid->objectId());
    QVERIFY(firstObject != nullptr);
    IDOSGridRenderProvider* firstProvider = dynamic_cast<IDOSGridRenderProvider*>(firstObject->provider());
    QVERIFY(firstProvider != nullptr);
    IDOSRenderView secondView;
    IDOSGridRenderProvider* secondProvider = new IDOSGridRenderProvider(grid);
    IDOSRenderObject* secondObject = new IDOSRenderObject(secondProvider);
    QVERIFY(secondView.scene()->add(secondObject));
    secondProvider->setProperty(property);
    const QVariantMap originalSecondSettings = secondProvider->coloringSettings();
    const QModelIndex index = window.caseTreeModel()->index(property->objectId());
    QVERIFY(index.isValid());
    QScopedPointer<QMenu> menu(tree->createContextMenu(index));
    QVERIFY(menu->findChild<QAction*>(QStringLiteral("deletePropertyAction")) != nullptr);
    QAction* settings = menu->findChild<QAction*>(QStringLiteral("gridSettingsAction"));
    QVERIFY(settings != nullptr);
    QVERIFY(settings->isEnabled());
    settings->trigger();
    QPointer<QWidget> widget(window.findChild<QWidget*>(QStringLiteral("gridSettingsWindow")));
    QVERIFY(!widget.isNull());
    QVERIFY(widget->inherits("IDOSGridSettingWidget"));
    QCOMPARE(menu->findChildren<QAction*>(QStringLiteral("gridSettingsAction")).size(), 1);
    QTabWidget* tabs = widget->findChild<QTabWidget*>(QStringLiteral("gridSettingTabs"));
    QVERIFY(tabs != nullptr);
    QCOMPARE(tabs->count(), 2);
    QCOMPARE(tabs->currentIndex(), 1);
    QCOMPARE(firstProvider->property(), property);
    window.renderServer()->addView(&secondView);
    QVERIFY(window.renderServer()->setCurrentView(&secondView));
    QComboBox* ramp = widget->findChild<QComboBox*>(QStringLiteral("gridSettingRampCombo"));
    QVERIFY(ramp != nullptr);
    QLabel* gridName = widget->findChild<QLabel*>(QStringLiteral("gridSettingGridNameLabel"));
    QLabel* viewName = widget->findChild<QLabel*>(QStringLiteral("gridSettingViewNameLabel"));
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
        vtkLookupTable* lookup = vtkLookupTable::SafeDownCast(actors(firstProvider).first()->GetMapper()->GetLookupTable());
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
    QCheckBox* reverse = widget->findChild<QCheckBox*>(QStringLiteral("gridSettingReverseCheck"));
    QVERIFY(reverse != nullptr);
    ramp->setCurrentIndex(ramp->findData(QStringLiteral("OceanBreeze")));
    reverse->setChecked(true);
    vtkLookupTable* reversedLookup = vtkLookupTable::SafeDownCast(actors(firstProvider).first()->GetMapper()->GetLookupTable());
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
                                              .filePath(QStringLiteral("grid-setting-ramps.png"))));
    ramp->hidePopup();
    ramp->setCurrentIndex(ramp->findData(QStringLiteral("gray")));
    QCOMPARE(firstProvider->coloringSettings().value(QStringLiteral("ramp")).toString(), QStringLiteral("gray"));
    QCOMPARE(secondProvider->coloringSettings(), originalSecondSettings);
    QCheckBox* automatic = widget->findChild<QCheckBox*>(QStringLiteral("gridSettingAutomaticCheck"));
    QDoubleSpinBox* minimum = widget->findChild<QDoubleSpinBox*>(QStringLiteral("gridSettingMinimumSpin"));
    QDoubleSpinBox* maximum = widget->findChild<QDoubleSpinBox*>(QStringLiteral("gridSettingMaximumSpin"));
    QVERIFY(automatic != nullptr && minimum != nullptr && maximum != nullptr);
    automatic->setChecked(false);
    minimum->setValue(0.0);
    maximum->setValue(1.0);
    QCOMPARE(firstProvider->coloringSettings().value(QStringLiteral("minimum")).toDouble(), 0.0);
    QCOMPARE(firstProvider->coloringSettings().value(QStringLiteral("maximum")).toDouble(), 1.0);
    QCOMPARE(actors(firstProvider).first()->GetMapper()->GetScalarRange()[0], 0.0);
    QCOMPARE(actors(firstProvider).first()->GetMapper()->GetScalarRange()[1], 1.0);
    QApplication::processEvents();
    const QString screenshotDirectory = qEnvironmentVariable("IDOS_TEST_SCREENSHOT_DIR", QDir::tempPath());
    QVERIFY(widget->grab().save(QDir(screenshotDirectory).filePath(QStringLiteral("grid-setting-coloring.png"))));
    minimum->setValue(2.0);
    QCOMPARE(firstProvider->coloringSettings().value(QStringLiteral("minimum")).toDouble(), 0.0);
    minimum->setValue(0.0);
    QCheckBox* legend = widget->findChild<QCheckBox*>(QStringLiteral("gridSettingLegendCheck"));
    QVERIFY(legend != nullptr);
    legend->setChecked(false);
    QVERIFY(legends(firstProvider).isEmpty());
    QVERIFY(property->setValues(QVector<double>{std::numeric_limits<double>::quiet_NaN()}));
    QVERIFY(std::isnan(actors(firstProvider).first()->GetMapper()->GetInput()->GetCellData()->GetScalars()->GetTuple1(0)));
    tabs->setCurrentIndex(0);
    QComboBox* mode = widget->findChild<QComboBox*>(QStringLiteral("gridSettingModeCombo"));
    QDoubleSpinBox* opacity = widget->findChild<QDoubleSpinBox*>(QStringLiteral("gridSettingOpacitySpin"));
    QDoubleSpinBox* edgeWidth = widget->findChild<QDoubleSpinBox*>(QStringLiteral("gridSettingEdgeWidthSpin"));
    QCheckBox* edges = widget->findChild<QCheckBox*>(QStringLiteral("gridSettingEdgesCheck"));
    QCheckBox* visible = widget->findChild<QCheckBox*>(QStringLiteral("gridSettingVisibleCheck"));
    QPushButton* edgeColor = widget->findChild<QPushButton*>(QStringLiteral("gridSettingEdgeColorButton"));
    QVERIFY(mode != nullptr && opacity != nullptr && edgeWidth != nullptr && edges != nullptr && visible != nullptr && edgeColor != nullptr);
    for (const IDOSDisplayMode displayMode : QList<IDOSDisplayMode>{IDOSDisplayMode::Wireframe,
        IDOSDisplayMode::Points, IDOSDisplayMode::SurfaceWithEdges, IDOSDisplayMode::Surface})
    {
        mode->setCurrentIndex(mode->findData(static_cast<int>(displayMode)));
        QCOMPARE(firstProvider->displayMode(), displayMode);
        const int representation = displayMode == IDOSDisplayMode::Wireframe ? VTK_WIREFRAME
            : displayMode == IDOSDisplayMode::Points ? VTK_POINTS : VTK_SURFACE;
        QCOMPARE(actors(firstProvider).first()->GetProperty()->GetRepresentation(), representation);
        QCOMPARE(secondProvider->displayMode(), IDOSDisplayMode::Surface);
    }
    edges->setChecked(true);
    QCOMPARE(firstProvider->displayMode(), IDOSDisplayMode::SurfaceWithEdges);
    QCOMPARE(mode->currentData().toInt(), static_cast<int>(IDOSDisplayMode::SurfaceWithEdges));
    edges->setChecked(false);
    QCOMPARE(firstProvider->displayMode(), IDOSDisplayMode::Surface);
    edges->setChecked(true);
    opacity->setValue(65.0);
    edgeWidth->setValue(4.5);
    QCOMPARE(firstProvider->opacity(), 0.65);
    QCOMPARE(firstProvider->edgeWidth(), 4.5);
    QCOMPARE(actors(firstProvider).first()->GetProperty()->GetLineWidth(), 4.5f);
    QCOMPARE(secondProvider->opacity(), 1.0);
    QCOMPARE(secondProvider->edgeWidth(), 1.0);
    edgeColor->click();
    QColorDialog* colorDialog = widget->findChild<QColorDialog*>();
    QVERIFY(colorDialog != nullptr);
    colorDialog->setCurrentColor(QColor(QStringLiteral("#123456")));
    colorDialog->accept();
    QCOMPARE(firstProvider->edgeColor(), QColor(QStringLiteral("#123456")));
    QCOMPARE(secondProvider->edgeColor(), QColor(Qt::black));
    firstProvider->setSelected(true);
    firstProvider->setSelected(false);
    const QColor restored(firstProvider->edgeColor());
    const double* restoredColor = actors(firstProvider).first()->GetProperty()->GetEdgeColor();
    QVERIFY(std::fabs(restoredColor[0] - restored.redF()) < 1.0e-8);
    QVERIFY(std::fabs(restoredColor[1] - restored.greenF()) < 1.0e-8);
    QVERIFY(std::fabs(restoredColor[2] - restored.blueF()) < 1.0e-8);
    QCOMPARE(actors(firstProvider).first()->GetProperty()->GetLineWidth(), 4.5f);
    visible->setChecked(false);
    QVERIFY(!firstProvider->isVisible());
    QCOMPARE(firstProvider->property(), property);
    visible->setChecked(true);
    QVERIFY(firstProvider->isVisible());
    firstProvider->update();
    QCOMPARE(firstProvider->edgeColor(), restored);
    QCOMPARE(firstProvider->edgeWidth(), 4.5);
    QVERIFY(!firstProvider->setEdgeWidth(std::numeric_limits<double>::quiet_NaN()));
    QVERIFY(!firstProvider->setEdgeColor(QColor()));
    QVERIFY(!firstProvider->setDisplayMode(static_cast<IDOSDisplayMode>(99)));
    QCOMPARE(firstProvider->displayMode(), IDOSDisplayMode::SurfaceWithEdges);
    QCOMPARE(firstProvider->edgeWidth(), 4.5);
    QApplication::processEvents();
    QVERIFY(widget->grab().save(QDir(screenshotDirectory).filePath(QStringLiteral("grid-setting-appearance.png"))));
    QVERIFY(window.renderServer()->setCurrentView(firstView));
    const QModelIndex gridIndex = window.caseTreeModel()->index(grid->objectId());
    QVERIFY(gridIndex.isValid());
    QScopedPointer<QMenu> gridMenu(tree->createContextMenu(gridIndex));
    QVERIFY(gridMenu->findChild<QAction*>(QStringLiteral("deleteGridAction")) != nullptr);
    QCOMPARE(project.currentObject(), static_cast<IDOSDataObject*>(grid));
    QCOMPARE(project.currentCaseId(), simulationCase->objectId());
    QAction* gridSettings = gridMenu->findChild<QAction*>(QStringLiteral("gridSettingsAction"));
    QVERIFY(gridSettings != nullptr);
    QCOMPARE(gridMenu->findChildren<QAction*>(QStringLiteral("gridSettingsAction")).size(), 1);
    gridSettings->trigger();
    widget = window.findChild<QWidget*>(QStringLiteral("gridSettingsWindow"));
    QVERIFY(!widget.isNull());
    QCOMPARE(widget->findChild<QTabWidget*>(QStringLiteral("gridSettingTabs"))->currentIndex(), 0);
    QCOMPARE(widget->findChild<QComboBox*>(QStringLiteral("gridSettingPropertyCombo"))->currentData().toString(), property->objectId());
    QCOMPARE(widget->findChild<QDoubleSpinBox*>(QStringLiteral("gridSettingOpacitySpin"))->value(), 65.0);
    QVERIFY(project.remove(property->objectId()));
    QVERIFY(firstProvider->property() == nullptr);
    QComboBox* properties = widget->findChild<QComboBox*>(QStringLiteral("gridSettingPropertyCombo"));
    QVERIFY(properties != nullptr);
    QCOMPARE(properties->currentData().toString(), QString());
    QVERIFY(project.remove(grid->objectId()));
    QTRY_VERIFY(widget.isNull());
    settings->trigger();
    QVERIFY(window.findChild<QWidget*>(QStringLiteral("gridSettingsWindow")) == nullptr);
    QVERIFY(window.renderServer()->setCurrentView(firstView));
    window.setProject(nullptr);
}

void CaseWorkflowTest::onRenderStateAndUndo()
{
    IDOSProject project;
    IDOSRenderProviderRegistry registry;
    QVERIFY(registry.registerMetadata(new IDOSGridRenderMetadata()));
    IDOSRenderServer server(&registry);
    IDOSRenderView first;
    IDOSRenderView second;
    first.setProperty("viewId", QStringLiteral("first"));
    second.setProperty("viewId", QStringLiteral("second"));
    server.addView(&first);
    server.addView(&second);
    server.setProject(&project);
    IDOSGridObject* grid = new IDOSGridObject();
    grid->setDimensions(1, 1, 1);
    QVERIFY(project.add(grid));
    const QString id = grid->objectId();
    QVERIFY(first.scene()->objects().isEmpty());
    server.setChecked(id, Qt::Checked, &first);
    server.setChecked(id, Qt::Checked, &second);
    IDOSRenderObject* firstObject = findRenderObject(&server, &first, id);
    QVERIFY(firstObject != nullptr);
    firstObject->setOpacity(0.35);
    first.setSelection({firstObject->id()});
    server.setChecked(id, Qt::Unchecked, &second);
    const QVariantMap saved = server.saveState();
    QVERIFY(project.commandManager()->execute(new IDOSDeleteGridCommand(&project, id)));
    QVERIFY(first.scene()->objects().isEmpty());
    QVERIFY(second.scene()->objects().isEmpty());
    project.commandManager()->undo();
    QCOMPARE(server.checkState(id, &first), Qt::Checked);
    QCOMPARE(server.checkState(id, &second), Qt::Unchecked);
    firstObject = findRenderObject(&server, &first, id);
    QVERIFY(firstObject != nullptr);
    QCOMPARE(firstObject->opacity(), 0.35);
    QVERIFY(first.selection().contains(firstObject->id()));
    IDOSGridObject* extra = new IDOSGridObject();
    extra->setDimensions(1, 1, 1);
    QVERIFY(project.add(extra));
    server.setChecked(extra->objectId(), Qt::Checked, &first);
    QCOMPARE(first.scene()->objects().size(), 2);
    QVERIFY(server.restoreState(saved));
    QCOMPARE(first.scene()->objects().size(), 1);
    QVERIFY(server.objectIds(&first, extra->objectId()).isEmpty());
    QCOMPARE(server.checkState(id, &first), Qt::Checked);
    QCOMPARE(server.checkState(id, &second), Qt::Unchecked);
    firstObject = findRenderObject(&server, &first, id);
    QCOMPARE(firstObject->opacity(), 0.35);
    const QString renderId = firstObject->id();
    QVariantMap invalid = saved;
    QVariantList entries = invalid.value(QStringLiteral("views")).toList();
    QVariantMap entry = entries.first().toMap();
    QVariantList objects = entry.value(QStringLiteral("objects")).toList();
    QVariantMap object = objects.first().toMap();
    object.insert(QStringLiteral("dataId"), QStringLiteral("missing"));
    objects[0] = object;
    entry.insert(QStringLiteral("objects"), objects);
    entries[0] = entry;
    invalid.insert(QStringLiteral("views"), entries);
    QVERIFY(!server.restoreState(invalid));
    QCOMPARE(findRenderObject(&server, &first, id)->id(), renderId);
    QCOMPARE(first.scene()->objects().size(), 1);
}

void CaseWorkflowTest::onRenderRestoreClipping()
{
    IDOSProject project;
    IDOSRenderProviderRegistry registry;
    QVERIFY(registry.registerMetadata(new IDOSGridRenderMetadata()));
    IDOSRenderServer server(&registry);
    IDOSRenderView view;
    view.setProperty("viewId", QStringLiteral("main"));
    server.addView(&view);
    server.setProject(&project);
    IDOSGridObject* grid = new IDOSGridObject();
    grid->setDimensions(1, 1, 1);
    QVERIFY(project.add(grid));
    server.setChecked(grid->objectId(), Qt::Checked, &view);
    vtkRenderer* renderer = view.vtkWidget()->renderWindow()->GetRenderers()->GetFirstRenderer();
    QVERIFY(renderer != nullptr);
    double bounds[6];
    renderer->ComputeVisiblePropBounds(bounds);
    QVERIFY(bounds[0] <= bounds[1]);
    const double x = (bounds[0] + bounds[1]) * 0.5;
    const double y = (bounds[2] + bounds[3]) * 0.5;
    const double z = (bounds[4] + bounds[5]) * 0.5;
    vtkCamera* camera = renderer->GetActiveCamera();
    camera->SetPosition(x, y, z + 5000.0);
    camera->SetFocalPoint(x, y, z);
    camera->SetViewUp(0.0, 1.0, 0.0);
    renderer->ResetCameraClippingRange();
    const QVariantMap saved = server.saveState();
    const QVariantMap savedCamera = view.saveState().value(QStringLiteral("camera")).toMap();
    view.scene()->clear();
    QVERIFY(server.restoreState(saved));
    QCOMPARE(server.checkState(grid->objectId(), &view), Qt::Checked);
    QCOMPARE(view.saveState().value(QStringLiteral("camera")).toMap(), savedCamera);
    const double* clipping = renderer->GetActiveCamera()->GetClippingRange();
    QVERIFY(clipping[0] < 5000.0);
    QVERIFY(clipping[1] > 5000.0);
}

void CaseWorkflowTest::onRenderLifecycle()
{
    IDOSProject project;
    IDOSRenderProviderRegistry registry;
    IDOSRenderServer server(&registry);
    QScopedPointer<IDOSRenderView> firstView(new IDOSRenderView());
    server.addView(firstView.data());
    registry.registerMetadata(new IDOSGridRenderMetadata());
    server.setProject(&project);
    IDOSGridObject* grid = new IDOSGridObject();
    grid->setDimensions(1, 1, 1);
    project.add(grid);
    QVERIFY(firstView->scene()->objects().isEmpty());
    server.setChecked(grid->objectId(), Qt::Checked, firstView.data());
    IDOSRenderObject* object = findRenderObject(&server, firstView.data(), grid->objectId());
    QVERIFY(object != nullptr);
    object->setVisible(true);
    QVERIFY(object->id() != grid->objectId());
    const QString renderId = object->id();
    IDOSGridRenderProvider* provider = dynamic_cast<IDOSGridRenderProvider*>(object->provider());
    QVERIFY(provider != nullptr);
    QCOMPARE(provider->grid(), grid);
    IDOSGridPropertyObject* property = new IDOSGridPropertyObject();
    property->setDimensions(1, 1, 1);
    property->setGridId(grid->objectId());
    property->setName(QStringLiteral("Porosity"));
    property->setValues(QVector<double>{0.25});
    project.add(property);
    provider->setProperty(property);
    provider->update();
    QCOMPARE(actors(provider).size(), 1);
    QCOMPARE(legends(provider).size(), 1);
    vtkDataArray* scalars = actors(provider).first()->GetMapper()->GetInput()->GetCellData()->GetScalars();
    QVERIFY(scalars != nullptr);
    QCOMPARE(scalars->GetTuple1(0), 0.25);
    vtkActor* gridActor = actors(provider).first();
    vtkDataSet* gridGeometry = gridActor->GetMapper()->GetInput();
    object->setSelected(true);
    QVERIFY(object->isSelected());
    QVERIFY(provider->isSelected());
    QVERIFY(gridActor->GetProperty()->GetEdgeVisibility() != 0);
    object->setOpacity(0.3);
    QCOMPARE(gridActor->GetProperty()->GetOpacity(), 0.3);
    object->setVisible(false);
    QCOMPARE(gridActor->GetVisibility(), 0);
    QVERIFY(legends(provider).isEmpty());
    object->setVisible(true);
    QCOMPARE(gridActor->GetVisibility(), 1);
    QCOMPARE(actors(provider).first(), gridActor);
    QCOMPARE(gridActor->GetMapper()->GetInput(), gridGeometry);
    QScopedPointer<IDOSRenderView> secondViewWidget(new IDOSRenderView());
    IDOSRenderView* secondView = secondViewWidget.data();
    server.addView(secondView);
    QVERIFY(secondView != nullptr);
    QVERIFY(secondView->scene()->objects().isEmpty());
    server.setChecked(grid->objectId(), Qt::Checked, secondView);
    IDOSRenderObject* secondGrid = secondView->scene()->objects().first();
    QVERIFY(secondGrid->provider() != provider);
    firstView->setSelection(QList<QString>{object->id()});
    object->setOpacity(0.4);
    provider->setDisplayMode(IDOSDisplayMode::Wireframe);
    QCOMPARE(secondGrid->opacity(), 1.0);
    QVERIFY(!secondGrid->isSelected());
    QCOMPARE(dynamic_cast<IDOSGridRenderProvider*>(secondGrid->provider())->displayMode(), IDOSDisplayMode::Surface);
    QVERIFY(provider->isVisible());
    QCOMPARE(actors(provider).first(), gridActor);
    QCOMPARE(findRenderObject(&server, firstView.data(), grid->objectId())->id(), renderId);
    QCOMPARE(provider->opacity(), 0.4);
    QCOMPARE(provider->displayMode(), IDOSDisplayMode::Wireframe);
    IDOSGridRenderProvider* secondProvider = dynamic_cast<IDOSGridRenderProvider*>(secondGrid->provider());
    QVERIFY(secondProvider != nullptr);
    secondGrid->setVisible(true);
    secondProvider->setProperty(property);
    property->setValues(QVector<double>{0.5});
    QCOMPARE(actors(provider).first()->GetMapper()->GetInput()->GetCellData()->GetScalars()->GetTuple1(0), 0.5);
    QCOMPARE(actors(secondProvider).first()->GetMapper()->GetInput()->GetCellData()->GetScalars()->GetTuple1(0), 0.5);
    QCOMPARE(provider->opacity(), 0.4);
    QCOMPARE(provider->displayMode(), IDOSDisplayMode::Wireframe);
    QVERIFY(provider->isSelected());
    QCOMPARE(secondProvider->opacity(), 1.0);
    QVERIFY(!secondProvider->isSelected());
    project.beginUpdate();
    property->setValues(QVector<double>{0.75});
    QCOMPARE(actors(provider).first()->GetMapper()->GetInput()->GetCellData()->GetScalars()->GetTuple1(0), 0.5);
    project.endUpdate();
    QCOMPARE(actors(provider).first()->GetMapper()->GetInput()->GetCellData()->GetScalars()->GetTuple1(0), 0.75);
    project.beginUpdate();
    IDOSDataObject* removedProperty = project.take(property->objectId());
    QVERIFY(provider->property() == nullptr);
    QVERIFY(secondProvider->property() == nullptr);
    QVERIFY(legends(provider).isEmpty());
    QVERIFY(legends(secondProvider).isEmpty());
    delete removedProperty;
    project.endUpdate();
    QVERIFY(legends(provider).isEmpty());
    project.beginUpdate();
    const QString gridId = grid->objectId();
    IDOSDataObject* removedGrid = project.take(gridId);
    QVERIFY(findRenderObject(&server, firstView.data(), gridId) == nullptr);
    delete removedGrid;
    project.endUpdate();
    QVERIFY(firstView->scene()->objects().isEmpty());
    QVERIFY(firstView->selection().isEmpty());

    registry.registerMetadata(new IDOSWellRenderMetadata());
    IDOSWellObject* well = new IDOSWellObject();
    IDOSWellHead head;
    head.setSurfaceX(10.0);
    head.setSurfaceY(20.0);
    well->setWellHead(head);
    IDOSWellPath path;
    path.setPoints(QVector<IDOSWellPathPoint>{IDOSWellPathPoint(0.0, 10.0, 20.0, 0.0, 0.0),
                                            IDOSWellPathPoint(100.0, 10.0, 20.0, -100.0, 100.0)});
    well->setPath(path);
    project.add(well);
    server.setChecked(well->objectId(), Qt::Checked, firstView.data());
    IDOSRenderObject* wellObject = findRenderObject(&server, firstView.data(), well->objectId());
    QVERIFY(wellObject != nullptr);
    IDOSWellRenderProvider* wellProvider = dynamic_cast<IDOSWellRenderProvider*>(wellObject->provider());
    QVERIFY(wellProvider != nullptr);
    wellProvider->update();
    QCOMPARE(actors(wellProvider).size(), 2);
    vtkActor* headActor = actors(wellProvider).first();
    vtkActor* pathActor = actors(wellProvider).last();
    wellObject->setSelected(true);
    QCOMPARE(headActor->GetProperty()->GetPointSize(), 18.0f);
    QCOMPARE(pathActor->GetProperty()->GetLineWidth(), 5.0f);
    wellObject->setOpacity(0.5);
    QCOMPARE(headActor->GetProperty()->GetOpacity(), 0.5);
    QCOMPARE(pathActor->GetProperty()->GetOpacity(), 0.5);
    wellObject->setVisible(false);
    QCOMPARE(headActor->GetVisibility(), 0);
    QCOMPARE(pathActor->GetVisibility(), 0);
    wellObject->setSelected(false);
    wellObject->setVisible(true);
    QCOMPARE(headActor->GetProperty()->GetPointSize(), 12.0f);
    QCOMPARE(pathActor->GetProperty()->GetLineWidth(), 2.5f);
    QCOMPARE(actors(wellProvider).first(), headActor);
    QCOMPARE(wellProvider->well(), well);
    QVERIFY(registry.unregisterMetadata(QStringLiteral("idos.well")));
    QVERIFY(firstView->scene()->objects().isEmpty());
}

void CaseWorkflowTest::onRenderArchitecture()
{
    IDOSProject project;
    IDOSRenderProviderRegistry registry;
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
    QCOMPARE(server.currentView(), &first);
    QVERIFY(server.setCurrentView(&second));
    QCOMPARE(server.currentView(), &second);
    QVERIFY(server.setCurrentView(&first));

    IDOSGridObject* grid = new IDOSGridObject();
    grid->setDimensions(1, 1, 1);
    project.add(grid);
    QVERIFY(first.scene()->objects().isEmpty());
    QVERIFY(registry.registerMetadata(new IDOSGridRenderMetadata()));
    const QString dataId = grid->objectId();
    QVERIFY(first.scene()->objects().isEmpty());
    QVERIFY(second.scene()->objects().isEmpty());
    server.setChecked(dataId, Qt::Unchecked, &first);
    QVERIFY(first.scene()->objects().isEmpty());
    server.setChecked(dataId, Qt::Checked, &first);
    server.setChecked(dataId, Qt::Checked, &second);
    const QString firstId = server.objectIds(&first, dataId).first();
    const QString secondId = server.objectIds(&second, dataId).first();
    QVERIFY(firstId != secondId);
    IDOSRenderObject* firstObject = first.scene()->object(firstId);
    IDOSRenderObject* secondObject = second.scene()->object(secondId);
    secondObject->setVisible(true);
    QVERIFY(firstObject != secondObject);
    QVERIFY(firstObject->provider() != secondObject->provider());
    QVERIFY(actors(firstObject->provider()).first() != actors(secondObject->provider()).first());
    QCOMPARE(server.dataObject(&first, firstId), static_cast<IDOSDataObject*>(grid));
    QVERIFY(server.dataObject(&second, firstId) == nullptr);

    QSignalSpy selectionSpy(&first, &IDOSRenderView::selectionChanged);
    first.setSelection(QList<QString>{firstId});
    QCOMPARE(selectionSpy.count(), 1);
    QVERIFY(firstObject->isSelected());
    QVERIFY(!secondObject->isSelected());
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
    QVERIFY(secondObject->isVisible());
    grid->setName(QStringLiteral("Renamed Grid"));
    QCOMPARE(firstObject->name(), QStringLiteral("Renamed Grid"));
    QCOMPARE(secondObject->name(), QStringLiteral("Renamed Grid"));
    QVERIFY(!firstObject->isVisible());
    QCOMPARE(firstObject->opacity(), 0.25);
    QVERIFY(firstObject->isSelected());

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

    IDOSGridObject* replacement = new IDOSGridObject();
    replacement->setDimensions(1, 1, 1);
    project.add(replacement);
    QVERIFY(first.scene()->objects().isEmpty());
    server.setChecked(replacement->objectId(), Qt::Checked, &first);
    QVERIFY(!first.scene()->objects().isEmpty());
    QVERIFY(registry.unregisterMetadata(QStringLiteral("idos.grid")));
    QVERIFY(first.scene()->objects().isEmpty());
    QVERIFY(second.scene()->objects().isEmpty());
    QVERIFY(registry.registerMetadata(new IDOSGridRenderMetadata()));
    QVERIFY(second.scene()->objects().isEmpty());
    server.setChecked(replacement->objectId(), Qt::Checked, &second);
    server.removeView(&first);
    QVERIFY(first.scene()->objects().isEmpty());
    QVERIFY(!second.scene()->objects().isEmpty());
    QCOMPARE(server.views().size(), 1);

    IDOSRenderView* temporaryView = new IDOSRenderView();
    server.addView(temporaryView);
    delete temporaryView;
    QCOMPARE(server.views().size(), 1);
    IDOSProject* temporaryProject = new IDOSProject();
    IDOSGridObject* temporaryGrid = new IDOSGridObject();
    temporaryGrid->setDimensions(1, 1, 1);
    temporaryProject->add(temporaryGrid);
    server.setProject(temporaryProject);
    QVERIFY(second.scene()->objects().isEmpty());
    server.setChecked(temporaryGrid->objectId(), Qt::Checked, &second);
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
    IDOSGridObject* grid = new IDOSGridObject();
    grid->setName(QStringLiteral("B1"));
    project.add(grid);
    IDOSWellObject* well = new IDOSWellObject();
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
        IDOSGridObject* grid = nullptr;
        int staticProperties = 0;
        int regionProperties = 0;
        for (IDOSDataObject* object : objects)
        {
            IDOSGridObject* candidateGrid = qobject_cast<IDOSGridObject*>(object);
            if (candidateGrid != nullptr)
            {
                grid = candidateGrid;
                continue;
            }
            IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(object);
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
        QScopedPointer<IDOSRenderObject> renderObject(new IDOSRenderObject(metadata.create(grid, QList<IDOSDataObject*>())));
        QVERIFY(renderObject != nullptr);
        IDOSRenderProvider* provider = renderObject->provider();
        renderObject->setVisible(true);
        provider->update();
        QCOMPARE(actors(provider).size(), 1);
        vtkDataSet* renderedGrid = actors(provider).first()->GetMapper()->GetInput();
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

void CaseWorkflowTest::onFluidRockReferences()
{
    IDOSMainWindow window;
    IDOSProject* project = new IDOSProject(&window);
    window.setProject(project);
    IDOSSimulationCaseObject* caseObject = new IDOSSimulationCaseObject();
    caseObject->setName(QStringLiteral("Table Case"));
    project->add(caseObject);
    QTemporaryDir directory;
    const QString path = directory.filePath(QStringLiteral("tables.txt"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("PROPS\nPVTW\n100 1 0.00001 0.5 0 /\nSWOF\n0.2 0 1 0\n1 1 0 0 /\n");
    file.close();
    QJsonObject arguments;
    arguments.insert(QStringLiteral("filePath"), path);
    arguments.insert(QStringLiteral("category"), QStringLiteral("fluid"));
    QVERIFY2(project->commandManager()->execute(QStringLiteral("data.fluidRock.import"), arguments)
                 .value(QStringLiteral("success")).toBool(), qPrintable(project->commandManager()->error()));
    QCOMPARE(project->objects().size(), 2);
    QVERIFY(caseObject->itemRefs().isEmpty());
    IDOSFluidRockTableObject* table = nullptr;
    for (IDOSDataObject* object : project->objects())
    {
        IDOSFluidRockTableObject* candidate = qobject_cast<IDOSFluidRockTableObject*>(object);
        if (candidate != nullptr)
        {
            table = candidate;
        }
    }
    QVERIFY(table != nullptr);
    QCOMPARE(table->category(), IDOSFluidRockTableObject::Category::Fluid);
    QCOMPARE(table->tableData().value(QStringLiteral("keyword")).toString(), QStringLiteral("PVTW"));
    arguments = QJsonObject();
    arguments.insert(QStringLiteral("targetCaseId"), caseObject->objectId());
    arguments.insert(QStringLiteral("tableId"), table->objectId());
    QVERIFY(project->commandManager()->execute(QStringLiteral("case.fluidRock.attach"), arguments)
                .value(QStringLiteral("success")).toBool());
    QCOMPARE(caseObject->itemRefs().size(), 1);
    QCOMPARE(caseObject->itemRefs().first().objectId(), table->objectId());
    caseObject->setCurveTables(QVariantList{table->tableData()});
    IDOSTreeModel* model = window.caseTreeModel();
    const QModelIndex caseIndex = model->index(0, 0, QModelIndex());
    const QModelIndex tableGroup = model->index(2, 0, caseIndex);
    QCOMPARE(model->rowCount(tableGroup), 1);
    QVERIFY(!project->commandManager()->execute(QStringLiteral("case.fluidRock.attach"), arguments)
                 .value(QStringLiteral("success")).toBool());
    project->commandManager()->undo();
    QVERIFY(caseObject->itemRefs().isEmpty());
    QCOMPARE(project->objects().size(), 2);
    project->commandManager()->redo();
    QCOMPARE(caseObject->itemRefs().size(), 1);
    project->commandManager()->undo();
    project->commandManager()->undo();
    QCOMPARE(project->objects().size(), 1);
    QVERIFY(QFile::remove(path));
    project->commandManager()->redo();
    QCOMPARE(project->objects().size(), 2);
    QVERIFY(caseObject->itemRefs().isEmpty());
}

void CaseWorkflowTest::onCreateAndImport()
{
    IDOSMainWindow window;
    IDOSTreeView* view = window.caseTreeView();
    IDOSTreeModel* model = window.caseTreeModel();
    QScopedPointer<QMenu> emptyMenu(view->createContextMenu(view->currentIndex()));
    QVERIFY(emptyMenu->actions().isEmpty());
    IDOSProject* project = new IDOSProject(&window);
    window.setProject(project);
    QScopedPointer<QMenu> menu(view->createContextMenu(view->currentIndex()));
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
    IDOSWellObject* well = new IDOSWellObject();
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
        if (model->object(index) == imported)
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

    IDOSGridObject* grid = new IDOSGridObject();
    grid->setName(QStringLiteral("Main Grid"));
    grid->setDimensions(1, 1, 1);
    project->add(grid);
    imported->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), grid->objectId()));
    const QStringList keywords = {QStringLiteral("PORO"), QStringLiteral("SATNUM"), QStringLiteral("PRESSURE")};
    for (int index = 0; index < keywords.size(); ++index)
    {
        IDOSGridPropertyObject* property = new IDOSGridPropertyObject();
        property->setName(keywords.at(index));
        property->setKeyword(keywords.at(index));
        property->setGridId(grid->objectId());
        property->setDimensions(1, 1, 1);
        QVERIFY(property->setValues(QVector<double>{0.2}));
        if (index == 2)
        {
            property->setKind(IDOSGridPropertyObject::Kind::Dynamic);
        }
        project->add(property);
    }
    for (int row = 0; row < model->rowCount(QModelIndex()); ++row)
    {
        const QModelIndex index = model->index(row, 0, QModelIndex());
        if (model->object(index) == imported)
        {
            const QModelIndex grids = model->index(1, 0, index);
            QCOMPARE(model->rowCount(grids), 1);
            const QModelIndex mainGrid = model->index(0, 0, grids);
            QCOMPARE(model->rowCount(mainGrid), 4);
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
            QLineEdit* fileName = fileDialog->findChild<QLineEdit*>(QStringLiteral("fileNameEdit"));
            QVERIFY(fileName != nullptr);
            fileName->setText(QFINDTESTDATA("data/case.DATA"));
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

IDOSRenderObject* CaseWorkflowTest::findRenderObject(IDOSRenderServer* server, IDOSRenderView* view, const QString& dataId) const
{
    for (IDOSRenderObject* object : view->scene()->objects())
    {
        IDOSDataObject* data = server->dataObject(view, object->id());
        if (data != nullptr && data->objectId() == dataId)
        {
            return object;
        }
    }
    return nullptr;
}

void CaseWorkflowTest::onProfileRenderStarted(vtkObject* source, unsigned long eventId, void* callData)
{
    Q_UNUSED(source)
    Q_UNUSED(eventId)
    Q_UNUSED(callData)
    m_profileFrameTimer.start();
    ++m_profileRenderCount;
}

void CaseWorkflowTest::onProfileRenderEnded(vtkObject* source, unsigned long eventId, void* callData)
{
    Q_UNUSED(source)
    Q_UNUSED(eventId)
    Q_UNUSED(callData)
    m_profileRenderNanoseconds += m_profileFrameTimer.nsecsElapsed();
}

void CaseWorkflowTest::onRenderPickingCache()
{
    IDOSGridObject grid;
    QVector<double> coord;
    for (int j = 0; j <= 1; ++j)
    {
        for (int i = 0; i <= 1; ++i)
        {
            coord << i * 10.0 << j * 10.0 << 0.0 << i * 10.0 << j * 10.0 << 10.0;
        }
    }
    grid.setCornerPointData(1, 1, 1, coord, {0, 0, 0, 0, 10, 10, 10, 10}, {1});
    IDOSToolProviderRegistry tools;
    QVERIFY(tools.registerMetadata(new IDOSRenderToolMetadata()));
    IDOSRenderView view;
    view.resize(640, 480);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    IDOSGridRenderProvider* provider = new IDOSGridRenderProvider(&grid);
    provider->update();
    IDOSRenderObject* object = new IDOSRenderObject(provider);
    object->setVisible(true);
    QVERIFY(view.scene()->add(object));
    view.resetCamera();
    vtkRenderWindow* window = view.vtkWidget()->renderWindow();
    vtkRenderer* renderer = window->GetRenderers()->GetFirstRenderer();
    const QPoint center(view.vtkWidget()->width() / 2, view.vtkWidget()->height() / 2);
    double cached[3];
    QVERIFY(view.pickWorldPosition(center, cached));
    vtkSmartPointer<vtkCellPicker> reference = vtkSmartPointer<vtkCellPicker>::New();
    reference->SetTolerance(0.005);
    const int* size = window->GetSize();
    QVERIFY(reference->Pick(static_cast<double>(center.x()) * size[0] / view.vtkWidget()->width(),
                            size[1] - 1.0 - static_cast<double>(center.y()) * size[1] / view.vtkWidget()->height(),
                            0.0, renderer));
    double expected[3];
    reference->GetPickPosition(expected);
    for (int axis = 0; axis < 3; ++axis)
    {
        QVERIFY(qAbs(cached[axis] - expected[axis]) < 1.0e-6);
    }
    reference = nullptr;
    view.setCursorTrackingEnabled(true);
    QTimer* trackingTimer = view.findChild<QTimer*>(QStringLiteral("cursorTrackingTimer"));
    QVERIFY(trackingTimer != nullptr);
    QSignalSpy trackingSpy(trackingTimer, &QTimer::timeout);
    QMouseEvent movement(QEvent::MouseMove, QPointF(center), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    for (int repeat = 0; repeat < 20; ++repeat)
    {
        QCoreApplication::sendEvent(view.vtkWidget(), &movement);
    }
    QTRY_COMPARE(trackingSpy.count(), 1);
    QCoreApplication::sendEvent(view.vtkWidget(), &movement);
    QVERIFY(trackingTimer->isActive());
    view.setCursorTrackingEnabled(false);
    QVERIFY(!trackingTimer->isActive());
    QTest::qWait(60);
    QCOMPARE(trackingSpy.count(), 1);
    view.setCursorTrackingEnabled(true);
    QMouseEvent dragging(QEvent::MouseMove, QPointF(center), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(view.vtkWidget(), &dragging);
    QVERIFY(!trackingTimer->isActive());
    view.setCursorTrackingEnabled(false);
    vtkActor* actor = actors(provider).first();
    vtkUnstructuredGrid* data = vtkUnstructuredGrid::SafeDownCast(actor->GetMapper()->GetInput());
    QVERIFY(data != nullptr);
    vtkWeakPointer<vtkDataSet> oldData = data;
    view.setBoundsVisible(true);
    QCOMPARE(actors(provider).first(), actor);
    QVERIFY(renderer->HasViewProp(actor));
    view.setBoundsVisible(false);
    for (vtkIdType index = 0; index < data->GetNumberOfPoints(); ++index)
    {
        double position[3];
        data->GetPoint(index, position);
        position[2] += 20.0;
        data->GetPoints()->SetPoint(index, position);
    }
    data->GetPoints()->Modified();
    data->Modified();
    view.scene()->notifyChanged(object->id());
    view.resetCamera();
    QVERIFY(view.pickWorldPosition(center, cached));
    QVERIFY(qAbs(cached[2] - expected[2] - 20.0) < 1.0e-5);
    view.setZAxisScale(2.0);
    QVERIFY(view.pickWorldPosition(center, cached));
    QVERIFY(qAbs(cached[2] - (expected[2] + 20.0) * 2.0) < 1.0e-5);
    object->setVisible(false);
    view.scene()->notifyChanged(object->id());
    QVERIFY(!view.pickWorldPosition(center, cached));
    object->setVisible(true);
    view.scene()->notifyChanged(object->id());
    QVERIFY(view.pickWorldPosition(center, cached));
    provider->update();
    view.scene()->notifyChanged(object->id());
    QVERIFY(oldData == nullptr);
    view.resetCamera();
    QVERIFY(view.pickWorldPosition(center, cached));
    IDOSGridRenderProvider* secondProvider = new IDOSGridRenderProvider(&grid);
    secondProvider->update();
    secondProvider->setVisible(true);
    QVERIFY(view.scene()->add(new IDOSRenderObject(secondProvider)));
    IDOSToolBar* toolbar = new IDOSToolBar();
    QVERIFY(view.setToolBar(toolbar));
    QVERIFY(toolbar->setRegistry(&tools));
    QAction* xray = toolbar->findChild<QAction*>(QStringLiteral("viewToolGridXrayAction"));
    QVERIFY(xray != nullptr);
    const unsigned long startTag = window->AddObserver(vtkCommand::StartEvent, this, &CaseWorkflowTest::onProfileRenderStarted);
    m_profileRenderCount = 0;
    xray->trigger();
    QCOMPARE(m_profileRenderCount, 1);
    xray->trigger();
    QCOMPARE(m_profileRenderCount, 2);
    window->RemoveObserver(startTag);
    vtkWeakPointer<vtkDataSet> removedData = actors(provider).first()->GetMapper()->GetInput();
    view.scene()->clear();
    QVERIFY(removedData == nullptr);
    QVERIFY(!view.pickWorldPosition(center, cached));
}

void CaseWorkflowTest::onGridSurfaceTopology()
{
    IDOSGridObject grid;
    QVector<double> coord;
    for (int j = 0; j <= 1; ++j)
    {
        for (int i = 0; i <= 2; ++i)
        {
            coord << i * 10.0 << j * 10.0 << 0.0 << i * 10.0 << j * 10.0 << 2000.0;
        }
    }
    const QVector<double> regular = {1000, 1000, 1000, 1000, 1010, 1010, 1010, 1010,
                                     1000, 1000, 1000, 1000, 1010, 1010, 1010, 1010};
    grid.setCornerPointData(2, 1, 1, coord, regular, {1, 1});
    IDOSGridPropertyObject property;
    property.setGridId(grid.objectId());
    property.setDimensions(2, 1, 1);
    property.setName(QStringLiteral("Topology property"));
    QVERIFY(property.setValues({10.0, 20.0}));
    IDOSGridRenderProvider* provider = new IDOSGridRenderProvider(&grid);
    provider->setProperty(&property);
    provider->update();
    IDOSRenderObject* object = new IDOSRenderObject(provider);
    object->setVisible(true);
    IDOSRenderView view;
    view.resize(640, 480);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    QVERIFY(view.scene()->add(object));
    view.setOrientation(IDOSOrientation::Top);
    view.resetCamera();
    vtkDataSetMapper* mapper = vtkDataSetMapper::SafeDownCast(actors(provider).first()->GetMapper());
    QVERIFY(mapper != nullptr && mapper->GetPolyDataMapper() != nullptr);
    QCOMPARE(mapper->GetInput()->GetNumberOfPoints(), vtkIdType(12));
    QCOMPARE(mapper->GetInput()->GetNumberOfCells(), vtkIdType(2));
    QCOMPARE(mapper->GetPolyDataMapper()->GetInput()->GetNumberOfCells(), vtkIdType(10));
    vtkDataArray* surfaceIds = mapper->GetPolyDataMapper()->GetInput()->GetCellData()->GetArray("GlobalIndex");
    QVERIFY(surfaceIds != nullptr);
    int faceCounts[2] = {0, 0};
    for (vtkIdType face = 0; face < surfaceIds->GetNumberOfTuples(); ++face)
    {
        const int id = static_cast<int>(surfaceIds->GetTuple1(face));
        QVERIFY(id >= 0 && id < 2);
        ++faceCounts[id];
    }
    QCOMPARE(faceCounts[0], 5);
    QCOMPARE(faceCounts[1], 5);
    vtkRenderer* renderer = view.vtkWidget()->renderWindow()->GetRenderers()->GetFirstRenderer();
    const int* renderSize = view.vtkWidget()->renderWindow()->GetSize();
    for (int cell = 0; cell < 2; ++cell)
    {
        renderer->SetWorldPoint(5.0 + cell * 10.0, 5.0, 1000.0, 1.0);
        renderer->WorldToDisplay();
        const double* display = renderer->GetDisplayPoint();
        const QPoint pixel(qRound(display[0] * view.vtkWidget()->width() / renderSize[0]),
                           qRound((renderSize[1] - 1.0 - display[1]) * view.vtkWidget()->height() / renderSize[1]));
        vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
        picker->SetTolerance(0.005);
        QVERIFY(view.pickCell(pixel, picker));
        QCOMPARE(picker->GetDataSet(), mapper->GetInput());
        QCOMPARE(picker->GetDataSet()->GetCellData()->GetArray("GlobalIndex")->GetTuple1(picker->GetCellId()),
                 static_cast<double>(cell));
        QCOMPARE(picker->GetDataSet()->GetCellData()->GetScalars()->GetTuple1(picker->GetCellId()),
                 10.0 + 10.0 * cell);
    }
    vtkDataSet* cachedGeometry = mapper->GetInput();
    vtkActor* cachedActor = actors(provider).first();
    QVariantMap coloring = provider->coloringSettings();
    coloring.insert(QStringLiteral("solidColor"), QStringLiteral("#123456"));
    QVERIFY(provider->setColoringSettings(coloring));
    QCOMPARE(actors(provider).first(), cachedActor);
    QCOMPARE(actors(provider).first()->GetMapper()->GetInput(), cachedGeometry);
    QVERIFY(property.setValues({11.0, 21.0}));
    provider->update();
    QCOMPARE(actors(provider).first(), cachedActor);
    QCOMPARE(actors(provider).first()->GetMapper()->GetInput(), cachedGeometry);
    QCOMPARE(cachedGeometry->GetCellData()->GetScalars()->GetTuple1(1), 21.0);
    provider->setProperty(nullptr);
    QCOMPARE(actors(provider).first()->GetMapper()->GetInput(), cachedGeometry);
    QVERIFY(cachedGeometry->GetCellData()->GetScalars() == nullptr);
    QVERIFY(property.setValues({10.0, 20.0}));
    provider->setProperty(&property);
    // A full fault, a partial shared face, and a sub-float source gap must remain open.
    const QVector<int> pointCounts = {16, 13, 16};
    for (int scenario = 0; scenario < pointCounts.size(); ++scenario)
    {
        QVector<double> corners = regular;
        for (int corner = 0; corner < 8; ++corner)
        {
            if (scenario != 1 || corner == 0)
            {
                corners[8 + corner] += scenario == 2 ? 1.0e-8 : 2.0;
            }
        }
        grid.setCornerPointData(2, 1, 1, coord, corners, {1, 1});
        provider->update();
        view.scene()->notifyChanged(object->id());
        mapper = vtkDataSetMapper::SafeDownCast(actors(provider).first()->GetMapper());
        QCOMPARE(mapper->GetInput()->GetNumberOfPoints(), static_cast<vtkIdType>(pointCounts.at(scenario)));
        QCOMPARE(mapper->GetPolyDataMapper()->GetInput()->GetNumberOfCells(), vtkIdType(12));
    }
    grid.setCornerPointData(2, 1, 1, coord, regular, {1, 0});
    provider->update();
    view.scene()->notifyChanged(object->id());
    mapper = vtkDataSetMapper::SafeDownCast(actors(provider).first()->GetMapper());
    QCOMPARE(mapper->GetInput()->GetNumberOfPoints(), vtkIdType(8));
    QCOMPARE(mapper->GetPolyDataMapper()->GetInput()->GetNumberOfCells(), vtkIdType(6));
    grid.setCornerPointData(2, 1, 1, coord, regular, {1, 1});
    const QVariantMap criteria{{QStringLiteral("axes"), QVariantMap{{QStringLiteral("i"),
        QVariantMap{{QStringLiteral("enabled"), true}, {QStringLiteral("minimum"), 2},
                    {QStringLiteral("maximum"), 2}}}}}};
    QVERIFY(provider->setFilterCriteria(criteria));
    provider->update();
    view.scene()->notifyChanged(object->id());
    mapper = vtkDataSetMapper::SafeDownCast(actors(provider).first()->GetMapper());
    QCOMPARE(mapper->GetInput()->GetNumberOfCells(), vtkIdType(1));
    QCOMPARE(mapper->GetInput()->GetNumberOfPoints(), vtkIdType(8));
    QCOMPARE(mapper->GetPolyDataMapper()->GetInput()->GetNumberOfCells(), vtkIdType(6));
    QCOMPARE(mapper->GetInput()->GetCellData()->GetArray("GlobalIndex")->GetTuple1(0), 1.0);
    QCOMPARE(mapper->GetInput()->GetCellData()->GetScalars()->GetTuple1(0), 20.0);
    QVERIFY(provider->setFilterCriteria(QVariantMap()));
    provider->setProperty(nullptr);
    QVector<double> overlapCoord;
    for (int j = 0; j <= 1; ++j)
    {
        for (int i = 0; i <= 3; ++i)
        {
            const double x = i % 2 == 0 ? 0.0 : 10.0;
            overlapCoord << x << j * 10.0 << 0.0 << x << j * 10.0 << 2000.0;
        }
    }
    QVector<double> overlapCorners = regular;
    overlapCorners += regular.mid(0, 8);
    grid.setCornerPointData(3, 1, 1, overlapCoord, overlapCorners, {1, 0, 1});
    provider->update();
    view.scene()->notifyChanged(object->id());
    mapper = vtkDataSetMapper::SafeDownCast(actors(provider).first()->GetMapper());
    QCOMPARE(mapper->GetInput()->GetNumberOfPoints(), vtkIdType(16));
    QCOMPARE(mapper->GetPolyDataMapper()->GetInput()->GetNumberOfCells(), vtkIdType(12));
    grid.setCornerPointData(1, 1, 1, coord.mid(0, 12) + coord.mid(18, 12),
                            QVector<double>(8, 1000.0), {1});
    provider->update();
    view.scene()->notifyChanged(object->id());
    mapper = vtkDataSetMapper::SafeDownCast(actors(provider).first()->GetMapper());
    QCOMPARE(mapper->GetInput()->GetNumberOfCells(), vtkIdType(1));
    QCOMPARE(mapper->GetInput()->GetNumberOfPoints(), vtkIdType(8));
}

void CaseWorkflowTest::onRenderPerformance()
{
    if (!qEnvironmentVariableIsSet("IDOS_RENDER_PROFILE_SIDE") &&
        !qEnvironmentVariableIsSet("IDOS_RENDER_PROFILE_GRID"))
    {
        QSKIP("Set IDOS_RENDER_PROFILE_SIDE to run the rendering diagnostic.");
    }
    const int side = qEnvironmentVariableIsSet("IDOS_RENDER_PROFILE_SIDE")
        ? qEnvironmentVariableIntValue("IDOS_RENDER_PROFILE_SIDE") : 40;
    QVERIFY(side > 0 && side <= 200);
    const int depth = 20;
    IDOSGridObject grid;
    QVector<double> coordinates;
    for (int j = 0; j <= side; ++j)
    {
        for (int i = 0; i <= side; ++i)
        {
            coordinates << i * 10.0 << j * 10.0 << 0.0 << i * 10.0 << j * 10.0 << depth * 10.0;
        }
    }
    QVector<double> corners;
    corners.reserve(side * side * depth * 8);
    for (int k = 0; k < depth; ++k)
    {
        for (int cell = 0; cell < side * side; ++cell)
        {
            for (int corner = 0; corner < 8; ++corner)
            {
                corners.append((k + (corner >= 4 ? 1 : 0)) * 10.0);
            }
        }
    }
    grid.setCornerPointData(side, side, depth, coordinates, corners, QVector<int>(side * side * depth, 1));
    const QString gridPath = qEnvironmentVariable("IDOS_RENDER_PROFILE_GRID");
    QTemporaryDir inputDirectory;
    if (!gridPath.isEmpty())
    {
        QVERIFY(inputDirectory.isValid());
        const QString isolatedPath = inputDirectory.filePath(QFileInfo(gridPath).fileName());
        QVERIFY(QFile::copy(gridPath, isolatedPath));
        std::unique_ptr<IDOSDataProvider> reader =
            IDOSDataProviderRegistry::instance().create(QStringLiteral("idos.grid.eclipse"));
        QVERIFY(reader != nullptr);
        QElapsedTimer readTimer;
        readTimer.start();
        const QList<IDOSDataObject*> objects = reader->read(isolatedPath);
        qInfo("RENDER_PROFILE phase=file-read elapsed_ms=%.3f", readTimer.nsecsElapsed() / 1000000.0);
        QVERIFY2(!objects.isEmpty(), qPrintable(reader->lastError()));
        IDOSGridObject* imported = qobject_cast<IDOSGridObject*>(objects.first());
        QVERIFY(imported != nullptr);
        grid.setCornerPointData(imported->nx(), imported->ny(), imported->nz(),
                                imported->coord(), imported->zcorn(), imported->actnum());
        qDeleteAll(objects);
    }
    IDOSToolProviderRegistry tools;
    QVERIFY(tools.registerMetadata(new IDOSRenderToolMetadata()));
    IDOSRenderView view;
    view.resize(960, 720);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    vtkRenderWindow* window = view.vtkWidget()->renderWindow();
    vtkRenderer* renderer = window->GetRenderers()->GetFirstRenderer();
    qInfo("RENDER_PROFILE backend=%s", renderer->GetClassName());
    m_profileRenderCount = 0;
    m_profileRenderNanoseconds = 0;
    const unsigned long startTag = window->AddObserver(vtkCommand::StartEvent, this, &CaseWorkflowTest::onProfileRenderStarted);
    const unsigned long endTag = window->AddObserver(vtkCommand::EndEvent, this, &CaseWorkflowTest::onProfileRenderEnded);
    IDOSGridRenderProvider* provider = new IDOSGridRenderProvider(&grid);
    QElapsedTimer elapsed;
    elapsed.start();
    provider->update();
    qInfo("RENDER_PROFILE phase=provider-build cells=%lld elapsed_ms=%.3f", static_cast<long long>(grid.totalCellCount()),
          elapsed.nsecsElapsed() / 1000000.0);
    vtkActor* originalActor = actors(provider).first();
    vtkDataSet* originalGeometry = originalActor->GetMapper()->GetInput();
    elapsed.restart();
    provider->update();
    qInfo("RENDER_PROFILE phase=provider-reuse elapsed_ms=%.3f", elapsed.nsecsElapsed() / 1000000.0);
    QCOMPARE(actors(provider).first(), originalActor);
    QCOMPARE(originalActor->GetMapper()->GetInput(), originalGeometry);
    IDOSRenderObject* object = new IDOSRenderObject(provider);
    object->setVisible(true);
    elapsed.restart();
    QVERIFY(view.scene()->add(object));
    view.resetCamera();
    qInfo("RENDER_PROFILE phase=scene-first-draw elapsed_ms=%.3f frames=%d render_ms=%.3f",
          elapsed.nsecsElapsed() / 1000000.0, m_profileRenderCount, m_profileRenderNanoseconds / 1000000.0);
    vtkDataSetMapper* mapper = vtkDataSetMapper::SafeDownCast(actors(provider).first()->GetMapper());
    QVERIFY(mapper != nullptr && mapper->GetPolyDataMapper() != nullptr);
    vtkPolyData* surface = mapper->GetPolyDataMapper()->GetInput();
    QVERIFY(surface != nullptr);
    qInfo("RENDER_PROFILE input_points=%lld surface_cells=%lld exterior_faces=%d",
          static_cast<long long>(mapper->GetInput()->GetNumberOfPoints()),
          static_cast<long long>(surface->GetNumberOfCells()), gridPath.isEmpty() ? 2 * (side * side + 2 * side * depth) : -1);
    qInfo("RENDER_PROFILE graphics=%s", window->ReportCapabilities());
    for (int repeat = 0; repeat < 3; ++repeat)
    {
        elapsed.restart();
        view.render();
        qInfo("RENDER_PROFILE phase=direct-render repeat=%d elapsed_ms=%.3f", repeat,
              elapsed.nsecsElapsed() / 1000000.0);
    }
    elapsed.restart();
    for (int repeat = 0; repeat < 100; ++repeat)
    {
        view.setSelection(QList<QString>());
    }
    qInfo("RENDER_PROFILE phase=unchanged-selection-100 elapsed_ms=%.3f", elapsed.nsecsElapsed() / 1000000.0);
    IDOSToolBar* toolbar = new IDOSToolBar();
    QVERIFY(view.setToolBar(toolbar));
    QVERIFY(toolbar->setRegistry(&tools));
    const QStringList actions = {QStringLiteral("viewToolFitAllAction"), QStringLiteral("viewToolRotateClockwiseAction"),
        QStringLiteral("viewToolProjectionAction"), QStringLiteral("viewToolAxesAction"),
        QStringLiteral("viewToolBoundsAction"), QStringLiteral("viewToolGridXrayAction")};
    for (const QString& actionName : actions)
    {
        QAction* action = toolbar->findChild<QAction*>(actionName);
        QVERIFY(action != nullptr);
        for (int repeat = 0; repeat < 2; ++repeat)
        {
            m_profileRenderCount = 0;
            m_profileRenderNanoseconds = 0;
            elapsed.restart();
            action->trigger();
            qInfo("RENDER_PROFILE action=%s repeat=%d elapsed_ms=%.3f frames=%d render_ms=%.3f renderer_last_ms=%.3f",
                  qPrintable(actionName), repeat, elapsed.nsecsElapsed() / 1000000.0, m_profileRenderCount,
                  m_profileRenderNanoseconds / 1000000.0, renderer->GetLastRenderTimeInSeconds() * 1000.0);
            elapsed.restart();
            QCoreApplication::processEvents();
            qInfo("RENDER_PROFILE phase=event-drain elapsed_ms=%.3f", elapsed.nsecsElapsed() / 1000000.0);
        }
    }
    elapsed.restart();
    for (int repeat = 0; repeat < 100; ++repeat)
    {
        QVERIFY(QMetaObject::invokeMethod(toolbar, "onViewStateChanged", Qt::DirectConnection));
    }
    qInfo("RENDER_PROFILE phase=toolbar-state-100 elapsed_ms=%.3f", elapsed.nsecsElapsed() / 1000000.0);
    const QVector<int> bucketSizes = {10, 64, 256};
    for (int bucketSize : bucketSizes)
    {
        vtkSmartPointer<vtkStaticCellLocator> locator = vtkSmartPointer<vtkStaticCellLocator>::New();
        locator->SetDataSet(mapper->GetInput());
        locator->SetNumberOfCellsPerNode(bucketSize);
        elapsed.restart();
        locator->BuildLocator();
        qInfo("RENDER_PROFILE phase=locator-build cells_per_bucket=%d elapsed_ms=%.3f", bucketSize,
              elapsed.nsecsElapsed() / 1000000.0);
        vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
        picker->SetTolerance(0.005);
        picker->AddLocator(locator);
        elapsed.restart();
        const int* size = window->GetSize();
        QVERIFY(picker->Pick(size[0] / 2.0, size[1] / 2.0, 0.0, renderer));
        qInfo("RENDER_PROFILE phase=locator-query cells_per_bucket=%d elapsed_ms=%.3f", bucketSize,
              elapsed.nsecsElapsed() / 1000000.0);
    }
    for (int repeat = 0; repeat < 3; ++repeat)
    {
        double position[3];
        elapsed.restart();
        const bool found = view.pickWorldPosition(QPoint(view.vtkWidget()->width() / 2,
                                                        view.vtkWidget()->height() / 2), position);
        qInfo("RENDER_PROFILE phase=cell-pick repeat=%d found=%d elapsed_ms=%.3f", repeat,
              found ? 1 : 0, elapsed.nsecsElapsed() / 1000000.0);
    }
    for (int repeat = 0; repeat < 3; ++repeat)
    {
        vtkSmartPointer<vtkCellPicker> probe = vtkSmartPointer<vtkCellPicker>::New();
        probe->SetTolerance(0.005);
        elapsed.restart();
        QVERIFY(view.pickCell(QPoint(view.vtkWidget()->width() / 2, view.vtkWidget()->height() / 2), probe));
        qInfo("RENDER_PROFILE phase=probe-pick repeat=%d elapsed_ms=%.3f", repeat,
              elapsed.nsecsElapsed() / 1000000.0);
    }
    for (int visible = 0; visible < 2; ++visible)
    {
        toolbar->setVisible(visible != 0);
        QCoreApplication::processEvents();
        for (int repeat = 0; repeat < 3; ++repeat)
        {
            elapsed.restart();
            view.render();
            QCoreApplication::processEvents();
            qInfo("RENDER_PROFILE phase=toolbar-visibility visible=%d repeat=%d elapsed_ms=%.3f", visible, repeat,
                  elapsed.nsecsElapsed() / 1000000.0);
        }
    }
    // Diagnostic comparison only: the application's provider is not changed.
    vtkSmartPointer<vtkStaticCleanUnstructuredGrid> clean = vtkSmartPointer<vtkStaticCleanUnstructuredGrid>::New();
    clean->SetInputData(mapper->GetInput());
    clean->SetTolerance(0.0);
    elapsed.restart();
    clean->Update();
    qInfo("RENDER_PROFILE phase=merge-identical-points elapsed_ms=%.3f points=%lld", elapsed.nsecsElapsed() / 1000000.0,
          static_cast<long long>(clean->GetOutput()->GetNumberOfPoints()));
    mapper->SetInputData(clean->GetOutput());
    for (int repeat = 0; repeat < 3; ++repeat)
    {
        elapsed.restart();
        view.rotateView(true);
        qInfo("RENDER_PROFILE phase=merged-rotate repeat=%d elapsed_ms=%.3f surface_cells=%lld", repeat,
              elapsed.nsecsElapsed() / 1000000.0,
              static_cast<long long>(mapper->GetPolyDataMapper()->GetInput()->GetNumberOfCells()));
    }
    window->RemoveObserver(startTag);
    window->RemoveObserver(endTag);
}

void CaseWorkflowTest::onAcceptToolDialog()
{
    QInputDialog* input = qobject_cast<QInputDialog*>(QApplication::activeModalWidget());
    if (input != nullptr)
    {
        input->setDoubleValue(1.5);
        input->accept();
        return;
    }
    QColorDialog* color = qobject_cast<QColorDialog*>(QApplication::activeModalWidget());
    if (color != nullptr)
    {
        color->setCurrentColor(QColor(32, 64, 96));
        color->accept();
    }
}

void CaseWorkflowTest::onToolbarTools()
{
    IDOSProject project;
    IDOSMainWindow main;
    main.setProject(&project);
    main.resize(1100, 800);
    main.show();
    QVERIFY(QTest::qWaitForWindowExposed(&main));
    IDOSRenderView* view = main.findChild<IDOSRenderView*>();
    for (IDOSRenderView* candidate : main.findChildren<IDOSRenderView*>())
    {
        if (candidate->isVisible())
        {
            view = candidate;
            break;
        }
    }
    QVERIFY(view != nullptr && view->isVisible());
    IDOSGridObject grid;
    const int side = qEnvironmentVariableIsSet("IDOS_RENDER_PROFILE_SIDE")
        ? qEnvironmentVariableIntValue("IDOS_RENDER_PROFILE_SIDE") : 8;
    QVERIFY(side > 0 && side <= 200);
    const int depth = 20;
    QVector<double> coordinates;
    QVector<double> corners;
    for (int j = 0; j <= side; ++j)
    {
        for (int i = 0; i <= side; ++i)
        {
            coordinates << i * 10.0 << j * 10.0 << 0.0 << i * 10.0 << j * 10.0 << depth * 10.0;
        }
    }
    for (int k = 0; k < depth; ++k)
    {
        for (int cell = 0; cell < side * side; ++cell)
        {
            for (int corner = 0; corner < 8; ++corner)
            {
                corners.append((k + (corner >= 4 ? 1 : 0)) * 10.0);
            }
        }
    }
    grid.setCornerPointData(side, side, depth, coordinates, corners, QVector<int>(side * side * depth, 1));
    IDOSGridPropertyObject property;
    property.setDimensions(side, side, depth);
    property.setName(QStringLiteral("Porosity"));
    property.setValues(QVector<double>(side * side * depth, 0.2));
    IDOSGridRenderProvider* provider = new IDOSGridRenderProvider(&grid);
    provider->setProperty(&property);
    IDOSRenderObject* object = new IDOSRenderObject(provider);
    object->setVisible(true);
    object->update();
    QVERIFY(view->scene()->add(object));
    view->resetCamera();
    view->setSelection(QList<QString>{object->id()});
    const QStringList navigation = {QStringLiteral("Top"), QStringLiteral("Bottom"),
        QStringLiteral("Front"), QStringLiteral("Back"), QStringLiteral("Left"),
        QStringLiteral("Right"), QStringLiteral("Isometric"), QStringLiteral("Reset"),
        QStringLiteral("FitSelection"), QStringLiteral("FitAll"),
        QStringLiteral("RotateCounterclockwise"), QStringLiteral("RotateClockwise"),
        QStringLiteral("Projection"), QStringLiteral("Axes"), QStringLiteral("Legend"),
        QStringLiteral("CursorTracking"), QStringLiteral("Bounds"), QStringLiteral("GridXray"),
        QStringLiteral("ClearDisplay"), QStringLiteral("Scale"), QStringLiteral("ZScale"),
        QStringLiteral("BackgroundColor"), QStringLiteral("Screenshot")};
    const QImage clipboard = QApplication::clipboard()->image();
    for (const QString& name : navigation)
    {
        QAction* action = view->findChild<QAction*>(QStringLiteral("viewTool") + name + QStringLiteral("Action"));
        QVERIFY2(action != nullptr && action->isEnabled(), qPrintable(name));
        const int repeats = action->isCheckable() || name == QStringLiteral("ClearDisplay") ? 2 : 1;
        for (int repeat = 0; repeat < repeats; ++repeat)
        {
            if (name == QStringLiteral("Scale") || name == QStringLiteral("ZScale") || name == QStringLiteral("BackgroundColor"))
            {
                QTimer::singleShot(100, this, &CaseWorkflowTest::onAcceptToolDialog);
            }
            QElapsedTimer timer;
            timer.start();
            action->trigger();
            qInfo("TOOL_TEST action=%s elapsed_ms=%.3f", qPrintable(name), timer.nsecsElapsed() / 1000000.0);
            QCoreApplication::processEvents();
        }
    }
    QVERIFY(!QApplication::clipboard()->image().isNull());
    QApplication::clipboard()->setImage(clipboard);
    QVERIFY(object->isVisible());
    QCOMPARE(view->zAxisScale(), 1.5);
    QCOMPARE(view->background(), QColor(32, 64, 96));
    const QStringList interactive = {QStringLiteral("PointZoom"), QStringLiteral("PointProbe"),
        QStringLiteral("DistanceMeasurement"), QStringLiteral("Selection"),
        QStringLiteral("ClippingPlane"), QStringLiteral("SliceByPoints")};
    for (const QString& name : interactive)
    {
        view->findChild<QAction*>(QStringLiteral("viewToolTopAction"))->trigger();
        view->resetCamera();
        QAction* action = view->findChild<QAction*>(QStringLiteral("viewTool") + name + QStringLiteral("Action"));
        QVERIFY2(action != nullptr && action->isEnabled(), qPrintable(name));
        action->trigger();
        QVERIFY(action->isChecked());
        const QPoint center(view->vtkWidget()->width() / 2, view->vtkWidget()->height() / 2);
        QElapsedTimer timer;
        timer.start();
        QTest::mouseClick(view->vtkWidget(), Qt::LeftButton, Qt::NoModifier, center);
        QTest::mouseClick(view->vtkWidget(), Qt::LeftButton, Qt::NoModifier, center + QPoint(24, 0));
        qInfo("TOOL_TEST interaction=%s two_clicks_ms=%.3f", qPrintable(name), timer.nsecsElapsed() / 1000000.0);
        if (name == QStringLiteral("SliceByPoints"))
        {
            timer.restart();
            QTest::keyClick(view->vtkWidget(), Qt::Key_Return);
            qInfo("TOOL_TEST phase=slice-execute elapsed_ms=%.3f", timer.nsecsElapsed() / 1000000.0);
            QVERIFY(view->findChild<QDialog*>(QStringLiteral("renderSliceProfileWindow")) != nullptr);
        }
        QTest::keyClick(view->vtkWidget(), Qt::Key_Escape);
        QTRY_VERIFY(!action->isChecked());
        QVERIFY(object->isVisible());
    }
    vtkSmartPointer<vtkCellPicker> restricted = vtkSmartPointer<vtkCellPicker>::New();
    const QPoint pixel(view->vtkWidget()->width() / 2, view->vtkWidget()->height() / 2);
    QVERIFY(view->pickCell(pixel, restricted, object->id()));
    QVERIFY(!view->pickCell(pixel, restricted, QStringLiteral("missing-object")));
    view->setTwoDimensional(true);
    QVERIFY(!view->findChild<QAction*>(QStringLiteral("viewToolClippingPlaneAction"))->isEnabled());
    QVERIFY(!view->findChild<QAction*>(QStringLiteral("viewToolSliceByPointsAction"))->isEnabled());
    view->setTwoDimensional(false);
    IDOSRenderView follower(&main);
    follower.resize(400, 300);
    follower.show();
    QAction* link = view->findChild<QAction*>(QStringLiteral("viewToolLinkedViewAction"));
    QVERIFY(link != nullptr && link->isEnabled());
    link->trigger();
    QVERIFY(follower.isCameraLinked());
    vtkRenderWindow* window = follower.vtkWidget()->renderWindow();
    const unsigned long tag = window->AddObserver(vtkCommand::StartEvent, this, &CaseWorkflowTest::onProfileRenderStarted);
    m_profileRenderCount = 0;
    vtkCamera* camera = view->vtkWidget()->renderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera();
    camera->Azimuth(10.0);
    camera->Elevation(5.0);
    camera->OrthogonalizeViewUp();
    QCOMPARE(m_profileRenderCount, 0);
    QTRY_COMPARE(m_profileRenderCount, 1);
    camera->Azimuth(5.0);
    link->trigger();
    QVERIFY(!follower.isCameraLinked());
    m_profileRenderCount = 0;
    QCoreApplication::processEvents();
    QCOMPARE(m_profileRenderCount, 0);
    window->RemoveObserver(tag);
}

void CaseWorkflowTest::onImportHeartbeat()
{
    ++m_importHeartbeatCount;
}

void CaseWorkflowTest::onBackgroundImport()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const int side = qEnvironmentVariableIsSet("IDOS_IMPORT_PROFILE_SIDE")
        ? qEnvironmentVariableIntValue("IDOS_IMPORT_PROFILE_SIDE") : 8;
    QVERIFY(side > 0 && side <= 200);
    const QString binary = directory.filePath(QStringLiteral("grid.EGRID"));
    const Opm::EclipseGrid reference(side, side, 20, 10.0, 10.0, 10.0);
    reference.save(binary.toStdString(), false, std::vector<Opm::NNCdata>(), Opm::UnitSystem::newMETRIC());
    const QString ascii = directory.filePath(QStringLiteral("grid.GRDECL"));
    QFile file(ascii);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream stream(&file);
    stream << "SPECGRID\n" << side << ' ' << side << " 20 1 F /\nCOORD\n";
    for (int j = 0; j <= side; ++j)
    {
        for (int i = 0; i <= side; ++i)
        {
            stream << i * 10 << ' ' << j * 10 << " 0 " << i * 10 << ' ' << j * 10 << " 200\n";
        }
    }
    stream << "/\nZCORN\n";
    for (int k = 0; k < 20; ++k)
    {
        stream << side * side * 4 << '*' << k * 10 << ' ' << side * side * 4 << '*' << (k + 1) * 10 << '\n';
    }
    stream << "/\nACTNUM\n" << side * side * 20 << "*1 /\nPORO\n" << side * side * 20 << "*0.2 /\n";
    stream.flush();
    file.close();
    std::unique_ptr<IDOSDataProvider> reader = IDOSDataProviderRegistry::instance().create(QStringLiteral("idos.grid.eclipse"));
    QVERIFY(reader != nullptr);
    QTimer heartbeat;
    heartbeat.setInterval(1);
    connect(&heartbeat, &QTimer::timeout, this, &CaseWorkflowTest::onImportHeartbeat);
    heartbeat.start();
    m_importHeartbeatCount = 0;
    QList<IDOSDataObject*> binaryObjects = reader->readInBackground(binary);
    QVERIFY2(!binaryObjects.isEmpty(), qPrintable(reader->lastError()));
    QList<IDOSDataObject*> asciiObjects = reader->readInBackground(ascii);
    QVERIFY2(!asciiObjects.isEmpty(), qPrintable(reader->lastError()));
    heartbeat.stop();
    QVERIFY(m_importHeartbeatCount > 0);
    IDOSGridObject* binaryGrid = qobject_cast<IDOSGridObject*>(binaryObjects.first());
    IDOSGridObject* asciiGrid = qobject_cast<IDOSGridObject*>(asciiObjects.first());
    QVERIFY(binaryGrid != nullptr && asciiGrid != nullptr);
    QCOMPARE(binaryGrid->totalCellCount(), side * side * 20);
    QCOMPARE(binaryGrid->coord(), asciiGrid->coord());
    QCOMPARE(binaryGrid->zcorn(), asciiGrid->zcorn());
    QCOMPARE(binaryGrid->actnum(), asciiGrid->actnum());
    for (IDOSDataObject* object : binaryObjects + asciiObjects)
    {
        QCOMPARE(object->thread(), QThread::currentThread());
        QVERIFY(object->parent() == nullptr);
    }
    QCOMPARE(asciiObjects.size(), 2);
    IDOSGridPropertyObject* porosity = qobject_cast<IDOSGridPropertyObject*>(asciiObjects.last());
    QVERIFY(porosity != nullptr);
    QCOMPARE(porosity->gridId(), asciiGrid->objectId());
    QCOMPARE(porosity->values().first(), 0.2);
    QCOMPARE(porosity->values().last(), 0.2);
    qDeleteAll(binaryObjects);
    qDeleteAll(asciiObjects);
    QVERIFY(reader->readInBackground(directory.filePath(QStringLiteral("missing.EGRID"))).isEmpty());
    QVERIFY(!reader->lastError().isEmpty());

    reader = IDOSDataProviderRegistry::instance().create(QStringLiteral("idos.case.simulation.eclipse"));
    QList<IDOSDataObject*> objects = reader->readInBackground(QFINDTESTDATA("data/case.DATA"));
    QVERIFY2(!objects.isEmpty(), qPrintable(reader->lastError()));
    IDOSProject project;
    QJsonObject arguments;
    // An unavailable path proves adoption and redo do not parse the source a second time.
    arguments.insert(QStringLiteral("filePath"), directory.filePath(QStringLiteral("removed.DATA")));
    arguments.insert(QStringLiteral("caseName"), QStringLiteral("Prepared Case"));
    IDOSImportCaseCommand* command = new IDOSImportCaseCommand(&project, arguments);
    const QString originalId = objects.first()->objectId();
    QVERIFY(command->adoptParsedObjects(objects));
    QVERIFY(objects.isEmpty());
    QVERIFY(project.commandManager()->execute(command));
    QCOMPARE(project.objects().size(), 1);
    QCOMPARE(project.objects().first()->objectId(), originalId);
    QCOMPARE(project.objects().first()->name(), QStringLiteral("Prepared Case"));
    project.commandManager()->undo();
    QVERIFY(project.objects().isEmpty());
    project.commandManager()->redo();
    QCOMPARE(project.objects().first()->objectId(), originalId);
}

void CaseWorkflowTest::onImportPerformance()
{
    const QString path = qEnvironmentVariable("IDOS_IMPORT_PROFILE_FILE");
    if (path.isEmpty())
    {
        QSKIP("Set IDOS_IMPORT_PROFILE_FILE to profile an actual import.");
    }
    const QString id = QFileInfo(path).suffix().compare(QStringLiteral("DATA"), Qt::CaseInsensitive) == 0
        ? QStringLiteral("idos.case.simulation.eclipse") : QStringLiteral("idos.grid.eclipse");
    std::unique_ptr<IDOSDataProvider> reader = IDOSDataProviderRegistry::instance().create(id);
    QVERIFY(reader != nullptr);
    QTimer heartbeat;
    heartbeat.setInterval(1);
    connect(&heartbeat, &QTimer::timeout, this, &CaseWorkflowTest::onImportHeartbeat);
    heartbeat.start();
    m_importHeartbeatCount = 0;
    QElapsedTimer timer;
    timer.start();
    const QList<IDOSDataObject*> objects = reader->readInBackground(path);
    qInfo("IMPORT_PROFILE actual_file=%s total_ms=%.3f objects=%d heartbeat=%d", qPrintable(path),
          timer.nsecsElapsed() / 1000000.0, objects.size(), m_importHeartbeatCount);
    QVERIFY2(!objects.isEmpty(), qPrintable(reader->lastError()));
    for (IDOSDataObject* object : objects)
    {
        QCOMPARE(object->thread(), QThread::currentThread());
    }
    if (qEnvironmentVariableIsSet("IDOS_IMPORT_PROFILE_VERIFY"))
    {
        for (IDOSDataObject* object : objects)
        {
            IDOSSimulationSummaryResultsObject* summary = qobject_cast<IDOSSimulationSummaryResultsObject*>(object);
            if (summary == nullptr)
            {
                continue;
            }
            Opm::EclIO::ESmry reference(summary->sourceFile().toStdString());
            const std::vector<Opm::time_point> dates = reference.dates();
            QCOMPARE(summary->timeValues().size(), static_cast<int>(dates.size()));
            for (int index = 0; index < summary->timeValues().size(); ++index)
            {
                QCOMPARE(summary->timeValues().at(index), dates[index].time_since_epoch().count());
            }
            for (const QVariantMap& series : summary->series())
            {
                const std::string key = series.value(QStringLiteral("key")).toString().toStdString();
                const std::vector<float>& expected = reference.get(key);
                const QVector<double> actual = series.value(QStringLiteral("values")).value<QVector<double>>();
                QCOMPARE(actual.size(), static_cast<int>(expected.size()));
                QCOMPARE(series.value(QStringLiteral("unit")).toString(), QString::fromStdString(reference.get_unit(key)));
                for (int index = 0; index < actual.size(); ++index)
                {
                    QCOMPARE(actual.at(index), static_cast<double>(expected[index]));
                }
            }
            qInfo("IMPORT_PROFILE verified_series=%d time_steps=%d", summary->series().size(), summary->timeValues().size());
        }
    }
    qDeleteAll(objects);
}

void CaseWorkflowTest::onWaitingDialog()
{
    QWidget parent;
    parent.resize(800, 600);
    parent.show();
    QVERIFY(QTest::qWaitForWindowExposed(&parent));
    IDOSWaitingDialog dialog(&parent);
    const QString detail = QStringLiteral("A_very_long_reservoir_model_with_properties_and_results_2026.EGRID");
    dialog.begin(QStringLiteral("Importing Data"), QStringLiteral("Reading the grid and its properties…"), detail);
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    QVERIFY(dialog.isRunning());
    QCOMPARE(dialog.windowModality(), Qt::ApplicationModal);
    QProgressBar* progress = dialog.findChild<QProgressBar*>(QStringLiteral("waitingProgress"));
    QLabel* file = dialog.findChild<QLabel*>(QStringLiteral("waitingDetail"));
    QLabel* elapsed = dialog.findChild<QLabel*>(QStringLiteral("waitingElapsed"));
    QVERIFY(progress != nullptr && file != nullptr && elapsed != nullptr);
    QCOMPARE(progress->minimum(), 0);
    QCOMPARE(progress->maximum(), 0);
    QCOMPARE(file->toolTip(), detail);
    QTest::keyClick(&dialog, Qt::Key_Escape);
    QVERIFY(dialog.isVisible());
    QVERIFY(!dialog.close());
    dialog.reject();
    dialog.accept();
    QVERIFY(dialog.isRunning() && dialog.isVisible());
    QTRY_VERIFY(elapsed->text().contains(QStringLiteral("1 s")));
    const QString screenshots = qEnvironmentVariable("IDOS_TEST_SCREENSHOT_DIR");
    if (!screenshots.isEmpty())
    {
        QVERIFY(dialog.grab().save(QDir(screenshots).filePath(QStringLiteral("waiting-dialog.png"))));
    }
    dialog.setMessage(QStringLiteral("Updating the project…"));
    QCOMPARE(dialog.findChild<QLabel*>(QStringLiteral("waitingMessage"))->text(), QStringLiteral("Updating the project…"));
    dialog.setProgress(7, 10);
    QCOMPARE(progress->value(), 7);
    QCOMPARE(progress->maximum(), 10);
    dialog.setProgress(20, 10);
    QCOMPARE(progress->value(), 10);
    dialog.setDetail(QString());
    QVERIFY(!file->isVisible());
    dialog.finish();
    QVERIFY(!dialog.isRunning() && !dialog.isVisible());
    dialog.begin(QStringLiteral("Processing"), QStringLiteral("<literal text>"));
    QCOMPARE(progress->maximum(), 0);
    QCOMPARE(dialog.findChild<QLabel*>(QStringLiteral("waitingMessage"))->textFormat(), Qt::PlainText);
    QVERIFY(elapsed->text().contains(QStringLiteral("0 s")));
    dialog.finish();
    QTranslator translator;
    QVERIFY(translator.load(QCoreApplication::applicationDirPath() + QStringLiteral("/i18n/idos_zh_CN.qm")));
    QCoreApplication::installTranslator(&translator);
    const QString title = QCoreApplication::translate("IDOSMainWindow", "Importing Data");
    const QString message = QCoreApplication::translate("IDOSMainWindow", "Reading data and updating the project…");
    IDOSWaitingDialog chinese(&parent);
    chinese.begin(title, message, detail);
    QTest::qWait(300);
    QVERIFY(chinese.findChild<QLabel*>(QStringLiteral("waitingElapsed"))->text().contains(QCoreApplication::translate("IDOSWaitingDialog", "%1 s").arg(0)));
    if (!screenshots.isEmpty())
    {
        QVERIFY(chinese.grab().save(QDir(screenshots).filePath(QStringLiteral("waiting-dialog-zh.png"))));
    }
    chinese.finish();
    const QPalette previous = QApplication::palette();
    QPalette dark = previous;
    dark.setColor(QPalette::Window, QColor(32, 36, 44));
    dark.setColor(QPalette::WindowText, QColor(236, 240, 246));
    dark.setColor(QPalette::Disabled, QPalette::WindowText, QColor(160, 173, 190));
    dark.setColor(QPalette::Midlight, QColor(61, 70, 84));
    dark.setColor(QPalette::Highlight, QColor(87, 155, 245));
    QApplication::setPalette(dark);
    IDOSWaitingDialog darkDialog;
    darkDialog.begin(title, message, detail);
    QTest::qWait(300);
    const QPixmap darkImage = darkDialog.grab();
    darkDialog.finish();
    QApplication::setPalette(previous);
    QCoreApplication::removeTranslator(&translator);
    if (!screenshots.isEmpty())
    {
        QVERIFY(darkImage.save(QDir(screenshots).filePath(QStringLiteral("waiting-dialog-dark.png"))));
    }
}

quint64 CaseWorkflowTest::processPrivateBytes()
{
#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS_EX counters = {};
    if (GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&counters), sizeof(counters)))
    {
        return static_cast<quint64>(counters.PrivateUsage);
    }
#endif
    return 0;
}

void CaseWorkflowTest::onMemorySample()
{
    m_memoryPeak = qMax(m_memoryPeak, processPrivateBytes());
}

void CaseWorkflowTest::readRestartForMemory(IDOSGridPropertyEclipseProvider* provider, const QString& path,
                                             const IDOSGridObject* grid, QThread* targetThread,
                                             QList<IDOSDataObject*>* objects)
{
    *objects = provider->readRestart(path, grid);
    for (IDOSDataObject* object : *objects)
    {
        object->moveToThread(targetThread);
    }
}

void CaseWorkflowTest::onRestartMemory()
{
    const int layers = qEnvironmentVariableIsSet("IDOS_MEMORY_PROFILE") ? 80 : 2;
    const int steps = qEnvironmentVariableIsSet("IDOS_MEMORY_PROFILE") ? 12 : 3;
    const int cells = 100 * 100 * layers;
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("memory.UNRST"));
    QVector<int> active(cells);
    for (int index = 0; index < cells; ++index)
    {
        active[index] = index % 2;
    }
    IDOSGridObject grid;
    grid.setCornerPointData(100, 100, layers, QVector<double>(), QVector<double>(), active);
    {
        Opm::EclIO::EclOutput file(path.toStdString(), false);
        std::vector<int> header(100, 0);
        header[8] = 100;
        header[9] = 100;
        header[10] = layers;
        header[11] = cells / 2;
        for (int step = 0; step < steps; ++step)
        {
            file.write("SEQNUM", std::vector<int>{step});
            file.write("INTEHEAD", header);
            file.write("PRESSURE", std::vector<float>(cells / 2, static_cast<float>(1000 + step)));
            file.write("SWAT", std::vector<double>(cells / 2, step * 0.03125));
            file.write("SGAS", std::vector<float>(cells, static_cast<float>(step * 0.0625)));
        }
    }
    IDOSGridPropertyEclipseProvider provider;
    QList<IDOSDataObject*> objects;
    const quint64 before = processPrivateBytes();
    m_memoryPeak = before;
    QTimer sampler;
    sampler.setInterval(1);
    connect(&sampler, &QTimer::timeout, this, &CaseWorkflowTest::onMemorySample);
    std::unique_ptr<QThread> worker(QThread::create(&CaseWorkflowTest::readRestartForMemory, &provider,
                                                   path, &grid, QThread::currentThread(), &objects));
    QEventLoop loop;
    connect(worker.get(), &QThread::finished, &loop, &QEventLoop::quit);
    sampler.start();
    QElapsedTimer timer;
    timer.start();
    worker->start();
    loop.exec();
    worker->wait();
    sampler.stop();
    onMemorySample();
    qInfo("MEMORY_PROFILE cells=%d steps=%d baseline_mib=%.2f peak_mib=%.2f delta_mib=%.2f loaded_mib=%.2f elapsed_ms=%.3f",
          cells, steps, before / 1048576.0, m_memoryPeak / 1048576.0, (m_memoryPeak - before) / 1048576.0,
          processPrivateBytes() / 1048576.0, timer.nsecsElapsed() / 1000000.0);
    QVERIFY2(provider.lastError().isEmpty(), qPrintable(provider.lastError()));
    QCOMPARE(objects.size(), 3);
    QList<QPointer<IDOSDataObject>> guards;
    for (IDOSDataObject* object : objects)
    {
        guards.append(object);
        QCOMPARE(object->thread(), QThread::currentThread());
        IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(object);
        QVERIFY(property != nullptr);
        QCOMPARE(property->reportSteps().size(), steps);
        QCOMPARE(property->gridId(), grid.objectId());
        for (int step = 0; step < steps; ++step)
        {
            const QVector<double> values = property->reportStepValues(step);
            QCOMPARE(values.size(), cells);
            for (int index = 0; index < cells; ++index)
            {
                const bool full = property->keyword() == QStringLiteral("SGAS");
                const double expected = !full && index % 2 == 0 ? 0.0
                    : property->keyword() == QStringLiteral("PRESSURE") ? 1000.0 + step
                    : full ? step * 0.0625 : step * 0.03125;
                QCOMPARE(values.at(index), expected);
            }
        }
    }
    qDeleteAll(objects);
    for (const QPointer<IDOSDataObject>& guard : guards)
    {
        QVERIFY(guard.isNull());
    }
    qInfo("MEMORY_PROFILE after_delete_mib=%.2f", processPrivateBytes() / 1048576.0);
    // Verify that releasing an INIT source array does not invalidate the next array's index.
    const QString initPath = directory.filePath(QStringLiteral("memory.INIT"));
    {
        Opm::EclIO::EclOutput file(initPath.toStdString(), false);
        std::vector<int> header(100, 0);
        header[8] = 2;
        header[9] = 2;
        header[10] = 2;
        header[11] = 4;
        file.write("INTEHEAD", header);
        file.write("PORO", std::vector<float>(4, 0.25f));
        file.write("PERMX", std::vector<double>(4, 100.0));
        file.write("SATNUM", std::vector<int>(4, 2));
    }
    const QList<IDOSDataObject*> properties = provider.read(initPath);
    QVERIFY2(provider.lastError().isEmpty(), qPrintable(provider.lastError()));
    QCOMPARE(properties.size(), 3);
    for (IDOSDataObject* object : properties)
    {
        IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(object);
        QVERIFY(property != nullptr);
        const double expected = property->keyword() == QStringLiteral("PORO") ? 0.25
            : property->keyword() == QStringLiteral("PERMX") ? 100.0 : 2.0;
        QCOMPARE(property->values().size(), 4);
        for (double value : property->values())
        {
            QCOMPARE(value, expected);
        }
    }
    qDeleteAll(properties);
}

void CaseWorkflowTest::onTreeCheckRendering()
{
    IDOSProject project;
    IDOSMainWindow main;
    main.setProject(&project);
    main.resize(1100, 800);
    main.show();
    QVERIFY(QTest::qWaitForWindowExposed(&main));
    IDOSRenderView* view = nullptr;
    for (IDOSRenderView* candidate : main.findChildren<IDOSRenderView*>())
    {
        if (candidate->isVisible())
        {
            view = candidate;
            break;
        }
    }
    QVERIFY(view != nullptr);
    const int side = qEnvironmentVariableIsSet("IDOS_RENDER_PROFILE_SIDE")
        ? qEnvironmentVariableIntValue("IDOS_RENDER_PROFILE_SIDE") : 8;
    QVERIFY(side > 0 && side <= 200);
    const int depth = 20;
    QVector<double> coordinates;
    QVector<double> corners;
    for (int j = 0; j <= side; ++j)
    {
        for (int i = 0; i <= side; ++i)
        {
            coordinates << i * 10.0 << j * 10.0 << 0.0 << i * 10.0 << j * 10.0 << depth * 10.0;
        }
    }
    for (int k = 0; k < depth; ++k)
    {
        for (int cell = 0; cell < side * side; ++cell)
        {
            for (int corner = 0; corner < 8; ++corner)
            {
                corners.append((k + (corner >= 4 ? 1 : 0)) * 10.0);
            }
        }
    }
    IDOSGridObject* grid = new IDOSGridObject();
    grid->setCornerPointData(side, side, depth, coordinates, corners, QVector<int>(side * side * depth, 1));
    grid->setName(QStringLiteral("Check Grid"));
    project.add(grid);
    IDOSGridPropertyObject* property = new IDOSGridPropertyObject();
    property->setName(QStringLiteral("Porosity"));
    property->setGridId(grid->objectId());
    property->setDimensions(side, side, depth);
    property->setValues(QVector<double>(side * side * depth, 0.25));
    project.add(property);
    IDOSSimulationCaseObject* scenario = new IDOSSimulationCaseObject();
    scenario->setName(QStringLiteral("Check Case"));
    scenario->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), grid->objectId()));
    scenario->addItemRef(IDOSCaseItemRef(QStringLiteral("case.gridProperty"), property->objectId()));
    project.add(scenario);
    IDOSTreeModel* model = main.caseTreeModel();
    const unsigned long tag = view->vtkWidget()->renderWindow()->AddObserver(
        vtkCommand::StartEvent, this, &CaseWorkflowTest::onProfileRenderStarted);
    const QStringList ids = {grid->objectId(), grid->objectId(), grid->objectId(),
        property->objectId(), property->objectId(), grid->objectId(), grid->objectId()};
    const QList<Qt::CheckState> states = {Qt::Checked, Qt::Unchecked, Qt::Checked,
        Qt::Checked, Qt::Unchecked, Qt::Unchecked, Qt::Checked};
    vtkActor* originalActor = nullptr;
    vtkDataSet* originalMesh = nullptr;
    for (int index = 0; index < ids.size(); ++index)
    {
        QCoreApplication::processEvents();
        const QModelIndex node = model->index(ids.at(index));
        QVERIFY(node.isValid());
        m_profileRenderCount = 0;
        QElapsedTimer timer;
        timer.start();
        QVERIFY(model->setData(node, states.at(index), Qt::CheckStateRole));
        qInfo("CHECK_PROFILE step=%d state=%d elapsed_ms=%.3f frames=%d", index, states.at(index),
              timer.nsecsElapsed() / 1000000.0, m_profileRenderCount);
        QCOMPARE(m_profileRenderCount, 1);
        IDOSRenderObject* object = IDOSGridRenderProvider::renderObject(grid->objectId(), view);
        QVERIFY(object != nullptr);
        vtkActor* actor = actors(object->provider()).first();
        if (originalActor == nullptr)
        {
            originalActor = actor;
            originalMesh = actor->GetMapper()->GetInput();
        }
        QCOMPARE(actor, originalActor);
        QCOMPARE(actor->GetMapper()->GetInput(), originalMesh);
        QCOMPARE(object->isVisible(), index != 1 && index != 5);
        if (!object->isVisible())
        {
            vtkRenderer* renderer = view->vtkWidget()->renderWindow()->GetRenderers()->GetFirstRenderer();
            QVERIFY(renderer->HasViewProp(actor));
            QCOMPARE(actor->GetVisibility(), 0);
            const QPoint center(view->vtkWidget()->width() / 2, view->vtkWidget()->height() / 2);
            double position[3];
            QVERIFY(!view->pickWorldPosition(center, position));
            QVERIFY(view->objectIdAt(center).isEmpty());
        }
        IDOSGridRenderProvider* provider = dynamic_cast<IDOSGridRenderProvider*>(object->provider());
        QVERIFY(provider != nullptr);
        QCOMPARE(provider->property(), index == 3 ? property : nullptr);
    }
    m_profileRenderCount = 0;
    main.renderServer()->setChecked(grid->objectId(), Qt::Checked, view);
    QCOMPARE(m_profileRenderCount, 0);
    vtkWeakPointer<vtkActor> actorGuard = originalActor;
    vtkWeakPointer<vtkDataSet> meshGuard = originalMesh;
    IDOSRenderObject* object = IDOSGridRenderProvider::renderObject(grid->objectId(), view);
    QVERIFY(view->scene()->remove(object->id()));
    QVERIFY(actorGuard == nullptr);
    QVERIFY(meshGuard == nullptr);
    view->vtkWidget()->renderWindow()->RemoveObserver(tag);
}

void CaseWorkflowTest::onSyntheticDemoData()
{
    const QString directory = qEnvironmentVariable("IDOS_DEMO_DATA_DIR");
    if (directory.isEmpty())
    {
        QSKIP("Set IDOS_DEMO_DATA_DIR to validate the generated demonstration data.");
    }
    std::unique_ptr<IDOSDataProvider> gridReader = IDOSDataProviderRegistry::instance().create(
        QStringLiteral("idos.grid.eclipse"));
    QVERIFY(gridReader != nullptr);
    const QList<IDOSDataObject*> gridObjects = gridReader->read(directory + QStringLiteral("/SYNTHETIC.GRDECL"));
    QVERIFY2(!gridObjects.isEmpty(), qPrintable(gridReader->lastError()));
    int gridCount = 0;
    int propertyCount = 0;
    for (IDOSDataObject* object : gridObjects)
    {
        IDOSGridObject* grid = qobject_cast<IDOSGridObject*>(object);
        if (grid != nullptr)
        {
            ++gridCount;
            QCOMPARE(grid->nx(), 50);
            QCOMPARE(grid->ny(), 40);
            QCOMPARE(grid->nz(), 20);
            QCOMPARE(grid->activeCellCount(), 39520);
        }
        IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(object);
        if (property != nullptr)
        {
            ++propertyCount;
            QCOMPARE(property->values().size(), 40000);
            for (double value : property->values())
            {
                QVERIFY(std::isfinite(value));
                QVERIFY(value > 0.0);
            }
        }
    }
    QCOMPARE(gridCount, 1);
    QCOMPARE(propertyCount, 5);
    qDeleteAll(gridObjects);
    std::unique_ptr<IDOSDataProvider> headerReader = IDOSDataProviderRegistry::instance().create(
        QStringLiteral("idos.well.header"));
    std::unique_ptr<IDOSDataProvider> pathReader = IDOSDataProviderRegistry::instance().create(
        QStringLiteral("idos.well.path"));
    QVERIFY(headerReader != nullptr);
    QVERIFY(pathReader != nullptr);
    const QList<IDOSDataObject*> headers = headerReader->read(directory + QStringLiteral("/Wellheader.txt"));
    QVERIFY2(headers.size() == 6, qPrintable(headerReader->lastError()));
    for (IDOSDataObject* object : headers)
    {
        IDOSWellObject* header = qobject_cast<IDOSWellObject*>(object);
        QVERIFY(header != nullptr);
        const QList<IDOSDataObject*> paths = pathReader->read(
            directory + QStringLiteral("/wells/") + header->name() + QStringLiteral(".dev"));
        QVERIFY2(paths.size() == 1, qPrintable(pathReader->lastError()));
        IDOSWellObject* well = qobject_cast<IDOSWellObject*>(paths.first());
        QVERIFY(well != nullptr);
        QCOMPARE(well->name(), header->name());
        const QVector<IDOSWellPathPoint> points = well->path().points();
        QCOMPARE(points.size(), 121);
        QCOMPARE(points.first().x(), header->wellHead().surfaceX());
        QCOMPARE(points.first().y(), header->wellHead().surfaceY());
        QCOMPARE(points.first().z(), 0.0);
        for (int index = 1; index < points.size(); ++index)
        {
            QVERIFY(points.at(index).md() > points.at(index - 1).md());
            QCOMPARE(points.at(index).z(), points.at(index).tvd());
        }
        qDeleteAll(paths);
    }
    qDeleteAll(headers);
}

void CaseWorkflowTest::onSubmitProjectCoordinates()
{
    IDOSNewProjectDialog* dialog = qobject_cast<IDOSNewProjectDialog*>(QApplication::activeModalWidget());
    if (dialog != nullptr)
    {
        dialog->findChild<QDoubleSpinBox*>(QStringLiteral("coordinateOriginZ"))->setValue(45.0);
        dialog->onAccepted();
        setProperty("coordinateDialogSubmitted", true);
    }
}

void CaseWorkflowTest::onProjectCoordinates()
{
    QTranslator coordinateTranslator;
    QVERIFY(coordinateTranslator.load(QCoreApplication::applicationDirPath() + QStringLiteral("/i18n/idos_zh_CN.qm")));
    QVERIFY(QApplication::installTranslator(&coordinateTranslator));
    QString error;
    QVector<double> point{1.0, 1.0, 25.0};
    QVERIFY2(IDOSProjectMetadata::transformCrs(QStringLiteral("EPSG:4326"), QStringLiteral("EPSG:3857"), point, error), qPrintable(error));
    QVERIFY(std::abs(point.at(0) - 111319.49079327357) < 1.0e-6);
    QVERIFY(std::abs(point.at(1) - 111325.1428663851) < 1.0e-6);
    QCOMPARE(point.at(2), 25.0);
    QVERIFY(IDOSProjectMetadata::transformCrs(QStringLiteral("EPSG:3857"), QStringLiteral("EPSG:4326"), point, error));
    QVERIFY(std::abs(point.at(0) - 1.0) < 1.0e-10);
    QVERIFY(std::abs(point.at(1) - 1.0) < 1.0e-10);
    QVector<double> earth{0.0, 0.0, 0.0};
    QVERIFY(IDOSProjectMetadata::transformCrs(QStringLiteral("EPSG:4979"), QStringLiteral("EPSG:4978"), earth, error));
    QVERIFY(std::abs(earth.at(0) - 6378137.0) < 1.0e-6);
    QVERIFY(std::abs(earth.at(1)) < 1.0e-6);
    QVERIFY(std::abs(earth.at(2)) < 1.0e-6);
    QVector<double> dynamic{0.0, 0.0, 0.0};
    QVERIFY(!IDOSProjectMetadata::transformCrs(QStringLiteral("EPSG:7912"), QStringLiteral("EPSG:7789"), dynamic, error));
    QVERIFY(IDOSProjectMetadata::transformCrs(QStringLiteral("EPSG:7912"), QStringLiteral("EPSG:7789"), dynamic, error, 2020.0));
    const QVector<double> before = point;
    QVERIFY(!IDOSProjectMetadata::transformCrs(QStringLiteral("EPSG:0"), QStringLiteral("EPSG:4326"), point, error));
    QCOMPARE(point, before);
    QVERIFY(!error.isEmpty());
    QVector<double> invalid{0.0, 100.0, 0.0};
    const QVector<double> invalidBefore = invalid;
    QVERIFY(!IDOSProjectMetadata::transformCrs(QStringLiteral("EPSG:4326"), QStringLiteral("EPSG:3857"), invalid, error));
    QCOMPARE(invalid, invalidBefore);
    QVector<double> missingGrid{0.0, 0.0, 0.0};
    QVERIFY(!IDOSProjectMetadata::transformCrs(QStringLiteral("EPSG:4326+5773"), QStringLiteral("EPSG:4979"), missingGrid, error));
    IDOSProjectMetadata source;
    source.setName(QStringLiteral("Source"));
    source.setCoordinateType(IDOSProjectMetadata::CoordinateType::Local);
    source.setLengthUnit(IDOSProjectMetadata::LengthUnit::Feet);
    source.setZDirection(IDOSProjectMetadata::ZDirection::PositiveDown);
    source.setOrigin(100.0, 200.0, 30.0);
    source.setRotationDegrees(90.0);
    source.setHorizontalScale(2.0);
    IDOSProjectMetadata target;
    target.setName(QStringLiteral("Target"));
    target.setZDirection(IDOSProjectMetadata::ZDirection::PositiveUp);
    target.setCoordinateType(IDOSProjectMetadata::CoordinateType::Local);
    QVector<double> local{10.0, 0.0, 20.0};
    QVERIFY(target.transformCoordinates(source, local, error));
    QVERIFY(std::abs(local.at(0) - 100.0) < 1.0e-10);
    QVERIFY(std::abs(local.at(1) - 206.096) < 1.0e-10);
    QVERIFY(std::abs(local.at(2) - 23.904) < 1.0e-10);
    QVERIFY(source.transformCoordinates(target, local, error));
    QVERIFY(std::abs(local.at(0) - 10.0) < 1.0e-10);
    QVERIFY(std::abs(local.at(1)) < 1.0e-10);
    QVERIFY(std::abs(local.at(2) - 20.0) < 1.0e-10);
    IDOSProjectMetadata restored;
    QVERIFY(restored.restoreProjectData(source.projectData(), error));
    QCOMPARE(restored.projectData(), source.projectData());
    QVariantMap legacy = source.projectData();
    const QStringList newFields{QStringLiteral("lengthUnit"), QStringLiteral("zDirection"), QStringLiteral("originX"),
        QStringLiteral("originY"), QStringLiteral("originZ"), QStringLiteral("rotationDegrees"), QStringLiteral("horizontalScale")};
    for (const QString& field : newFields)
    {
        legacy.remove(field);
    }
    QVERIFY(restored.restoreProjectData(legacy, error));
    QCOMPARE(restored.originX(), 0.0);
    QCOMPARE(restored.horizontalScale(), 1.0);
    target.setCoordinateType(IDOSProjectMetadata::CoordinateType::Custom);
    target.setCoordinateReference(QStringLiteral("EPSG:32650"));
    IDOSNewProjectDialog dialog;
    dialog.setProjectMetadata(target);
    QCOMPARE(dialog.projectMetadata().projectData(), target.projectData());
    dialog.findChild<QLineEdit*>(QStringLiteral("conversionSourceCrs"))->setText(QStringLiteral("EPSG:4326"));
    dialog.findChild<QLineEdit*>(QStringLiteral("conversionInput"))->setText(QStringLiteral("117, 0, 50"));
    dialog.findChild<QPushButton*>(QStringLiteral("convertCoordinates"))->click();
    QVERIFY(dialog.findChild<QLabel*>(QStringLiteral("conversionResult"))->text().contains(QStringLiteral("500000")));
    dialog.findChild<QToolButton*>()->setChecked(true);
    dialog.show();
    QTest::qWait(30);
    QScrollArea* coordinateScroll = dialog.findChild<QScrollArea*>();
    QVERIFY(coordinateScroll != nullptr);
    QVERIFY(coordinateScroll->verticalScrollBar()->maximum() > 0);
    const QString screenshotDirectory = qEnvironmentVariable("IDOS_TEST_SCREENSHOT_DIR");
    if (!screenshotDirectory.isEmpty())
    {
        QVERIFY(dialog.grab().save(screenshotDirectory + QStringLiteral("/project-coordinates.png")));
        coordinateScroll->ensureWidgetVisible(dialog.findChild<QLabel*>(QStringLiteral("conversionResult")));
        QTest::qWait(20);
        QVERIFY(dialog.grab().save(screenshotDirectory + QStringLiteral("/project-coordinate-result.png")));
    }
    dialog.hide();
    QVERIFY(!target.transformCoordinates(source, local, error));
    source.setCoordinateReference(QStringLiteral("EPSG:32650"));
    QVERIFY(target.transformCoordinates(source, local, error));
    source.setVerticalDatum(QStringLiteral("KB"));
    QVERIFY(!target.transformCoordinates(source, local, error));
    IDOSProject project;
    IDOSMainWindow window;
    window.setProject(&project);
    IDOSProjectMetadata original = project.metadata();
    original.setName(QStringLiteral("Coordinate Study"));
    QVERIFY(project.setMetadata(original));
    QJsonObject arguments;
    arguments.insert(QStringLiteral("metadata"), QJsonObject::fromVariantMap(target.projectData()));
    QVERIFY(project.commandManager()->execute(QStringLiteral("project.settings.update"), arguments).value(QStringLiteral("success")).toBool());
    QCOMPARE(project.metadata().coordinateReference(), QStringLiteral("EPSG:32650"));
    QVERIFY(project.commandManager()->canUndo());
    project.commandManager()->undo();
    QCOMPARE(project.metadata().projectData(), original.projectData());
    QVERIFY(project.commandManager()->canRedo());
    project.commandManager()->redo();
    QCOMPARE(project.metadata().coordinateReference(), QStringLiteral("EPSG:32650"));
    QJsonObject query;
    query.insert(QStringLiteral("sourceCrs"), QStringLiteral("EPSG:4326"));
    query.insert(QStringLiteral("targetCrs"), QStringLiteral("EPSG:3857"));
    query.insert(QStringLiteral("xyz"), QJsonArray{1.0, 1.0, 25.0});
    const QString historyText = project.commandManager()->undoText();
    QVERIFY(project.commandManager()->execute(QStringLiteral("coordinates.transform"), query).value(QStringLiteral("success")).toBool());
    QCOMPARE(project.commandManager()->undoText(), historyText);
    const QJsonArray queryResult = project.commandManager()->result().value(QStringLiteral("xyz")).toArray();
    QVERIFY(std::abs(queryResult.at(0).toDouble() - 111319.49079327357) < 1.0e-6);
    QAction* settings = window.findChild<QAction*>(QStringLiteral("projectSettingsAction"));
    QVERIFY(settings != nullptr && settings->isEnabled());
    setProperty("coordinateDialogSubmitted", false);
    QTimer submit;
    connect(&submit, &QTimer::timeout, this, &CaseWorkflowTest::onSubmitProjectCoordinates);
    submit.start(20);
    settings->trigger();
    submit.stop();
    QVERIFY(property("coordinateDialogSubmitted").toBool());
    QCOMPARE(project.metadata().originZ(), 45.0);
    project.commandManager()->undo();
    QCOMPARE(project.metadata().originZ(), 0.0);
    QTemporaryDir savedDirectory;
    QVERIFY(savedDirectory.isValid());
    const QString savedPath = savedDirectory.filePath(QStringLiteral("coordinates.idos"));
    QVERIFY2(project.save(savedPath, window.dataRegistry(), error), qPrintable(error));
    QScopedPointer<IDOSProject> loaded(IDOSProject::load(savedPath, window.dataRegistry(), error));
    QVERIFY2(loaded != nullptr, qPrintable(error));
    QCOMPARE(loaded->metadata().projectData(), project.metadata().projectData());
    window.setProject(nullptr);
}

QTEST_MAIN(CaseWorkflowTest)

CaseWorkflowTest::CaseWorkflowTest()
    :
    m_memoryPeak(0)
    , m_importHeartbeatCount(0)
    , m_profileRenderNanoseconds(0)
    , m_profileRenderCount(0)
    , m_cancel(false)
    , m_expectDisabled(false)
    , m_settingsDirectory(new QTemporaryDir())
    , m_previousSettingsFormat(QSettings::defaultFormat())
{
    const QSettings previousSettings(QSettings::IniFormat, QSettings::UserScope,
                                      QStringLiteral("IDOS-Codex-Isolation"), QStringLiteral("Probe"));
    QDir previousRoot = QFileInfo(previousSettings.fileName()).dir();
    previousRoot.cdUp();
    m_previousSettingsRoot = previousRoot.absolutePath();
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_settingsDirectory->path());
    QSettings::setDefaultFormat(QSettings::IniFormat);
    IDOSApplication::initProviders();
}

CaseWorkflowTest::~CaseWorkflowTest()
{
    QSettings::setDefaultFormat(m_previousSettingsFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_previousSettingsRoot);
    delete m_settingsDirectory;
}

void CaseWorkflowTest::onGridFilters()
{
    QTranslator translator;
    QVERIFY(translator.load(QCoreApplication::applicationDirPath() + QStringLiteral("/i18n/idos_zh_CN.qm")));
    QVERIFY(QApplication::installTranslator(&translator));
    IDOSProject project;
    IDOSMainWindow window;
    window.setProject(&project);
    IDOSGridObject* grid = new IDOSGridObject();
    grid->setName(QStringLiteral("Filter Grid"));
    grid->setDimensions(3, 2, 1);
    grid->setCornerPointData(3, 2, 1, grid->coord(), grid->zcorn(), QVector<int>{1, 1, 1, 1, 0, 1});
    project.add(grid);
    IDOSGridPropertyObject* property = new IDOSGridPropertyObject();
    property->setName(QStringLiteral("Pressure"));
    property->setGridId(grid->objectId());
    property->setDimensions(3, 2, 1);
    property->setKind(IDOSGridPropertyObject::Kind::Dynamic);
    QVERIFY(property->setReportStepValues(0, QVector<double>{10, 20, 30, 40, 50, 60}));
    QVERIFY(property->setReportStepValues(1, QVector<double>{60, 50, 40, 30, 20, 10}));
    QVERIFY(property->setCurrentReportStep(0));
    project.add(property);
    IDOSSimulationCaseObject* simulationCase = new IDOSSimulationCaseObject();
    simulationCase->setName(QStringLiteral("Filter Case"));
    simulationCase->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), grid->objectId()));
    project.add(simulationCase);
    IDOSTreeView* tree = window.caseTreeView();
    IDOSRenderView* view = window.renderServer()->currentView();
    QVERIFY(view != nullptr);
    window.renderServer()->setChecked(grid->objectId(), Qt::Checked, view);
    IDOSRenderObject* sourceObject = findRenderObject(window.renderServer(), view, grid->objectId());
    QVERIFY(sourceObject != nullptr);
    IDOSGridRenderProvider* sourceProvider = dynamic_cast<IDOSGridRenderProvider*>(sourceObject->provider());
    QVERIFY(sourceProvider != nullptr);
    QCOMPARE(sourceProvider->filteredCellCount(), 5);
    IDOSRenderView otherView;
    IDOSGridRenderProvider* otherProvider = new IDOSGridRenderProvider(grid);
    otherProvider->update();
    QVERIFY(otherView.scene()->add(new IDOSRenderObject(otherProvider)));
    const QModelIndex gridIndex = window.caseTreeModel()->index(grid->objectId());
    QVERIFY(gridIndex.isValid());
    QScopedPointer<QMenu> menu(tree->createContextMenu(gridIndex));
    QAction* action = menu->findChild<QAction*>(QStringLiteral("gridFilterAction"));
    QVERIFY(action != nullptr && action->isEnabled());
    action->trigger();
    QPointer<IDOSGridFilterWidget> widget(window.findChild<IDOSGridFilterWidget*>(QStringLiteral("gridFilterWindow")));
    QVERIFY(widget != nullptr);
    QCOMPARE(widget->findChild<QTabWidget*>(QStringLiteral("gridFilterTabs"))->currentIndex(), 0);
    QVERIFY(window.findChild<QWidget*>(QStringLiteral("gridSettingsWindow")) == nullptr);
    QCheckBox* iEnabled = widget->findChild<QCheckBox*>(QStringLiteral("gridFilterIEnabled"));
    QSpinBox* iMaximum = widget->findChild<QSpinBox*>(QStringLiteral("gridFilterIMaximum"));
    QPushButton* apply = widget->findChild<QPushButton*>(QStringLiteral("gridFilterApply"));
    QVERIFY(iEnabled != nullptr && iMaximum != nullptr && apply != nullptr);
    iEnabled->setChecked(true);
    iMaximum->setValue(2);
    apply->click();
    QCOMPARE(sourceProvider->filteredCellCount(), 3);
    QCOMPARE(otherProvider->filteredCellCount(), 5);
    QCOMPARE(project.objects().size(), 3);
    vtkDataSet* selected = actors(sourceProvider).first()->GetMapper()->GetInput();
    QCOMPARE(selected->GetCellData()->GetArray("GlobalIndex")->GetTuple1(2), 3.0);
    QCOMPARE(selected->GetCellData()->GetArray("I")->GetTuple1(2), 1.0);
    QCOMPARE(selected->GetCellData()->GetArray("J")->GetTuple1(2), 2.0);
    const QVariantMap temporary = sourceProvider->filterCriteria();
    QCheckBox* jEnabled = widget->findChild<QCheckBox*>(QStringLiteral("gridFilterJEnabled"));
    QSpinBox* jMaximum = widget->findChild<QSpinBox*>(QStringLiteral("gridFilterJMaximum"));
    jEnabled->setChecked(true);
    jMaximum->setValue(1);
    widget->findChild<QRadioButton*>(QStringLiteral("gridFilterIntersection"))->setChecked(true);
    apply->click();
    QCOMPARE(sourceProvider->filteredCellCount(), 2);
    widget->findChild<QRadioButton*>(QStringLiteral("gridFilterUnion"))->setChecked(true);
    apply->click();
    QCOMPARE(sourceProvider->filteredCellCount(), 4);
    widget->findChild<QRadioButton*>(QStringLiteral("gridFilterInvert"))->setChecked(true);
    apply->click();
    QCOMPARE(sourceProvider->filteredCellCount(), 1);
    widget->findChild<QPushButton*>(QStringLiteral("gridFilterReset"))->click();
    QCOMPARE(sourceProvider->filteredCellCount(), 5);
    QListWidget* iList = widget->findChild<QListWidget*>(QStringLiteral("gridFilterIList"));
    QVERIFY(iList != nullptr && iList->count() == 3);
    widget->findChild<QSpinBox*>(QStringLiteral("gridFilterIStep"))->setValue(2);
    widget->findChild<QToolButton*>(QStringLiteral("gridFilterISelect"))->click();
    QCOMPARE(iList->item(0)->checkState(), Qt::Checked);
    QCOMPARE(iList->item(1)->checkState(), Qt::Unchecked);
    QCOMPARE(iList->item(2)->checkState(), Qt::Checked);
    apply->click();
    QCOMPARE(sourceProvider->filteredCellCount(), 4);
    widget->findChild<QToolButton*>(QStringLiteral("gridFilterIClear"))->click();
    apply->click();
    QCOMPARE(sourceProvider->filteredCellCount(), 5);
    iEnabled->setChecked(true);
    iMaximum->setValue(2);
    QTest::qWait(200);
    QVERIFY(widget->grab().save(QDir(qEnvironmentVariable("IDOS_TEST_SCREENSHOT_DIR", QDir::tempPath())).filePath(QStringLiteral("grid-filter-indices.png"))));
    widget->findChild<QTabWidget*>(QStringLiteral("gridFilterTabs"))->setCurrentIndex(1);
    QListWidget* attributes = widget->findChild<QListWidget*>(QStringLiteral("gridFilterPropertyList"));
    QVERIFY(attributes != nullptr && attributes->count() == 1);
    attributes->item(0)->setCheckState(Qt::Checked);
    widget->findChild<QDoubleSpinBox*>(QStringLiteral("gridFilterPropertyMinimum"))->setValue(15.0);
    widget->findChild<QDoubleSpinBox*>(QStringLiteral("gridFilterPropertyMaximum"))->setValue(45.0);
    QComboBox* displayProperty = widget->findChild<QComboBox*>(QStringLiteral("gridFilterDisplayProperty"));
    displayProperty->setCurrentIndex(displayProperty->findData(property->objectId()));
    QTest::qWait(200);
    QLabel* rangeBar = widget->findChild<QLabel*>(QStringLiteral("gridFilterRangeBar"));
    QVERIFY(rangeBar != nullptr && rangeBar->width() > 100);
    const int originalX = 12 + static_cast<int>(0.1 * (rangeBar->width() - 24));
    const int selectedX = 12 + static_cast<int>(0.2 * (rangeBar->width() - 24));
    QTest::mousePress(rangeBar, Qt::LeftButton, Qt::NoModifier, QPoint(originalX, 21));
    QTest::mouseMove(rangeBar, QPoint(selectedX, 21));
    QTest::mouseRelease(rangeBar, Qt::LeftButton, Qt::NoModifier, QPoint(selectedX, 21));
    QVERIFY(widget->findChild<QDoubleSpinBox*>(QStringLiteral("gridFilterPropertyMinimum"))->value() > 18.0);
    widget->findChild<QDoubleSpinBox*>(QStringLiteral("gridFilterPropertyMinimum"))->setValue(15.0);
    apply->click();
    QCOMPARE(sourceProvider->filteredCellCount(), 2);
    const QVariantMap appliedCriteria = sourceProvider->filterCriteria();
    widget->findChild<QDoubleSpinBox*>(QStringLiteral("gridFilterPropertyMinimum"))->setValue(16.0);
    QCOMPARE(sourceProvider->filterCriteria(), appliedCriteria);
    widget->findChild<QPushButton*>(QStringLiteral("gridFilterSaveAs"))->click();
    QInputDialog* nameDialog = widget->findChild<QInputDialog*>(QStringLiteral("gridFilterNameDialog"));
    QVERIFY(nameDialog != nullptr);
    nameDialog->setTextValue(QStringLiteral("Pressure subset"));
    nameDialog->accept();
    const QString filterId = project.commandManager()->result().value(QStringLiteral("filterId")).toString();
    IDOSGridFilterObject* filter = qobject_cast<IDOSGridFilterObject*>(project.object(filterId));
    QVERIFY(filter != nullptr);
    QCOMPARE(filter->gridId(), grid->objectId());
    QCOMPARE(filter->containerId(), grid->objectId());
    QCOMPARE(project.objects().size(), 4);
    const QVariantMap savedCriteria = filter->criteria();
    QVERIFY(savedCriteria != appliedCriteria);
    QCOMPARE(sourceProvider->filterCriteria(), savedCriteria);
    QVERIFY(project.commandManager()->contains(QStringLiteral("grid.filter.create")));
    QMap<QString, IDOSGridPropertyObject*> references;
    references.insert(property->objectId(), property);
    QVariantMap testCondition;
    testCondition.insert(QStringLiteral("propertyId"), property->objectId());
    testCondition.insert(QStringLiteral("minimum"), 30.0);
    testCondition.insert(QStringLiteral("maximum"), 50.0);
    const QStringList operations = {QStringLiteral("between"), QStringLiteral("greaterEqual"), QStringLiteral("lessEqual"), QStringLiteral("equal")};
    const QList<int> expected = {3, 4, 5, 1};
    for (int operation = 0; operation < operations.size(); ++operation)
    {
        testCondition.insert(QStringLiteral("operation"), operations.at(operation));
        QVariantMap comparisonCriteria;
        comparisonCriteria.insert(QStringLiteral("conditions"), QVariantList{testCondition});
        const QVector<quint8> mask = IDOSGridFilterObject::selection(comparisonCriteria, 3, 2, 1, references);
        int matched = 0;
        for (int cell = 0; cell < mask.size(); ++cell)
        {
            matched += mask.at(cell);
            QCOMPARE(mask.at(cell) != 0, IDOSGridFilterObject::matches(comparisonCriteria, cell % 3, cell / 3, 0, cell, references));
        }
        QCOMPARE(matched, expected.at(operation));
    }
    QVariantMap secondCondition = testCondition;
    secondCondition.insert(QStringLiteral("minimum"), 60.0);
    secondCondition.insert(QStringLiteral("maximum"), 60.0);
    QVariantMap combinationCriteria;
    combinationCriteria.insert(QStringLiteral("conditions"), QVariantList{testCondition, secondCondition});
    const QVector<quint8> intersected = IDOSGridFilterObject::selection(combinationCriteria, 3, 2, 1, references);
    QCOMPARE(intersected.size(), 6);
    for (quint8 selected : intersected)
    {
        QCOMPARE(selected, quint8(0));
    }
    combinationCriteria.insert(QStringLiteral("anyProperty"), true);
    const QVector<quint8> united = IDOSGridFilterObject::selection(combinationCriteria, 3, 2, 1, references);
    QCOMPARE(united.size(), 6);
    for (int index = 0; index < united.size(); ++index)
    {
        QCOMPARE(united.at(index), quint8(index == 2 || index == 5 ? 1 : 0));
    }
    QString validationError;
    QVariantMap malformed;
    malformed.insert(QStringLiteral("conditions"), QStringLiteral("invalid"));
    QVERIFY(!IDOSGridFilterObject::validateCriteria(malformed, validationError));
    IDOSRenderObject* resultObject = findRenderObject(window.renderServer(), view, filterId);
    QVERIFY(resultObject != nullptr && resultObject->isVisible());
    IDOSGridRenderProvider* resultProvider = dynamic_cast<IDOSGridRenderProvider*>(resultObject->provider());
    QVERIFY(resultProvider != nullptr);
    QCOMPARE(resultProvider->filteredCellCount(), 2);
    QCOMPARE(resultProvider->property(), property);
    QVERIFY(window.caseTreeModel()->index(filterId).isValid());
    QVERIFY(sourceObject->isVisible());
    QCOMPARE(sourceProvider->previewFilterId(), filterId);
    QVERIFY(window.renderServer()->setCurrentView(view));
    const QModelIndex filterIndex = window.caseTreeModel()->index(filterId);
    QVERIFY(window.caseTreeModel()->setData(filterIndex, Qt::Unchecked, Qt::CheckStateRole));
    QVERIFY(!resultObject->isVisible());
    QVERIFY(sourceObject->isVisible());
    QCOMPARE(sourceProvider->filteredCellCount(), 5);
    QVERIFY(sourceProvider->filterCriteria().isEmpty());
    QVERIFY(sourceProvider->previewFilterId().isEmpty());
    QCOMPARE(sourceProvider->property(), property);
    QCOMPARE(filter->criteria(), savedCriteria);
    QCOMPARE(otherProvider->filteredCellCount(), 5);
    QAction* clearDisplay = window.findChild<QAction*>(QStringLiteral("viewToolClearDisplayAction"));
    QVERIFY(clearDisplay != nullptr);
    clearDisplay->trigger();
    QVERIFY(!sourceObject->isVisible());
    QVERIFY(!resultObject->isVisible());
    clearDisplay->trigger();
    QVERIFY(sourceObject->isVisible());
    QVERIFY(!resultObject->isVisible());
    QVERIFY(sourceProvider->filterCriteria().isEmpty());
    QVERIFY(window.caseTreeModel()->setData(filterIndex, Qt::Checked, Qt::CheckStateRole));
    QVERIFY(resultObject->isVisible());
    QCOMPARE(resultProvider->filteredCellCount(), 2);
    QCOMPARE(sourceProvider->filterCriteria(), savedCriteria);
    QCOMPARE(sourceProvider->previewFilterId(), filterId);
    QCOMPARE(sourceProvider->filteredCellCount(), 2);
    QVERIFY(sourceProvider->setFilterCriteria(temporary));
    sourceProvider->update();
    view->scene()->notifyChanged(sourceObject->id());
    QVERIFY(window.caseTreeModel()->setData(filterIndex, Qt::Unchecked, Qt::CheckStateRole));
    QCOMPARE(sourceProvider->filteredCellCount(), 3);
    QCOMPARE(sourceProvider->filterCriteria(), temporary);
    QVERIFY(sourceProvider->setFilterCriteria(QVariantMap()));
    sourceProvider->update();
    view->scene()->notifyChanged(sourceObject->id());
    QVERIFY(window.caseTreeModel()->setData(filterIndex, Qt::Checked, Qt::CheckStateRole));
    QScopedPointer<QMenu> resultMenu(tree->createContextMenu(window.caseTreeModel()->index(filterId)));
    QVERIFY(resultMenu->findChild<QAction*>(QStringLiteral("gridFilterEditAction")) != nullptr);
    QVERIFY(resultMenu->findChild<QAction*>(QStringLiteral("gridFilterDeleteAction")) != nullptr);
    QAction* renameAction = resultMenu->findChild<QAction*>(QStringLiteral("gridFilterRenameAction"));
    QVERIFY(renameAction != nullptr);
    renameAction->trigger();
    QInputDialog* renameDialog = tree->findChild<QInputDialog*>();
    QVERIFY(renameDialog != nullptr);
    renameDialog->setTextValue(QStringLiteral("Renamed subset"));
    renameDialog->accept();
    QCOMPARE(filter->name(), QStringLiteral("Renamed subset"));
    project.commandManager()->undo();
    QCOMPARE(filter->name(), QStringLiteral("Pressure subset"));
    resultProvider->setOpacity(0.3);
    QCOMPARE(sourceProvider->opacity(), 1.0);
    QTest::qWait(200);
    QVERIFY(widget->grab().save(QDir(qEnvironmentVariable("IDOS_TEST_SCREENSHOT_DIR", QDir::tempPath())).filePath(QStringLiteral("grid-filter-settings.png"))));
    resultProvider->setProperty(nullptr);
    QVERIFY(property->setCurrentReportStep(1));
    QCOMPARE(resultProvider->property(), static_cast<IDOSGridPropertyObject*>(nullptr));
    QCOMPARE(resultProvider->filteredCellCount(), 1);
    QCOMPARE(sourceProvider->filteredCellCount(), 1);
    QCOMPARE(filter->criteria(), savedCriteria);
    QJsonObject updateArguments;
    updateArguments.insert(QStringLiteral("filterId"), filterId);
    updateArguments.insert(QStringLiteral("criteria"), QJsonObject::fromVariantMap(temporary));
    QVERIFY(project.commandManager()->execute(QStringLiteral("grid.filter.update"), updateArguments).value(QStringLiteral("success")).toBool());
    QCOMPARE(resultProvider->filteredCellCount(), 3);
    project.commandManager()->undo();
    QCOMPARE(filter->criteria(), savedCriteria);
    QCOMPARE(resultProvider->filteredCellCount(), 1);
    project.commandManager()->redo();
    QCOMPARE(resultProvider->filteredCellCount(), 3);
    project.commandManager()->undo();
    QJsonObject invalidArguments = updateArguments;
    QVariantMap missingCriteria = savedCriteria;
    QVariantList missingConditions = missingCriteria.value(QStringLiteral("conditions")).toList();
    QVariantMap missingCondition = missingConditions.first().toMap();
    missingCondition.insert(QStringLiteral("propertyId"), QStringLiteral("missing-property"));
    missingConditions[0] = missingCondition;
    missingCriteria.insert(QStringLiteral("conditions"), missingConditions);
    invalidArguments.insert(QStringLiteral("criteria"), QJsonObject::fromVariantMap(missingCriteria));
    QVERIFY(!project.commandManager()->execute(new IDOSUpdateGridFilterCommand(&project, invalidArguments)));
    QCOMPARE(filter->criteria(), savedCriteria);
    QVariantMap invertedSaved = savedCriteria;
    invertedSaved.insert(QStringLiteral("invert"), true);
    filter->setCriteria(invertedSaved);
    IDOSDataObject* detachedProperty = project.take(property->objectId());
    QVERIFY(actors(resultProvider).isEmpty());
    QVERIFY(!resultProvider->filterError().isEmpty());
    QCOMPARE(filter->criteria(), invertedSaved);
    filter->setCriteria(savedCriteria);
    project.add(detachedProperty);
    QCOMPARE(resultProvider->filteredCellCount(), 1);
    QJsonObject deleteArguments;
    deleteArguments.insert(QStringLiteral("filterId"), filterId);
    QVERIFY(sourceProvider->setFilterCriteria(savedCriteria));
    sourceProvider->setPreviewFilterId(filterId);
    sourceProvider->setPropertyInput(property);
    sourceProvider->update();
    QCOMPARE(sourceProvider->filteredCellCount(), 1);
    QVERIFY(project.commandManager()->execute(QStringLiteral("grid.filter.delete"), deleteArguments).value(QStringLiteral("success")).toBool());
    QVERIFY(project.object(filterId) == nullptr);
    QVERIFY(findRenderObject(window.renderServer(), view, filterId) == nullptr);
    QCOMPARE(sourceProvider->filteredCellCount(), 5);
    QVERIFY(sourceProvider->previewFilterId().isEmpty());
    project.commandManager()->undo();
    QCOMPARE(project.object(filterId), static_cast<IDOSDataObject*>(filter));
    QVERIFY(window.caseTreeModel()->index(filterId).isValid());
    project.commandManager()->redo();
    QVERIFY(project.object(filterId) == nullptr);
    project.commandManager()->undo();
    const QString gridId = grid->objectId();
    QVERIFY(project.commandManager()->execute(new IDOSDeleteGridCommand(&project, gridId)));
    QVERIFY(project.object(gridId) == nullptr && project.object(filterId) == nullptr);
    project.commandManager()->undo();
    QCOMPARE(project.object(gridId), static_cast<IDOSDataObject*>(grid));
    QCOMPARE(project.object(filterId), static_cast<IDOSDataObject*>(filter));
    QVERIFY(findRenderObject(window.renderServer(), view, filterId) != nullptr);
    project.commandManager()->redo();
    QVERIFY(project.object(filterId) == nullptr);
    project.commandManager()->undo();
    project.commandManager()->undo();
    QVERIFY(project.object(filterId) == nullptr);
    project.commandManager()->redo();
    QCOMPARE(project.object(filterId), static_cast<IDOSDataObject*>(filter));
    QVERIFY(findRenderObject(window.renderServer(), view, filterId) != nullptr);
    QApplication::removeTranslator(&translator);
}

void CaseWorkflowTest::onGridFilterCriteria()
{
    IDOSGridPropertyObject property;
    property.setDimensions(3, 2, 1);
    const double invalid = std::numeric_limits<double>::quiet_NaN();
    QVERIFY(property.setValues(QVector<double>{0.0, 1.0, 2.0, invalid, 4.0, 5.0}));
    QMap<QString, IDOSGridPropertyObject*> references;
    references.insert(property.objectId(), &property);
    QVariantMap condition;
    condition.insert(QStringLiteral("propertyId"), property.objectId());
    condition.insert(QStringLiteral("minimum"), 2.0);
    condition.insert(QStringLiteral("maximum"), 4.0);
    const QStringList operations = {QStringLiteral("between"), QStringLiteral("greater"), QStringLiteral("greaterEqual"),
        QStringLiteral("less"), QStringLiteral("lessEqual"), QStringLiteral("equal"), QStringLiteral("notEqual")};
    const QList<int> counts = {2, 2, 3, 3, 4, 1, 4};
    for (int operation = 0; operation < operations.size(); ++operation)
    {
        condition.insert(QStringLiteral("operation"), operations.at(operation));
        QVariantMap criteria;
        criteria.insert(QStringLiteral("conditions"), QVariantList{condition});
        const QVector<quint8> mask = IDOSGridFilterObject::selection(criteria, 3, 2, 1, references);
        int count = 0;
        for (int index = 0; index < mask.size(); ++index)
        {
            count += mask.at(index);
        }
        QCOMPARE(count, counts.at(operation));
    }
    condition.insert(QStringLiteral("operation"), QStringLiteral("between"));
    condition.insert(QStringLiteral("minimum"), 1.0);
    condition.insert(QStringLiteral("includeMinimum"), false);
    condition.insert(QStringLiteral("includeMaximum"), false);
    QVariantMap criteria;
    criteria.insert(QStringLiteral("conditions"), QVariantList{condition});
    QVERIFY(sameSelection(IDOSGridFilterObject::selection(criteria, 3, 2, 1, references), QVector<quint8>({0, 0, 1, 0, 0, 0})));
    condition.insert(QStringLiteral("includeInvalidValues"), true);
    criteria.insert(QStringLiteral("conditions"), QVariantList{condition});
    QVERIFY(sameSelection(IDOSGridFilterObject::selection(criteria, 3, 2, 1, references), QVector<quint8>({0, 0, 1, 1, 0, 0})));
    criteria.insert(QStringLiteral("propertyOperation"), QStringLiteral("hide"));
    QVERIFY(sameSelection(IDOSGridFilterObject::selection(criteria, 3, 2, 1, references), QVector<quint8>({1, 1, 0, 0, 1, 1})));
    condition.insert(QStringLiteral("propertyId"), QStringLiteral("missing"));
    criteria.insert(QStringLiteral("conditions"), QVariantList{condition});
    QVERIFY(sameSelection(IDOSGridFilterObject::selection(criteria, 3, 2, 1, references), QVector<quint8>({0, 0, 0, 0, 0, 0})));
    condition.insert(QStringLiteral("enabled"), false);
    criteria.insert(QStringLiteral("conditions"), QVariantList{condition});
    QVERIFY(sameSelection(IDOSGridFilterObject::selection(criteria, 3, 2, 1, references), QVector<quint8>({1, 1, 1, 1, 1, 1})));
    QVERIFY(IDOSGridFilterObject::propertyIds(criteria).isEmpty());
    QVERIFY(property.setValues(QVector<double>{1.0, 1.0000005, 1.000002, invalid, 4.0, 5.0}));
    condition.insert(QStringLiteral("propertyId"), property.objectId());
    condition.insert(QStringLiteral("enabled"), true);
    condition.insert(QStringLiteral("operation"), QStringLiteral("equal"));
    condition.insert(QStringLiteral("epsilon"), 1.0e-6);
    condition.insert(QStringLiteral("includeInvalidValues"), false);
    criteria.remove(QStringLiteral("propertyOperation"));
    criteria.insert(QStringLiteral("conditions"), QVariantList{condition});
    QVERIFY(sameSelection(IDOSGridFilterObject::selection(criteria, 3, 2, 1, references), QVector<quint8>({1, 1, 0, 0, 0, 0})));
    condition.insert(QStringLiteral("operation"), QStringLiteral("notEqual"));
    criteria.insert(QStringLiteral("conditions"), QVariantList{condition});
    QVERIFY(sameSelection(IDOSGridFilterObject::selection(criteria, 3, 2, 1, references), QVector<quint8>({0, 0, 1, 0, 1, 1})));
    criteria.clear();
    QVariantMap iAxis;
    iAxis.insert(QStringLiteral("enabled"), true);
    iAxis.insert(QStringLiteral("minimum"), 2);
    iAxis.insert(QStringLiteral("maximum"), 2);
    iAxis.insert(QStringLiteral("values"), QVariantList{1});
    QVariantMap jAxis;
    jAxis.insert(QStringLiteral("enabled"), true);
    jAxis.insert(QStringLiteral("minimum"), 1);
    jAxis.insert(QStringLiteral("maximum"), 1);
    QVariantMap axes;
    axes.insert(QStringLiteral("i"), iAxis);
    criteria.insert(QStringLiteral("axes"), axes);
    QVERIFY(sameSelection(IDOSGridFilterObject::selection(criteria, 3, 2, 1, references), QVector<quint8>({1, 1, 0, 1, 1, 0})));
    axes.insert(QStringLiteral("j"), jAxis);
    criteria.insert(QStringLiteral("axes"), axes);
    criteria.insert(QStringLiteral("axisUnion"), true);
    QVERIFY(sameSelection(IDOSGridFilterObject::selection(criteria, 3, 2, 1, references), QVector<quint8>({1, 1, 1, 1, 1, 0})));
    criteria.insert(QStringLiteral("axisUnion"), false);
    QVERIFY(sameSelection(IDOSGridFilterObject::selection(criteria, 3, 2, 1, references), QVector<quint8>({1, 1, 0, 0, 0, 0})));
    criteria.insert(QStringLiteral("axisOperation"), QStringLiteral("hide"));
    const QVector<quint8> hidden = IDOSGridFilterObject::selection(criteria, 3, 2, 1, references);
    QVERIFY(sameSelection(hidden, QVector<quint8>({0, 0, 1, 1, 1, 1})));
    criteria.insert(QStringLiteral("axisOperation"), QStringLiteral("invert"));
    QVERIFY(sameSelection(IDOSGridFilterObject::selection(criteria, 3, 2, 1, references), hidden));
    QString error;
    iAxis.insert(QStringLiteral("values"), QVariantList{0});
    axes.insert(QStringLiteral("i"), iAxis);
    criteria.insert(QStringLiteral("axes"), axes);
    QVERIFY(!IDOSGridFilterObject::validateCriteria(criteria, error));
}


bool CaseWorkflowTest::sameSelection(const QVector<quint8>& first, const QVector<quint8>& second) const
{
    if (first.size() != second.size())
    {
        return false;
    }
    for (int index = 0; index < first.size(); ++index)
    {
        if (first.at(index) != second.at(index))
        {
            return false;
        }
    }
    return true;
}

void CaseWorkflowTest::onExamplePluginMenu()
{
    IDOSProject project;
    IDOSProject replacement;
    IDOSMainWindow window;
    window.setProject(&project);
    const QString path = QCoreApplication::applicationDirPath() + QStringLiteral("/plugins/idosexampleplugin.dll");
    IDOSPluginRegistry* plugins = IDOSPluginRegistry::instance();
    plugins->loadCppPlugin(path);
    QVERIFY(plugins->isLoaded(QStringLiteral("idosexampleplugin")));
    QJsonObject arguments;
    arguments.insert(QStringLiteral("name"), QStringLiteral("Menu sample"));
    const QJsonObject response = project.commandManager()->execute(QStringLiteral("pointset.create"), arguments);
    QVERIFY(response.value(QStringLiteral("success")).toBool());
    const QString id = response.value(QStringLiteral("result")).toObject().value(QStringLiteral("objectId")).toString();
    const QModelIndex index = window.dataTreeModel()->index(id);
    QVERIFY(index.isValid());
    QScopedPointer<QMenu> menu(window.dataTreeView()->createContextMenu(index));
    QAction* remove = menu->findChild<QAction*>(QStringLiteral("deletePointSetAction"));
    QVERIFY(remove != nullptr);
    remove->trigger();
    QVERIFY(project.object(id) == nullptr);
    project.commandManager()->undo();
    QVERIFY(project.object(id) != nullptr);
    window.setProject(&replacement);
    remove->trigger();
    QVERIFY(project.object(id) != nullptr);
    QVERIFY(replacement.objects().isEmpty());
}

QList<vtkActor*> CaseWorkflowTest::actors(const IDOSRenderProvider* provider)
{
    QList<vtkActor*> result;
    if (provider == nullptr)
    {
        return result;
    }
    for (vtkProp* prop : provider->props())
    {
        if (prop != nullptr && vtkActor::SafeDownCast(prop) != nullptr)
        {
            result.append(vtkActor::SafeDownCast(prop));
        }
    }
    return result;
}

QList<vtkProp*> CaseWorkflowTest::legends(const IDOSRenderProvider* provider)
{
    QList<vtkProp*> result;
    if (provider == nullptr)
    {
        return result;
    }
    for (vtkProp* prop : provider->props())
    {
        if (prop != nullptr && prop->GetVisibility() != 0 && vtkActor::SafeDownCast(prop) == nullptr)
        {
            result.append(prop);
        }
    }
    return result;
}
