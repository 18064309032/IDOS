#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QString>
#include <QStyle>
#include <QtGlobal>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include "idoscaseitemref.h"
#include "idosdataobject.h"
#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idosproject.h"
#include "idossimulationcaseobject.h"

#include "idossimulationresultplaybackwidget.h"

IDOSSimulationResultPlaybackWidget::IDOSSimulationResultPlaybackWidget(QWidget* parent)
    :
    QWidget(parent)
    , m_project(nullptr)
    , m_simulationCase(nullptr)
    , m_property(nullptr)
    , m_propertyCombo(nullptr)
    , m_reportStepSlider(nullptr)
    , m_reportStepLabel(nullptr)
    , m_previousButton(nullptr)
    , m_playButton(nullptr)
    , m_nextButton(nullptr)
    , m_intervalSpinBox(nullptr)
    , m_statusLabel(nullptr)
    , m_timer(new QTimer(this))
    , m_reportSteps()
{
    setObjectName(QStringLiteral("simulationResultPlaybackWidget"));
    setMinimumWidth(300);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(2);

    m_propertyCombo = new QComboBox(this);
    m_propertyCombo->setMinimumContentsLength(14);
    m_propertyCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_propertyCombo->setToolTip(tr("Select a dynamic property to browse."));
    layout->addWidget(m_propertyCombo);

    QHBoxLayout* playbackLayout = new QHBoxLayout();
    playbackLayout->setSpacing(2);
    m_previousButton = new QToolButton(this);
    m_previousButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    m_previousButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_previousButton->setAutoRaise(true);
    m_previousButton->setAccessibleName(tr("Previous"));
    m_previousButton->setToolTip(tr("Show the previous report step."));
    m_playButton = new QToolButton(this);
    m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    m_playButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_playButton->setAutoRaise(true);
    m_playButton->setAccessibleName(tr("Play"));
    m_playButton->setToolTip(tr("Play report steps in sequence."));
    m_nextButton = new QToolButton(this);
    m_nextButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    m_nextButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_nextButton->setAutoRaise(true);
    m_nextButton->setAccessibleName(tr("Next"));
    m_nextButton->setToolTip(tr("Show the next report step."));
    playbackLayout->addWidget(m_previousButton);
    playbackLayout->addWidget(m_playButton);
    playbackLayout->addWidget(m_nextButton);

    m_reportStepSlider = new QSlider(Qt::Horizontal, this);
    m_reportStepSlider->setToolTip(tr("Choose a report step."));
    playbackLayout->addWidget(m_reportStepSlider, 1);
    m_reportStepLabel = new QLabel(tr("No step"), this);
    m_reportStepLabel->setMinimumWidth(62);
    m_reportStepLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    playbackLayout->addWidget(m_reportStepLabel);
    layout->addLayout(playbackLayout);

    QHBoxLayout* intervalLayout = new QHBoxLayout();
    intervalLayout->addStretch(1);
    intervalLayout->addWidget(new QLabel(tr("Playback interval"), this));
    m_intervalSpinBox = new QSpinBox(this);
    m_intervalSpinBox->setRange(100, 10000);
    m_intervalSpinBox->setSingleStep(100);
    m_intervalSpinBox->setValue(500);
    m_intervalSpinBox->setSuffix(tr(" ms"));
    m_intervalSpinBox->setToolTip(tr("Set the delay between report steps."));
    intervalLayout->addWidget(m_intervalSpinBox);
    layout->addLayout(intervalLayout);

    m_statusLabel = new QLabel(tr("Select a simulation case to browse imported results."), this);
    m_statusLabel->setWordWrap(true);
    layout->addWidget(m_statusLabel);

    m_timer->setInterval(m_intervalSpinBox->value());
    connect(m_propertyCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &IDOSSimulationResultPlaybackWidget::onPropertyChanged);
    connect(m_reportStepSlider,
            &QSlider::valueChanged,
            this,
            &IDOSSimulationResultPlaybackWidget::onReportStepChanged);
    connect(m_previousButton,
            &QToolButton::clicked,
            this,
            &IDOSSimulationResultPlaybackWidget::onPreviousTriggered);
    connect(m_playButton,
            &QToolButton::clicked,
            this,
            &IDOSSimulationResultPlaybackWidget::onPlayTriggered);
    connect(m_nextButton,
            &QToolButton::clicked,
            this,
            &IDOSSimulationResultPlaybackWidget::onNextTriggered);
    connect(m_intervalSpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &IDOSSimulationResultPlaybackWidget::onIntervalChanged);
    connect(m_timer,
            &QTimer::timeout,
            this,
            &IDOSSimulationResultPlaybackWidget::onTimerTimeout);

    updateControls();
}

