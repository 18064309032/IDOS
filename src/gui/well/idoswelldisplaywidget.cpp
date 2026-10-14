#include <QCheckBox>
#include <QColor>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QSize>
#include <QTabWidget>
#include <QVariantMap>
#include <QVBoxLayout>

#include "data/well/idoswell.h"
#include "idosproject.h"
#include "idosrenderobject.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"
#include "providers/well/idoswellrenderprovider.h"

#include "idoswelldisplaywidget.h"

IDOSWellDisplayWidget::IDOSWellDisplayWidget(QWidget* parent)
    :
    QWidget(parent, Qt::Window)
    , m_project(nullptr)
    , m_well(nullptr)
    , m_view(nullptr)
    , m_renderId()
    , m_loading(false)
    , m_checks()
    , m_numbers()
    , m_colors()
    , m_shape(new QComboBox(this))
    , m_symbol(new QComboBox(this))
    , m_tabs(new QTabWidget(this))
    , m_message(new QLabel(this))
    , m_wellName(new QLabel(this))
    , m_viewName(new QLabel(this))
    , m_trajectory(false)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QStringLiteral("wellDisplaySettingsWindow"));
    setWindowTitle(tr("Display Settings"));
    QVBoxLayout* layout = new QVBoxLayout(this);
    QFormLayout* information = new QFormLayout();
    m_wellName->setObjectName(QStringLiteral("wellDisplayNameLabel"));
    m_viewName->setObjectName(QStringLiteral("wellDisplayViewLabel"));
    m_wellName->setWordWrap(true);
    m_viewName->setWordWrap(true);
    information->addRow(tr("Well:"), m_wellName);
    information->addRow(tr("Target view:"), m_viewName);
    layout->addLayout(information);
    const QStringList titles{tr("Wellhead"), tr("Trajectory"), tr("Completions"), tr("Well label"), tr("Status colors")};
    QList<QFormLayout*> forms;
    for (const QString& title : titles)
    {
        QWidget* page = new QWidget(m_tabs);
        QFormLayout* form = new QFormLayout(page);
        forms.append(form);
        m_tabs->addTab(page, title);
    }
    addCheck(forms.at(0), QStringLiteral("headVisible"), tr("Show wellhead"));
    m_shape->setObjectName(QStringLiteral("wellDisplayHeadShape"));
    m_shape->addItem(tr("Well symbol"), QStringLiteral("symbol"));
    m_shape->addItem(tr("Point"), QStringLiteral("point"));
    m_shape->addItem(tr("Sphere"), QStringLiteral("sphere"));
    m_shape->addItem(tr("Cone"), QStringLiteral("cone"));
    forms.at(0)->addRow(tr("Wellhead shape"), m_shape);
    m_symbol->setObjectName(QStringLiteral("wellDisplaySymbol"));
    m_symbol->setIconSize(QSize(20, 20));
    m_symbol->addItem(IDOSWellRenderProvider::symbolIcon(-1), tr("Automatic (file symbol or well state)"), -1);
    const QStringList symbols{
        tr("(0) Undefined"), tr("(1) Proposed"), tr("(2) Dry"), tr("(3) Oil"),
        tr("(4) Minor oil"), tr("(5) Gas"), tr("(6) Minor gas"), tr("(7) Condensate"),
        tr("(8) Platform"), tr("(9) Abandoned oil and gas"), tr("(10) Abandoned oil minor gas"),
        tr("(11) Abandoned oil condensate"), tr("(12) Abandoned gas residual oil"),
        tr("(13) Abandoned gas condensate"), tr("(14) Abandoned minor oil and gas"),
        tr("(15) Injection water"), tr("(16) Injection gas"), tr("(17) Shallow borehole"),
        tr("(18) Drilling well"), tr("(19) Abandoned for technical reasons"),
        tr("(20) Temporarily abandoned"), tr("(21) Plugged and abandoned"), tr("(22) Not observed"),
        tr("(23) Residual oil"), tr("(24) Proposed platform"), tr("(25) Subsea installation"),
        tr("(26) Dry, plugged and abandoned"), tr("(27) Oil, plugged and abandoned")};
    for (int symbol = 0; symbol < symbols.size(); ++symbol)
    {
        m_symbol->addItem(IDOSWellRenderProvider::symbolIcon(symbol), symbols.at(symbol), symbol);
    }
    forms.at(0)->addRow(tr("Well symbol"), m_symbol);
    addNumber(forms.at(0), QStringLiteral("headSize"), tr("Size (point: pixels; shape: model units)"), 1.0, 10000.0);
    addColor(forms.at(0), QStringLiteral("headColor"), tr("Wellhead color"));
    addNumber(forms.at(0), QStringLiteral("symbolWidth"), tr("Symbol line width (pixels)"), 1.0, 100.0);
    addCheck(forms.at(1), QStringLiteral("trajectoryVisible"), tr("Show trajectory"));
    addColor(forms.at(1), QStringLiteral("trajectoryColor"), tr("Trajectory color"));
    addNumber(forms.at(1), QStringLiteral("trajectoryWidth"), tr("Line width (pixels)"), 1.0, 100.0);
    addCheck(forms.at(1), QStringLiteral("smooth"), tr("Smooth trajectory"));
    addCheck(forms.at(1), QStringLiteral("tubes"), tr("Render lines as tubes"));
    addCheck(forms.at(1), QStringLiteral("clip"), tr("Clip by measured depth"));
    addNumber(forms.at(1), QStringLiteral("minimum"), tr("Minimum MD (m)"), -1.0e9, 1.0e9);
    addNumber(forms.at(1), QStringLiteral("maximum"), tr("Maximum MD (m)"), -1.0e9, 1.0e9);
    addCheck(forms.at(2), QStringLiteral("completionVisible"), tr("Show measured completions"));
    addColor(forms.at(2), QStringLiteral("completionColor"), tr("Completion color"));
    addNumber(forms.at(2), QStringLiteral("completionWidth"), tr("Line width (pixels)"), 1.0, 100.0);
    QLabel* completionNote = new QLabel(tr("Measured intervals follow the available trajectory. Simulation grid layers are not drawn as measured depths."), this);
    completionNote->setWordWrap(true);
    forms.at(2)->addRow(completionNote);
    addCheck(forms.at(3), QStringLiteral("labelVisible"), tr("Show well name"));
    addColor(forms.at(3), QStringLiteral("labelColor"), tr("Label color"));
    addNumber(forms.at(3), QStringLiteral("labelSize"), tr("Font size (pixels)"), 1.0, 100.0);
    addCheck(forms.at(4), QStringLiteral("statusColors"), tr("Color by well status"));
    addCheck(forms.at(4), QStringLiteral("sameCompletionColors"), tr("Use well colors for completions"));
    addColor(forms.at(4), QStringLiteral("producerColor"), tr("Producer"));
    addColor(forms.at(4), QStringLiteral("injectorColor"), tr("Injector"));
    addColor(forms.at(4), QStringLiteral("closedColor"), tr("Closed"));
    addColor(forms.at(4), QStringLiteral("stoppedColor"), tr("Stopped"));
    addColor(forms.at(4), QStringLiteral("producerCompletionColor"), tr("Producer completions"));
    addColor(forms.at(4), QStringLiteral("injectorCompletionColor"), tr("Injector completions"));
    addColor(forms.at(4), QStringLiteral("closedCompletionColor"), tr("Closed completions"));
    addColor(forms.at(4), QStringLiteral("stoppedCompletionColor"), tr("Stopped completions"));
    layout->addWidget(m_tabs);
    QFormLayout* common = new QFormLayout();
    addCheck(common, QStringLiteral("visible"), tr("Show well"));
    addNumber(common, QStringLiteral("opacity"), tr("Opacity (%)"), 0.0, 100.0);
    layout->addLayout(common);
    m_message->setWordWrap(true);
    layout->addWidget(m_message);
    QPushButton* closeButton = new QPushButton(tr("Close"), this);
    layout->addWidget(closeButton);
    connect(closeButton, &QPushButton::clicked, this, &QWidget::close);
    connect(m_shape, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &IDOSWellDisplayWidget::onApply);
    connect(m_symbol, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &IDOSWellDisplayWidget::onApply);
    resize(480, sizeHint().height());
}

