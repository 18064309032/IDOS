#include <QCheckBox>
#include <QColor>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QIcon>
#include <QImage>
#include <QLabel>
#include <QMap>
#include <QPixmap>
#include <QPushButton>
#include <QSize>
#include <QVariantMap>
#include <QVBoxLayout>

#include "grid/idosgridrenderprovider.h"
#include "idosdataobject.h"
#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idosproject.h"
#include "idosrenderobject.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"

#include "idoscoloringwidget.h"

IDOSColoringWidget::IDOSColoringWidget(IDOSProject* project, IDOSGrid* grid, IDOSRenderView* view,
                                     const QString& propertyId, QWidget* parent)
    :
    QWidget(parent, Qt::Window)
    , m_project(project)
    , m_grid(grid)
    , m_view(view)
    , m_renderId()
    , m_solidColor()
    , m_invalidColor()
    , m_loading(false)
    , m_propertyCombo(new QComboBox(this))
    , m_rampCombo(new QComboBox(this))
    , m_automatic(new QCheckBox(tr("Automatic range"), this))
    , m_reversed(new QCheckBox(tr("Reverse color ramp"), this))
    , m_legend(new QCheckBox(tr("Show legend"), this))
    , m_minimum(new QDoubleSpinBox(this))
    , m_maximum(new QDoubleSpinBox(this))
    , m_solidButton(new QPushButton(this))
    , m_invalidButton(new QPushButton(this))
    , m_message(new QLabel(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QStringLiteral("coloringSettingsWindow"));
    setWindowTitle(tr("Coloring Settings"));
    m_propertyCombo->setObjectName(QStringLiteral("coloringPropertyCombo"));
    m_rampCombo->setObjectName(QStringLiteral("coloringRampCombo"));
    m_automatic->setObjectName(QStringLiteral("coloringAutomaticCheck"));
    m_reversed->setObjectName(QStringLiteral("coloringReverseCheck"));
    m_legend->setObjectName(QStringLiteral("coloringLegendCheck"));
    m_minimum->setObjectName(QStringLiteral("coloringMinimumSpin"));
    m_maximum->setObjectName(QStringLiteral("coloringMaximumSpin"));
    QVBoxLayout* layout = new QVBoxLayout(this);
    QFormLayout* targetLayout = new QFormLayout();
    targetLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    QLabel* gridName = new QLabel(grid->name(), this);
    gridName->setObjectName(QStringLiteral("coloringGridNameLabel"));
    gridName->setWordWrap(true);
    QLabel* viewName = new QLabel(view->windowTitle(), this);
    viewName->setObjectName(QStringLiteral("coloringViewNameLabel"));
    viewName->setWordWrap(true);
    targetLayout->addRow(tr("Grid:"), gridName);
    targetLayout->addRow(tr("Target view:"), viewName);
    connect(view, &QWidget::windowTitleChanged, viewName, &QLabel::setText);
    layout->addLayout(targetLayout);
    QFormLayout* form = new QFormLayout();
    form->addRow(tr("Color by"), m_propertyCombo);
    populateColorRamps();
    form->addRow(tr("Color ramp"), m_rampCombo);
    form->addRow(QString(), m_reversed);
    form->addRow(QString(), m_automatic);
    m_minimum->setRange(-1.0e100, 1.0e100);
    m_maximum->setRange(-1.0e100, 1.0e100);
    m_minimum->setDecimals(8);
    m_maximum->setDecimals(8);
    m_minimum->setKeyboardTracking(false);
    m_maximum->setKeyboardTracking(false);
    form->addRow(tr("Minimum"), m_minimum);
    form->addRow(tr("Maximum"), m_maximum);
    form->addRow(tr("Solid color"), m_solidButton);
    form->addRow(tr("Invalid value color"), m_invalidButton);
    form->addRow(QString(), m_legend);
    layout->addLayout(form);
    m_message->setWordWrap(true);
    layout->addWidget(m_message);
    QPushButton* closeButton = new QPushButton(tr("Close"), this);
    layout->addWidget(closeButton);
    for (IDOSRenderObject* object : view->scene()->objects())
    {
        if (object->provider()->dataSource() == grid)
        {
            m_renderId = object->id();
            break;
        }
    }
    onRefresh();
    connect(m_propertyCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &IDOSColoringWidget::onApply);
    connect(m_rampCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &IDOSColoringWidget::onApply);
    connect(m_automatic, &QCheckBox::toggled, this, &IDOSColoringWidget::onApply);
    connect(m_reversed, &QCheckBox::toggled, this, &IDOSColoringWidget::onApply);
    connect(m_legend, &QCheckBox::toggled, this, &IDOSColoringWidget::onApply);
    connect(m_minimum, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &IDOSColoringWidget::onApply);
    connect(m_maximum, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &IDOSColoringWidget::onApply);
    connect(m_solidButton, &QPushButton::clicked, this, &IDOSColoringWidget::onSolidColor);
    connect(m_invalidButton, &QPushButton::clicked, this, &IDOSColoringWidget::onInvalidColor);
    connect(closeButton, &QPushButton::clicked, this, &QWidget::close);
    connect(project, &QObject::destroyed, this, &IDOSColoringWidget::onInvalidate);
    connect(grid, &QObject::destroyed, this, &IDOSColoringWidget::onInvalidate);
    connect(view, &QObject::destroyed, this, &IDOSColoringWidget::onInvalidate);
    connect(project, &IDOSProject::objectAboutToBeRemoved, this, &IDOSColoringWidget::onDataRemoving);
    connect(project, &IDOSProject::objectAdded, this, &IDOSColoringWidget::onRefresh);
    connect(project, &IDOSProject::objectRemoved, this, &IDOSColoringWidget::onRefresh);
    connect(project, &IDOSProject::objectsAdded, this, &IDOSColoringWidget::onRefresh);
    connect(project, &IDOSProject::objectsRemoved, this, &IDOSColoringWidget::onRefresh);
    connect(view->scene(), &IDOSRenderScene::changed, this, &IDOSColoringWidget::onRefresh);
    connect(view->scene(), &IDOSRenderScene::objectAboutToBeRemoved,
            this, &IDOSColoringWidget::onRenderObjectRemoving);
    if (!propertyId.isEmpty())
    {
        m_propertyCombo->setCurrentIndex(m_propertyCombo->findData(propertyId));
    }
    resize(380, sizeHint().height());
}