void IDOSSimulationResultPlaybackWidget::setCurrentObject(IDOSDataObject* object)
{
    IDOSSimulationCaseObject* simulationCase = resolveSimulationCase(object);
    IDOSProject* project = simulationCase != nullptr
        ? qobject_cast<IDOSProject*>(simulationCase->parent())
        : nullptr;
    if (m_project != project)
    {
        if (m_project != nullptr)
        {
            disconnect(m_project, nullptr, this, nullptr);
        }
        m_project = project;
        if (m_project != nullptr)
        {
            connect(m_project,
                    &IDOSProject::objectsAdded,
                    this,
                    &IDOSSimulationResultPlaybackWidget::onProjectObjectsChanged);
            connect(m_project,
                    &IDOSProject::objectsRemoved,
                    this,
                    &IDOSSimulationResultPlaybackWidget::onProjectObjectsChanged);
        }
    }
    if (m_simulationCase == simulationCase)
    {
        return;
    }

    m_timer->stop();
    m_simulationCase = simulationCase;
    refreshProperties();
}

void IDOSSimulationResultPlaybackWidget::onPropertyChanged(int index)
{
    m_timer->stop();
    m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    m_playButton->setAccessibleName(tr("Play"));
    m_playButton->setToolTip(tr("Play report steps in sequence."));
    if (!m_property.isNull())
    {
        disconnect(m_property.data(),
                   &IDOSDataObject::dataChanged,
                   this,
                   &IDOSSimulationResultPlaybackWidget::onPropertyDataChanged);
    }
    if (index < 0 || m_project.isNull())
    {
        m_property = nullptr;
        m_reportSteps.clear();
        updateControls();
        return;
    }

    m_property = qobject_cast<IDOSGridProperty*>(
        m_project->object(m_propertyCombo->itemData(index).toString()));
    if (!m_property.isNull())
    {
        connect(m_property.data(),
                &IDOSDataObject::dataChanged,
                this,
                &IDOSSimulationResultPlaybackWidget::onPropertyDataChanged);
    }
    refreshReportSteps();
}

void IDOSSimulationResultPlaybackWidget::onReportStepChanged(int index)
{
    if (m_property.isNull() || index < 0 || index >= m_reportSteps.size())
    {
        updateControls();
        return;
    }

    const int reportStep = m_reportSteps.at(index);
    if (!m_property->setCurrentReportStep(reportStep))
    {
        m_statusLabel->setText(tr("The selected report step is unavailable."));
        return;
    }
    m_reportStepLabel->setText(tr("Step %1").arg(reportStep));
    m_statusLabel->setText(tr("Displaying %1 at report step %2.")
                               .arg(m_property->keyword())
                               .arg(reportStep));
    updateControls();
}

void IDOSSimulationResultPlaybackWidget::onPreviousTriggered()
{
    if (m_reportStepSlider->value() > m_reportStepSlider->minimum())
    {
        m_reportStepSlider->setValue(m_reportStepSlider->value() - 1);
    }
}

void IDOSSimulationResultPlaybackWidget::onPlayTriggered()
{
    if (m_timer->isActive())
    {
        m_timer->stop();
        m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        m_playButton->setAccessibleName(tr("Play"));
        m_playButton->setToolTip(tr("Play report steps in sequence."));
        return;
    }
    if (m_reportSteps.size() < 2)
    {
        return;
    }
    if (m_reportStepSlider->value() >= m_reportStepSlider->maximum())
    {
        m_reportStepSlider->setValue(m_reportStepSlider->minimum());
    }
    m_timer->start();
    m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    m_playButton->setAccessibleName(tr("Pause"));
    m_playButton->setToolTip(tr("Pause report-step playback."));
}

void IDOSSimulationResultPlaybackWidget::onNextTriggered()
{
    if (m_reportStepSlider->value() < m_reportStepSlider->maximum())
    {
        m_reportStepSlider->setValue(m_reportStepSlider->value() + 1);
    }
}

void IDOSSimulationResultPlaybackWidget::onIntervalChanged(int interval)
{
    m_timer->setInterval(interval);
}

void IDOSSimulationResultPlaybackWidget::onTimerTimeout()
{
    if (m_reportStepSlider->value() >= m_reportStepSlider->maximum())
    {
        m_timer->stop();
        m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        m_playButton->setAccessibleName(tr("Play"));
        m_playButton->setToolTip(tr("Play report steps in sequence."));
        updateControls();
        return;
    }
    m_reportStepSlider->setValue(m_reportStepSlider->value() + 1);
}

void IDOSSimulationResultPlaybackWidget::onPropertyDataChanged()
{
    if (m_property.isNull())
    {
        return;
    }
    const int reportStepIndex = m_reportSteps.indexOf(m_property->currentReportStep());
    if (reportStepIndex < 0)
    {
        return;
    }
    m_reportStepSlider->blockSignals(true);
    m_reportStepSlider->setValue(reportStepIndex);
    m_reportStepSlider->blockSignals(false);
    m_reportStepLabel->setText(tr("Step %1").arg(m_property->currentReportStep()));
    updateControls();
}

void IDOSSimulationResultPlaybackWidget::onProjectObjectsChanged(const QStringList& objectIds)
{
    Q_UNUSED(objectIds);
    refreshProperties();
}

