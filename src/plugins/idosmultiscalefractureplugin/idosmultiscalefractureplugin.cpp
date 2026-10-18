#include <memory>
#include <stdexcept>

#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QIcon>
#include <QLocale>
#include <QObject>
#include <QString>
#include <QTranslator>

#include <SARibbonCategory.h>
#include <SARibbonPanel.h>

#include "command/idoscommandregistry.h"
#include "data/grid/idosgridobject.h"
#include "idoscurveevaluationcommand.h"
#include "idoscurvefitevaluationwidget.h"
#include "idosfielddifferencecommand.h"
#include "idosfielddifferencewidget.h"
#include "idosinterface.h"
#include "idosproject.h"

#include "idosmultiscalefractureplugin.h"

static const QString sName = QObject::tr("Multiscale Fractures");
static const QString sDescription = QObject::tr("Multiscale fracture plugin");
static const QString sCategory = QObject::tr("Multiscale Fractures");
static const QString sPluginVersion = QStringLiteral("0.1.0");
static const IDOSPlugin::PluginType sPluginType = IDOSPlugin::UI;
static const QString sPluginIcon = QStringLiteral(":/images/multiscale-dfn.svg");

IDOSMultiscaleFracturePlugin::IDOSMultiscaleFracturePlugin(IDOSInterface* interface)
    :
    QObject(nullptr)
    , IDOSPlugin(sName, sDescription, sCategory, sPluginVersion, sPluginIcon, sPluginType)
    , m_interface(interface)
    , m_category(nullptr)
    , m_translator(nullptr)
    , m_initialized(false)
    , m_commandRegistry(nullptr)
    , m_differenceAdded(false)
    , m_curveAdded(false)
{
}

IDOSMultiscaleFracturePlugin::~IDOSMultiscaleFracturePlugin()
{
    unload();
}

void IDOSMultiscaleFracturePlugin::uninstallTranslator()
{
    if (m_translator == nullptr)
    {
        return;
    }

    qApp->removeTranslator(m_translator);
    delete m_translator;
    m_translator = nullptr;
}

