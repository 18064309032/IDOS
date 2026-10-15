#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include "data/grid/idosgrid.h"
#include "data/grid/idosgridproperty.h"
#include "idoscommandmanager.h"
#include "idosproject.h"

#include "idosfielddifferencewidget.h"

IDOSFieldDifferenceWidget::IDOSFieldDifferenceWidget(IDOSProject* project, QWidget* parent)
    :
    QWidget(parent, Qt::Window)
    , m_project(project)
    , m_grid(new QComboBox(this))
    , m_before(new QComboBox(this))
    , m_after(new QComboBox(this))
    , m_beforeStep(new QComboBox(this))
    , m_afterStep(new QComboBox(this))
    , m_name(new QLineEdit(tr("Field difference"), this))
    , m_confirm(new QCheckBox(tr("I confirm matching physical quantities and units for inputs with unknown or different metadata."), this))
    , m_result(new QLabel(this))
{
    setObjectName(QStringLiteral("idosFieldDifferenceWidget"));
    setWindowTitle(tr("Three-Field Difference"));
    setAttribute(Qt::WA_DeleteOnClose);
    m_grid->setObjectName(QStringLiteral("differenceGrid"));
    m_before->setObjectName(QStringLiteral("differenceBefore"));
    m_after->setObjectName(QStringLiteral("differenceAfter"));
    m_beforeStep->setObjectName(QStringLiteral("differenceBeforeStep"));
    m_afterStep->setObjectName(QStringLiteral("differenceAfterStep"));
    m_name->setObjectName(QStringLiteral("differenceName"));
    m_confirm->setObjectName(QStringLiteral("differenceConfirmUnits"));
    m_result->setObjectName(QStringLiteral("differenceResult"));
    m_result->setWordWrap(true);
    m_result->setTextInteractionFlags(Qt::TextSelectableByMouse);
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* explanation = new QLabel(tr("Result = after - before on active cells. This is a property comparison, not a coupled three-field solver. No unit conversion is performed. Dynamic report steps are read without changing the displayed report step."), this);
    explanation->setWordWrap(true);
    layout->addWidget(explanation);
    QFormLayout* form = new QFormLayout();
    form->addRow(tr("Grid:"), m_grid);
    form->addRow(tr("Before property:"), m_before);
    form->addRow(tr("Before report step:"), m_beforeStep);
    form->addRow(tr("After property:"), m_after);
    form->addRow(tr("After report step:"), m_afterStep);
    form->addRow(tr("Output name:"), m_name);
    layout->addLayout(form);
    layout->addWidget(m_confirm);
    layout->addWidget(m_result);
    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addStretch();
    QPushButton* calculate = new QPushButton(tr("Calculate"), this);
    calculate->setObjectName(QStringLiteral("differenceCalculate"));
    buttons->addWidget(calculate);
    QPushButton* closeButton = new QPushButton(tr("Close"), this);
    buttons->addWidget(closeButton);
    layout->addLayout(buttons);
    connect(calculate, &QPushButton::clicked, this, &IDOSFieldDifferenceWidget::onCalculate);
    connect(closeButton, &QPushButton::clicked, this, &QWidget::close);
    connect(m_grid, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &IDOSFieldDifferenceWidget::onGridChanged);
    connect(m_before, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &IDOSFieldDifferenceWidget::onBeforeChanged);
    connect(m_after, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &IDOSFieldDifferenceWidget::onAfterChanged);
    if (project != nullptr)
    {
        connect(project, &IDOSProject::objectAdded, this, &IDOSFieldDifferenceWidget::onRefresh);
        connect(project, &IDOSProject::objectRemoved, this, &IDOSFieldDifferenceWidget::onRefresh);
        connect(project, &IDOSProject::objectsAdded, this, &IDOSFieldDifferenceWidget::onRefresh);
        connect(project, &IDOSProject::objectsRemoved, this, &IDOSFieldDifferenceWidget::onRefresh);
        connect(project, &IDOSProject::objectDataChanged, this, &IDOSFieldDifferenceWidget::onRefresh);
        connect(project, &IDOSProject::objectsDataChanged, this, &IDOSFieldDifferenceWidget::onRefresh);
        connect(project, &QObject::destroyed, this, &QWidget::close);
    }
    onRefresh();
    resize(680, sizeHint().height());
}