IDOSSimulationCaseObject* IDOSSimulationResultPlaybackWidget::resolveSimulationCase(
    IDOSDataObject* object) const
{
    IDOSSimulationCaseObject* simulationCase = qobject_cast<IDOSSimulationCaseObject*>(object);
    if (simulationCase != nullptr)
    {
        return simulationCase;
    }

    QString gridId;
    IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(object);
    if (property != nullptr)
    {
        gridId = property->gridId();
    }
    else if (qobject_cast<IDOSGrid*>(object) != nullptr)
    {
        gridId = object->objectId();
    }
    if (gridId.isEmpty())
    {
        return nullptr;
    }

    IDOSProject* project = qobject_cast<IDOSProject*>(object->parent());
    if (project == nullptr)
    {
        return nullptr;
    }
    for (IDOSDataObject* candidate : project->objects())
    {
        simulationCase = qobject_cast<IDOSSimulationCaseObject*>(candidate);
        if (simulationCase == nullptr)
        {
            continue;
        }
        for (const IDOSCaseItemRef& reference : simulationCase->itemRefs())
        {
            if (reference.role() == QStringLiteral("case.grid") && reference.objectId() == gridId)
            {
                return simulationCase;
            }
        }
    }
    return nullptr;
}

void IDOSSimulationResultPlaybackWidget::refreshProperties()
{
    m_timer->stop();
    m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    m_playButton->setAccessibleName(tr("Play"));
    m_playButton->setToolTip(tr("Play report steps in sequence."));
    const QString previousPropertyId = m_propertyCombo->currentData().toString();
    if (!m_property.isNull())
    {
        disconnect(m_property.data(),
                   &IDOSDataObject::dataChanged,
                   this,
                   &IDOSSimulationResultPlaybackWidget::onPropertyDataChanged);
    }
    m_property = nullptr;
    m_reportSteps.clear();
    m_propertyCombo->blockSignals(true);
    m_propertyCombo->clear();

    if (m_project.isNull() || m_simulationCase.isNull())
    {
        m_propertyCombo->blockSignals(false);
        m_statusLabel->setText(tr("Select a simulation case to browse imported results."));
        updateControls();
        return;
    }

    QString gridId;
    for (const IDOSCaseItemRef& reference : m_simulationCase->itemRefs())
    {
        if (reference.role() == QStringLiteral("case.grid")
            && qobject_cast<IDOSGrid*>(m_project->object(reference.objectId())) != nullptr)
        {
            gridId = reference.objectId();
            break;
        }
    }

    for (IDOSDataObject* object : m_project->objects())
    {
        IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(object);
        if (property == nullptr || property->gridId() != gridId
            || property->kind() != IDOSGridProperty::Kind::Dynamic
            || property->reportSteps().isEmpty())
        {
            continue;
        }
        const QString propertyName = property->keyword().isEmpty() ? property->name() : property->keyword();
        m_propertyCombo->addItem(propertyName, property->objectId());
    }
    const int previousPropertyIndex = m_propertyCombo->findData(previousPropertyId);
    if (previousPropertyIndex >= 0)
    {
        m_propertyCombo->setCurrentIndex(previousPropertyIndex);
    }
    m_propertyCombo->blockSignals(false);

    if (m_propertyCombo->count() == 0)
    {
        m_statusLabel->setText(tr("No imported report-step data is available. Import an UNRST file first."));
        updateControls();
        return;
    }
    onPropertyChanged(m_propertyCombo->currentIndex());
}

void IDOSSimulationResultPlaybackWidget::refreshReportSteps()
{
    const int previousReportStep = m_property.isNull() ? -1 : m_property->currentReportStep();
    m_reportSteps = m_property.isNull() ? QList<int>() : m_property->reportSteps();
    m_reportStepSlider->blockSignals(true);
    m_reportStepSlider->setRange(0, qMax(0, m_reportSteps.size() - 1));
    int currentIndex = m_reportSteps.indexOf(previousReportStep);
    if (currentIndex < 0 && !m_reportSteps.isEmpty())
    {
        currentIndex = 0;
    }
    m_reportStepSlider->setValue(qMax(0, currentIndex));
    m_reportStepSlider->blockSignals(false);
    if (!m_reportSteps.isEmpty())
    {
        onReportStepChanged(currentIndex);
    }
    else
    {
        updateControls();
    }
}

void IDOSSimulationResultPlaybackWidget::updateControls()
{
    const bool hasSteps = !m_property.isNull() && !m_reportSteps.isEmpty();
    m_propertyCombo->setEnabled(!m_project.isNull() && m_propertyCombo->count() > 0);
    m_reportStepSlider->setEnabled(hasSteps);
    m_previousButton->setEnabled(hasSteps && m_reportStepSlider->value() > m_reportStepSlider->minimum());
    m_nextButton->setEnabled(hasSteps && m_reportStepSlider->value() < m_reportStepSlider->maximum());
    m_playButton->setEnabled(hasSteps && (m_reportSteps.size() > 1 || m_timer->isActive()));
    m_intervalSpinBox->setEnabled(hasSteps);
    if (!hasSteps)
    {
        m_reportStepLabel->setText(tr("No step"));
    }
}
