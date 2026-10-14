#include <cmath>
#include <limits>

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QSpinBox>
#include <QSplitter>
#include <QTabWidget>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include "command/idosgridfiltercommand.h"
#include "data/grid/idosgrid.h"
#include "data/grid/idosgridfilter.h"
#include "data/grid/idosgridproperty.h"
#include "idoscommandmanager.h"
#include "idosproject.h"
#include "idosrenderobject.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"
#include "providers/grid/idosgridrenderprovider.h"

#include "idosgridfilterwidget.h"

IDOSGridFilterWidget::IDOSGridFilterWidget(QWidget* parent)
    :
    QWidget(parent, Qt::Window)
    , m_project(nullptr)
    , m_grid(nullptr)
    , m_view(nullptr)
    , m_gridName(new QLabel(this))
    , m_viewName(new QLabel(this))
    , m_filterId()
    , m_renderId()
    , m_propertyId()
    , m_loading(true)
    , m_conditionStates()
    , m_filterObjects(nullptr)
    , m_save(nullptr)
    , m_tabs(nullptr)
    , m_union(nullptr)
    , m_intersection(nullptr)
    , m_show(nullptr)
    , m_hide(nullptr)
    , m_invert(nullptr)
    , m_axisEnabled()
    , m_axisMinimum()
    , m_axisMaximum()
    , m_axisLists()
    , m_axisSteps()
    , m_propertySearch(nullptr)
    , m_propertyList(nullptr)
    , m_selectedCount(nullptr)
    , m_propertyTitle(nullptr)
    , m_propertyRange(nullptr)
    , m_displayProperty(nullptr)
    , m_propertyLogic(nullptr)
    , m_propertyEnabled(nullptr)
    , m_comparison(nullptr)
    , m_minimum(nullptr)
    , m_maximum(nullptr)
    , m_propertyShow(nullptr)
    , m_propertyHide(nullptr)
    , m_propertyInvert(nullptr)
    , m_includeMinimum(nullptr)
    , m_includeMaximum(nullptr)
    , m_includeInvalid(nullptr)
    , m_epsilon(nullptr)
    , m_histogram(nullptr)
    , m_rangeBar(nullptr)
    , m_summary(nullptr)
    , m_summaryTimer(nullptr)
    , m_dataMinimum(0.0)
    , m_dataMaximum(1.0)
    , m_bins(32, 0)
    , m_selectedBins(32, 0)
    , m_dragHandle(-1)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QStringLiteral("gridFilterWindow"));
    setWindowTitle(tr("Grid Filter"));
}

void IDOSGridFilterWidget::setProject(IDOSProject* project)
{
    m_project = project;
}

void IDOSGridFilterWidget::setGrid(IDOSGrid* grid)
{
    m_grid = grid;
}

void IDOSGridFilterWidget::setView(IDOSRenderView* view)
{
    m_view = view;
}

void IDOSGridFilterWidget::setFilter(const QString& filterId)
{
    m_filterId = filterId;
    bindTarget();
}

void IDOSGridFilterWidget::bindTarget()
{
    if (m_project.isNull() || m_grid.isNull() || m_view.isNull()
        || m_project->object(m_grid->objectId()) != m_grid)
    {
        onInvalidate();
        return;
    }
    buildUi();
    m_gridName->setText(m_grid->name());
    m_viewName->setText(m_view->windowTitle());
    connect(m_grid.data(), &IDOSGrid::nameChanged, m_gridName, &QLabel::setText);
    connect(m_view.data(), &QWidget::windowTitleChanged, m_viewName, &QLabel::setText);
    populateObjects();
    if (!selectTarget(m_filterId))
    {
        onInvalidate();
        return;
    }
    m_loading = false;
    connect(m_project.data(), &QObject::destroyed, this, &IDOSGridFilterWidget::onInvalidate);
    connect(m_grid.data(), &QObject::destroyed, this, &IDOSGridFilterWidget::onInvalidate);
    connect(m_view.data(), &QObject::destroyed, this, &IDOSGridFilterWidget::onInvalidate);
    connect(m_project.data(), &IDOSProject::objectAboutToBeRemoved, this, &IDOSGridFilterWidget::onDataRemoving);
    connect(m_project.data(), &IDOSProject::objectAdded, this, &IDOSGridFilterWidget::onRefresh);
    connect(m_project.data(), &IDOSProject::objectRemoved, this, &IDOSGridFilterWidget::onRefresh);
    connect(m_project.data(), &IDOSProject::objectsAdded, this, &IDOSGridFilterWidget::onRefresh);
    connect(m_project.data(), &IDOSProject::objectsRemoved, this, &IDOSGridFilterWidget::onRefresh);
    connect(m_project.data(), &IDOSProject::objectDataChanged, this, &IDOSGridFilterWidget::onRefresh);
    connect(m_project.data(), &IDOSProject::objectsDataChanged, this, &IDOSGridFilterWidget::onRefresh);
    connect(m_view->scene(), &IDOSRenderScene::changed, this, &IDOSGridFilterWidget::onRefresh);
    connect(m_view->scene(), &IDOSRenderScene::objectAboutToBeRemoved, this, &IDOSGridFilterWidget::onRenderObjectRemoving);
    resize(1050, 660);
    onSummaryRefresh();
}

