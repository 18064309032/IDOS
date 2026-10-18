#include <cmath>
#include <memory>
#include <stdexcept>

#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QDialog>
#include <QIcon>
#include <QLocale>
#include <QObject>
#include <QString>
#include <QTranslator>

#include <SARibbonCategory.h>
#include <SARibbonPanel.h>

#include "command/idoscommandregistry.h"
#include "data/fracture/idosnaturalfracturesetobject.h"
#include "data/grid/idosgridobject.h"
#include "idosdfngeneratecommand.h"
#include "idosdfngenerationwidget.h"
#include "idosfracturecheckwidget.h"
#include "idosinterface.h"
#include "idosproject.h"
#include "idosrenderobject.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"
#include "render/idosrenderserver.h"

#include "idosfracturemodelingplugin.h"

static const QString sName = QObject::tr("Fracture Modeling");
static const QString sDescription = QObject::tr("Fracture modeling plugin");
static const QString sCategory = QObject::tr("Fracture Modeling");
static const QString sPluginVersion = QStringLiteral("0.1.0");
static const IDOSPlugin::PluginType sPluginType = IDOSPlugin::UI;
static const QString sPluginIcon = QStringLiteral(":/images/fracture-azimuth.svg");

IDOSFractureModelingPlugin::IDOSFractureModelingPlugin(IDOSInterface* interface)
    :
    QObject(nullptr)
    , IDOSPlugin(sName, sDescription, sCategory, sPluginVersion, sPluginIcon, sPluginType)
    , m_interface(interface)
    , m_category(nullptr)
    , m_translator(nullptr)
    , m_initialized(false)
    , m_commandRegistry(nullptr)
    , m_commandAdded(false)
{
}

IDOSFractureModelingPlugin::~IDOSFractureModelingPlugin()
{
    unload();
}

void IDOSFractureModelingPlugin::uninstallTranslator()
{
    if (m_translator == nullptr)
    {
        return;
    }

    qApp->removeTranslator(m_translator);
    delete m_translator;
    m_translator = nullptr;
}

void IDOSFractureModelingPlugin::initGui()
{
    if (m_interface == nullptr || m_initialized)
    {
        return;
    }

    m_translator = new QTranslator(this);
    const QString translationPath = qApp->applicationDirPath()
        + QStringLiteral("/i18n/idosfracturemodelingplugin_") + QLocale().name()
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
    std::unique_ptr<IDOSCommandMetadata> commandMetadata0(new IDOSDfnGenerateCommandMetadata());
    m_commandAdded = m_commandRegistry->registerMetadata(commandMetadata0.get());
    if (m_commandAdded)
    {
        commandMetadata0.release();
    }
    if (!m_commandAdded)
    {
        throw std::runtime_error(tr("DFN command registration failed.").toStdString());
    }
    m_category = m_interface->insertCategoryPage(QStringLiteral("idosfractureModelingCategory"), tr("Fracture Modeling"), 3);
    if (m_category != nullptr)
    {
        SARibbonPanel* propertyPanel = m_category->addPanel(tr("Property Calculation"));
        QAction* azimuthAction = new QAction(tr("Azimuth"), m_category);
        azimuthAction->setIcon(QIcon(QStringLiteral(":/images/fracture-azimuth.svg")));
        azimuthAction->setEnabled(false);
        azimuthAction->setToolTip(tr("Planned action; not implemented yet."));
        propertyPanel->addLargeAction(azimuthAction);

        QAction* reconstructionAction = new QAction(tr("Fracture Reconstruction"), m_category);
        reconstructionAction->setIcon(QIcon(QStringLiteral(":/images/fracture-reconstruct.svg")));
        reconstructionAction->setEnabled(false);
        reconstructionAction->setToolTip(tr("Planned action; not implemented yet."));
        propertyPanel->addLargeAction(reconstructionAction);

        QAction* generateAction = new QAction(tr("DFN Generation"), m_category);
        generateAction->setObjectName(QStringLiteral("idosGenerateDfnAction"));
        generateAction->setIcon(QIcon(QStringLiteral(":/images/fracture-reconstruct.svg")));
        generateAction->setToolTip(tr("Generate a stochastic fracture set from a grid."));
        m_generateAction = generateAction;
        connect(generateAction, &QAction::triggered, this, &IDOSFractureModelingPlugin::onGenerateDfn);

        SARibbonPanel* fracturePanel = m_category->addPanel(tr("Fracture Operations"));
        fracturePanel->addLargeAction(generateAction);
        QAction* checkFracturesAction = new QAction(tr("Check Fractures"), m_category);
        checkFracturesAction->setIcon(QIcon(QStringLiteral(":/images/fracture-check.svg")));
        checkFracturesAction->setObjectName(QStringLiteral("idosCheckFracturesAction"));
        checkFracturesAction->setToolTip(tr("Check the selected fracture set, or all fracture sets when none is selected."));
        m_checkAction = checkFracturesAction;
        connect(checkFracturesAction, &QAction::triggered, this, &IDOSFractureModelingPlugin::onCheckFractures);
        fracturePanel->addLargeAction(checkFracturesAction);

        SARibbonPanel* analysisPanel = m_category->addPanel(tr("Fracture Analysis"));
        QAction* reserveAction = new QAction(tr("Fracture-Controlled Reserve"), m_category);
        reserveAction->setIcon(QIcon(QStringLiteral(":/images/fracture-reserve.svg")));
        reserveAction->setEnabled(false);
        reserveAction->setToolTip(tr("Planned action; not implemented yet."));
        analysisPanel->addLargeAction(reserveAction);
    }
    m_initialized = m_category != nullptr;
    connect(m_interface, &IDOSInterface::projectChanged, this, &IDOSFractureModelingPlugin::onProjectChanged);
    onProjectChanged(m_interface->project());
}