void IDOSWellDisplayWidget::setProject(IDOSProject* project)
{
    m_project = project;
}

void IDOSWellDisplayWidget::setWell(IDOSWell* well)
{
    m_well = well;
}

void IDOSWellDisplayWidget::setView(IDOSRenderView* view)
{
    m_view = view;
}

void IDOSWellDisplayWidget::setTrajectory(bool trajectory)
{
    m_trajectory = trajectory;
    bindTarget();
}

void IDOSWellDisplayWidget::bindTarget()
{
    if (m_project.isNull() || m_well.isNull() || m_view.isNull()
        || m_project->object(m_well->objectId()) != m_well)
    {
        onInvalidate();
        return;
    }
    m_wellName->setText(m_well->name());
    m_viewName->setText(m_view->windowTitle());
    connect(m_well.data(), &IDOSWell::nameChanged, m_wellName, &QLabel::setText);
    connect(m_view.data(), &QWidget::windowTitleChanged, m_viewName, &QLabel::setText);
    for (IDOSRenderObject* object : m_view->scene()->objects())
    {
        if (object->provider()->dataSource() == m_well)
        {
            m_renderId = object->id();
            break;
        }
    }
    connect(m_project.data(), &QObject::destroyed, this, &IDOSWellDisplayWidget::onInvalidate);
    connect(m_well.data(), &QObject::destroyed, this, &IDOSWellDisplayWidget::onInvalidate);
    connect(m_view.data(), &QObject::destroyed, this, &IDOSWellDisplayWidget::onInvalidate);
    connect(m_project.data(), &IDOSProject::objectAboutToBeRemoved, this, &IDOSWellDisplayWidget::onDataRemoving);
    connect(m_view->scene(), &IDOSRenderScene::objectAboutToBeRemoved, this, &IDOSWellDisplayWidget::onRenderObjectRemoving);
    connect(m_view->scene(), &IDOSRenderScene::changed, this, &IDOSWellDisplayWidget::onRefresh);
    onRefresh();
    m_tabs->setCurrentIndex(m_trajectory ? 1 : 0);
}