void IDOSGridFilterWidget::buildUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    QFormLayout* target = new QFormLayout();
    m_gridName->setObjectName(QStringLiteral("gridFilterGridName"));
    m_viewName->setObjectName(QStringLiteral("gridFilterViewName"));
    target->addRow(tr("Grid:"), m_gridName);
    target->addRow(tr("Target view:"), m_viewName);
    layout->addLayout(target);
    QHBoxLayout* objects = new QHBoxLayout();
    objects->addWidget(new QLabel(tr("Filter object"), this));
    m_filterObjects = new QComboBox(this);
    m_filterObjects->setObjectName(QStringLiteral("gridFilterObjects"));
    objects->addWidget(m_filterObjects, 1);
    m_save = new QPushButton(tr("Save Filter"), this);
    m_save->setObjectName(QStringLiteral("gridFilterSave"));
    QPushButton* saveAs = new QPushButton(tr("Save As New Object..."), this);
    saveAs->setObjectName(QStringLiteral("gridFilterSaveAs"));
    objects->addWidget(m_save);
    objects->addWidget(saveAs);
    layout->addLayout(objects);
    m_tabs = new QTabWidget(this);
    m_tabs->setObjectName(QStringLiteral("gridFilterTabs"));
    QWidget* gridPage = new QWidget(m_tabs);
    QVBoxLayout* gridLayout = new QVBoxLayout(gridPage);
    QHBoxLayout* modes = new QHBoxLayout();
    QGroupBox* combination = new QGroupBox(tr("I/J/K Combination"), gridPage);
    QHBoxLayout* combinationLayout = new QHBoxLayout(combination);
    m_union = new QRadioButton(tr("Union"), combination);
    m_union->setObjectName(QStringLiteral("gridFilterUnion"));
    m_intersection = new QRadioButton(tr("Intersection"), combination);
    m_intersection->setObjectName(QStringLiteral("gridFilterIntersection"));
    m_union->setChecked(true);
    combinationLayout->addWidget(m_union);
    combinationLayout->addWidget(m_intersection);
    QGroupBox* operation = new QGroupBox(tr("Display Operation"), gridPage);
    QHBoxLayout* operationLayout = new QHBoxLayout(operation);
    m_show = new QRadioButton(tr("Show"), operation);
    m_hide = new QRadioButton(tr("Hide"), operation);
    m_invert = new QRadioButton(tr("Invert"), operation);
    m_show->setObjectName(QStringLiteral("gridFilterShow"));
    m_hide->setObjectName(QStringLiteral("gridFilterHide"));
    m_invert->setObjectName(QStringLiteral("gridFilterInvert"));
    m_show->setChecked(true);
    operationLayout->addWidget(m_show);
    operationLayout->addWidget(m_hide);
    operationLayout->addWidget(m_invert);
    modes->addWidget(combination);
    modes->addWidget(operation);
    modes->addStretch();
    gridLayout->addLayout(modes);
    QHBoxLayout* axes = new QHBoxLayout();
    for (int axis = 0; axis < 3; ++axis)
    {
        axes->addWidget(buildAxisPanel(axis), 1);
    }
    gridLayout->addLayout(axes, 1);
    QLabel* hint = new QLabel(tr("Indices start at 1. Each direction combines its range and checked indices. Hide and Invert retain unmatched active cells."), gridPage);
    hint->setWordWrap(true);
    gridLayout->addWidget(hint);
    m_tabs->addTab(gridPage, tr("Grid Filter"));
    m_tabs->addTab(buildPropertyPage(), tr("Attribute Filter"));
    layout->addWidget(m_tabs, 1);
    m_summary = new QLabel(this);
    m_summary->setObjectName(QStringLiteral("gridFilterSummary"));
    m_summary->setWordWrap(true);
    layout->addWidget(m_summary);
    QHBoxLayout* buttons = new QHBoxLayout();
    QPushButton* reset = new QPushButton(tr("Restore Default"), this);
    reset->setObjectName(QStringLiteral("gridFilterReset"));
    QPushButton* apply = new QPushButton(tr("Apply Filters"), this);
    apply->setObjectName(QStringLiteral("gridFilterApply"));
    QPushButton* close = new QPushButton(tr("Close"), this);
    buttons->addStretch();
    buttons->addWidget(reset);
    buttons->addWidget(apply);
    buttons->addWidget(close);
    layout->addLayout(buttons);
    m_summaryTimer = new QTimer(this);
    m_summaryTimer->setSingleShot(true);
    m_summaryTimer->setInterval(120);
    connect(m_summaryTimer, &QTimer::timeout, this, &IDOSGridFilterWidget::onSummaryRefresh);
    connect(m_filterObjects, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &IDOSGridFilterWidget::onFilterSelected);
    connect(m_save, &QPushButton::clicked, this, &IDOSGridFilterWidget::onSave);
    connect(saveAs, &QPushButton::clicked, this, &IDOSGridFilterWidget::onSaveAs);
    connect(reset, &QPushButton::clicked, this, &IDOSGridFilterWidget::onReset);
    connect(apply, &QPushButton::clicked, this, &IDOSGridFilterWidget::onApply);
    connect(close, &QPushButton::clicked, this, &QWidget::close);
    for (QRadioButton* radio : QList<QRadioButton*>{m_union, m_intersection, m_show, m_hide, m_invert})
    {
        connect(radio, &QRadioButton::toggled, this, &IDOSGridFilterWidget::onEditorChanged);
    }
}

QWidget* IDOSGridFilterWidget::buildAxisPanel(int axis)
{
    const QStringList names = {QStringLiteral("I"), QStringLiteral("J"), QStringLiteral("K")};
    const QList<int> dimensions = {m_grid->nx(), m_grid->ny(), m_grid->nz()};
    QGroupBox* panel = new QGroupBox(names.at(axis), this);
    QVBoxLayout* layout = new QVBoxLayout(panel);
    QHBoxLayout* range = new QHBoxLayout();
    QCheckBox* enabled = new QCheckBox(tr("Range:"), panel);
    QSpinBox* minimum = new QSpinBox(panel);
    QSpinBox* maximum = new QSpinBox(panel);
    enabled->setObjectName(QStringLiteral("gridFilter%1Enabled").arg(names.at(axis)));
    minimum->setObjectName(QStringLiteral("gridFilter%1Minimum").arg(names.at(axis)));
    maximum->setObjectName(QStringLiteral("gridFilter%1Maximum").arg(names.at(axis)));
    minimum->setRange(1, qMax(1, dimensions.at(axis)));
    maximum->setRange(1, qMax(1, dimensions.at(axis)));
    maximum->setValue(maximum->maximum());
    minimum->setKeyboardTracking(false);
    maximum->setKeyboardTracking(false);
    range->addWidget(enabled);
    range->addWidget(minimum);
    range->addWidget(maximum);
    layout->addLayout(range);
    QListWidget* list = new QListWidget(panel);
    list->setObjectName(QStringLiteral("gridFilter%1List").arg(names.at(axis)));
    list->setSelectionMode(QAbstractItemView::NoSelection);
    list->setUniformItemSizes(true);
    for (int index = 1; index <= dimensions.at(axis); ++index)
    {
        QListWidgetItem* item = new QListWidgetItem(QString::number(index), list);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }
    layout->addWidget(list, 1);
    QHBoxLayout* each = new QHBoxLayout();
    each->addWidget(new QLabel(tr("Each:"), panel));
    QSpinBox* step = new QSpinBox(panel);
    step->setRange(1, qMax(1, dimensions.at(axis)));
    step->setValue(qMin(10, step->maximum()));
    step->setObjectName(QStringLiteral("gridFilter%1Step").arg(names.at(axis)));
    each->addWidget(step, 1);
    QToolButton* select = new QToolButton(panel);
    select->setText(tr("Select"));
    select->setProperty("axis", axis);
    select->setObjectName(QStringLiteral("gridFilter%1Select").arg(names.at(axis)));
    QToolButton* clear = new QToolButton(panel);
    clear->setText(tr("Clear"));
    clear->setProperty("axis", axis);
    clear->setObjectName(QStringLiteral("gridFilter%1Clear").arg(names.at(axis)));
    each->addWidget(select);
    each->addWidget(clear);
    layout->addLayout(each);
    m_axisEnabled.append(enabled);
    m_axisMinimum.append(minimum);
    m_axisMaximum.append(maximum);
    m_axisLists.append(list);
    m_axisSteps.append(step);
    connect(enabled, &QCheckBox::toggled, this, &IDOSGridFilterWidget::onEditorChanged);
    connect(minimum, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &IDOSGridFilterWidget::onEditorChanged);
    connect(maximum, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &IDOSGridFilterWidget::onEditorChanged);
    connect(list, &QListWidget::itemChanged, this, &IDOSGridFilterWidget::onEditorChanged);
    connect(select, &QToolButton::clicked, this, &IDOSGridFilterWidget::onAxisSelect);
    connect(clear, &QToolButton::clicked, this, &IDOSGridFilterWidget::onAxisClear);
    return panel;
}

