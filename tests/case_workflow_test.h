#ifndef CASE_WORKFLOW_TEST_H
#define CASE_WORKFLOW_TEST_H

#include <QElapsedTimer>
#include <QList>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVector>

class IDOSRenderObject;
class IDOSRenderView;
class IDOSRenderServer;
class QTemporaryDir;

class IDOSGridObject;
class IDOSGridPropertyEclipseProvider;
class QThread;
class IDOSDataObject;

class vtkActor;
class vtkObject;

class vtkProp;

class IDOSRenderProvider;

class CaseWorkflowTest : public QObject
{
    Q_OBJECT

  public:
    CaseWorkflowTest();
    ~CaseWorkflowTest() override;
  private slots:
    void onProvider();
    void onProviderDimensions();
    void onGridCompanionFiles();
    void onFractureGeometry();
    void onDfnGeneration();
    void onFieldDifference();
    void onCurveEvaluation();
    void onFractureDisplay();
    void onDeleteImportedData();
    void onDataGroupChecks();
    void onConfirmDataDeletion();
    void onViewDecorations();
    void onPlatformSimulation();
    void onGridSettings();
    void onGridFilters();
    void onGridFilterCriteria();
    void onWellInputMenus();
    void onDataTreeImportMenus();
    void onDismissImportDialog();
    void onChooseImportMenu();
    void onWellDisplaySettings();
    void onWellSymbolsAndStatusColors();
    void onRenderLifecycle();
    void onRenderStateAndUndo();
    void onRenderRestoreClipping();
    void onRenderArchitecture();
    void onRenderPerformance();
    void onRenderPickingCache();
    void onGridSurfaceTopology();
    void onToolbarTools();
    void onBackgroundImport();
    void onWaitingDialog();
    void onRestartMemory();
    void onTreeCheckRendering();
    void onMemorySample();
    void onImportPerformance();
    void onSyntheticDemoData();
    void onProjectCoordinates();
    void onSubmitProjectCoordinates();
    void onImportHeartbeat();
    void onAcceptToolDialog();
    void onExamplePlugin();
    void onExamplePluginMenu();
    void onCreateAndImport();
    void onFluidRockReferences();
    void onDialog();
    void onFlowExecution();
    void onSimulationOutput();
    void onFlowOutputFraming();

  private:
    void onProfileRenderStarted(vtkObject* source, unsigned long eventId, void* callData);
    void onProfileRenderEnded(vtkObject* source, unsigned long eventId, void* callData);
    static quint64 processPrivateBytes();
    static void readRestartForMemory(IDOSGridPropertyEclipseProvider* provider, const QString& path,
                                     const IDOSGridObject* grid, QThread* targetThread,
                                     QList<IDOSDataObject*>* objects);
    quint64 m_memoryPeak;
    int m_importHeartbeatCount;
    QElapsedTimer m_profileFrameTimer;
    qint64 m_profileRenderNanoseconds;
    int m_profileRenderCount;
    static QList<vtkProp*> legends(const IDOSRenderProvider* provider);
    static QList<vtkActor*> actors(const IDOSRenderProvider* provider);
    QVariantMap fracture(const QVector<double>& coordinates, int id) const;
    QStringList fractureIssueCodes(const QVariantMap& report) const;
    bool sameSelection(const QVector<quint8>& first, const QVector<quint8>& second) const;
    void onCloseContextMenu();
    void onExportCurveDialog();
    IDOSRenderObject* findRenderObject(IDOSRenderServer* server, IDOSRenderView* view, const QString& dataId) const;
    QString m_name;
    bool m_cancel;
    bool m_expectDisabled;
    QTemporaryDir* m_settingsDirectory;
    QSettings::Format m_previousSettingsFormat;
    QString m_previousSettingsRoot;
};
#endif