void IDOSWellDisplayWidget::addCheck(QFormLayout* form, const QString& key, const QString& text)
{
    QCheckBox* check = new QCheckBox(text, this);
    check->setObjectName(QStringLiteral("wellDisplay_") + key);
    m_checks.insert(key, check);
    form->addRow(check);
    connect(check, &QCheckBox::toggled, this, &IDOSWellDisplayWidget::onApply);
}

void IDOSWellDisplayWidget::addNumber(QFormLayout* form, const QString& key, const QString& text, double minimum, double maximum)
{
    QDoubleSpinBox* number = new QDoubleSpinBox(this);
    number->setObjectName(QStringLiteral("wellDisplay_") + key);
    number->setRange(minimum, maximum);
    number->setDecimals(2);
    number->setKeyboardTracking(false);
    m_numbers.insert(key, number);
    form->addRow(text, number);
    connect(number, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &IDOSWellDisplayWidget::onApply);
}

void IDOSWellDisplayWidget::addColor(QFormLayout* form, const QString& key, const QString& text)
{
    QPushButton* button = new QPushButton(this);
    button->setObjectName(QStringLiteral("wellDisplay_") + key);
    button->setProperty("settingKey", key);
    m_colors.insert(key, button);
    form->addRow(text, button);
    connect(button, &QPushButton::clicked, this, &IDOSWellDisplayWidget::onColor);
}

IDOSRenderObject* IDOSWellDisplayWidget::renderObject() const
{
    if (m_project.isNull() || m_well.isNull() || m_view.isNull() ||
        m_project->object(m_well->objectId()) != m_well.data())
    {
        return nullptr;
    }
    IDOSRenderObject* object = m_view->scene()->object(m_renderId);
    return object != nullptr && object->provider()->dataSource() == m_well.data() ? object : nullptr;
}

IDOSWellRenderProvider* IDOSWellDisplayWidget::provider() const
{
    IDOSRenderObject* object = renderObject();
    return object != nullptr ? dynamic_cast<IDOSWellRenderProvider*>(object->provider()) : nullptr;
}

void IDOSWellDisplayWidget::onRefresh()
{
    if (m_loading)
    {
        return;
    }
    IDOSWellRenderProvider* target = provider();
    if (target == nullptr)
    {
        onInvalidate();
        return;
    }
    m_loading = true;
    const QVariantMap settings = target->displaySettings();
    for (QMap<QString, QCheckBox*>::const_iterator entry = m_checks.constBegin(); entry != m_checks.constEnd(); ++entry)
    {
        entry.value()->setChecked(entry.key() == QStringLiteral("visible")
            ? target->visible() : settings.value(entry.key()).toBool());
    }
    for (QMap<QString, QDoubleSpinBox*>::const_iterator entry = m_numbers.constBegin(); entry != m_numbers.constEnd(); ++entry)
    {
        entry.value()->setValue(entry.key() == QStringLiteral("opacity") ? target->opacity() * 100.0 : settings.value(entry.key()).toDouble());
    }
    for (QMap<QString, QPushButton*>::const_iterator entry = m_colors.constBegin(); entry != m_colors.constEnd(); ++entry)
    {
        entry.value()->setText(settings.value(entry.key()).toString());
    }
    m_shape->setCurrentIndex(m_shape->findData(settings.value(QStringLiteral("headShape"))));
    m_symbol->setCurrentIndex(m_symbol->findData(settings.value(QStringLiteral("symbol"))));
    m_symbol->setEnabled(m_shape->currentData().toString() == QStringLiteral("symbol"));
    const bool statusColors = settings.value(QStringLiteral("statusColors")).toBool();
    const bool sameColors = settings.value(QStringLiteral("sameCompletionColors")).toBool();
    for (QMap<QString, QPushButton*>::const_iterator entry = m_colors.constBegin(); entry != m_colors.constEnd(); ++entry)
    {
        const QString key = entry.key();
        const bool individual = key == QStringLiteral("headColor") || key == QStringLiteral("trajectoryColor") ||
            key == QStringLiteral("completionColor");
        const bool label = key == QStringLiteral("labelColor");
        entry.value()->setEnabled(label || (individual ? !statusColors
            : statusColors && (!key.endsWith(QStringLiteral("CompletionColor")) || !sameColors)));
    }
    m_checks.value(QStringLiteral("sameCompletionColors"))->setEnabled(statusColors);
    m_numbers.value(QStringLiteral("minimum"))->setEnabled(m_checks.value(QStringLiteral("clip"))->isChecked());
    m_numbers.value(QStringLiteral("maximum"))->setEnabled(m_checks.value(QStringLiteral("clip"))->isChecked());
    m_loading = false;
}