QWidget* IDOSGridFilterWidget::buildPropertyPage()
{
    QSplitter* page = new QSplitter(Qt::Horizontal, m_tabs);
    QGroupBox* treePanel = new QGroupBox(tr("Attribute Tree"), page);
    QVBoxLayout* treeLayout = new QVBoxLayout(treePanel);
    m_propertySearch = new QLineEdit(treePanel);
    m_propertySearch->setPlaceholderText(tr("Search attributes..."));
    m_propertySearch->setObjectName(QStringLiteral("gridFilterPropertySearch"));
    treeLayout->addWidget(m_propertySearch);
    m_propertyList = new QListWidget(treePanel);
    m_propertyList->setObjectName(QStringLiteral("gridFilterPropertyList"));
    m_propertyList->setUniformItemSizes(true);
    treeLayout->addWidget(m_propertyList, 1);
    QHBoxLayout* footer = new QHBoxLayout();
    m_selectedCount = new QLabel(treePanel);
    QPushButton* clear = new QPushButton(tr("Clear"), treePanel);
    clear->setObjectName(QStringLiteral("gridFilterPropertyClear"));
    footer->addWidget(m_selectedCount, 1);
    footer->addWidget(clear);
    treeLayout->addLayout(footer);
    QWidget* editor = new QWidget(page);
    QVBoxLayout* layout = new QVBoxLayout(editor);
    QFormLayout* group = new QFormLayout();
    m_displayProperty = new QComboBox(editor);
    m_displayProperty->setObjectName(QStringLiteral("gridFilterDisplayProperty"));
    m_propertyLogic = new QComboBox(editor);
    m_propertyLogic->setObjectName(QStringLiteral("gridFilterPropertyLogic"));
    m_propertyLogic->addItem(tr("Any condition"), true);
    m_propertyLogic->addItem(tr("All conditions"), false);
    group->addRow(tr("Display attribute"), m_displayProperty);
    group->addRow(tr("Conditions"), m_propertyLogic);
    layout->addLayout(group);
    QHBoxLayout* heading = new QHBoxLayout();
    m_propertyEnabled = new QCheckBox(editor);
    m_propertyEnabled->setObjectName(QStringLiteral("gridFilterPropertyEnabled"));
    m_propertyEnabled->setToolTip(tr("Enable attribute filtering"));
    m_propertyTitle = new QLabel(tr("No attribute selected"), editor);
    m_propertyRange = new QLabel(editor);
    m_propertyRange->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    heading->addWidget(m_propertyEnabled);
    heading->addWidget(m_propertyTitle, 1);
    heading->addWidget(m_propertyRange);
    layout->addLayout(heading);
    QHBoxLayout* range = new QHBoxLayout();
    m_minimum = new QDoubleSpinBox(editor);
    m_maximum = new QDoubleSpinBox(editor);
    m_minimum->setObjectName(QStringLiteral("gridFilterPropertyMinimum"));
    m_maximum->setObjectName(QStringLiteral("gridFilterPropertyMaximum"));
    for (QDoubleSpinBox* spin : QList<QDoubleSpinBox*>{m_minimum, m_maximum})
    {
        spin->setRange(-1.0e100, 1.0e100);
        spin->setDecimals(8);
        spin->setKeyboardTracking(false);
    }
    range->addWidget(new QLabel(tr("Range:"), editor));
    range->addWidget(m_minimum);
    range->addWidget(new QLabel(tr("to"), editor));
    range->addWidget(m_maximum);
    m_comparison = new QComboBox(editor);
    m_comparison->setObjectName(QStringLiteral("gridFilterComparison"));
    m_comparison->addItem(tr("Between"), QStringLiteral("between"));
    m_comparison->addItem(tr("Greater than"), QStringLiteral("greater"));
    m_comparison->addItem(tr("Greater than or equal"), QStringLiteral("greaterEqual"));
    m_comparison->addItem(tr("Less than"), QStringLiteral("less"));
    m_comparison->addItem(tr("Less than or equal"), QStringLiteral("lessEqual"));
    m_comparison->addItem(tr("Equal"), QStringLiteral("equal"));
    m_comparison->addItem(tr("Not equal"), QStringLiteral("notEqual"));
    range->addWidget(m_comparison);
    layout->addLayout(range);
    m_rangeBar = new QLabel(editor);
    m_rangeBar->setObjectName(QStringLiteral("gridFilterRangeBar"));
    m_rangeBar->setFixedHeight(42);
    m_rangeBar->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    m_rangeBar->setMouseTracking(true);
    m_rangeBar->setToolTip(tr("Drag the handles to select the minimum and maximum attribute values."));
    m_rangeBar->installEventFilter(this);
    layout->addWidget(m_rangeBar);
    QGroupBox* operation = new QGroupBox(tr("Attribute Operation"), editor);
    QHBoxLayout* operationLayout = new QHBoxLayout(operation);
    m_propertyShow = new QRadioButton(tr("Retain interval"), operation);
    m_propertyHide = new QRadioButton(tr("Hide interval"), operation);
    m_propertyInvert = new QRadioButton(tr("Invert interval"), operation);
    m_propertyShow->setObjectName(QStringLiteral("gridFilterPropertyShow"));
    m_propertyHide->setObjectName(QStringLiteral("gridFilterPropertyHide"));
    m_propertyInvert->setObjectName(QStringLiteral("gridFilterPropertyInvert"));
    m_propertyShow->setChecked(true);
    operationLayout->addWidget(m_propertyShow);
    operationLayout->addWidget(m_propertyHide);
    operationLayout->addWidget(m_propertyInvert);
    layout->addWidget(operation);
    m_histogram = new QLabel(editor);
    m_histogram->setObjectName(QStringLiteral("gridFilterHistogram"));
    m_histogram->setMinimumSize(320, 150);
    m_histogram->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_histogram->installEventFilter(this);
    layout->addWidget(m_histogram, 1);
    QHBoxLayout* boundaries = new QHBoxLayout();
    m_includeMinimum = new QCheckBox(tr("Include minimum"), editor);
    m_includeMaximum = new QCheckBox(tr("Include maximum"), editor);
    m_includeInvalid = new QCheckBox(tr("Include invalid values"), editor);
    m_includeMinimum->setObjectName(QStringLiteral("gridFilterIncludeMinimum"));
    m_includeMaximum->setObjectName(QStringLiteral("gridFilterIncludeMaximum"));
    m_includeInvalid->setObjectName(QStringLiteral("gridFilterIncludeInvalid"));
    m_includeMinimum->setChecked(true);
    m_includeMaximum->setChecked(true);
    boundaries->addWidget(m_includeMinimum);
    boundaries->addWidget(m_includeMaximum);
    boundaries->addWidget(m_includeInvalid);
    layout->addLayout(boundaries);
    QFormLayout* tolerance = new QFormLayout();
    m_epsilon = new QDoubleSpinBox(editor);
    m_epsilon->setObjectName(QStringLiteral("gridFilterTolerance"));
    m_epsilon->setRange(0.0, 1.0e12);
    m_epsilon->setDecimals(8);
    m_epsilon->setSingleStep(1.0e-6);
    m_epsilon->setValue(1.0e-6);
    tolerance->addRow(tr("Tolerance"), m_epsilon);
    layout->addLayout(tolerance);
    QLabel* hint = new QLabel(tr("Greater comparisons use the minimum; less comparisons use the maximum. Equal and Not equal use the minimum and tolerance. Property values follow the current report step."), editor);
    hint->setWordWrap(true);
    layout->addWidget(hint);
    page->addWidget(treePanel);
    page->addWidget(editor);
    page->setStretchFactor(0, 1);
    page->setStretchFactor(1, 3);
    page->setSizes(QList<int>{260, 760});
    connect(m_propertySearch, &QLineEdit::textChanged, this, &IDOSGridFilterWidget::onPropertySearch);
    connect(m_propertyList, &QListWidget::currentItemChanged, this, &IDOSGridFilterWidget::onPropertySelected);
    connect(m_propertyList, &QListWidget::itemChanged, this, &IDOSGridFilterWidget::onPropertyChecked);
    connect(clear, &QPushButton::clicked, this, &IDOSGridFilterWidget::onPropertyClear);
    connect(m_displayProperty, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &IDOSGridFilterWidget::onEditorChanged);
    connect(m_propertyLogic, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &IDOSGridFilterWidget::onEditorChanged);
    connect(m_comparison, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &IDOSGridFilterWidget::onEditorChanged);
    for (QDoubleSpinBox* spin : QList<QDoubleSpinBox*>{m_minimum, m_maximum, m_epsilon})
    {
        connect(spin, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &IDOSGridFilterWidget::onEditorChanged);
    }
    for (QCheckBox* check : QList<QCheckBox*>{m_propertyEnabled, m_includeMinimum, m_includeMaximum, m_includeInvalid})
    {
        connect(check, &QCheckBox::toggled, this, &IDOSGridFilterWidget::onEditorChanged);
    }
    for (QRadioButton* radio : QList<QRadioButton*>{m_propertyShow, m_propertyHide, m_propertyInvert})
    {
        connect(radio, &QRadioButton::toggled, this, &IDOSGridFilterWidget::onEditorChanged);
    }
    return page;
}