void IDOSMultiscaleFracturePlugin::initGui()
{
    if (m_interface == nullptr || m_initialized)
    {
        return;
    }

    m_translator = new QTranslator(this);
    const QString translationPath = qApp->applicationDirPath()
        + QStringLiteral("/i18n/idosmultiscalefractureplugin_") + QLocale().name()
        + QStringLiteral(".qm");
    if (!m_translator->load(translationPath) || !qApp->installTranslator(m_translator))
    {
        delete m_translator;
        m_translator = nullptr;
    }

    m_commandRegistry = m_interface->commandRegistry();
    if (m_commandRegistry == nullptr)
    {
        throw std::runtime_error(tr("The command registry is unavailable.").toStdString());
    }
    std::unique_ptr<IDOSCommandMetadata> commandMetadata0(new IDOSFieldDifferenceCommandMetadata());
    m_differenceAdded = m_commandRegistry->registerMetadata(commandMetadata0.get());
    if (m_differenceAdded)
    {
        commandMetadata0.release();
    }
    std::unique_ptr<IDOSCommandMetadata> commandMetadata1(new IDOSCurveEvaluationCommandMetadata());
    m_curveAdded = m_commandRegistry->registerMetadata(commandMetadata1.get());
    if (m_curveAdded)
    {
        commandMetadata1.release();
    }
    if (!m_differenceAdded || !m_curveAdded)
    {
        throw std::runtime_error(tr("Analysis command registration failed.").toStdString());
    }
    m_category = m_interface->insertCategoryPage(
        QStringLiteral("idosmultiscaleFractureCategory"),
        tr("Multiscale Fractures"),
        4);
    if (m_category != nullptr)
    {
        SARibbonPanel* analysisPanel = m_category->addPanel(tr("Analysis"));
        QAction* difference = new QAction(tr("Three-Field Difference"), m_category);
        difference->setObjectName(QStringLiteral("idosFieldDifferenceAction"));
        difference->setIcon(QIcon(QStringLiteral(":/images/multiscale-equivalent.svg")));
        m_differenceAction = difference;
        analysisPanel->addLargeAction(difference);
        connect(difference, &QAction::triggered, this, &IDOSMultiscaleFracturePlugin::onFieldDifference);
        QAction* curves = new QAction(tr("Curve Fit Evaluation"), m_category);
        curves->setObjectName(QStringLiteral("idosCurveEvaluationAction"));
        curves->setIcon(QIcon(QStringLiteral(":/images/multiscale-connection-check.svg")));
        m_curveAction = curves;
        analysisPanel->addLargeAction(curves);
        connect(curves, &QAction::triggered, this, &IDOSMultiscaleFracturePlugin::onCurveEvaluation);

        SARibbonPanel* equivalentPanel = m_category->addPanel(tr("Equivalent Properties"));
        QAction* equivalentPropertiesAction = new QAction(tr("Equivalent Porosity/Permeability"), m_category);
        equivalentPropertiesAction->setIcon(QIcon(QStringLiteral(":/images/multiscale-equivalent.svg")));
        equivalentPropertiesAction->setEnabled(false);
        equivalentPropertiesAction->setToolTip(tr("Planned action; not implemented yet."));
        equivalentPanel->addLargeAction(equivalentPropertiesAction);

        QAction* equivalentMechanicsAction = new QAction(tr("Equivalent Mechanics"), m_category);
        equivalentMechanicsAction->setIcon(QIcon(QStringLiteral(":/images/multiscale-equivalent-mechanics.svg")));
        equivalentMechanicsAction->setEnabled(false);
        equivalentMechanicsAction->setToolTip(tr("Planned action; not implemented yet."));
        equivalentPanel->addLargeAction(equivalentMechanicsAction);

        SARibbonPanel* gridPanel = m_category->addPanel(tr("Grid Processing"));
        QAction* gridCoarseningAction = new QAction(tr("Grid Coarsening"), m_category);
        gridCoarseningAction->setIcon(QIcon(QStringLiteral(":/images/multiscale-coarsen.svg")));
        gridCoarseningAction->setEnabled(false);
        gridCoarseningAction->setToolTip(tr("Planned action; not implemented yet."));
        gridPanel->addLargeAction(gridCoarseningAction);

        QAction* localGridAction = new QAction(tr("Local Grid Extraction"), m_category);
        localGridAction->setIcon(QIcon(QStringLiteral(":/images/gui-grid-geometry.svg")));
        localGridAction->setEnabled(false);
        localGridAction->setToolTip(tr("Planned action; not implemented yet."));
        gridPanel->addLargeAction(localGridAction);

        SARibbonPanel* modelPanel = m_category->addPanel(tr("Model Construction"));
        QAction* edfmAction = new QAction(tr("EDFM Modeling"), m_category);
        edfmAction->setIcon(QIcon(QStringLiteral(":/images/multiscale-edfm.svg")));
        edfmAction->setEnabled(false);
        edfmAction->setToolTip(tr("Planned action; not implemented yet."));
        modelPanel->addLargeAction(edfmAction);

        QAction* connectionAction = new QAction(tr("Connection Check"), m_category);
        connectionAction->setIcon(QIcon(QStringLiteral(":/images/multiscale-connection-check.svg")));
        connectionAction->setEnabled(false);
        connectionAction->setToolTip(tr("Planned action; not implemented yet."));
        modelPanel->addLargeAction(connectionAction);
    }
    m_initialized = m_category != nullptr;
    connect(m_interface, &IDOSInterface::projectChanged, this, &IDOSMultiscaleFracturePlugin::onProjectChanged);
    onProjectChanged(m_interface->project());
}

void IDOSMultiscaleFracturePlugin::unload()
{
    closeWindows();
    if (m_interface != nullptr)
    {
        disconnect(m_interface, nullptr, this, nullptr);
    }
    if (!m_project.isNull())
    {
        disconnect(m_project.data(), nullptr, this, nullptr);
    }
    if (m_commandRegistry != nullptr && m_differenceAdded)
    {
        m_commandRegistry->unregisterMetadata(QStringLiteral("analysis.field.difference"));
    }
    if (m_commandRegistry != nullptr && m_curveAdded)
    {
        m_commandRegistry->unregisterMetadata(QStringLiteral("analysis.curve.evaluate"));
    }
    m_differenceAdded = false;
    m_curveAdded = false;
    m_commandRegistry = nullptr;
    m_differenceAction = nullptr;
    m_curveAction = nullptr;
    m_project = nullptr;
    if (m_interface != nullptr && m_category != nullptr)
    {
        m_interface->removeRibbonCategory(m_category);
    }

    m_category = nullptr;
    m_initialized = false;
    uninstallTranslator();
}

