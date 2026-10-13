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
#include <QTabWidget>
#include <QVariantMap>
#include <QVBoxLayout>

#include "idosdataobject.h"
#include "idosgrid.h"
#include "idosgridfilter.h"
#include "idosgridproperty.h"
#include "idosgridrenderprovider.h"
#include "idosproject.h"
#include "idosrenderobject.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"

#include "idosgridsettingwidget.h"

IDOSGridSettingWidget::IDOSGridSettingWidget(QWidget* parent)
    :
    QWidget(parent, Qt::Window)
    , m_project(nullptr)
    , m_grid(nullptr)
    , m_view(nullptr)
    , m_renderId()
    , m_solidColor()
    , m_invalidColor()
    , m_edgeColor()
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
    , m_gridName(new QLabel(this))
    , m_viewName(new QLabel(this))
    , m_tabs(new QTabWidget(this))
    , m_modeCombo(new QComboBox(this))
    , m_edgeCheck(new QCheckBox(tr("Show edges"), this))
    , m_visibleCheck(new QCheckBox(tr("Show grid"), this))
    , m_opacitySpin(new QDoubleSpinBox(this))
    , m_edgeWidthSpin(new QDoubleSpinBox(this))
    , m_edgeButton(new QPushButton(this))
    , m_targetDataId()
    , m_propertyId()
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QStringLiteral("gridSettingsWindow"));
    setWindowTitle(tr("Display Settings"));
    m_propertyCombo->setObjectName(QStringLiteral("gridSettingPropertyCombo"));
    m_rampCombo->setObjectName(QStringLiteral("gridSettingRampCombo"));
    m_automatic->setObjectName(QStringLiteral("gridSettingAutomaticCheck"));
    m_reversed->setObjectName(QStringLiteral("gridSettingReverseCheck"));
    m_legend->setObjectName(QStringLiteral("gridSettingLegendCheck"));
    m_minimum->setObjectName(QStringLiteral("gridSettingMinimumSpin"));
    m_maximum->setObjectName(QStringLiteral("gridSettingMaximumSpin"));
    QVBoxLayout* layout = new QVBoxLayout(this);
    QFormLayout* targetLayout = new QFormLayout();
    targetLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    m_gridName->setObjectName(QStringLiteral("gridSettingGridNameLabel"));
    m_gridName->setWordWrap(true);
    m_viewName->setObjectName(QStringLiteral("gridSettingViewNameLabel"));
    m_viewName->setWordWrap(true);
    targetLayout->addRow(tr("Grid:"), m_gridName);
    targetLayout->addRow(tr("Target view:"), m_viewName);
    layout->addLayout(targetLayout);
    QWidget* appearancePage = new QWidget(m_tabs);
    QWidget* coloringPage = new QWidget(m_tabs);
    m_tabs->setObjectName(QStringLiteral("gridSettingTabs"));
    m_tabs->addTab(appearancePage, tr("Appearance"));
    m_tabs->addTab(coloringPage, tr("Coloring"));
    QFormLayout* appearance = new QFormLayout(appearancePage);
    m_modeCombo->setObjectName(QStringLiteral("gridSettingModeCombo"));
    m_modeCombo->addItem(tr("Surface"), static_cast<int>(IDOSDisplayMode::Surface));
    m_modeCombo->addItem(tr("Wireframe"), static_cast<int>(IDOSDisplayMode::Wireframe));
    m_modeCombo->addItem(tr("Surface with edges"), static_cast<int>(IDOSDisplayMode::SurfaceWithEdges));
    m_modeCombo->addItem(tr("Points"), static_cast<int>(IDOSDisplayMode::Points));
    m_visibleCheck->setObjectName(QStringLiteral("gridSettingVisibleCheck"));
    m_edgeCheck->setObjectName(QStringLiteral("gridSettingEdgesCheck"));
    m_opacitySpin->setObjectName(QStringLiteral("gridSettingOpacitySpin"));
    m_opacitySpin->setRange(0.0, 100.0);
    m_opacitySpin->setDecimals(1);
    m_opacitySpin->setSuffix(tr("%"));
    m_opacitySpin->setKeyboardTracking(false);
    m_edgeWidthSpin->setObjectName(QStringLiteral("gridSettingEdgeWidthSpin"));
    m_edgeWidthSpin->setRange(0.5, 20.0);
    m_edgeWidthSpin->setDecimals(1);
    m_edgeWidthSpin->setKeyboardTracking(false);
    m_edgeButton->setObjectName(QStringLiteral("gridSettingEdgeColorButton"));
    appearance->addRow(QString(), m_visibleCheck);
    appearance->addRow(tr("Display mode"), m_modeCombo);
    appearance->addRow(tr("Opacity"), m_opacitySpin);
    appearance->addRow(QString(), m_edgeCheck);
    appearance->addRow(tr("Edge color"), m_edgeButton);
    appearance->addRow(tr("Line width (pixels)"), m_edgeWidthSpin);
    QFormLayout* form = new QFormLayout(coloringPage);
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
    layout->addWidget(m_tabs);
    m_message->setWordWrap(true);
    layout->addWidget(m_message);
    QPushButton* closeButton = new QPushButton(tr("Close"), this);
    layout->addWidget(closeButton);
    connect(m_propertyCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &IDOSGridSettingWidget::onApply);
    connect(m_rampCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &IDOSGridSettingWidget::onApply);
    connect(m_automatic, &QCheckBox::toggled, this, &IDOSGridSettingWidget::onApply);
    connect(m_reversed, &QCheckBox::toggled, this, &IDOSGridSettingWidget::onApply);
    connect(m_legend, &QCheckBox::toggled, this, &IDOSGridSettingWidget::onApply);
    connect(m_minimum, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &IDOSGridSettingWidget::onApply);
    connect(m_maximum, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &IDOSGridSettingWidget::onApply);
    connect(m_solidButton, &QPushButton::clicked, this, &IDOSGridSettingWidget::onSolidColor);
    connect(m_invalidButton, &QPushButton::clicked, this, &IDOSGridSettingWidget::onInvalidColor);
    connect(m_modeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &IDOSGridSettingWidget::onAppearanceApply);
    connect(m_edgeCheck, &QCheckBox::toggled, this, &IDOSGridSettingWidget::onAppearanceApply);
    connect(m_visibleCheck, &QCheckBox::toggled, this, &IDOSGridSettingWidget::onAppearanceApply);
    connect(m_opacitySpin, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &IDOSGridSettingWidget::onAppearanceApply);
    connect(m_edgeWidthSpin, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &IDOSGridSettingWidget::onAppearanceApply);
    connect(m_edgeButton, &QPushButton::clicked, this, &IDOSGridSettingWidget::onEdgeColor);
    connect(closeButton, &QPushButton::clicked, this, &QWidget::close);
    resize(420, sizeHint().height());
}