void IDOSWellDisplayWidget::onApply()
{
    if (m_loading)
    {
        return;
    }
    IDOSWellRenderProvider* target = provider();
    if (target == nullptr)
    {
        onInvalidate();
        return;
    }
    QVariantMap settings = target->displaySettings();
    for (QMap<QString, QCheckBox*>::const_iterator entry = m_checks.constBegin(); entry != m_checks.constEnd(); ++entry)
    {
        if (entry.key() != QStringLiteral("visible"))
        {
            settings.insert(entry.key(), entry.value()->isChecked());
        }
    }
    for (QMap<QString, QDoubleSpinBox*>::const_iterator entry = m_numbers.constBegin(); entry != m_numbers.constEnd(); ++entry)
    {
        if (entry.key() != QStringLiteral("opacity"))
        {
            settings.insert(entry.key(), entry.value()->value());
        }
    }
    for (QMap<QString, QPushButton*>::const_iterator entry = m_colors.constBegin(); entry != m_colors.constEnd(); ++entry)
    {
        settings.insert(entry.key(), entry.value()->text());
    }
    settings.insert(QStringLiteral("headShape"), m_shape->currentData());
    settings.insert(QStringLiteral("symbol"), m_symbol->currentData());
    if (!target->setDisplaySettings(settings))
    {
        m_message->setText(tr("Minimum MD must be less than maximum MD."));
        m_numbers.value(QStringLiteral("minimum"))->setEnabled(m_checks.value(QStringLiteral("clip"))->isChecked());
        m_numbers.value(QStringLiteral("maximum"))->setEnabled(m_checks.value(QStringLiteral("clip"))->isChecked());
        return;
    }
    target->setOpacity(m_numbers.value(QStringLiteral("opacity"))->value() / 100.0);
    target->setVisible(m_checks.value(QStringLiteral("visible"))->isChecked());
    m_message->clear();
    m_view->scene()->notifyChanged(m_renderId);
}

void IDOSWellDisplayWidget::onColor()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button == nullptr || provider() == nullptr)
    {
        return;
    }
    QColorDialog* dialog = new QColorDialog(QColor(button->text()), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("Select color"));
    dialog->setProperty("settingKey", button->property("settingKey"));
    connect(dialog, &QColorDialog::colorSelected, this, &IDOSWellDisplayWidget::onColorSelected);
    dialog->open();
}

void IDOSWellDisplayWidget::onColorSelected(const QColor& color)
{
    QColorDialog* dialog = qobject_cast<QColorDialog*>(sender());
    if (dialog != nullptr && color.isValid() && provider() != nullptr)
    {
        const QString key = dialog->property("settingKey").toString();
        m_colors.value(key)->setText(color.name());
        onApply();
    }
}

void IDOSWellDisplayWidget::onInvalidate()
{
    if (m_renderId.isEmpty() && !isEnabled())
    {
        return;
    }
    m_renderId.clear();
    setEnabled(false);
    close();
}

void IDOSWellDisplayWidget::onRenderObjectRemoving(const QString& id)
{
    if (id == m_renderId)
    {
        onInvalidate();
    }
}

void IDOSWellDisplayWidget::onDataRemoving(const QString& id)
{
    if (!m_well.isNull() && id == m_well->objectId())
    {
        onInvalidate();
    }
}
