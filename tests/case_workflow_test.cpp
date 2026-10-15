#include <cmath>
#include <limits>
#include <memory>
#include <type_traits>
#include <vector>

#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QColor>
#include <QColorDialog>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QDialog>
#include <QDateTime>
#include <QDir>
#include <QDoubleSpinBox>
#include <QEvent>
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
#include <QPixmap>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QScopedPointer>
#include <QSettings>
#include <QSignalSpy>
#include <QSize>
#include <QSlider>
#include <QSpinBox>
#include <QTableWidget>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTimer>
#include <QToolButton>
#include <QTranslator>
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
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Grid/NNC.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <opm/io/eclipse/EclOutput.hpp>
#include <SARibbonBar.h>
#include <SARibbonCategory.h>
#include <SARibbonPanel.h>

#include "command/idosdeletedatacommand.h"
#include "command/idosdeletegridcommand.h"
#include "command/idosgridfiltercommand.h"
#include "command/idosimportrestartresultscommand.h"
#include "data/case/idoscaseitemref.h"
#include "data/case/idossimulationcaseobject.h"
#include "data/case/idossimulationsummaryresults.h"
#include "data/fluidrock/idosfluidrocktable.h"
#include "data/fracture/idosnaturalfractureset.h"
#include "data/grid/idosgrid.h"
#include "data/grid/idosgridboundary.h"
#include "data/grid/idosgridfilter.h"
#include "data/grid/idosgridproperty.h"
#include "data/well/idoswell.h"
#include "data/well/idoswellhead.h"
#include "data/well/idoswelllogchannel.h"
#include "data/well/idoswelllogset.h"
#include "data/well/idoswellpath.h"
#include "data/well/idoswellpathpoint.h"
#include "grid/idosgridfilterwidget.h"
#include "grid/idosgridsettingwidget.h"
#include "idosapplication.h"
#include "idoscasedialog.h"
#include "idoscommandmanager.h"
#include "idoscommandregistry.h"
#include "idosmainwindow.h"
#include "idospluginregistry.h"
#include "idosproject.h"
#include "idosrenderobject.h"
#include "idosrenderregistry.h"
#include "idosrenderscene.h"
#include "idosrenderserver.h"
#include "idosrenderview.h"
#include "idosruntimeinfowidget.h"
#include "idostoolregistry.h"
#include "logger/idoslogger.h"
#include "providers/case/idossimulationcasetreeguiprovider.h"
#include "providers/data/idosdatagrouptreeguiprovider.h"
#include "providers/data/idosnaturalfracturerendermetadata.h"
#include "providers/data/idosnaturalfracturerenderprovider.h"
#include "providers/grid/idosgridrendermetadata.h"
#include "providers/grid/idosgridrenderprovider.h"
#include "providers/idosdatametadata.h"
#include "providers/idosdataprovider.h"
#include "providers/idosdataproviderregistry.h"
#include "providers/idosdataregistry.h"
#include "providers/idosrenderprovider.h"
#include "providers/idostreeregistry.h"
#include "providers/well/idoswellrendermetadata.h"
#include "providers/well/idoswellrenderprovider.h"
#include "simulation/idossimulationinitializewidget.h"
#include "simulation/idossimulationresultplaybackwidget.h"
#include "simulation/idossimulationrunstatuswidget.h"
#include "tree/idostree.h"
#include "tree/idostreeitem.h"
#include "tree/idostreemodel.h"
#include "tree/idostreeview.h"

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
    IDOSGrid* grid = new IDOSGrid();
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
    IDOSGridProperty* field = new IDOSGridProperty();
    field->setName(QStringLiteral("Pressure"));
    field->setKeyword(QStringLiteral("PRESSURE"));
    field->setValueUnits(IDOSGridProperty::ValueUnits::SI);
    field->setGridId(grid->objectId());
    field->setDimensions(2, 1, 1);
    field->setKind(IDOSGridProperty::Kind::Dynamic);
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
    IDOSGridProperty* output = qobject_cast<IDOSGridProperty*>(project->object(outputId));
    QVERIFY(output != nullptr);
    QCOMPARE(output->values().first(), 4.0);
    QVERIFY(std::isnan(output->values().last()));
    QCOMPARE(field->currentReportStep(), 1);
    QVERIFY(window.caseTreeModel()->indexFromDataId(outputId).isValid());
    project->commandManager()->undo();
    QVERIFY(project->object(outputId) == nullptr);
    QJsonObject invalid = arguments;
    invalid.insert(QStringLiteral("afterStep"), 99);
    QVERIFY(!project->commandManager()->execute(command, invalid).value(QStringLiteral("success")).toBool());
    QVERIFY(project->commandManager()->canRedo());
    QVERIFY(field->setReportStepValues(2, {50.0, 0.0}));
    project->commandManager()->redo();
    QCOMPARE(output->values().first(), 4.0);
    field->setValueUnits(IDOSGridProperty::ValueUnits::Unknown);
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
    IDOSGridProperty* loaded = qobject_cast<IDOSGridProperty*>(restored->object(outputId));
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
    IDOSSimulationSummaryResults* summary = new IDOSSimulationSummaryResults();
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
    QVERIFY(window.commandRegistry()->find(QStringLiteral("fracture.dfn.generate")) != nullptr);
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
    IDOSGrid* grid = new IDOSGrid();
    grid->setName(QStringLiteral("DFN source grid"));
    grid->setCornerPointData(2, 1, 2, coord, zcorn, {1, 0, 1, 1});
    project->add(grid);
    IDOSGridProperty* density = new IDOSGridProperty();
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
    IDOSNaturalFractureSet* output = qobject_cast<IDOSNaturalFractureSet*>(project->object(outputId));
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
    const QModelIndex node = window.dataTreeModel()->indexFromDataId(outputId);
    QVERIFY(node.isValid());
    QVERIFY(window.dataTreeModel()->setData(node, Qt::Checked, Qt::CheckStateRole));
    QVERIFY(findRenderObject(window.renderServer()->currentView(), outputId)->visible());
    QTemporaryDir saved;
    QString error;
    QVERIFY2(project->save(saved.filePath(QStringLiteral("dfn.idos")), window.dataRegistry(), error), qPrintable(error));
    QScopedPointer<IDOSProject> loaded(IDOSProject::load(saved.filePath(QStringLiteral("dfn.idos")), window.dataRegistry(), error));
    QVERIFY2(!loaded.isNull(), qPrintable(error));
    IDOSNaturalFractureSet* restored = qobject_cast<IDOSNaturalFractureSet*>(loaded->object(outputId));
    QVERIFY(restored != nullptr);
    QCOMPARE(restored->fractures(), original);
    plugins->unloadCppPlugin(pluginPath);
    QVERIFY(plugins->isLoaded(QStringLiteral("idosfracturemodelingplugin")));
    project->commandManager()->undo();
    QVERIFY(project->object(outputId) == nullptr);
    QVERIFY(findRenderObject(window.renderServer()->currentView(), outputId) == nullptr);
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
    IDOSNaturalFractureSet* repeated = qobject_cast<IDOSNaturalFractureSet*>(project->object(response.value(QStringLiteral("result")).toObject().value(QStringLiteral("objectId")).toString()));
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
    IDOSNaturalFractureSet* different = qobject_cast<IDOSNaturalFractureSet*>(project->object(response.value(QStringLiteral("result")).toObject().value(QStringLiteral("objectId")).toString()));
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
    QVERIFY(window.commandRegistry()->find(QStringLiteral("fracture.dfn.generate")) == nullptr);
    QVERIFY(findRenderObject(window.renderServer()->currentView(), outputId) != nullptr);
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
    IDOSNaturalFractureSet data;
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
            IDOSNaturalFractureSet* field = qobject_cast<IDOSNaturalFractureSet*>(owner.data());
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
                IDOSNaturalFractureSet* fractures = new IDOSNaturalFractureSet();
                fractures->setFractures({fracture({0, 0, 0, 2, 0, 0, 2, 2, 0, 0, 2, 0}, 1)});
                data = fractures;
            }
            else if (category == 1)
            {
                data = new IDOSWell();
            }
            else
            {
                IDOSGridBoundary* boundary = new IDOSGridBoundary();
                boundary->setXCoordinates({0, 2, 2, 0});
                boundary->setYCoordinates({0, 0, 2, 2});
                boundary->setZCoordinates({0, 0, 0, 0});
                boundary->setGroupIds({1, 1, 1, 1});
                data = boundary;
            }
            data->setName(QStringLiteral("Group test %1 %2").arg(category).arg(index));
            data->setVisible(false);
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
    view->setTargetView(primary);
    IDOSTreeModel* model = window.dataTreeModel();
    view->refreshCheckStates();
    const QModelIndex wellParent = model->indexFromNode(model->tree()->find(QStringLiteral("data.wellGroup")));
    for (int category = 0; category < groups.size(); ++category)
    {
        const QModelIndex group = model->indexFromNode(model->tree()->find(groups.at(category)));
        QVERIFY(group.isValid());
        QVERIFY(model->flags(group).testFlag(Qt::ItemIsUserCheckable));
        QCOMPARE(model->data(group, Qt::CheckStateRole).toInt(), int(Qt::Unchecked));
        const QModelIndex first = model->indexFromDataId(firstIds.at(category));
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
        QVERIFY(findRenderObject(primary, secondIds.at(category))->visible());
        QVERIFY(!findRenderObject(&other, firstIds.at(category))->visible());
        findRenderObject(primary, secondIds.at(category))->setVisible(false);
        primary->scene()->notifyChanged(findRenderObject(primary, secondIds.at(category))->id());
        view->refreshCheckStates();
        QCOMPARE(model->data(group, Qt::CheckStateRole).toInt(), int(Qt::PartiallyChecked));
        view->setTargetView(&other);
        QCOMPARE(model->data(group, Qt::CheckStateRole).toInt(), int(Qt::Unchecked));
        view->setTargetView(primary);
        QVERIFY(model->setData(group, Qt::Unchecked, Qt::CheckStateRole));
        QVERIFY(!findRenderObject(primary, firstIds.at(category))->visible());
        QCOMPARE(model->data(group, Qt::CheckStateRole).toInt(), int(Qt::Unchecked));
    }
    QVERIFY(model->setData(wellParent, Qt::Checked, Qt::CheckStateRole));
    QVERIFY(findRenderObject(primary, firstIds.at(1))->visible());
    QVERIFY(findRenderObject(primary, secondIds.at(1))->visible());
    QVERIFY(model->setData(wellParent, Qt::Unchecked, Qt::CheckStateRole));
    IDOSDataGroupTreeGuiProvider provider(window.treeRegistry());
    QVERIFY(!provider.supports(window.caseTreeModel()->tree()->root()));
    IDOSTree caseTree;
    caseTree.setName(QStringLiteral("case"));
    IDOSTreeItem* foreignGroup = new IDOSTreeItem();
    foreignGroup->setGroup(true);
    foreignGroup->setKey(QStringLiteral("data.naturalFractures"));
    QVERIFY(caseTree.insert(caseTree.root(), foreignGroup));
    QVERIFY(!provider.supports(foreignGroup));
    QVERIFY(!provider.setChecked(foreignGroup, true, window.caseTreeView()));
    const QModelIndex fracturesGroup = model->indexFromNode(model->tree()->find(groups.first()));
    QVERIFY(project->remove(firstIds.first()));
    QVERIFY(project->remove(secondIds.first()));
    view->refreshCheckStates();
    QVERIFY(!model->flags(fracturesGroup).testFlag(Qt::ItemIsUserCheckable));
    QVERIFY(!model->data(fracturesGroup, Qt::CheckStateRole).isValid());
    view->setTargetView(nullptr);
    QVERIFY(!model->setData(wellParent, Qt::Checked, Qt::CheckStateRole));
    view->setTargetView(primary);
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
    IDOSNaturalFractureSet* first = new IDOSNaturalFractureSet();
    first->setName(QStringLiteral("First fractures"));
    first->setFractures({fracture({0, 0, 0, 2, 0, 0, 2, 2, 0, 0, 2, 0}, 1)});
    IDOSNaturalFractureSet* second = new IDOSNaturalFractureSet();
    second->setName(QStringLiteral("Second fractures"));
    second->setFractures(first->fractures());
    IDOSGrid* grid = new IDOSGrid();
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
    QVERIFY(findRenderObject(primary, firstId) != nullptr);
    QVERIFY(findRenderObject(&additional, secondId) != nullptr);
    IDOSTreeView* view = window.dataTreeView();
    const QModelIndex firstIndex = window.dataTreeModel()->indexFromDataId(firstId);
    const QModelIndex secondIndex = window.dataTreeModel()->indexFromDataId(secondId);
    QVERIFY(firstIndex.isValid() && secondIndex.isValid());
    QScopedPointer<QMenu> single(view->createMenu(firstIndex));
    QVERIFY(single->findChild<QAction*>(QStringLiteral("deleteDataAction")) != nullptr);
    view->selectionModel()->select(secondIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    QScopedPointer<QMenu> multiple(view->createMenu(firstIndex));
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
    QVERIFY(!window.dataTreeModel()->indexFromDataId(firstId).isValid());
    QVERIFY(findRenderObject(primary, firstId) == nullptr);
    QVERIFY(findRenderObject(&additional, secondId) == nullptr);
    QCOMPARE(caseObject->itemRefs().size(), 1);
    QCOMPARE(caseObject->itemRefs().first().objectId(), grid->objectId());
    project->commandManager()->undo();
    QCOMPARE(project->object(firstId), static_cast<IDOSDataObject*>(first));
    QCOMPARE(project->object(secondId), static_cast<IDOSDataObject*>(second));
    QCOMPARE(caseObject->itemRefs().size(), 3);
    QCOMPARE(caseObject->itemRefs().first().refId(), referenceId);
    QVERIFY(window.dataTreeModel()->indexFromDataId(firstId).isValid());
    QVERIFY(findRenderObject(primary, firstId) != nullptr);
    QVERIFY(findRenderObject(&additional, secondId) != nullptr);
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
    QPointer<IDOSNaturalFractureSet> lifetime(first);
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
    IDOSNaturalFractureSet* data = new IDOSNaturalFractureSet();
    data->setName(QStringLiteral("Sample fractures"));
    data->setFractures({fracture({0, 0, 0, 2, 0, 0, 2, 2, 0, 0, 2, 0}, 1),
                        fracture({0, 0, 1, 1, 0, 1, 2, 0, 1}, 2)});
    data->setVisible(false);
    project->add(data);
    project->setCurrentObject(data);
    QVERIFY(check->isEnabled());
    const QString dataId = data->objectId();
    IDOSRenderServer* server = window.renderServer();
    IDOSRenderView* firstView = server->currentView();
    QVERIFY(firstView != nullptr);
    IDOSRenderView secondView;
    server->addView(&secondView);
    IDOSRenderObject* first = findRenderObject(firstView, dataId);
    IDOSRenderObject* second = findRenderObject(&secondView, dataId);
    QVERIFY(first != nullptr && second != nullptr);
    QVERIFY(first->provider() != second->provider());
    QCOMPARE(first->provider()->actors().size(), 1);
    vtkActor* actor = first->provider()->actors().first();
    vtkPolyDataMapper* mapper = vtkPolyDataMapper::SafeDownCast(actor->GetMapper());
    QVERIFY(mapper != nullptr);
    mapper->Update();
    QCOMPARE(mapper->GetInput()->GetNumberOfCells(), vtkIdType(2));
    QVERIFY(mapper->GetInput()->GetCellData()->GetArray("FractureId") != nullptr);
    QCOMPARE(mapper->GetInput()->GetCellData()->GetArray("FractureId")->GetComponent(0, 0), 1.0);
    const QModelIndex node = window.dataTreeModel()->indexFromDataId(dataId);
    QVERIFY(node.isValid());
    QVERIFY(window.dataTreeModel()->flags(node).testFlag(Qt::ItemIsUserCheckable));
    QVERIFY(window.dataTreeModel()->setData(node, Qt::Checked, Qt::CheckStateRole));
    QVERIFY(first->visible());
    QVERIFY(!second->visible());
    first->setOpacity(0.4);
    firstView->setSelection({first->id()});
    QVERIFY(first->highlighted());
    QVERIFY(!second->highlighted());
    QCOMPARE(second->opacity(), 1.0);
    const QVariantMap appearance = first->provider()->presentationState();
    first->setOpacity(0.9);
    QVERIFY(first->provider()->restorePresentationState(appearance));
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
    QPointer<QDialog> dialog = window.findChild<QDialog*>(QStringLiteral("idosFractureGeometryReport"));
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
    QVERIFY(!second->visible());
    server->setCurrentView(&secondView);
    QVERIFY(QMetaObject::invokeMethod(table, "cellDoubleClicked", Qt::DirectConnection,
                                     Q_ARG(int, 0), Q_ARG(int, 0)));
    QVERIFY(!second->visible());
    data->setFractures({fracture({0, 0, 0, 3, 0, 0, 3, 3, 0, 0, 3, 0}, 3)});
    QTRY_VERIFY(dialog.isNull());
    QCOMPARE(first->opacity(), 0.4);
    QVERIFY(first->visible());
    QCOMPARE(first->provider()->actors().size(), 1);

    QTemporaryDir saved;
    QString error;
    QVERIFY2(project->save(saved.filePath(QStringLiteral("fractures.idos")), window.dataRegistry(), error), qPrintable(error));
    QScopedPointer<IDOSProject> loaded(IDOSProject::load(saved.filePath(QStringLiteral("fractures.idos")),
                                                       window.dataRegistry(), error));
    QVERIFY2(!loaded.isNull(), qPrintable(error));
    IDOSNaturalFractureSet* restored = qobject_cast<IDOSNaturalFractureSet*>(loaded->object(dataId));
    QVERIFY(restored != nullptr);
    IDOSNaturalFractureRenderProvider restoredProvider(restored);
    QCOMPARE(restoredProvider.actors().size(), 1);

    check->trigger();
    dialog = window.findChild<QDialog*>(QStringLiteral("idosFractureGeometryReport"));
    QVERIFY(!dialog.isNull());
    IDOSDataObject* detached = project->take(dataId);
    QCOMPARE(detached, static_cast<IDOSDataObject*>(data));
    QTRY_VERIFY(dialog.isNull());
    QVERIFY(findRenderObject(firstView, dataId) == nullptr);
    QVERIFY(!check->isEnabled());
    project->add(detached);
    project->setCurrentObject(data);
    QVERIFY(findRenderObject(firstView, dataId) != nullptr);
    QVERIFY(check->isEnabled());

    server->setCurrentView(firstView);
    check->trigger();
    dialog = window.findChild<QDialog*>(QStringLiteral("idosFractureGeometryReport"));
    QVERIFY(!dialog.isNull());
    IDOSProject* nextProject = new IDOSProject(&window);
    window.setProject(nextProject);
    QTRY_VERIFY(dialog.isNull());
    QVERIFY(!check->isEnabled());
    window.setProject(project);
    project->setCurrentObject(data);
    check->trigger();
    dialog = window.findChild<QDialog*>(QStringLiteral("idosFractureGeometryReport"));
    QVERIFY(!dialog.isNull());
    plugins->unloadCppPlugin(pluginPath);
    QVERIFY(dialog.isNull());
    QVERIFY(!plugins->isLoaded(QStringLiteral("idosfracturemodelingplugin")));
    QVERIFY(findRenderObject(firstView, dataId) != nullptr);
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
        if (qobject_cast<IDOSGrid*>(object) != nullptr)
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
        IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(object);
        if (property != nullptr)
        {
            ++properties;
            QCOMPARE(property->gridId(), gridId);
            QCOMPARE(property->totalCellCount(), 2);
            QCOMPARE(property->values().at(1), 0.0);
            if (property->kind() == IDOSGridProperty::Kind::Dynamic)
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
        IDOSSimulationSummaryResults* summary = qobject_cast<IDOSSimulationSummaryResults*>(object);
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
    QVERIFY(!manager->execute(QStringLiteral("grid.import"), arguments)
                 .value(QStringLiteral("success")).toBool());
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
    QVERIFY(!manager->execute(QStringLiteral("grid.import"), arguments)
                 .value(QStringLiteral("success")).toBool());
    QCOMPARE(project->objects().size(), 1);
    QVERIFY(caseObject->itemRefs().isEmpty());
    QVERIFY(manager->canRedo());
    QVERIFY(QFile::remove(base + QStringLiteral(".INIT")));
    QVERIFY(QFile::remove(base + QStringLiteral(".UNRST")));
    QFile brokenSummary(base + QStringLiteral(".UNSMRY"));
    QVERIFY(brokenSummary.open(QIODevice::WriteOnly));
    brokenSummary.write("missing specification");
    brokenSummary.close();
    QVERIFY(!manager->execute(QStringLiteral("grid.import"), arguments)
                 .value(QStringLiteral("success")).toBool());
    QCOMPARE(project->objects().size(), 1);
    QVERIFY(manager->canRedo());
    QVERIFY(QFile::remove(brokenSummary.fileName()));
    QVERIFY2(manager->execute(QStringLiteral("grid.import"), arguments)
                 .value(QStringLiteral("success")).toBool(), qPrintable(manager->error()));
    QCOMPARE(project->objects().size(), 2);
    for (IDOSDataObject* object : project->objects())
    {
        if (qobject_cast<IDOSGrid*>(object) != nullptr)
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
    IDOSGrid* otherGrid = new IDOSGrid();
    otherGrid->setDimensions(1, 1, 1);
    project.add(otherGrid);
    IDOSSimulationCaseObject* otherCase = new IDOSSimulationCaseObject();
    otherCase->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), otherGrid->objectId()));
    project.add(otherCase);
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

