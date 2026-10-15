#include <limits>

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QVBoxLayout>

#include "data/grid/idosgrid.h"
#include "data/grid/idosgridproperty.h"
#include "idoscommandmanager.h"
#include "idosproject.h"

#include "idosdfngenerationwidget.h"

IDOSDfnGenerationWidget::IDOSDfnGenerationWidget(IDOSProject* project, QWidget* parent)
    :
    QWidget(parent, Qt::Window)
    , m_project(project)
    , m_grid(new QComboBox(this))
    , m_density(new QComboBox(this))
    , m_name(new QLineEdit(tr("DFN fractures"), this))
    , m_count(new QSpinBox(this))
    , m_seed(new QSpinBox(this))
    , m_firstLayer(new QSpinBox(this))
    , m_lastLayer(new QSpinBox(this))
    , m_minimumLength(createNumber(QStringLiteral("dfnMinimumLength"), 0.000001, 1.0e9, 10.0))
    , m_maximumLength(createNumber(QStringLiteral("dfnMaximumLength"), 0.000001, 1.0e9, 100.0))
    , m_aspectRatio(createNumber(QStringLiteral("dfnAspectRatio"), 1.0, 1000.0, 2.0))
    , m_strike(createNumber(QStringLiteral("dfnStrike"), -360.0, 360.0, 90.0))
    , m_dip(createNumber(QStringLiteral("dfnDip"), 0.0, 90.0, 60.0))
    , m_strikeSpread(createNumber(QStringLiteral("dfnStrikeSpread"), 0.0, 180.0, 10.0))
    , m_dipSpread(createNumber(QStringLiteral("dfnDipSpread"), 0.0, 90.0, 5.0))
    , m_result(new QLabel(this))
    , m_generate(new QPushButton(tr("Generate"), this))
{
    setObjectName(QStringLiteral("idosDfnGenerationWidget"));
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("DFN Generation"));
    m_grid->setObjectName(QStringLiteral("dfnGrid"));
    m_density->setObjectName(QStringLiteral("dfnDensity"));
    m_name->setObjectName(QStringLiteral("dfnName"));
    m_count->setObjectName(QStringLiteral("dfnCount"));
    m_seed->setObjectName(QStringLiteral("dfnSeed"));
    m_firstLayer->setObjectName(QStringLiteral("dfnFirstLayer"));
    m_lastLayer->setObjectName(QStringLiteral("dfnLastLayer"));
    m_result->setObjectName(QStringLiteral("dfnResult"));
    m_generate->setObjectName(QStringLiteral("dfnGenerate"));
    m_count->setRange(1, 10000);
    m_count->setValue(100);
    m_seed->setRange(0, std::numeric_limits<int>::max());
    m_seed->setValue(2026);
    m_result->setWordWrap(true);
    m_result->setTextInteractionFlags(Qt::TextSelectableByMouse);
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* explanation = new QLabel(tr("Stochastic rectangular fractures, not fracture inversion. Lengths use grid coordinates. Strike is measured from +X toward +Y; dip is toward +Z. Normal spreads use degrees. Centers are sampled from active cells; positive density values are relative sampling weights."), this);
    explanation->setWordWrap(true);
    layout->addWidget(explanation);
    QFormLayout* form = new QFormLayout();
    form->addRow(tr("Grid:"), m_grid);
    form->addRow(tr("Density weight:"), m_density);
    form->addRow(tr("Output name:"), m_name);
    form->addRow(tr("Fracture count:"), m_count);
    form->addRow(tr("Random seed:"), m_seed);
    form->addRow(tr("First layer (1-based):"), m_firstLayer);
    form->addRow(tr("Last layer (inclusive):"), m_lastLayer);
    form->addRow(tr("Minimum requested length:"), m_minimumLength);
    form->addRow(tr("Maximum requested length:"), m_maximumLength);
    form->addRow(tr("Length / width:"), m_aspectRatio);
    form->addRow(tr("Mean strike (degrees):"), m_strike);
    form->addRow(tr("Mean dip (degrees):"), m_dip);
    form->addRow(tr("Strike standard deviation:"), m_strikeSpread);
    form->addRow(tr("Dip standard deviation:"), m_dipSpread);
    layout->addLayout(form);
    QLabel* boundsNote = new QLabel(tr("Faces may be shortened to fit the selected cells' bounding box. They are not clipped to irregular grid boundaries or inactive holes. The same seed and inputs reproduce a network in this build."), this);
    boundsNote->setWordWrap(true);
    layout->addWidget(boundsNote);
    layout->addWidget(m_result);
    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addStretch();
    buttons->addWidget(m_generate);
    QPushButton* closeButton = new QPushButton(tr("Close"), this);
    buttons->addWidget(closeButton);
    layout->addLayout(buttons);
    connect(closeButton, &QPushButton::clicked, this, &QWidget::close);
    connect(m_generate, &QPushButton::clicked, this, &IDOSDfnGenerationWidget::onGenerate);
    connect(m_grid, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &IDOSDfnGenerationWidget::onGridChanged);
    if (project != nullptr)
    {
        connect(project, &IDOSProject::objectAdded, this, &IDOSDfnGenerationWidget::onRefreshInputs);
        connect(project, &IDOSProject::objectRemoved, this, &IDOSDfnGenerationWidget::onRefreshInputs);
        connect(project, &IDOSProject::objectsAdded, this, &IDOSDfnGenerationWidget::onRefreshInputs);
        connect(project, &IDOSProject::objectsRemoved, this, &IDOSDfnGenerationWidget::onRefreshInputs);
        connect(project, &IDOSProject::objectDataChanged, this, &IDOSDfnGenerationWidget::onRefreshInputs);
        connect(project, &IDOSProject::objectsDataChanged, this, &IDOSDfnGenerationWidget::onRefreshInputs);
        connect(project, &QObject::destroyed, this, &QWidget::close);
    }
    onRefreshInputs();
    resize(620, sizeHint().height());
}