QVariantMap IDOSGridFilterWidget::defaultCondition(IDOSGridProperty* property) const
{
    double minimum = std::numeric_limits<double>::infinity();
    double maximum = -std::numeric_limits<double>::infinity();
    if (property != nullptr)
    {
        for (const double value : property->values())
        {
            if (std::isfinite(value))
            {
                minimum = qMin(minimum, value);
                maximum = qMax(maximum, value);
            }
        }
    }
    QVariantMap condition;
    condition.insert(QStringLiteral("enabled"), false);
    condition.insert(QStringLiteral("propertyId"), property != nullptr ? property->objectId() : QString());
    condition.insert(QStringLiteral("operation"), QStringLiteral("between"));
    condition.insert(QStringLiteral("minimum"), std::isfinite(minimum) ? minimum : 0.0);
    condition.insert(QStringLiteral("maximum"), std::isfinite(maximum) ? maximum : 1.0);
    condition.insert(QStringLiteral("epsilon"), 1.0e-6);
    condition.insert(QStringLiteral("includeMinimum"), true);
    condition.insert(QStringLiteral("includeMaximum"), true);
    condition.insert(QStringLiteral("includeInvalidValues"), false);
    return condition;
}

void IDOSGridFilterWidget::populateObjects()
{
    const QSignalBlocker filterBlock(m_filterObjects);
    const QSignalBlocker propertyBlock(m_propertyList);
    const QSignalBlocker displayBlock(m_displayProperty);
    const QString displayId = m_displayProperty->currentData().toString();
    m_filterObjects->clear();
    m_filterObjects->addItem(tr("Temporary filter"), QString());
    m_displayProperty->clear();
    m_displayProperty->addItem(tr("Solid color"), QString());
    m_propertyList->clear();
    QStringList available;
    for (IDOSDataObject* data : m_project->objects())
    {
        IDOSGridFilter* filter = qobject_cast<IDOSGridFilter*>(data);
        if (filter != nullptr && filter->gridId() == m_grid->objectId())
        {
            m_filterObjects->addItem(filter->name(), filter->objectId());
        }
        IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(data);
        if (property != nullptr && property->gridId() == m_grid->objectId())
        {
            available.append(property->objectId());
            if (!m_conditionStates.contains(property->objectId()))
            {
                m_conditionStates.insert(property->objectId(), defaultCondition(property));
            }
            QListWidgetItem* item = new QListWidgetItem(property->name(), m_propertyList);
            item->setData(Qt::UserRole, property->objectId());
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(m_conditionStates.value(property->objectId()).value(QStringLiteral("enabled"), true).toBool() ? Qt::Checked : Qt::Unchecked);
            m_displayProperty->addItem(property->name(), property->objectId());
        }
    }
    for (QMap<QString, QVariantMap>::const_iterator it = m_conditionStates.constBegin(); it != m_conditionStates.constEnd(); ++it)
    {
        if (!available.contains(it.key()))
        {
            QListWidgetItem* item = new QListWidgetItem(tr("Missing attribute: %1").arg(it.key()), m_propertyList);
            item->setData(Qt::UserRole, it.key());
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(it.value().value(QStringLiteral("enabled"), true).toBool() ? Qt::Checked : Qt::Unchecked);
        }
    }
    m_propertyList->sortItems();
    m_filterObjects->setCurrentIndex(qMax(0, m_filterObjects->findData(m_filterId)));
    m_displayProperty->setCurrentIndex(qMax(0, m_displayProperty->findData(displayId)));
    for (int row = 0; row < m_propertyList->count(); ++row)
    {
        if (m_propertyList->item(row)->data(Qt::UserRole).toString() == m_propertyId)
        {
            m_propertyList->setCurrentRow(row);
        }
    }
    m_save->setEnabled(!m_filterId.isEmpty());
    onPropertySearch(m_propertySearch->text());
}

void IDOSGridFilterWidget::storePropertyCondition()
{
    if (m_propertyId.isEmpty())
    {
        return;
    }
    QVariantMap condition = m_conditionStates.value(m_propertyId);
    condition.insert(QStringLiteral("propertyId"), m_propertyId);
    condition.insert(QStringLiteral("enabled"), m_propertyEnabled->isChecked());
    condition.insert(QStringLiteral("operation"), m_comparison->currentData());
    condition.insert(QStringLiteral("minimum"), m_minimum->value());
    condition.insert(QStringLiteral("maximum"), m_maximum->value());
    condition.insert(QStringLiteral("epsilon"), m_epsilon->value());
    condition.insert(QStringLiteral("includeMinimum"), m_includeMinimum->isChecked());
    condition.insert(QStringLiteral("includeMaximum"), m_includeMaximum->isChecked());
    condition.insert(QStringLiteral("includeInvalidValues"), m_includeInvalid->isChecked());
    m_conditionStates.insert(m_propertyId, condition);
    const QSignalBlocker blocker(m_propertyList);
    for (int row = 0; row < m_propertyList->count(); ++row)
    {
        QListWidgetItem* item = m_propertyList->item(row);
        if (item->data(Qt::UserRole).toString() == m_propertyId)
        {
            item->setCheckState(m_propertyEnabled->isChecked() ? Qt::Checked : Qt::Unchecked);
        }
    }
}