void IDOSGridSettingWidget::setProject(IDOSProject* project)
{
    m_project = project;
}

void IDOSGridSettingWidget::setGrid(IDOSGrid* grid)
{
    m_grid = grid;
    m_targetDataId = grid != nullptr ? grid->objectId() : QString();
}

void IDOSGridSettingWidget::setView(IDOSRenderView* view)
{
    m_view = view;
}

void IDOSGridSettingWidget::setPropertyId(const QString& propertyId)
{
    m_propertyId = propertyId;
    bindTarget();
}

void IDOSGridSettingWidget::bindTarget()
{
    if (m_project.isNull() || m_grid.isNull() || m_view.isNull()
        || m_project->object(m_grid->objectId()) != m_grid)
    {
        onInvalidate();
        return;
    }
    m_gridName->setText(m_grid->name());
    m_viewName->setText(m_view->windowTitle());
    connect(m_grid.data(), &IDOSGrid::nameChanged, m_gridName, &QLabel::setText);
    connect(m_view.data(), &QWidget::windowTitleChanged, m_viewName, &QLabel::setText);
    for (IDOSRenderObject* object : m_view->scene()->objects())
    {
        if (object->provider()->dataSource() == m_grid)
        {
            m_renderId = object->id();
            break;
        }
    }
    onRefresh();
    connect(m_project.data(), &QObject::destroyed, this, &IDOSGridSettingWidget::onInvalidate);
    connect(m_grid.data(), &QObject::destroyed, this, &IDOSGridSettingWidget::onInvalidate);
    connect(m_view.data(), &QObject::destroyed, this, &IDOSGridSettingWidget::onInvalidate);
    connect(m_project.data(), &IDOSProject::objectAboutToBeRemoved, this, &IDOSGridSettingWidget::onDataRemoving);
    connect(m_project.data(), &IDOSProject::objectAdded, this, &IDOSGridSettingWidget::onRefresh);
    connect(m_project.data(), &IDOSProject::objectRemoved, this, &IDOSGridSettingWidget::onRefresh);
    connect(m_project.data(), &IDOSProject::objectsAdded, this, &IDOSGridSettingWidget::onRefresh);
    connect(m_project.data(), &IDOSProject::objectsRemoved, this, &IDOSGridSettingWidget::onRefresh);
    connect(m_view->scene(), &IDOSRenderScene::changed, this, &IDOSGridSettingWidget::onRefresh);
    connect(m_view->scene(), &IDOSRenderScene::objectAboutToBeRemoved,
            this, &IDOSGridSettingWidget::onRenderObjectRemoving);
    if (!m_propertyId.isEmpty())
    {
        m_propertyCombo->setCurrentIndex(m_propertyCombo->findData(m_propertyId));
    }
    m_tabs->setCurrentIndex(m_propertyId.isEmpty() ? 0 : 1);
}

