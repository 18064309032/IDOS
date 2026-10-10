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

#include "plugin/idosinterface.h"

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
{
}

IDOSMultiscaleFracturePlugin::~IDOSMultiscaleFracturePlugin()
{
    uninstallTranslator();
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

    m_category = m_interface->insertCategoryPage(
        QStringLiteral("idosmultiscaleFractureCategory"),
        tr("Multiscale Fractures"),
        4);
    if (m_category != nullptr)
    {
        SARibbonPanel* networkPanel = m_category->addPanel(tr("Network Generation"));
        QAction* dfnGenerationAction = new QAction(tr("DFN Generation"), m_category);
        dfnGenerationAction->setIcon(QIcon(QStringLiteral(":/images/multiscale-dfn.svg")));
        dfnGenerationAction->setEnabled(false);
        dfnGenerationAction->setToolTip(tr("Planned action; not implemented yet."));
        networkPanel->addLargeAction(dfnGenerationAction);

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
}

void IDOSMultiscaleFracturePlugin::unload()
{
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