IDOSRenderObject* IDOSColoringWidget::renderObject() const
{
    if (m_project.isNull() || m_grid.isNull() || m_view.isNull() ||
        m_project->object(m_grid->objectId()) != m_grid.data())
    {
        return nullptr;
    }
    IDOSRenderObject* object = m_view->scene()->object(m_renderId);
    return object != nullptr && object->provider()->dataSource() == m_grid.data() ? object : nullptr;
}

IDOSGridRenderProvider* IDOSColoringWidget::provider() const
{
    IDOSRenderObject* object = renderObject();
    return object != nullptr ? dynamic_cast<IDOSGridRenderProvider*>(object->provider()) : nullptr;
}

void IDOSColoringWidget::onRefresh()
{
    if (m_loading)
    {
        return;
    }
    IDOSGridRenderProvider* target = provider();
    if (target == nullptr)
    {
        onInvalidate();
        return;
    }
    m_loading = true;
    m_propertyCombo->clear();
    m_propertyCombo->addItem(tr("Solid color"), QString());
    for (IDOSDataObject* data : m_project->objects())
    {
        IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(data);
        if (property != nullptr && property->gridId() == m_grid->objectId())
        {
            m_propertyCombo->addItem(property->name(), property->objectId());
        }
    }
    m_propertyCombo->setCurrentIndex(target->property() != nullptr
        ? m_propertyCombo->findData(target->property()->objectId()) : 0);
    const QVariantMap settings = target->coloringSettings();
    m_rampCombo->setCurrentIndex(m_rampCombo->findData(settings.value(QStringLiteral("ramp"))));
    m_automatic->setChecked(settings.value(QStringLiteral("automatic")).toBool());
    m_reversed->setChecked(settings.value(QStringLiteral("reversed")).toBool());
    updateRampPreviews(m_reversed->isChecked());
    m_legend->setChecked(settings.value(QStringLiteral("legend")).toBool());
    m_minimum->setValue(settings.value(QStringLiteral("minimum")).toDouble());
    m_maximum->setValue(settings.value(QStringLiteral("maximum")).toDouble());
    m_solidColor = settings.value(QStringLiteral("solidColor")).toString();
    m_invalidColor = settings.value(QStringLiteral("invalidColor")).toString();
    m_solidButton->setText(m_solidColor);
    m_invalidButton->setText(m_invalidColor);
    const bool scalar = target->property() != nullptr;
    m_solidButton->setEnabled(!scalar);
    m_rampCombo->setEnabled(scalar);
    m_reversed->setEnabled(scalar);
    m_automatic->setEnabled(scalar);
    m_minimum->setEnabled(scalar && !m_automatic->isChecked());
    m_maximum->setEnabled(scalar && !m_automatic->isChecked());
    m_invalidButton->setEnabled(scalar);
    m_legend->setEnabled(scalar);
    m_loading = false;
}