QDoubleSpinBox* IDOSDfnGenerationWidget::createNumber(const QString& name, double minimum, double maximum, double value)
{
    QDoubleSpinBox* control = new QDoubleSpinBox(this);
    control->setObjectName(name);
    control->setDecimals(6);
    control->setRange(minimum, maximum);
    control->setValue(value);
    return control;
}

void IDOSDfnGenerationWidget::onRefreshInputs()
{
    const QString selected = m_grid->currentData().toString();
    const QSignalBlocker blocker(m_grid);
    m_grid->clear();
    if (!m_project.isNull())
    {
        for (IDOSDataObject* object : m_project->objects())
        {
            IDOSGrid* grid = qobject_cast<IDOSGrid*>(object);
            if (grid != nullptr && grid->type() == IDOSGrid::Type::CornerPoint)
            {
                m_grid->addItem(grid->name(), grid->objectId());
            }
        }
    }
    const int previous = m_grid->findData(selected);
    if (previous >= 0)
    {
        m_grid->setCurrentIndex(previous);
    }
    onGridChanged();
}

void IDOSDfnGenerationWidget::onGridChanged()
{
    const QString selectedDensity = m_density->currentData().toString();
    const QString selectedGrid = m_grid->currentData().toString();
    const IDOSGrid* grid = !m_project.isNull() ? qobject_cast<IDOSGrid*>(m_project->object(selectedGrid)) : nullptr;
    m_density->clear();
    m_density->addItem(tr("Uniform (no property)"), QString());
    m_generate->setEnabled(grid != nullptr);
    const int layers = grid != nullptr ? qMax(1, grid->nz()) : 1;
    m_firstLayer->setRange(1, layers);
    m_lastLayer->setRange(1, layers);
    if (m_gridId != selectedGrid)
    {
        m_firstLayer->setValue(1);
        m_lastLayer->setValue(layers);
    }
    m_gridId = selectedGrid;
    if (grid == nullptr)
    {
        return;
    }
    for (IDOSDataObject* object : m_project->objects())
    {
        const IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(object);
        if (property != nullptr && property->gridId() == selectedGrid && property->kind() == IDOSGridProperty::Kind::Static)
        {
            m_density->addItem(property->name(), property->objectId());
        }
    }
    const int previous = m_density->findData(selectedDensity);
    if (previous >= 0)
    {
        m_density->setCurrentIndex(previous);
    }
}

void IDOSDfnGenerationWidget::onGenerate()
{
    if (m_project.isNull() || m_project->commandManager() == nullptr)
    {
        return;
    }
    QJsonObject parameters;
    parameters.insert(QStringLiteral("name"), m_name->text());
    parameters.insert(QStringLiteral("gridId"), m_grid->currentData().toString());
    parameters.insert(QStringLiteral("densityId"), m_density->currentData().toString());
    parameters.insert(QStringLiteral("count"), m_count->value());
    parameters.insert(QStringLiteral("seed"), m_seed->value());
    parameters.insert(QStringLiteral("firstLayer"), m_firstLayer->value());
    parameters.insert(QStringLiteral("lastLayer"), m_lastLayer->value());
    parameters.insert(QStringLiteral("minimumLength"), m_minimumLength->value());
    parameters.insert(QStringLiteral("maximumLength"), m_maximumLength->value());
    parameters.insert(QStringLiteral("aspectRatio"), m_aspectRatio->value());
    parameters.insert(QStringLiteral("strike"), m_strike->value());
    parameters.insert(QStringLiteral("dip"), m_dip->value());
    parameters.insert(QStringLiteral("strikeSpread"), m_strikeSpread->value());
    parameters.insert(QStringLiteral("dipSpread"), m_dipSpread->value());
    const QJsonObject response = m_project->commandManager()->execute(QStringLiteral("fracture.dfn.generate"), parameters);
    if (!response.value(QStringLiteral("success")).toBool())
    {
        m_result->setText(response.value(QStringLiteral("error")).toString());
        return;
    }
    const QString id = response.value(QStringLiteral("result")).toObject().value(QStringLiteral("objectId")).toString();
    m_project->setCurrentObject(m_project->object(id));
    m_result->setText(tr("Generated %1 fractures. Select the fracture set in the data tree to display or check it.").arg(m_count->value()));
}