void IDOSGridFilterWidget::loadPropertyCondition(const QString& id)
{
    const bool loading = m_loading;
    m_loading = true;
    m_propertyId = id;
    const QVariantMap condition = m_conditionStates.value(id);
    m_propertyEnabled->setChecked(condition.value(QStringLiteral("enabled"), false).toBool());
    m_comparison->setCurrentIndex(qMax(0, m_comparison->findData(condition.value(QStringLiteral("operation")))));
    m_minimum->setValue(condition.value(QStringLiteral("minimum"), 0.0).toDouble());
    m_maximum->setValue(condition.value(QStringLiteral("maximum"), 1.0).toDouble());
    m_epsilon->setValue(condition.value(QStringLiteral("epsilon"), 1.0e-6).toDouble());
    m_includeMinimum->setChecked(condition.value(QStringLiteral("includeMinimum"), true).toBool());
    m_includeMaximum->setChecked(condition.value(QStringLiteral("includeMaximum"), true).toBool());
    m_includeInvalid->setChecked(condition.value(QStringLiteral("includeInvalidValues"), false).toBool());
    IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(m_project->object(id));
    m_propertyTitle->setText(property != nullptr ? property->name() : id.isEmpty() ? tr("No attribute selected") : tr("Missing attribute"));
    m_loading = loading;
}

QVariantMap IDOSGridFilterWidget::criteriaFromUi() const
{
    QVariantMap criteria;
    QVariantMap axes;
    const QStringList names = {QStringLiteral("i"), QStringLiteral("j"), QStringLiteral("k")};
    for (int axis = 0; axis < 3; ++axis)
    {
        QVariantMap range;
        range.insert(QStringLiteral("enabled"), m_axisEnabled.at(axis)->isChecked());
        range.insert(QStringLiteral("minimum"), m_axisMinimum.at(axis)->value());
        range.insert(QStringLiteral("maximum"), m_axisMaximum.at(axis)->value());
        QVariantList values;
        for (int row = 0; row < m_axisLists.at(axis)->count(); ++row)
        {
            if (m_axisLists.at(axis)->item(row)->checkState() == Qt::Checked)
            {
                values.append(row + 1);
            }
        }
        range.insert(QStringLiteral("values"), values);
        axes.insert(names.at(axis), range);
    }
    QVariantList conditions;
    for (QMap<QString, QVariantMap>::const_iterator it = m_conditionStates.constBegin(); it != m_conditionStates.constEnd(); ++it)
    {
        conditions.append(it.value());
    }
    criteria.insert(QStringLiteral("axes"), axes);
    criteria.insert(QStringLiteral("axisUnion"), m_union->isChecked());
    criteria.insert(QStringLiteral("axisOperation"), m_hide->isChecked() ? QStringLiteral("hide") : m_invert->isChecked() ? QStringLiteral("invert") : QStringLiteral("show"));
    criteria.insert(QStringLiteral("conditions"), conditions);
    criteria.insert(QStringLiteral("anyProperty"), m_propertyLogic->currentData());
    criteria.insert(QStringLiteral("propertyOperation"), m_propertyHide->isChecked() ? QStringLiteral("hide") : m_propertyInvert->isChecked() ? QStringLiteral("invert") : QStringLiteral("show"));
    criteria.insert(QStringLiteral("displayPropertyId"), m_displayProperty->currentData());
    return criteria;
}

void IDOSGridFilterWidget::loadCriteria(const QVariantMap& criteria)
{
    const bool loading = m_loading;
    m_loading = true;
    m_propertyId.clear();
    m_conditionStates.clear();
    for (const QVariant& entry : criteria.value(QStringLiteral("conditions")).toList())
    {
        QVariantMap condition = entry.toMap();
        if (!condition.contains(QStringLiteral("enabled")))
        {
            condition.insert(QStringLiteral("enabled"), true);
        }
        m_conditionStates.insert(condition.value(QStringLiteral("propertyId")).toString(), condition);
    }
    populateObjects();
    const QVariantMap axes = criteria.value(QStringLiteral("axes")).toMap();
    const QStringList names = {QStringLiteral("i"), QStringLiteral("j"), QStringLiteral("k")};
    for (int axis = 0; axis < 3; ++axis)
    {
        const QVariantMap range = axes.value(names.at(axis)).toMap();
        m_axisEnabled.at(axis)->setChecked(range.value(QStringLiteral("enabled")).toBool());
        m_axisMinimum.at(axis)->setValue(range.value(QStringLiteral("minimum"), 1).toInt());
        m_axisMaximum.at(axis)->setValue(range.value(QStringLiteral("maximum"), m_axisMaximum.at(axis)->maximum()).toInt());
        const QVariantList values = range.value(QStringLiteral("values")).toList();
        const QSignalBlocker blocker(m_axisLists.at(axis));
        for (int row = 0; row < m_axisLists.at(axis)->count(); ++row)
        {
            m_axisLists.at(axis)->item(row)->setCheckState(values.contains(row + 1) ? Qt::Checked : Qt::Unchecked);
        }
    }
    m_union->setChecked(criteria.value(QStringLiteral("axisUnion"), true).toBool());
    m_intersection->setChecked(!m_union->isChecked());
    const QString axisOperation = criteria.value(QStringLiteral("axisOperation"), criteria.value(QStringLiteral("invert")).toBool() ? QStringLiteral("invert") : QStringLiteral("show")).toString();
    m_show->setChecked(axisOperation == QStringLiteral("show"));
    m_hide->setChecked(axisOperation == QStringLiteral("hide"));
    m_invert->setChecked(axisOperation == QStringLiteral("invert"));
    const QString propertyOperation = criteria.value(QStringLiteral("propertyOperation"), QStringLiteral("show")).toString();
    m_propertyShow->setChecked(propertyOperation == QStringLiteral("show"));
    m_propertyHide->setChecked(propertyOperation == QStringLiteral("hide"));
    m_propertyInvert->setChecked(propertyOperation == QStringLiteral("invert"));
    m_propertyLogic->setCurrentIndex(criteria.value(QStringLiteral("anyProperty"), true).toBool() ? 0 : 1);
    const QString displayId = criteria.contains(QStringLiteral("displayPropertyId")) ? criteria.value(QStringLiteral("displayPropertyId")).toString()
        : provider() != nullptr && provider()->property() != nullptr ? provider()->property()->objectId() : QString();
    m_displayProperty->setCurrentIndex(qMax(0, m_displayProperty->findData(displayId)));
    if (m_propertyList->count() > 0)
    {
        m_propertyList->setCurrentRow(0);
        loadPropertyCondition(m_propertyList->item(0)->data(Qt::UserRole).toString());
    }
    else
    {
        loadPropertyCondition(QString());
    }
    m_loading = loading;
}

IDOSRenderObject* IDOSGridFilterWidget::renderObject() const
{
    if (m_project == nullptr || m_grid == nullptr || m_view == nullptr || m_project->object(m_grid->objectId()) != m_grid.data())
    {
        return nullptr;
    }
    IDOSDataObject* source = m_filterId.isEmpty() ? static_cast<IDOSDataObject*>(m_grid.data()) : m_project->object(m_filterId);
    IDOSRenderObject* object = m_view->scene()->object(m_renderId);
    return source != nullptr && object != nullptr && object->provider()->dataSource() == source ? object : nullptr;
}