void IDOSFractureModelingPlugin::unload()
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
    m_project = nullptr;
    if (m_interface != nullptr && m_category != nullptr)
    {
        m_interface->removeRibbonCategory(m_category);
    }

    if (m_commandRegistry != nullptr && m_commandAdded)
    {
        m_commandRegistry->unregisterMetadata(QStringLiteral("fracture.dfn.generate"));
    }
    m_commandAdded = false;
    m_commandRegistry = nullptr;
    m_generateAction = nullptr;
    m_category = nullptr;
    m_checkAction = nullptr;
    m_initialized = false;
    uninstallTranslator();
}

void IDOSFractureModelingPlugin::closeWindows()
{
    const QList<QPointer<QWidget>> dialogs = m_windows;
    m_windows.clear();
    for (const QPointer<QWidget>& dialog : dialogs)
    {
        if (!dialog.isNull())
        {
            delete dialog.data();
        }
    }
}

void IDOSFractureModelingPlugin::onProjectChanged(IDOSProject* project)
{
    closeWindows();
    if (!m_project.isNull())
    {
        disconnect(m_project.data(), nullptr, this, nullptr);
    }
    m_project = project;
    if (project != nullptr)
    {
        connect(project, &IDOSProject::objectAdded, this, &IDOSFractureModelingPlugin::onUpdateCheckAction);
        connect(project, &IDOSProject::objectRemoved, this, &IDOSFractureModelingPlugin::onUpdateCheckAction);
        connect(project, &IDOSProject::objectsAdded, this, &IDOSFractureModelingPlugin::onUpdateCheckAction);
        connect(project, &IDOSProject::objectsRemoved, this, &IDOSFractureModelingPlugin::onUpdateCheckAction);
        connect(project, &QObject::destroyed, this, &IDOSFractureModelingPlugin::onUpdateCheckAction);
    }
    onUpdateCheckAction();
}

void IDOSFractureModelingPlugin::onUpdateCheckAction()
{
    bool fracturesAvailable = false;
    bool gridAvailable = false;
    if (!m_project.isNull())
    {
        for (IDOSDataObject* object : m_project->objects())
        {
            fracturesAvailable = fracturesAvailable || qobject_cast<IDOSNaturalFractureSetObject*>(object) != nullptr;
            const IDOSGridObject* grid = qobject_cast<IDOSGridObject*>(object);
            gridAvailable = gridAvailable || (grid != nullptr && grid->type() == IDOSGridObject::Type::CornerPoint);
        }
    }
    if (!m_checkAction.isNull())
    {
        m_checkAction->setEnabled(fracturesAvailable);
    }
    if (!m_generateAction.isNull())
    {
        m_generateAction->setEnabled(gridAvailable && m_commandAdded);
    }
}

void IDOSFractureModelingPlugin::showWindow(QWidget* widget)
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

void IDOSFractureModelingPlugin::onGenerateDfn()
{
    if (m_project.isNull() || m_interface == nullptr || m_interface->project() != m_project.data())
    {
        return;
    }
    showWindow(new IDOSDfnGenerationWidget(m_interface));
}

void IDOSFractureModelingPlugin::onCheckFractures()
{
    if (m_project.isNull() || m_interface == nullptr || m_interface->project() != m_project.data())
    {
        return;
    }
    IDOSFractureCheckWidget* dialog = new IDOSFractureCheckWidget(m_interface);
    connect(dialog, &IDOSFractureCheckWidget::issueActivated,
        this, &IDOSFractureModelingPlugin::onIssueActivated);
    showWindow(dialog);
}

void IDOSFractureModelingPlugin::onIssueActivated(const QString& objectId, const QVariantList& center)
{
    IDOSFractureCheckWidget* dialog = qobject_cast<IDOSFractureCheckWidget*>(sender());
    if (dialog == nullptr || m_project.isNull() || dialog->project() != m_project.data()
        || m_interface == nullptr || m_interface->project() != m_project.data())
    {
        return;
    }
    IDOSDataObject* data = m_project->object(objectId);
    if (data == nullptr)
    {
        return;
    }
    m_project->setCurrentObject(data);
    IDOSRenderServer* server = m_interface->renderServer();
    IDOSRenderView* view = dialog->targetView();
    if (server == nullptr || view == nullptr || !server->views().contains(view))
    {
        return;
    }
    const QList<QString> ids = server->objectIds(view, data->objectId());
    for (const QString& id : ids)
    {
        IDOSRenderObject* object = view->scene()->object(id);
        if (object != nullptr)
        {
            object->setVisible(true);
            view->scene()->notifyChanged(id);
        }
    }
    view->setSelection(ids);
    if (center.size() == 3)
    {
        const double position[3] = {center.at(0).toDouble(), center.at(1).toDouble(), center.at(2).toDouble()};
        if (std::isfinite(position[0]) && std::isfinite(position[1]) && std::isfinite(position[2]))
        {
            view->zoomToWorldPosition(position);
        }
    }
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
    return new IDOSFractureModelingPlugin(interface);
}

bool IDOSFractureModelingPlugin::canUnload(QString& reason) const
{
    reason.clear();
    if (IDOSDfnGenerateCommand::liveCount() > 0)
    {
        reason = tr("DFN commands are still alive, including undo or redo history. Clear that history or close the owning project before unloading.");
        return false;
    }
    return true;
}