void IDOSColoringWidget::onApply()
{
    if (m_loading)
    {
        return;
    }
    IDOSGridRenderProvider* target = provider();
    if (target == nullptr)
    {
        onInvalidate();
        return;
    }
    const QString propertyId = m_propertyCombo->currentData().toString();
    IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(m_project->object(propertyId));
    if (!propertyId.isEmpty() && (property == nullptr || property->gridId() != m_grid->objectId()))
    {
        onInvalidate();
        return;
    }
    QVariantMap settings = target->coloringSettings();
    settings.insert(QStringLiteral("ramp"), m_rampCombo->currentData());
    settings.insert(QStringLiteral("automatic"), m_automatic->isChecked());
    settings.insert(QStringLiteral("reversed"), m_reversed->isChecked());
    settings.insert(QStringLiteral("legend"), m_legend->isChecked());
    settings.insert(QStringLiteral("minimum"), m_minimum->value());
    settings.insert(QStringLiteral("maximum"), m_maximum->value());
    settings.insert(QStringLiteral("solidColor"), m_solidColor);
    settings.insert(QStringLiteral("invalidColor"), m_invalidColor);
    if (!target->setColoringSettings(settings))
    {
        m_message->setText(tr("Minimum must be less than maximum."));
        return;
    }
    m_loading = true;
    if (target->property() != property)
    {
        target->setProperty(property);
    }
    IDOSRenderObject* object = renderObject();
    object->setVisible(true);
    m_view->scene()->notifyChanged(object->id());
    m_loading = false;
    m_message->clear();
    onRefresh();
}

void IDOSColoringWidget::populateColorRamps()
{
    const QMap<QString, QString> names = {
        {QStringLiteral("Default"), tr("Default rainbow")},
        {QStringLiteral("SoftRainbow"), tr("Soft rainbow")},
        {QStringLiteral("OceanBreeze"), tr("Ocean breeze")},
        {QStringLiteral("WarmAmber"), tr("Warm amber")},
        {QStringLiteral("FreshMint"), tr("Fresh mint")},
        {QStringLiteral("TwilightPurple"), tr("Twilight purple")},
        {QStringLiteral("EarthTones"), tr("Earth tones")},
        {QStringLiteral("GlacierBlue"), tr("Glacier blue")},
        {QStringLiteral("WarmColdDiverging"), tr("Warm and cool")},
        {QStringLiteral("LavaMetal"), tr("Lava metal")},
        {QStringLiteral("DarkForest"), tr("Dark forest")},
        {QStringLiteral("VintageSunset"), tr("Vintage sunset")},
        {QStringLiteral("Aurora"), tr("Aurora")},
        {QStringLiteral("MineralSand"), tr("Mineral sand")},
        {QStringLiteral("Porcelain"), tr("Porcelain")},
        {QStringLiteral("AutumnBerry"), tr("Autumn berry")},
        {QStringLiteral("MonochromeJade"), tr("Jade")},
        {QStringLiteral("GraphiteScale"), tr("Graphite")},
        {QStringLiteral("Cyberpunk"), tr("Cyberpunk")},
        {QStringLiteral("blueRed"), tr("Blue to red")},
        {QStringLiteral("blueYellow"), tr("Blue to yellow")},
        {QStringLiteral("gray"), tr("Grayscale")}};
    for (const QString& key : IDOSGridRenderProvider::colorRampKeys())
    {
        m_rampCombo->addItem(names.value(key), key);
        m_rampCombo->setItemData(m_rampCombo->count() - 1, QSize(260, 24), Qt::SizeHintRole);
    }
    m_rampCombo->setIconSize(QSize(120, 16));
    m_rampCombo->setMaxVisibleItems(22);
    updateRampPreviews(false);
}

void IDOSColoringWidget::updateRampPreviews(bool reversed)
{
    for (int index = 0; index < m_rampCombo->count(); ++index)
    {
        const QList<QColor> colors = IDOSGridRenderProvider::colorRampColors(
            m_rampCombo->itemData(index).toString(), reversed);
        QImage preview(256, 16, QImage::Format_RGB32);
        for (int column = 0; column < colors.size(); ++column)
        {
            for (int row = 0; row < preview.height(); ++row)
            {
                preview.setPixelColor(column, row, colors.at(column));
            }
        }
        m_rampCombo->setItemIcon(index, QIcon(QPixmap::fromImage(preview)));
    }
}

void IDOSColoringWidget::onSolidColor()
{
    QPointer<IDOSColoringWidget> guard(this);
    const QColor color = QColorDialog::getColor(QColor(m_solidColor), this, tr("Solid color"));
    if (guard.isNull())
    {
        return;
    }
    if (color.isValid())
    {
        m_solidColor = color.name();
        onApply();
    }
}

void IDOSColoringWidget::onInvalidColor()
{
    QPointer<IDOSColoringWidget> guard(this);
    const QColor color = QColorDialog::getColor(QColor(m_invalidColor), this, tr("Invalid value color"));
    if (guard.isNull())
    {
        return;
    }
    if (color.isValid())
    {
        m_invalidColor = color.name();
        onApply();
    }
}

void IDOSColoringWidget::onInvalidate()
{
    if (m_project.isNull() && m_grid.isNull() && m_view.isNull())
    {
        return;
    }
    setEnabled(false);
    m_project = nullptr;
    m_grid = nullptr;
    m_view = nullptr;
    close();
}

void IDOSColoringWidget::onRenderObjectRemoving(const QString& id)
{
    if (id == m_renderId)
    {
        onInvalidate();
    }
}

void IDOSColoringWidget::onDataRemoving(const QString& id)
{
    if (!m_grid.isNull() && (id == m_grid->objectId() || id == m_propertyCombo->currentData().toString()))
    {
        onInvalidate();
    }
}