void CaseWorkflowTest::onWellInputMenus()
{
    IDOSProject project;
    IDOSMainWindow window;
    window.setProject(&project);
    IDOSWell* well = new IDOSWell();
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
    const QModelIndex root = model->indexFromDataId(well->objectId());
    QVERIFY(root.isValid());
    QCOMPARE(model->rowCount(root), 5);
    QScopedPointer<QMenu> summaryMenu(window.dataTreeView()->createMenu(root));
    QAction* summary = summaryMenu->findChild<QAction*>(QStringLiteral("viewWellSummaryAction"));
    QVERIFY(summary != nullptr);
    summary->trigger();
    QDialog* summaryDialog = window.dataTreeView()->findChild<QDialog*>(QStringLiteral("wellInputDataWindow"));
    QVERIFY(summaryDialog != nullptr);
    QCOMPARE(summaryDialog->findChild<QTableWidget*>()->rowCount(), 5);
    delete summaryDialog;

    QScopedPointer<QMenu> trajectoryMenu(window.dataTreeView()->createMenu(model->index(0, 0, root)));
    QAction* trajectoryView = trajectoryMenu->findChild<QAction*>(QStringLiteral("viewWellPathAction"));
    QVERIFY(trajectoryView != nullptr);
    QVERIFY(trajectoryMenu->findChild<QAction*>(QStringLiteral("importWellPathAction")) != nullptr);
    trajectoryView->trigger();
    QDialog* trajectoryDialog = window.dataTreeView()->findChild<QDialog*>(QStringLiteral("wellInputDataWindow"));
    QVERIFY(trajectoryDialog != nullptr);
    QCOMPARE(trajectoryDialog->findChild<QTableWidget*>()->rowCount(), 2);
    QCOMPARE(trajectoryDialog->findChild<QTableWidget*>()->columnCount(), 5);
    delete trajectoryDialog;

    QScopedPointer<QMenu> curvesMenu(window.dataTreeView()->createMenu(model->index(1, 0, root)));
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
        QScopedPointer<QMenu> menu(window.dataTreeView()->createMenu(model->index(row, 0, root)));
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
    QScopedPointer<QMenu> markerMenu(window.dataTreeView()->createMenu(markerIndex));
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
    IDOSWell well;
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
        QCOMPARE(provider.actors().size(), 4);
        vtkPolyDataMapper* mapper = vtkPolyDataMapper::SafeDownCast(provider.actors().first()->GetMapper());
        QVERIFY(mapper != nullptr);
        mapper->Update();
        QCOMPARE(mapper->GetInput()->GetNumberOfLines(), static_cast<vtkIdType>(lineCounts[symbol]));
        QVERIFY(!IDOSWellRenderProvider::symbolIcon(symbol).isNull());
        const QColor expected = symbol == 15 || symbol == 16 ? injector
            : symbol == 2 || (symbol >= 9 && symbol <= 14) || (symbol >= 19 && symbol <= 22) || symbol >= 26
                ? closed : producer;
        const double* color = provider.actors().at(1)->GetProperty()->GetColor();
        QVERIFY(std::fabs(color[0] - expected.redF()) < 1.0e-8);
        QVERIFY(std::fabs(color[1] - expected.greenF()) < 1.0e-8);
        QVERIFY(std::fabs(color[2] - expected.blueF()) < 1.0e-8);
        QVERIFY(provider.actors().at(1)->GetProperty()->GetRenderLinesAsTubes());
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
        const double* color = provider.actors().at(1)->GetProperty()->GetColor();
        QVERIFY(std::fabs(color[0] - expected.redF()) < 1.0e-8);
        QVERIFY(std::fabs(color[1] - expected.greenF()) < 1.0e-8);
        QVERIFY(std::fabs(color[2] - expected.blueF()) < 1.0e-8);
        const QColor expectedCompletion(completionColors.at(index));
        color = provider.actors().at(2)->GetProperty()->GetColor();
        QVERIFY(std::fabs(color[0] - expectedCompletion.redF()) < 1.0e-8);
        QVERIFY(std::fabs(color[1] - expectedCompletion.greenF()) < 1.0e-8);
        QVERIFY(std::fabs(color[2] - expectedCompletion.blueF()) < 1.0e-8);
        const QColor expectedClosed(completionColors.at(index == 3 ? 3 : 2));
        color = provider.actors().at(3)->GetProperty()->GetColor();
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
    QVERIFY(!provider.actors().at(1)->GetProperty()->GetRenderLinesAsTubes());
    well.setType(IDOSWell::Type::Injector);
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
    IDOSWell* well = new IDOSWell();
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
    IDOSRenderView* firstView = qobject_cast<IDOSRenderView*>(tree->targetView());
    QVERIFY(firstView != nullptr);
    IDOSRenderObject* firstObject = findRenderObject(firstView, well->objectId());
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
    const QModelIndex root = model->indexFromDataId(well->objectId());
    QVERIFY(root.isValid());
    QScopedPointer<QMenu> menu(tree->createMenu(root));
    QAction* action = menu->findChild<QAction*>(QStringLiteral("wellDisplaySettingsAction"));
    QVERIFY(action != nullptr);
    QVERIFY(action->isEnabled());
    tree->setTargetView(&secondView);
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
    QCOMPARE(firstProvider->actors().at(1)->GetProperty()->GetLineWidth(), 8.0f);
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
    vtkPolyDataMapper* mapper = vtkPolyDataMapper::SafeDownCast(firstProvider->actors().at(1)->GetMapper());
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
    QCOMPARE(firstProvider->actors().size(), 3);
    QVERIFY(firstProvider->actors().at(2)->GetVisibility() != 0);
    QCheckBox* label = widget->findChild<QCheckBox*>(QStringLiteral("wellDisplay_labelVisible"));
    QVERIFY(label != nullptr);
    label->setChecked(true);
    QCOMPARE(firstProvider->legends().size(), 1);
    QComboBox* shape = widget->findChild<QComboBox*>(QStringLiteral("wellDisplayHeadShape"));
    QVERIFY(shape != nullptr);
    shape->setCurrentIndex(shape->findData(QStringLiteral("sphere")));
    vtkPolyDataMapper* headMapper = vtkPolyDataMapper::SafeDownCast(firstProvider->actors().at(0)->GetMapper());
    QVERIFY(headMapper != nullptr);
    headMapper->Update();
    QVERIFY(headMapper->GetInput()->GetNumberOfPolys() > 0);
    QCheckBox* smooth = widget->findChild<QCheckBox*>(QStringLiteral("wellDisplay_smooth"));
    QVERIFY(smooth != nullptr);
    clip->setChecked(false);
    smooth->setChecked(true);
    mapper = vtkPolyDataMapper::SafeDownCast(firstProvider->actors().at(1)->GetMapper());
    mapper->Update();
    QVERIFY(mapper->GetInput()->GetNumberOfPoints() > path.pointCount());
    const QVariantMap saved = firstProvider->displaySettings();
    firstProvider->setHighlighted(true);
    firstProvider->setHighlighted(false);
    firstProvider->update();
    QCOMPARE(firstProvider->displaySettings(), saved);
    QCOMPARE(well->path().pointCount(), 3);
    QCheckBox* visibility = widget->findChild<QCheckBox*>(QStringLiteral("wellDisplay_visible"));
    QDoubleSpinBox* opacity = widget->findChild<QDoubleSpinBox*>(QStringLiteral("wellDisplay_opacity"));
    QVERIFY(visibility != nullptr && opacity != nullptr);
    opacity->setValue(40.0);
    QCOMPARE(firstProvider->opacity(), 0.4);
    visibility->setChecked(false);
    QVERIFY(!firstProvider->visible());
    visibility->setChecked(true);
    QVERIFY(firstProvider->visible());
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
    tree->setTargetView(firstView);
    const QModelIndex trajectory = model->index(0, 0, root);
    QScopedPointer<QMenu> pathMenu(tree->createMenu(trajectory));
    QAction* pathAction = pathMenu->findChild<QAction*>(QStringLiteral("wellDisplaySettingsAction"));
    QVERIFY(pathAction != nullptr);
    pathAction->trigger();
    widget = window.findChild<QWidget*>(QStringLiteral("wellDisplaySettingsWindow"));
    QVERIFY(!widget.isNull());
    QCOMPARE(widget->findChild<QTabWidget*>()->currentIndex(), 1);
    const QModelIndex completion = model->index(1, 0, root);
    QScopedPointer<QMenu> completionMenu(tree->createMenu(completion));
    QVERIFY(completionMenu->findChild<QAction*>(QStringLiteral("wellDisplaySettingsAction")) == nullptr);
    tree->setTargetView(nullptr);
    QScopedPointer<QMenu> disabledMenu(tree->createMenu(root));
    QAction* disabled = disabledMenu->findChild<QAction*>(QStringLiteral("wellDisplaySettingsAction"));
    QVERIFY(disabled != nullptr && !disabled->isEnabled());
    project.remove(well->objectId());
    QTRY_VERIFY(widget.isNull());
    action->trigger();
    QVERIFY(window.findChild<QWidget*>(QStringLiteral("wellDisplaySettingsWindow")) == nullptr);
    IDOSWell* nextWell = new IDOSWell();
    nextWell->setName(QStringLiteral("Next Well"));
    nextWell->setPath(path);
    project.add(nextWell);
    tree->setTargetView(firstView);
    QScopedPointer<QMenu> nextMenu(tree->createMenu(model->indexFromDataId(nextWell->objectId())));
    QAction* nextAction = nextMenu->findChild<QAction*>(QStringLiteral("wellDisplaySettingsAction"));
    QVERIFY(nextAction != nullptr);
    nextAction->trigger();
    widget = window.findChild<QWidget*>(QStringLiteral("wellDisplaySettingsWindow"));
    QVERIFY(!widget.isNull());
    IDOSRenderObject* nextObject = findRenderObject(firstView, nextWell->objectId());
    QVERIFY(nextObject != nullptr);
    firstView->scene()->remove(nextObject->id());
    QTRY_VERIFY(widget.isNull());
    tree->setTargetView(&secondView);
    IDOSWellRenderProvider* nextProvider = new IDOSWellRenderProvider(nextWell);
    nextProvider->update();
    QVERIFY(secondView.scene()->add(new IDOSRenderObject(nextProvider)));
    QScopedPointer<QMenu> switchMenu(tree->createMenu(model->indexFromDataId(nextWell->objectId())));
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
    tree->setTargetView(&secondView);
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
    QCheckBox* reverse = widget->findChild<QCheckBox*>(QStringLiteral("gridSettingReverseCheck"));
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
    QCOMPARE(firstProvider->actors().first()->GetMapper()->GetScalarRange()[0], 0.0);
    QCOMPARE(firstProvider->actors().first()->GetMapper()->GetScalarRange()[1], 1.0);
    QApplication::processEvents();
    const QString screenshotDirectory = qEnvironmentVariable("IDOS_TEST_SCREENSHOT_DIR", QDir::tempPath());
    QVERIFY(widget->grab().save(QDir(screenshotDirectory).filePath(QStringLiteral("grid-setting-coloring.png"))));
    minimum->setValue(2.0);
    QCOMPARE(firstProvider->coloringSettings().value(QStringLiteral("minimum")).toDouble(), 0.0);
    minimum->setValue(0.0);
    QCheckBox* legend = widget->findChild<QCheckBox*>(QStringLiteral("gridSettingLegendCheck"));
    QVERIFY(legend != nullptr);
    legend->setChecked(false);
    QVERIFY(firstProvider->legends().isEmpty());
    QVERIFY(property->setValues(QVector<double>{std::numeric_limits<double>::quiet_NaN()}));
    QVERIFY(std::isnan(firstProvider->actors().first()->GetMapper()->GetInput()->GetCellData()->GetScalars()->GetTuple1(0)));
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
        QCOMPARE(firstProvider->actors().first()->GetProperty()->GetRepresentation(), representation);
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
    QCOMPARE(firstProvider->actors().first()->GetProperty()->GetLineWidth(), 4.5f);
    QCOMPARE(secondProvider->opacity(), 1.0);
    QCOMPARE(secondProvider->edgeWidth(), 1.0);
    edgeColor->click();
    QColorDialog* colorDialog = widget->findChild<QColorDialog*>();
    QVERIFY(colorDialog != nullptr);
    colorDialog->setCurrentColor(QColor(QStringLiteral("#123456")));
    colorDialog->accept();
    QCOMPARE(firstProvider->edgeColor(), QColor(QStringLiteral("#123456")));
    QCOMPARE(secondProvider->edgeColor(), QColor(Qt::black));
    firstProvider->setHighlighted(true);
    firstProvider->setHighlighted(false);
    const QColor restored(firstProvider->edgeColor());
    const double* restoredColor = firstProvider->actors().first()->GetProperty()->GetEdgeColor();
    QVERIFY(std::fabs(restoredColor[0] - restored.redF()) < 1.0e-8);
    QVERIFY(std::fabs(restoredColor[1] - restored.greenF()) < 1.0e-8);
    QVERIFY(std::fabs(restoredColor[2] - restored.blueF()) < 1.0e-8);
    QCOMPARE(firstProvider->actors().first()->GetProperty()->GetLineWidth(), 4.5f);
    visible->setChecked(false);
    QVERIFY(!firstProvider->visible());
    QCOMPARE(firstProvider->property(), property);
    visible->setChecked(true);
    QVERIFY(firstProvider->visible());
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
    tree->setTargetView(firstView);
    const QModelIndex gridIndex = window.caseTreeModel()->indexFromDataId(grid->objectId());
    QVERIFY(gridIndex.isValid());
    QScopedPointer<QMenu> gridMenu(tree->createMenu(gridIndex));
    QVERIFY(gridMenu->findChild<QAction*>(QStringLiteral("deleteGridAction")) != nullptr);
    QCOMPARE(project.currentObject(), static_cast<IDOSDataObject*>(grid));
    QCOMPARE(project.currentCaseId(), simulationCase->objectId());
    QAction* gridSettings = gridMenu->findChild<QAction*>(QStringLiteral("gridSettingsAction"));
    QVERIFY(gridSettings != nullptr);
    QCOMPARE(gridMenu->findChildren<QAction*>(QStringLiteral("gridSettingsAction")).size(), 1);
    tree->setTargetView(&secondView);
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
    QCOMPARE(server.currentView(), &first);
    QVERIFY(server.setCurrentView(&second));
    QCOMPARE(server.currentView(), &second);
    QVERIFY(server.setCurrentView(&first));

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
    IDOSFluidRockTable* table = nullptr;
    for (IDOSDataObject* object : project->objects())
    {
        IDOSFluidRockTable* candidate = qobject_cast<IDOSFluidRockTable*>(object);
        if (candidate != nullptr)
        {
            table = candidate;
        }
    }
    QVERIFY(table != nullptr);
    QCOMPARE(table->category(), IDOSFluidRockTable::Category::Fluid);
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
    :
    m_cancel(false)
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
    IDOSGrid* grid = new IDOSGrid();
    grid->setName(QStringLiteral("Filter Grid"));
    grid->setDimensions(3, 2, 1);
    grid->setCornerPointData(3, 2, 1, grid->coord(), grid->zcorn(), QVector<int>{1, 1, 1, 1, 0, 1});
    project.add(grid);
    IDOSGridProperty* property = new IDOSGridProperty();
    property->setName(QStringLiteral("Pressure"));
    property->setGridId(grid->objectId());
    property->setDimensions(3, 2, 1);
    property->setKind(IDOSGridProperty::Kind::Dynamic);
    QVERIFY(property->setReportStepValues(0, QVector<double>{10, 20, 30, 40, 50, 60}));
    QVERIFY(property->setReportStepValues(1, QVector<double>{60, 50, 40, 30, 20, 10}));
    QVERIFY(property->setCurrentReportStep(0));
    project.add(property);
    IDOSSimulationCaseObject* simulationCase = new IDOSSimulationCaseObject();
    simulationCase->setName(QStringLiteral("Filter Case"));
    simulationCase->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), grid->objectId()));
    project.add(simulationCase);
    IDOSTreeView* tree = window.caseTreeView();
    IDOSRenderView* view = qobject_cast<IDOSRenderView*>(tree->targetView());
    QVERIFY(view != nullptr);
    IDOSRenderObject* sourceObject = findRenderObject(view, grid->objectId());
    QVERIFY(sourceObject != nullptr);
    IDOSGridRenderProvider* sourceProvider = dynamic_cast<IDOSGridRenderProvider*>(sourceObject->provider());
    QVERIFY(sourceProvider != nullptr);
    QCOMPARE(sourceProvider->filteredCellCount(), 5);
    IDOSRenderView otherView;
    IDOSGridRenderProvider* otherProvider = new IDOSGridRenderProvider(grid);
    otherProvider->update();
    QVERIFY(otherView.scene()->add(new IDOSRenderObject(otherProvider)));
    const QModelIndex gridIndex = window.caseTreeModel()->indexFromDataId(grid->objectId());
    QVERIFY(gridIndex.isValid());
    QScopedPointer<QMenu> menu(tree->createMenu(gridIndex));
    QAction* action = menu->findChild<QAction*>(QStringLiteral("gridFilterAction"));
    QVERIFY(action != nullptr && action->isEnabled());
    tree->setTargetView(&otherView);
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
    vtkDataSet* selected = sourceProvider->actors().first()->GetMapper()->GetInput();
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
    IDOSGridFilter* filter = qobject_cast<IDOSGridFilter*>(project.object(filterId));
    QVERIFY(filter != nullptr);
    QCOMPARE(filter->gridId(), grid->objectId());
    QCOMPARE(filter->containerId(), grid->objectId());
    QCOMPARE(project.objects().size(), 4);
    const QVariantMap savedCriteria = filter->criteria();
    QVERIFY(savedCriteria != appliedCriteria);
    QCOMPARE(sourceProvider->filterCriteria(), savedCriteria);
    QVERIFY(project.commandManager()->contains(QStringLiteral("grid.filter.create")));
    QMap<QString, IDOSGridProperty*> references;
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
        const QVector<quint8> mask = IDOSGridFilter::selection(comparisonCriteria, 3, 2, 1, references);
        int matched = 0;
        for (int cell = 0; cell < mask.size(); ++cell)
        {
            matched += mask.at(cell);
            QCOMPARE(mask.at(cell) != 0, IDOSGridFilter::matches(comparisonCriteria, cell % 3, cell / 3, 0, cell, references));
        }
        QCOMPARE(matched, expected.at(operation));
    }
    QVariantMap secondCondition = testCondition;
    secondCondition.insert(QStringLiteral("minimum"), 60.0);
    secondCondition.insert(QStringLiteral("maximum"), 60.0);
    QVariantMap combinationCriteria;
    combinationCriteria.insert(QStringLiteral("conditions"), QVariantList{testCondition, secondCondition});
    const QVector<quint8> intersected = IDOSGridFilter::selection(combinationCriteria, 3, 2, 1, references);
    QCOMPARE(intersected.size(), 6);
    for (quint8 selected : intersected)
    {
        QCOMPARE(selected, quint8(0));
    }
    combinationCriteria.insert(QStringLiteral("anyProperty"), true);
    const QVector<quint8> united = IDOSGridFilter::selection(combinationCriteria, 3, 2, 1, references);
    QCOMPARE(united.size(), 6);
    for (int index = 0; index < united.size(); ++index)
    {
        QCOMPARE(united.at(index), quint8(index == 2 || index == 5 ? 1 : 0));
    }
    QString validationError;
    QVariantMap malformed;
    malformed.insert(QStringLiteral("conditions"), QStringLiteral("invalid"));
    QVERIFY(!IDOSGridFilter::validateCriteria(malformed, validationError));
    IDOSRenderObject* resultObject = findRenderObject(view, filterId);
    QVERIFY(resultObject != nullptr && resultObject->visible());
    IDOSGridRenderProvider* resultProvider = dynamic_cast<IDOSGridRenderProvider*>(resultObject->provider());
    QVERIFY(resultProvider != nullptr);
    QCOMPARE(resultProvider->filteredCellCount(), 2);
    QCOMPARE(resultProvider->property(), property);
    QVERIFY(window.caseTreeModel()->indexFromDataId(filterId).isValid());
    QVERIFY(sourceObject->visible());
    QCOMPARE(sourceProvider->previewFilterId(), filterId);
    tree->setTargetView(view);
    const QModelIndex filterIndex = window.caseTreeModel()->indexFromDataId(filterId);
    QVERIFY(window.caseTreeModel()->setData(filterIndex, Qt::Unchecked, Qt::CheckStateRole));
    QVERIFY(!resultObject->visible());
    QVERIFY(sourceObject->visible());
    QCOMPARE(sourceProvider->filteredCellCount(), 5);
    QVERIFY(sourceProvider->filterCriteria().isEmpty());
    QVERIFY(sourceProvider->previewFilterId().isEmpty());
    QCOMPARE(sourceProvider->property(), property);
    QCOMPARE(filter->criteria(), savedCriteria);
    QCOMPARE(otherProvider->filteredCellCount(), 5);
    QAction* clearDisplay = window.findChild<QAction*>(QStringLiteral("viewToolClearDisplayAction"));
    QVERIFY(clearDisplay != nullptr);
    clearDisplay->trigger();
    QVERIFY(!sourceObject->visible());
    QVERIFY(!resultObject->visible());
    clearDisplay->trigger();
    QVERIFY(sourceObject->visible());
    QVERIFY(!resultObject->visible());
    QVERIFY(sourceProvider->filterCriteria().isEmpty());
    QVERIFY(window.caseTreeModel()->setData(filterIndex, Qt::Checked, Qt::CheckStateRole));
    QVERIFY(resultObject->visible());
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
    QScopedPointer<QMenu> resultMenu(tree->createMenu(window.caseTreeModel()->indexFromDataId(filterId)));
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
    QCOMPARE(resultProvider->property(), static_cast<IDOSGridProperty*>(nullptr));
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
    QVERIFY(!project.commandManager()->execute(new IDOSGridFilterCommand(&project, QStringLiteral("update"), invalidArguments)));
    QCOMPARE(filter->criteria(), savedCriteria);
    QVariantMap invertedSaved = savedCriteria;
    invertedSaved.insert(QStringLiteral("invert"), true);
    filter->setCriteria(invertedSaved);
    IDOSDataObject* detachedProperty = project.take(property->objectId());
    QVERIFY(resultProvider->actors().isEmpty());
    QVERIFY(!resultProvider->filterError().isEmpty());
    QCOMPARE(filter->criteria(), invertedSaved);
    filter->setCriteria(savedCriteria);
    project.add(detachedProperty);
    QCOMPARE(resultProvider->filteredCellCount(), 1);
    QJsonObject deleteArguments;
    deleteArguments.insert(QStringLiteral("filterId"), filterId);
    QVERIFY(sourceProvider->setFilterCriteria(savedCriteria));
    sourceProvider->setPreviewFilterId(filterId);
    sourceProvider->setDataReference(property);
    sourceProvider->update();
    QCOMPARE(sourceProvider->filteredCellCount(), 1);
    QVERIFY(project.commandManager()->execute(QStringLiteral("grid.filter.delete"), deleteArguments).value(QStringLiteral("success")).toBool());
    QVERIFY(project.object(filterId) == nullptr);
    QVERIFY(findRenderObject(view, filterId) == nullptr);
    QCOMPARE(sourceProvider->filteredCellCount(), 5);
    QVERIFY(sourceProvider->previewFilterId().isEmpty());
    project.commandManager()->undo();
    QCOMPARE(project.object(filterId), static_cast<IDOSDataObject*>(filter));
    QVERIFY(window.caseTreeModel()->indexFromDataId(filterId).isValid());
    project.commandManager()->redo();
    QVERIFY(project.object(filterId) == nullptr);
    project.commandManager()->undo();
    const QString gridId = grid->objectId();
    QVERIFY(project.commandManager()->execute(new IDOSDeleteGridCommand(&project, gridId)));
    QVERIFY(project.object(gridId) == nullptr && project.object(filterId) == nullptr);
    project.commandManager()->undo();
    QCOMPARE(project.object(gridId), static_cast<IDOSDataObject*>(grid));
    QCOMPARE(project.object(filterId), static_cast<IDOSDataObject*>(filter));
    QVERIFY(findRenderObject(view, filterId) != nullptr);
    project.commandManager()->redo();
    QVERIFY(project.object(filterId) == nullptr);
    project.commandManager()->undo();
    project.commandManager()->undo();
    QVERIFY(project.object(filterId) == nullptr);
    project.commandManager()->redo();
    QCOMPARE(project.object(filterId), static_cast<IDOSDataObject*>(filter));
    QVERIFY(findRenderObject(view, filterId) != nullptr);
    QApplication::removeTranslator(&translator);
}