extern "C" IDOS_PLUGIN_EXPORT const QString* name()
{
    return &sName;
}

extern "C" IDOS_PLUGIN_EXPORT const QString* description()
{
    return &sDescription;
}

extern "C" IDOS_PLUGIN_EXPORT const QString* category()
{
    return &sCategory;
}

extern "C" IDOS_PLUGIN_EXPORT const QString* version()
{
    return &sPluginVersion;
}

extern "C" IDOS_PLUGIN_EXPORT int type()
{
    return sPluginType;
}

extern "C" IDOS_PLUGIN_EXPORT const QString* icon()
{
    return &sPluginIcon;
}

extern "C" IDOS_PLUGIN_EXPORT void unload(IDOSPlugin* plugin)
{
    if (plugin == nullptr)
    {
        return;
    }

    delete plugin;
}

extern "C" IDOS_PLUGIN_EXPORT IDOSPlugin* classFactory(IDOSInterface* interface)
{
    return new IDOSMultiscaleFracturePlugin(interface);
}

bool IDOSMultiscaleFracturePlugin::canUnload(QString& reason) const
{
    reason.clear();
    if (IDOSFieldDifferenceCommand::liveCount() > 0 || IDOSCurveEvaluationCommand::liveCount() > 0)
    {
        reason = tr("Analysis commands are still alive, including undo or redo history. Close their project or clear the history before unloading.");
        return false;
    }
    for (const QPointer<QWidget>& widget : m_windows)
    {
        IDOSCurveFitEvaluationWidget* curves = qobject_cast<IDOSCurveFitEvaluationWidget*>(widget.data());
        if (curves != nullptr && curves->isBusy())
        {
            reason = tr("Close the analysis file dialog before unloading the plugin.");
            return false;
        }
    }
    return true;
}

void IDOSMultiscaleFracturePlugin::closeWindows()
{
    const QList<QPointer<QWidget>> windows = m_windows;
    m_windows.clear();
    for (const QPointer<QWidget>& widget : windows)
    {
        if (!widget.isNull())
        {
            delete widget.data();
        }
    }
}

void IDOSMultiscaleFracturePlugin::showWindow(QWidget* widget)
{
    for (int index = m_windows.size() - 1; index >= 0; --index)
    {
        if (m_windows.at(index).isNull())
        {
            m_windows.removeAt(index);
        }
    }
    m_windows.append(widget);
    widget->show();
}

void IDOSMultiscaleFracturePlugin::onProjectChanged(IDOSProject* project)
{
    closeWindows();
    if (!m_project.isNull())
    {
        disconnect(m_project.data(), nullptr, this, nullptr);
    }
    m_project = project;
    if (project != nullptr)
    {
        connect(project, &IDOSProject::objectAdded, this, &IDOSMultiscaleFracturePlugin::onUpdateActions);
        connect(project, &IDOSProject::objectRemoved, this, &IDOSMultiscaleFracturePlugin::onUpdateActions);
        connect(project, &IDOSProject::objectsAdded, this, &IDOSMultiscaleFracturePlugin::onUpdateActions);
        connect(project, &IDOSProject::objectsRemoved, this, &IDOSMultiscaleFracturePlugin::onUpdateActions);
        connect(project, &QObject::destroyed, this, &IDOSMultiscaleFracturePlugin::onUpdateActions);
    }
    onUpdateActions();
}

void IDOSMultiscaleFracturePlugin::onUpdateActions()
{
    bool grid = false;
    if (!m_project.isNull())
    {
        for (IDOSDataObject* object : m_project->objects())
        {
            grid = grid || qobject_cast<IDOSGridObject*>(object) != nullptr;
        }
    }
    if (!m_differenceAction.isNull())
    {
        m_differenceAction->setEnabled(grid && m_differenceAdded);
    }
    if (!m_curveAction.isNull())
    {
        m_curveAction->setEnabled(!m_project.isNull() && m_curveAdded);
    }
}

void IDOSMultiscaleFracturePlugin::onFieldDifference()
{
    if (!m_project.isNull() && m_interface != nullptr && m_interface->project() == m_project.data())
    {
        showWindow(new IDOSFieldDifferenceWidget(m_interface));
    }
}

void IDOSMultiscaleFracturePlugin::onCurveEvaluation()
{
    if (!m_project.isNull() && m_interface != nullptr && m_interface->project() == m_project.data())
    {
        showWindow(new IDOSCurveFitEvaluationWidget(m_interface));
    }
}
