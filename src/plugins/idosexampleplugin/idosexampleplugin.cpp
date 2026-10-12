#include <memory>
#include <stdexcept>

#include <QAction>
#include <QApplication>
#include <QJsonObject>
#include <QLocale>
#include <QMessageBox>
#include <QTranslator>

#include <SARibbonCategory.h>
#include <SARibbonPanel.h>

#include "command/idoscommandmanager.h"
#include "command/idoscommandregistry.h"
#include "idosdataobject.h"
#include "idosdataregistry.h"
#include "idosinterface.h"
#include "idospointsetcommands.h"
#include "idospointsetmetadata.h"
#include "idospointsetrendermetadata.h"
#include "idospointsettreeguiprovider.h"
#include "idospointsettreemetadata.h"
#include "idosproject.h"
#include "idosrenderregistry.h"
#include "idostreeregistry.h"

#include "idosexampleplugin.h"

IDOSExamplePlugin::IDOSExamplePlugin(IDOSInterface* interface)
    :
    QObject(nullptr)
    , IDOSPlugin(*pluginName(), *pluginDescription(), *pluginCategory(), *pluginVersion(), *pluginIcon(), UI)
    , m_interface(interface)
    , m_commandRegistry(nullptr)
    , m_translator(nullptr)
    , m_dataAdded(false)
    , m_treeAdded(false)
    , m_renderAdded(false)
    , m_createAdded(false)
    , m_deleteAdded(false)
{
}

IDOSExamplePlugin::~IDOSExamplePlugin()
{
    unload();
}

void IDOSExamplePlugin::initGui()
{
    if (m_interface == nullptr || m_dataAdded)
    {
        return;
    }
    m_dataRegistry = m_interface->dataRegistry();
    m_treeRegistry = m_interface->treeRegistry();
    m_renderRegistry = m_interface->renderRegistry();
    m_commandRegistry = m_interface->commandRegistry();
    const QString typeId = QString::fromLatin1("idos.example.pointset");
    if (m_dataRegistry == nullptr || m_treeRegistry == nullptr ||
        m_renderRegistry == nullptr || m_commandRegistry == nullptr ||
        m_dataRegistry->metadata(typeId) != nullptr || m_treeRegistry->metadata(typeId) != nullptr ||
        m_renderRegistry->metadata(typeId) != nullptr ||
        m_commandRegistry->find(QString::fromLatin1("pointset.create")) != nullptr ||
        m_commandRegistry->find(QString::fromLatin1("pointset.delete")) != nullptr)
    {
        throw std::runtime_error(tr("Point set registration is unavailable or already in use.").toStdString());
    }
    m_translator = new QTranslator(this);
    if (m_translator->load(qApp->applicationDirPath() + QString::fromLatin1("/i18n/idosexampleplugin_") +
                          QLocale().name() + QString::fromLatin1(".qm")))
    {
        qApp->installTranslator(m_translator);
    }
    std::unique_ptr<IDOSPointSetMetadata> data(new IDOSPointSetMetadata());
    m_dataAdded = m_dataRegistry->add(data.get());
    if (m_dataAdded)
    {
        data.release();
    }
    std::unique_ptr<IDOSPointSetTreeMetadata> tree(new IDOSPointSetTreeMetadata());
    m_treeAdded = m_treeRegistry->add(tree.get());
    if (m_treeAdded)
    {
        tree.release();
    }
    std::unique_ptr<IDOSPointSetRenderMetadata> render(new IDOSPointSetRenderMetadata());
    m_renderAdded = m_renderRegistry->add(render.get());
    if (m_renderAdded)
    {
        render.release();
    }
    m_treeGui = std::make_shared<IDOSPointSetTreeGuiProvider>();
    if (!m_treeRegistry->addGuiProvider(m_treeGui))
    {
        throw std::runtime_error(tr("Point set menu registration failed.").toStdString());
    }
    m_createAdded = m_commandRegistry->add(
        std::unique_ptr<IDOSCommandMetadata>(new IDOSCreatePointSetCommandMetadata(m_dataRegistry)));
    m_deleteAdded = m_commandRegistry->add(
        std::unique_ptr<IDOSCommandMetadata>(new IDOSDeletePointSetCommandMetadata()));
    if (!m_dataAdded || !m_treeAdded || !m_renderAdded || !m_createAdded || !m_deleteAdded)
    {
        throw std::runtime_error(tr("Point set registration failed.").toStdString());
    }
    m_category = m_interface->addRibbonCategory(QString::fromLatin1("idosExampleCategory"), tr("Example"));
    if (m_category != nullptr)
    {
        SARibbonPanel* panel = m_category->addPanel(tr("Point Sets"));
        m_createAction = new QAction(tr("Create Point Set"), m_category);
        m_createAction->setObjectName(QString::fromLatin1("createPointSetAction"));
        panel->addLargeAction(m_createAction);
        connect(m_createAction, &QAction::triggered, this, &IDOSExamplePlugin::onCreatePointSet);
    }
    connect(m_interface, &IDOSInterface::projectChanged, this, &IDOSExamplePlugin::onProjectChanged);
    onProjectChanged(m_interface->project());
}

