#ifndef CASE_WORKFLOW_TEST_H
#define CASE_WORKFLOW_TEST_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVector>

class IDOSRenderObject;
class IDOSRenderView;
class QTemporaryDir;

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
    void onWellDisplaySettings();
    void onWellSymbolsAndStatusColors();
    void onRenderLifecycle();
    void onRenderArchitecture();
    void onExamplePlugin();
    void onCreateAndImport();
    void onFluidRockReferences();
    void onDialog();
    void onFlowExecution();
    void onSimulationOutput();
    void onFlowOutputFraming();

  private:
    QVariantMap fracture(const QVector<double>& coordinates, int id) const;
    QStringList fractureIssueCodes(const QVariantMap& report) const;
    bool sameSelection(const QVector<quint8>& first, const QVector<quint8>& second) const;
    void onCloseContextMenu();
    void onExportCurveDialog();
    IDOSRenderObject* findRenderObject(IDOSRenderView* view, const QString& dataId) const;
    QString m_name;
    bool m_cancel;
    bool m_expectDisabled;
    QTemporaryDir* m_settingsDirectory;
    QSettings::Format m_previousSettingsFormat;
    QString m_previousSettingsRoot;
};
#endif