void CaseWorkflowTest::onGridFilterCriteria()
{
    IDOSGridProperty property;
    property.setDimensions(3, 2, 1);
    const double invalid = std::numeric_limits<double>::quiet_NaN();
    QVERIFY(property.setValues(QVector<double>{0.0, 1.0, 2.0, invalid, 4.0, 5.0}));
    QMap<QString, IDOSGridProperty*> references;
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
        const QVector<quint8> mask = IDOSGridFilter::selection(criteria, 3, 2, 1, references);
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
    QVERIFY(sameSelection(IDOSGridFilter::selection(criteria, 3, 2, 1, references), QVector<quint8>({0, 0, 1, 0, 0, 0})));
    condition.insert(QStringLiteral("includeInvalidValues"), true);
    criteria.insert(QStringLiteral("conditions"), QVariantList{condition});
    QVERIFY(sameSelection(IDOSGridFilter::selection(criteria, 3, 2, 1, references), QVector<quint8>({0, 0, 1, 1, 0, 0})));
    criteria.insert(QStringLiteral("propertyOperation"), QStringLiteral("hide"));
    QVERIFY(sameSelection(IDOSGridFilter::selection(criteria, 3, 2, 1, references), QVector<quint8>({1, 1, 0, 0, 1, 1})));
    condition.insert(QStringLiteral("propertyId"), QStringLiteral("missing"));
    criteria.insert(QStringLiteral("conditions"), QVariantList{condition});
    QVERIFY(sameSelection(IDOSGridFilter::selection(criteria, 3, 2, 1, references), QVector<quint8>({0, 0, 0, 0, 0, 0})));
    condition.insert(QStringLiteral("enabled"), false);
    criteria.insert(QStringLiteral("conditions"), QVariantList{condition});
    QVERIFY(sameSelection(IDOSGridFilter::selection(criteria, 3, 2, 1, references), QVector<quint8>({1, 1, 1, 1, 1, 1})));
    QVERIFY(IDOSGridFilter::propertyIds(criteria).isEmpty());
    QVERIFY(property.setValues(QVector<double>{1.0, 1.0000005, 1.000002, invalid, 4.0, 5.0}));
    condition.insert(QStringLiteral("propertyId"), property.objectId());
    condition.insert(QStringLiteral("enabled"), true);
    condition.insert(QStringLiteral("operation"), QStringLiteral("equal"));
    condition.insert(QStringLiteral("epsilon"), 1.0e-6);
    condition.insert(QStringLiteral("includeInvalidValues"), false);
    criteria.remove(QStringLiteral("propertyOperation"));
    criteria.insert(QStringLiteral("conditions"), QVariantList{condition});
    QVERIFY(sameSelection(IDOSGridFilter::selection(criteria, 3, 2, 1, references), QVector<quint8>({1, 1, 0, 0, 0, 0})));
    condition.insert(QStringLiteral("operation"), QStringLiteral("notEqual"));
    criteria.insert(QStringLiteral("conditions"), QVariantList{condition});
    QVERIFY(sameSelection(IDOSGridFilter::selection(criteria, 3, 2, 1, references), QVector<quint8>({0, 0, 1, 0, 1, 1})));
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
    QVERIFY(sameSelection(IDOSGridFilter::selection(criteria, 3, 2, 1, references), QVector<quint8>({1, 1, 0, 1, 1, 0})));
    axes.insert(QStringLiteral("j"), jAxis);
    criteria.insert(QStringLiteral("axes"), axes);
    criteria.insert(QStringLiteral("axisUnion"), true);
    QVERIFY(sameSelection(IDOSGridFilter::selection(criteria, 3, 2, 1, references), QVector<quint8>({1, 1, 1, 1, 1, 0})));
    criteria.insert(QStringLiteral("axisUnion"), false);
    QVERIFY(sameSelection(IDOSGridFilter::selection(criteria, 3, 2, 1, references), QVector<quint8>({1, 1, 0, 0, 0, 0})));
    criteria.insert(QStringLiteral("axisOperation"), QStringLiteral("hide"));
    const QVector<quint8> hidden = IDOSGridFilter::selection(criteria, 3, 2, 1, references);
    QVERIFY(sameSelection(hidden, QVector<quint8>({0, 0, 1, 1, 1, 1})));
    criteria.insert(QStringLiteral("axisOperation"), QStringLiteral("invert"));
    QVERIFY(sameSelection(IDOSGridFilter::selection(criteria, 3, 2, 1, references), hidden));
    QString error;
    iAxis.insert(QStringLiteral("values"), QVariantList{0});
    axes.insert(QStringLiteral("i"), iAxis);
    criteria.insert(QStringLiteral("axes"), axes);
    QVERIFY(!IDOSGridFilter::validateCriteria(criteria, error));
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