void IDOSExamplePlugin::onProjectChanged(IDOSProject* project)
{
    if (project != nullptr && !m_projects.contains(QPointer<IDOSProject>(project)))
    {
        m_projects.append(QPointer<IDOSProject>(project));
    }
    if (m_createAction != nullptr)
    {
        m_createAction->setEnabled(project != nullptr);
    }
}

void IDOSExamplePlugin::onCreatePointSet()
{
    IDOSProject* project = m_interface != nullptr ? m_interface->project() : nullptr;
    if (project == nullptr)
    {
        return;
    }
    QJsonObject arguments;
    arguments.insert(QString::fromLatin1("name"), tr("Example points"));
    project->commandManager()->execute(QString::fromLatin1("pointset.create"), arguments);
    if (!project->commandManager()->error().isEmpty())
    {
        QMessageBox::warning(m_interface->mainWindow(), tr("Create Point Set"), project->commandManager()->error());
    }
}

bool IDOSExamplePlugin::canUnload(QString& reason) const
{
    reason.clear();
    for (const QPointer<IDOSProject>& project : m_projects)
    {
        if (project == nullptr)
        {
            continue;
        }
        if (project->commandManager()->contains(QString::fromLatin1("pointset.create")) ||
            project->commandManager()->contains(QString::fromLatin1("pointset.delete")))
        {
            reason = tr("Point set commands remain in the undo or redo history.");
            return false;
        }
        for (IDOSDataObject* data : project->objects())
        {
            if (data->typeId() == QString::fromLatin1("idos.example.pointset"))
            {
                reason = tr("Point set data still exists in a project.");
                return false;
            }
        }
    }
    const IDOSPointSetMetadata* metadata = m_dataRegistry != nullptr
        ? dynamic_cast<const IDOSPointSetMetadata*>(m_dataRegistry->metadata(QString::fromLatin1("idos.example.pointset")))
        : nullptr;
    if (metadata != nullptr && metadata->isInUse())
    {
        reason = tr("Point set instances are still alive outside project history.");
        return false;
    }
    return true;
}

void IDOSExamplePlugin::unload()
{
    if (m_interface != nullptr)
    {
        disconnect(m_interface, &IDOSInterface::projectChanged, this, &IDOSExamplePlugin::onProjectChanged);
        if (m_category != nullptr)
        {
            m_interface->removeRibbonCategory(m_category);
        }
        if (m_commandRegistry != nullptr && m_createAdded)
        {
            m_commandRegistry->remove(QString::fromLatin1("pointset.create"));
        }
        if (m_commandRegistry != nullptr && m_deleteAdded)
        {
            m_commandRegistry->remove(QString::fromLatin1("pointset.delete"));
        }
    }
    m_category = nullptr;
    m_createAction = nullptr;
    if (m_renderRegistry != nullptr)
    {
        if (m_renderAdded)
        {
            m_renderRegistry->remove(QString::fromLatin1("idos.example.pointset"));
        }
    }
    if (m_treeRegistry != nullptr)
    {
        m_treeRegistry->removeGuiProvider(m_treeGui.get());
        if (m_treeAdded)
        {
            m_treeRegistry->remove(QString::fromLatin1("idos.example.pointset"));
        }
    }
    m_treeGui.reset();
    if (m_dataRegistry != nullptr && m_dataAdded)
    {
        m_dataRegistry->remove(QString::fromLatin1("idos.example.pointset"));
    }
    if (m_translator != nullptr)
    {
        qApp->removeTranslator(m_translator);
        delete m_translator;
        m_translator = nullptr;
    }
    m_dataAdded = false;
    m_treeAdded = false;
    m_renderAdded = false;
    m_createAdded = false;
    m_deleteAdded = false;
}

IDOSPlugin* IDOSExamplePlugin::create(IDOSInterface* interface)
{
    return new IDOSExamplePlugin(interface);
}

const QString* IDOSExamplePlugin::pluginName()
{
    static const QString value = QObject::tr("Example");
    return &value;
}

const QString* IDOSExamplePlugin::pluginDescription()
{
    static const QString value = QObject::tr("Point set extension example");
    return &value;
}

const QString* IDOSExamplePlugin::pluginCategory()
{
    static const QString value = QObject::tr("Examples");
    return &value;
}

const QString* IDOSExamplePlugin::pluginVersion()
{
    static const QString value = QString::fromLatin1("1.0.0");
    return &value;
}

const QString* IDOSExamplePlugin::pluginIcon()
{
    static const QString value = QString::fromLatin1(":/images/gui-grid.svg");
    return &value;
}

int IDOSExamplePlugin::pluginType()
{
    return IDOSPlugin::UI;
}