IDOSGridRenderProvider* IDOSGridFilterWidget::provider() const
{
    IDOSRenderObject* object = renderObject();
    return object != nullptr ? dynamic_cast<IDOSGridRenderProvider*>(object->provider()) : nullptr;
}

bool IDOSGridFilterWidget::selectTarget(const QString& filterId)
{
    if (m_project == nullptr || m_grid == nullptr || m_view == nullptr)
    {
        return false;
    }
    IDOSGridFilter* filter = qobject_cast<IDOSGridFilter*>(m_project->object(filterId));
    if (!filterId.isEmpty() && (filter == nullptr || filter->gridId() != m_grid->objectId()))
    {
        return false;
    }
    IDOSDataObject* source = filter != nullptr ? static_cast<IDOSDataObject*>(filter) : m_grid.data();
    for (IDOSRenderObject* object : m_view->scene()->objects())
    {
        if (object->provider()->dataSource() == source && dynamic_cast<IDOSGridRenderProvider*>(object->provider()) != nullptr)
        {
            m_filterId = filterId;
            m_renderId = object->id();
            loadCriteria(filter != nullptr ? filter->criteria() : provider()->filterCriteria());
            setWindowTitle(filter != nullptr ? tr("Grid Filter - %1").arg(filter->name()) : tr("Grid Filter"));
            return true;
        }
    }
    return false;
}

QMap<QString, IDOSGridProperty*> IDOSGridFilterWidget::propertyReferences() const
{
    QMap<QString, IDOSGridProperty*> properties;
    if (m_project != nullptr && m_grid != nullptr)
    {
        for (IDOSDataObject* data : m_project->objects())
        {
            IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(data);
            if (property != nullptr && property->gridId() == m_grid->objectId() && property->numX() == m_grid->nx()
                && property->numY() == m_grid->ny() && property->numZ() == m_grid->nz())
            {
                properties.insert(property->objectId(), property);
            }
        }
    }
    return properties;
}

void IDOSGridFilterWidget::onAxisSelect()
{
    const int axis = sender()->property("axis").toInt();
    const QSignalBlocker blocker(m_axisLists.at(axis));
    const int step = m_axisSteps.at(axis)->value();
    for (int row = 0; row < m_axisLists.at(axis)->count(); ++row)
    {
        m_axisLists.at(axis)->item(row)->setCheckState(row % step == 0 ? Qt::Checked : Qt::Unchecked);
    }
    onEditorChanged();
}

void IDOSGridFilterWidget::onAxisClear()
{
    const int axis = sender()->property("axis").toInt();
    const QSignalBlocker blocker(m_axisLists.at(axis));
    for (int row = 0; row < m_axisLists.at(axis)->count(); ++row)
    {
        m_axisLists.at(axis)->item(row)->setCheckState(Qt::Unchecked);
    }
    onEditorChanged();
}

void IDOSGridFilterWidget::onPropertySelected()
{
    if (m_loading)
    {
        return;
    }
    storePropertyCondition();
    QListWidgetItem* item = m_propertyList->currentItem();
    loadPropertyCondition(item != nullptr ? item->data(Qt::UserRole).toString() : QString());
    onEditorChanged();
}

void IDOSGridFilterWidget::onPropertyChecked(QListWidgetItem* item)
{
    if (m_loading || item == nullptr)
    {
        return;
    }
    const QString id = item->data(Qt::UserRole).toString();
    QVariantMap condition = m_conditionStates.value(id);
    condition.insert(QStringLiteral("enabled"), item->checkState() == Qt::Checked);
    m_conditionStates.insert(id, condition);
    if (id == m_propertyId)
    {
        const QSignalBlocker blocker(m_propertyEnabled);
        m_propertyEnabled->setChecked(item->checkState() == Qt::Checked);
    }
    onEditorChanged();
}

void IDOSGridFilterWidget::onPropertyClear()
{
    m_loading = true;
    for (int row = 0; row < m_propertyList->count(); ++row)
    {
        QListWidgetItem* item = m_propertyList->item(row);
        item->setCheckState(Qt::Unchecked);
        QVariantMap condition = m_conditionStates.value(item->data(Qt::UserRole).toString());
        condition.insert(QStringLiteral("enabled"), false);
        m_conditionStates.insert(item->data(Qt::UserRole).toString(), condition);
    }
    m_propertyEnabled->setChecked(false);
    m_loading = false;
    onEditorChanged();
}

void IDOSGridFilterWidget::onPropertySearch(const QString& text)
{
    for (int row = 0; row < m_propertyList->count(); ++row)
    {
        QListWidgetItem* item = m_propertyList->item(row);
        item->setHidden(!item->text().contains(text, Qt::CaseInsensitive));
    }
}

void IDOSGridFilterWidget::onEditorChanged()
{
    if (m_loading || m_summaryTimer == nullptr)
    {
        return;
    }
    storePropertyCondition();
    m_summaryTimer->start();
}

void IDOSGridFilterWidget::onFilterSelected()
{
    if (!m_loading)
    {
        if (!selectTarget(m_filterObjects->currentData().toString()))
        {
            onInvalidate();
            return;
        }
        onSummaryRefresh();
    }
}

void IDOSGridFilterWidget::onApply()
{
    IDOSGridRenderProvider* target = provider();
    if (target == nullptr)
    {
        onInvalidate();
        return;
    }
    storePropertyCondition();
    const QVariantMap criteria = criteriaFromUi();
    QString error;
    if (!IDOSGridFilter::validateCriteria(criteria, error))
    {
        m_summary->setText(error);
        return;
    }
    m_loading = true;
    target->setProperty(qobject_cast<IDOSGridProperty*>(m_project->object(m_displayProperty->currentData().toString())));
    target->setFilterCriteria(criteria);
    for (const QString& id : IDOSGridFilter::propertyIds(criteria))
    {
        target->setDataReference(m_project->object(id));
    }
    target->IDOSGridRenderProvider::update();
    target->setVisible(true);
    m_view->scene()->notifyChanged(m_renderId);
    m_loading = false;
    onSummaryRefresh();
}

void IDOSGridFilterWidget::onReset()
{
    if (provider() == nullptr)
    {
        return;
    }
    loadCriteria(QVariantMap());
    onApply();
}

