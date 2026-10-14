#ifndef IDOS_SIMULATION_RESULT_PLAYBACK_WIDGET_H
#define IDOS_SIMULATION_RESULT_PLAYBACK_WIDGET_H

#include <QList>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QWidget>

#include "idos_app.h"

class IDOSDataObject;
class IDOSGridProperty;
class IDOSProject;
class IDOSSimulationCaseObject;
class QComboBox;
class QLabel;
class QSlider;
class QSpinBox;
class QToolButton;
class QTimer;

class APP_EXPORT IDOSSimulationResultPlaybackWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSSimulationResultPlaybackWidget(QWidget* parent = nullptr);
    void setCurrentObject(IDOSDataObject* object);

  private slots:
    void onPropertyChanged(int index);
    void onReportStepChanged(int index);
    void onPreviousTriggered();
    void onPlayTriggered();
    void onNextTriggered();
    void onIntervalChanged(int interval);
    void onTimerTimeout();
    void onPropertyDataChanged();
    void onProjectObjectsChanged(const QStringList& objectIds);

  private:
    IDOSSimulationCaseObject* resolveSimulationCase(IDOSDataObject* object) const;
    void refreshProperties();
    void refreshReportSteps();
    void updateControls();
    void reportCurrentStep();

    QPointer<IDOSProject> m_project;
    QPointer<IDOSSimulationCaseObject> m_simulationCase;
    QPointer<IDOSGridProperty> m_property;
    QComboBox* m_propertyCombo;
    QSlider* m_reportStepSlider;
    QLabel* m_reportStepLabel;
    QToolButton* m_previousButton;
    QToolButton* m_playButton;
    QToolButton* m_nextButton;
    QSpinBox* m_intervalSpinBox;
    QTimer* m_timer;
    QList<int> m_reportSteps;
    QString m_loggedPropertyId;
    int m_loggedReportStep;
};

#endif // IDOS_SIMULATION_RESULT_PLAYBACK_WIDGET_H