IDOSRenderObject* IDOSGridSettingWidget::renderObject() const
{
    if (m_project.isNull() || m_grid.isNull() || m_view.isNull() ||
        m_project->object(m_grid->objectId()) != m_grid.data())
    {
        return nullptr;
    }
    IDOSRenderObject* object = m_view->scene()->object(m_renderId);
    IDOSDataObject* source = m_targetDataId == m_grid->objectId() ? static_cast<IDOSDataObject*>(m_grid.data()) : m_project->object(m_targetDataId);
    return source != nullptr && object != nullptr && object->provider()->dataSource() == source ? object : nullptr;
}

IDOSGridRenderProvider* IDOSGridSettingWidget::provider() const
{
    IDOSRenderObject* object = renderObject();
    return object != nullptr ? dynamic_cast<IDOSGridRenderProvider*>(object->provider()) : nullptr;
}

void IDOSGridSettingWidget::onRefresh()
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
    IDOSDataObject* source = m_targetDataId == m_grid->objectId() ? static_cast<IDOSDataObject*>(m_grid.data()) : m_project->object(m_targetDataId);
    setWindowTitle(source != nullptr && m_targetDataId != m_grid->objectId() ? tr("Display Settings - %1").arg(source->name()) : tr("Display Settings"));
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
    m_modeCombo->setCurrentIndex(m_modeCombo->findData(static_cast<int>(target->displayMode())));
    const bool surface = target->displayMode() == IDOSDisplayMode::Surface ||
        target->displayMode() == IDOSDisplayMode::SurfaceWithEdges;
    m_edgeCheck->setChecked(target->displayMode() == IDOSDisplayMode::SurfaceWithEdges);
    m_edgeCheck->setEnabled(surface);
    m_edgeButton->setEnabled(target->displayMode() == IDOSDisplayMode::SurfaceWithEdges);
    m_edgeWidthSpin->setEnabled(target->displayMode() == IDOSDisplayMode::Wireframe ||
        target->displayMode() == IDOSDisplayMode::SurfaceWithEdges);
    m_visibleCheck->setChecked(target->visible());
    m_opacitySpin->setValue(target->opacity() * 100.0);
    m_edgeWidthSpin->setValue(target->edgeWidth());
    m_edgeColor = target->edgeColor().name();
    m_edgeButton->setText(m_edgeColor);
    m_loading = false;
}

void IDOSGridSettingWidget::onApply()
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