bool IDOSGridFilterWidget::saveFilter(const QString& name, bool create)
{
    if (provider() == nullptr || m_project->commandManager() == nullptr)
    {
        return false;
    }
    storePropertyCondition();
    const QVariantMap savedCriteria = criteriaFromUi();
    QJsonObject arguments;
    arguments.insert(QStringLiteral("criteria"), QJsonObject::fromVariantMap(savedCriteria));
    arguments.insert(QStringLiteral("gridId"), m_grid->objectId());
    arguments.insert(QStringLiteral("filterId"), m_filterId);
    arguments.insert(QStringLiteral("name"), name);
    const bool sourcePreview = m_filterId.isEmpty();
    const QString previewRenderId = m_renderId;
    IDOSCommandManager* manager = m_project->commandManager();
    if (!manager->execute(new IDOSGridFilterCommand(m_project.data(), create ? QStringLiteral("create") : QStringLiteral("update"), arguments)))
    {
        m_summary->setText(manager->error());
        return false;
    }
    const QString id = manager->result().value(QStringLiteral("filterId")).toString();
    if (sourcePreview && m_view != nullptr && m_grid != nullptr)
    {
        IDOSRenderObject* preview = m_view->scene()->object(previewRenderId);
        IDOSGridRenderProvider* previewProvider = preview != nullptr ? dynamic_cast<IDOSGridRenderProvider*>(preview->provider()) : nullptr;
        if (previewProvider != nullptr && previewProvider->dataSource() == m_grid.data()
            && previewProvider->setFilterCriteria(savedCriteria))
        {
            for (const QString& propertyId : IDOSGridFilter::propertyIds(savedCriteria))
            {
                previewProvider->setDataReference(m_project->object(propertyId));
            }
            previewProvider->setProperty(qobject_cast<IDOSGridProperty*>(
                m_project->object(savedCriteria.value(QStringLiteral("displayPropertyId")).toString())));
            previewProvider->setPreviewFilterId(id);
            m_view->scene()->notifyChanged(preview->id());
        }
    }
    if (selectTarget(id))
    {
        renderObject()->setVisible(true);
        m_view->scene()->notifyChanged(m_renderId);
    }
    return true;
}

void IDOSGridFilterWidget::onSave()
{
    if (!m_filterId.isEmpty())
    {
        saveFilter(QString(), false);
    }
}

void IDOSGridFilterWidget::onSaveAs()
{
    if (provider() == nullptr)
    {
        return;
    }
    QInputDialog* dialog = new QInputDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("Save Grid Filter"));
    dialog->setLabelText(tr("Filter name:"));
    dialog->setTextValue(tr("Grid filter"));
    dialog->setObjectName(QStringLiteral("gridFilterNameDialog"));
    connect(dialog, &QDialog::accepted, this, &IDOSGridFilterWidget::onNameAccepted);
    dialog->open();
}

void IDOSGridFilterWidget::onNameAccepted()
{
    QInputDialog* dialog = qobject_cast<QInputDialog*>(sender());
    if (dialog != nullptr)
    {
        saveFilter(dialog->textValue(), true);
    }
}

void IDOSGridFilterWidget::onRefresh()
{
    if (m_loading)
    {
        return;
    }
    if (provider() == nullptr)
    {
        onInvalidate();
        return;
    }
    storePropertyCondition();
    m_loading = true;
    populateObjects();
    loadPropertyCondition(m_propertyId);
    m_loading = false;
    m_summaryTimer->start();
}

void IDOSGridFilterWidget::onInvalidate()
{
    setEnabled(false);
    m_project = nullptr;
    m_grid = nullptr;
    m_view = nullptr;
    close();
}

void IDOSGridFilterWidget::onDataRemoving(const QString& id)
{
    if ((m_grid != nullptr && m_grid->objectId() == id) || (!m_filterId.isEmpty() && m_filterId == id))
    {
        onInvalidate();
    }
}

void IDOSGridFilterWidget::onRenderObjectRemoving(const QString& id)
{
    if (id == m_renderId)
    {
        onInvalidate();
    }
}

void IDOSGridFilterWidget::onSummaryRefresh()
{
    if (m_loading || provider() == nullptr)
    {
        return;
    }
    storePropertyCondition();
    const QVariantMap criteria = criteriaFromUi();
    QString error;
    const QMap<QString, IDOSGridProperty*> references = propertyReferences();
    int selectedAttributes = 0;
    for (QMap<QString, QVariantMap>::const_iterator it = m_conditionStates.constBegin(); it != m_conditionStates.constEnd(); ++it)
    {
        selectedAttributes += it.value().value(QStringLiteral("enabled"), true).toBool() ? 1 : 0;
    }
    m_selectedCount->setText(tr("Selected %1 attributes").arg(selectedAttributes));
    for (int axis = 0; axis < 3; ++axis)
    {
        m_axisMinimum.at(axis)->setEnabled(m_axisEnabled.at(axis)->isChecked());
        m_axisMaximum.at(axis)->setEnabled(m_axisEnabled.at(axis)->isChecked());
    }
    const bool editable = !m_propertyId.isEmpty();
    const QString comparison = m_comparison->currentData().toString();
    const bool between = comparison == QStringLiteral("between");
    m_propertyEnabled->setEnabled(editable);
    m_comparison->setEnabled(editable);
    m_minimum->setEnabled(editable && comparison != QStringLiteral("less") && comparison != QStringLiteral("lessEqual"));
    m_maximum->setEnabled(editable && (between || comparison == QStringLiteral("less") || comparison == QStringLiteral("lessEqual")));
    m_includeMinimum->setEnabled(editable && between);
    m_includeMaximum->setEnabled(editable && between);
    m_includeInvalid->setEnabled(editable);
    m_epsilon->setEnabled(editable && (comparison == QStringLiteral("equal") || comparison == QStringLiteral("notEqual")));
    if (!IDOSGridFilter::validateCriteria(criteria, error))
    {
        m_summary->setText(error);
        return;
    }
    for (const QString& id : IDOSGridFilter::propertyIds(criteria))
    {
        if (!references.contains(id) || references.value(id)->values().size() != m_grid->totalCellCount())
        {
            m_summary->setText(tr("An enabled attribute is missing or incompatible. The filter cannot be applied."));
            return;
        }
    }
    const QVector<quint8> selection = IDOSGridFilter::selection(criteria, m_grid->nx(), m_grid->ny(), m_grid->nz(), references);
    int matched = 0;
    for (int index = 0; index < selection.size(); ++index)
    {
        if (m_grid->isActive(index % m_grid->nx(), (index / m_grid->nx()) % m_grid->ny(), index / (m_grid->nx() * m_grid->ny())) && selection.at(index))
        {
            ++matched;
        }
    }
    m_summary->setText(tr("Matched %1 / %2 active cells. Displayed in target view: %3.").arg(matched).arg(m_grid->activeCellCount()).arg(provider()->filteredCellCount()));
    m_bins.fill(0);
    m_selectedBins.fill(0);
    IDOSGridProperty* property = references.value(m_propertyId);
    m_dataMinimum = std::numeric_limits<double>::infinity();
    m_dataMaximum = -std::numeric_limits<double>::infinity();
    if (property != nullptr)
    {
        for (int index = 0; index < property->values().size(); ++index)
        {
            const double value = property->values().at(index);
            if (std::isfinite(value) && m_grid->isActive(index % m_grid->nx(), (index / m_grid->nx()) % m_grid->ny(), index / (m_grid->nx() * m_grid->ny())))
            {
                m_dataMinimum = qMin(m_dataMinimum, value);
                m_dataMaximum = qMax(m_dataMaximum, value);
            }
        }
    }
    m_propertyRange->setText(std::isfinite(m_dataMinimum) ? tr("Data range: %1 – %2").arg(m_dataMinimum, 0, 'g', 6).arg(m_dataMaximum, 0, 'g', 6) : tr("No finite attribute values"));
    if (!std::isfinite(m_dataMinimum))
    {
        m_dataMinimum = 0.0;
        m_dataMaximum = 1.0;
    }
    else if (m_dataMinimum == m_dataMaximum)
    {
        const double padding = qMax(1.0, std::abs(m_dataMinimum) * 0.01);
        m_dataMinimum -= padding;
        m_dataMaximum += padding;
    }
    QVariantMap gridCriteria = criteria;
    gridCriteria.remove(QStringLiteral("conditions"));
    gridCriteria.remove(QStringLiteral("propertyOperation"));
    const QVector<quint8> gridSelection = IDOSGridFilter::selection(gridCriteria, m_grid->nx(), m_grid->ny(), m_grid->nz(), references);
    if (property != nullptr)
    {
        for (int index = 0; index < property->values().size(); ++index)
        {
            const double value = property->values().at(index);
            if (!std::isfinite(value) || !gridSelection.at(index) || !m_grid->isActive(index % m_grid->nx(), (index / m_grid->nx()) % m_grid->ny(), index / (m_grid->nx() * m_grid->ny())))
            {
                continue;
            }
            const int bin = qBound(0, static_cast<int>((value - m_dataMinimum) / (m_dataMaximum - m_dataMinimum) * m_bins.size()), m_bins.size() - 1);
            ++m_bins[bin];
            m_selectedBins[bin] += selection.at(index) ? 1 : 0;
        }
    }
    drawPropertyCharts();
}