void IDOSFieldDifferenceWidget::onRefresh()
{
    const QString selected = m_grid->currentData().toString();
    const QSignalBlocker blocker(m_grid);
    m_grid->clear();
    if (!m_project.isNull())
    {
        for (IDOSDataObject* object : m_project->objects())
        {
            if (qobject_cast<IDOSGrid*>(object) != nullptr)
            {
                m_grid->addItem(object->name(), object->objectId());
            }
        }
    }
    const int index = m_grid->findData(selected);
    if (index >= 0)
    {
        m_grid->setCurrentIndex(index);
    }
    onGridChanged();
}

void IDOSFieldDifferenceWidget::onGridChanged()
{
    const QString before = m_before->currentData().toString();
    const QString after = m_after->currentData().toString();
    const QSignalBlocker blockBefore(m_before);
    const QSignalBlocker blockAfter(m_after);
    m_before->clear();
    m_after->clear();
    if (!m_project.isNull())
    {
        for (IDOSDataObject* object : m_project->objects())
        {
            const IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(object);
            if (property != nullptr && property->gridId() == m_grid->currentData().toString())
            {
                const QString label = property->name() + QStringLiteral(" [") + property->keyword() + QStringLiteral("]");
                m_before->addItem(label, property->objectId());
                m_after->addItem(label, property->objectId());
            }
        }
    }
    if (m_before->findData(before) >= 0)
    {
        m_before->setCurrentIndex(m_before->findData(before));
    }
    if (m_after->findData(after) >= 0)
    {
        m_after->setCurrentIndex(m_after->findData(after));
    }
    onBeforeChanged();
    onAfterChanged();
}

void IDOSFieldDifferenceWidget::populateSteps(QComboBox* propertyBox, QComboBox* steps)
{
    const QVariant previous = steps->currentData();
    steps->clear();
    const IDOSGridProperty* property = !m_project.isNull()
        ? qobject_cast<IDOSGridProperty*>(m_project->object(propertyBox->currentData().toString())) : nullptr;
    if (property == nullptr)
    {
        return;
    }
    if (property->kind() == IDOSGridProperty::Kind::Static)
    {
        steps->addItem(tr("Static field"), -1);
    }
    else
    {
        for (int step : property->reportSteps())
        {
            steps->addItem(QString::number(step), step);
        }
    }
    const int index = steps->findData(previous);
    if (index >= 0)
    {
        steps->setCurrentIndex(index);
    }
}

void IDOSFieldDifferenceWidget::onBeforeChanged()
{
    populateSteps(m_before, m_beforeStep);
}

void IDOSFieldDifferenceWidget::onAfterChanged()
{
    populateSteps(m_after, m_afterStep);
}

void IDOSFieldDifferenceWidget::onCalculate()
{
    if (m_project.isNull())
    {
        return;
    }
    QJsonObject arguments{{QStringLiteral("gridId"), m_grid->currentData().toString()},
        {QStringLiteral("beforeId"), m_before->currentData().toString()}, {QStringLiteral("afterId"), m_after->currentData().toString()},
        {QStringLiteral("beforeStep"), m_beforeStep->currentData().toInt()}, {QStringLiteral("afterStep"), m_afterStep->currentData().toInt()},
        {QStringLiteral("name"), m_name->text()}, {QStringLiteral("confirmUnits"), m_confirm->isChecked()}};
    const QJsonObject response = m_project->commandManager()->execute(QStringLiteral("analysis.field.difference"), arguments);
    if (!response.value(QStringLiteral("success")).toBool())
    {
        m_result->setText(response.value(QStringLiteral("error")).toString());
        return;
    }
    const QJsonObject result = response.value(QStringLiteral("result")).toObject();
    m_project->setCurrentObject(m_project->object(result.value(QStringLiteral("objectId")).toString()));
    m_result->setText(tr("Created %1 active-cell differences. Minimum: %2; maximum: %3; mean: %4. Select the new property in the grid tree to display it.")
        .arg(result.value(QStringLiteral("activeCount")).toInt()).arg(result.value(QStringLiteral("minimum")).toDouble(), 0, 'g', 12)
        .arg(result.value(QStringLiteral("maximum")).toDouble(), 0, 'g', 12).arg(result.value(QStringLiteral("mean")).toDouble(), 0, 'g', 12));
}