void IDOSGridSettingWidget::populateColorRamps()
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

void IDOSGridSettingWidget::updateRampPreviews(bool reversed)
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

void IDOSGridSettingWidget::onAppearanceApply()
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
    IDOSDisplayMode mode = static_cast<IDOSDisplayMode>(m_modeCombo->currentData().toInt());
    if (sender() == m_edgeCheck)
    {
        mode = m_edgeCheck->isChecked() ? IDOSDisplayMode::SurfaceWithEdges : IDOSDisplayMode::Surface;
    }
    if (!target->setEdgeColor(QColor(m_edgeColor)) || !target->setEdgeWidth(m_edgeWidthSpin->value()) ||
        !target->setDisplayMode(mode))
    {
        onRefresh();
        return;
    }
    target->setOpacity(m_opacitySpin->value() / 100.0);
    target->setVisible(m_visibleCheck->isChecked());
    m_view->scene()->notifyChanged(m_renderId);
}

void IDOSGridSettingWidget::onSolidColor()
{
    if (provider() == nullptr)
    {
        return;
    }
    QColorDialog* dialog = new QColorDialog(QColor(m_solidColor), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("Solid color"));
    dialog->setProperty("setting", QStringLiteral("solid"));
    connect(dialog, &QColorDialog::colorSelected, this, &IDOSGridSettingWidget::onColorSelected);
    dialog->open();
}

void IDOSGridSettingWidget::onInvalidColor()
{
    if (provider() == nullptr)
    {
        return;
    }
    QColorDialog* dialog = new QColorDialog(QColor(m_invalidColor), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("Invalid value color"));
    dialog->setProperty("setting", QStringLiteral("invalid"));
    connect(dialog, &QColorDialog::colorSelected, this, &IDOSGridSettingWidget::onColorSelected);
    dialog->open();
}

void IDOSGridSettingWidget::onEdgeColor()
{
    if (provider() == nullptr)
    {
        return;
    }
    QColorDialog* dialog = new QColorDialog(QColor(m_edgeColor), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("Edge color"));
    dialog->setProperty("setting", QStringLiteral("edge"));
    connect(dialog, &QColorDialog::colorSelected, this, &IDOSGridSettingWidget::onColorSelected);
    dialog->open();
}

void IDOSGridSettingWidget::onColorSelected(const QColor& color)
{
    QColorDialog* dialog = qobject_cast<QColorDialog*>(sender());
    if (dialog == nullptr || !color.isValid() || provider() == nullptr)
    {
        return;
    }
    const QString setting = dialog->property("setting").toString();
    if (setting == QStringLiteral("solid"))
    {
        m_solidColor = color.name();
        onApply();
    }
    else if (setting == QStringLiteral("invalid"))
    {
        m_invalidColor = color.name();
        onApply();
    }
    else if (setting == QStringLiteral("edge"))
    {
        m_edgeColor = color.name();
        onAppearanceApply();
    }
}
void IDOSGridSettingWidget::onInvalidate()
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

void IDOSGridSettingWidget::onRenderObjectRemoving(const QString& id)
{
    if (id == m_renderId)
    {
        onInvalidate();
    }
}

void IDOSGridSettingWidget::onDataRemoving(const QString& id)
{
    if (m_grid != nullptr && (id == m_grid->objectId() || id == m_targetDataId))
    {
        onInvalidate();
    }
}

bool IDOSGridSettingWidget::setRenderTarget(const QString& dataId)
{
    if (m_project == nullptr || m_view == nullptr || m_grid == nullptr)
    {
        return false;
    }
    IDOSGridFilter* filter = qobject_cast<IDOSGridFilter*>(m_project->object(dataId));
    if (dataId != m_grid->objectId() && (filter == nullptr || filter->gridId() != m_grid->objectId()))
    {
        return false;
    }
    for (IDOSRenderObject* object : m_view->scene()->objects())
    {
        if (object->provider()->dataSource() == m_project->object(dataId))
        {
            m_targetDataId = dataId;
            m_renderId = object->id();
            onRefresh();
            return true;
        }
    }
    return false;
}