void IDOSGridFilterWidget::drawPropertyCharts()
{
    const int histogramWidth = qMax(320, m_histogram->width());
    const int histogramHeight = qMax(150, m_histogram->height());
    QPixmap histogram(histogramWidth, histogramHeight);
    histogram.fill(QColor(QStringLiteral("#f7f9fc")));
    QPainter painter(&histogram);
    const QRect plot(35, 16, histogramWidth - 50, histogramHeight - 45);
    int peak = 1;
    for (const int bin : m_bins)
    {
        peak = qMax(peak, bin);
    }
    const double binWidth = static_cast<double>(plot.width()) / m_bins.size();
    for (int index = 0; index < m_bins.size(); ++index)
    {
        const int x = plot.left() + static_cast<int>(index * binWidth);
        const int width = qMax(1, static_cast<int>(binWidth) - 1);
        const int height = static_cast<int>(static_cast<double>(m_bins.at(index)) / peak * plot.height());
        painter.fillRect(QRect(x, plot.bottom() - height, width, height), QColor(QStringLiteral("#b8c7d9")));
        const int selectedHeight = static_cast<int>(static_cast<double>(m_selectedBins.at(index)) / peak * plot.height());
        painter.fillRect(QRect(x, plot.bottom() - selectedHeight, width, selectedHeight), QColor(QStringLiteral("#3279c4")));
    }
    painter.setPen(QColor(QStringLiteral("#677386")));
    painter.drawLine(plot.bottomLeft(), plot.bottomRight());
    painter.drawText(QRect(plot.left(), plot.bottom() + 4, plot.width() / 2, 22), Qt::AlignLeft, QString::number(m_dataMinimum, 'g', 6));
    painter.drawText(QRect(plot.center().x(), plot.bottom() + 4, plot.width() / 2, 22), Qt::AlignRight, QString::number(m_dataMaximum, 'g', 6));
    painter.drawText(QRect(0, 0, 32, 22), Qt::AlignRight, QString::number(peak));
    painter.end();
    m_histogram->setPixmap(histogram);
    const int width = qMax(100, m_rangeBar->width());
    QPixmap range(width, 42);
    range.fill(QColor(QStringLiteral("#f7f9fc")));
    QPainter rangePainter(&range);
    const int minimumX = 12 + static_cast<int>(qBound(0.0, (m_minimum->value() - m_dataMinimum) / (m_dataMaximum - m_dataMinimum), 1.0) * (width - 24));
    const int maximumX = 12 + static_cast<int>(qBound(0.0, (m_maximum->value() - m_dataMinimum) / (m_dataMaximum - m_dataMinimum), 1.0) * (width - 24));
    rangePainter.fillRect(QRect(12, 18, width - 24, 6), QColor(QStringLiteral("#ced7e1")));
    const QString operation = m_comparison->currentData().toString();
    const int first = operation == QStringLiteral("less") || operation == QStringLiteral("lessEqual") ? 12 : minimumX;
    const int last = operation == QStringLiteral("greater") || operation == QStringLiteral("greaterEqual") ? width - 12 : maximumX;
    rangePainter.fillRect(QRect(first, 18, qMax(1, last - first), 6), QColor(QStringLiteral("#3279c4")));
    if (m_minimum->isEnabled())
    {
        rangePainter.fillRect(QRect(minimumX - 4, 10, 8, 22), QColor(QStringLiteral("#1f5fd0")));
    }
    if (m_maximum->isEnabled())
    {
        rangePainter.fillRect(QRect(maximumX - 4, 10, 8, 22), QColor(QStringLiteral("#1f5fd0")));
    }
    rangePainter.end();
    m_rangeBar->setPixmap(range);
}

void IDOSGridFilterWidget::updateRangeFromPosition(int x)
{
    const double fraction = qBound(0.0, static_cast<double>(x - 12) / qMax(1, m_rangeBar->width() - 24), 1.0);
    const double value = m_dataMinimum + fraction * (m_dataMaximum - m_dataMinimum);
    if (m_dragHandle == 0)
    {
        m_minimum->setValue(m_comparison->currentData().toString() == QStringLiteral("between") ? qMin(value, m_maximum->value()) : value);
    }
    else if (m_dragHandle == 1)
    {
        m_maximum->setValue(m_comparison->currentData().toString() == QStringLiteral("between") ? qMax(value, m_minimum->value()) : value);
    }
    drawPropertyCharts();
}

bool IDOSGridFilterWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_rangeBar && !m_propertyId.isEmpty())
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* mouse = static_cast<QMouseEvent*>(event);
            if (mouse->button() == Qt::LeftButton)
            {
                const double value = m_dataMinimum + qBound(0.0, static_cast<double>(mouse->pos().x() - 12) / qMax(1, m_rangeBar->width() - 24), 1.0) * (m_dataMaximum - m_dataMinimum);
                m_dragHandle = !m_minimum->isEnabled() ? 1 : !m_maximum->isEnabled() ? 0
                    : std::abs(value - m_minimum->value()) <= std::abs(value - m_maximum->value()) ? 0 : 1;
                updateRangeFromPosition(mouse->pos().x());
                return true;
            }
        }
        else if (event->type() == QEvent::MouseMove && m_dragHandle >= 0)
        {
            updateRangeFromPosition(static_cast<QMouseEvent*>(event)->pos().x());
            return true;
        }
        else if (event->type() == QEvent::MouseButtonRelease && m_dragHandle >= 0)
        {
            QMouseEvent* mouse = static_cast<QMouseEvent*>(event);
            if (mouse->button() == Qt::LeftButton)
            {
                updateRangeFromPosition(mouse->pos().x());
                m_dragHandle = -1;
                return true;
            }
        }
    }
    if ((watched == m_rangeBar || watched == m_histogram) && event->type() == QEvent::Resize && m_summaryTimer != nullptr)
    {
        m_summaryTimer->start();
    }
    return QWidget::eventFilter(watched, event);
}
