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
{
}

IDOSFractureModelingPlugin::~IDOSFractureModelingPlugin()
{
    uninstallTranslator();
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

        SARibbonPanel* fracturePanel = m_category->addPanel(tr("Fracture Operations"));
        QAction* importFracturesAction = new QAction(tr("Import Fractures"), m_category);
        importFracturesAction->setIcon(QIcon(QStringLiteral(":/images/fracture-import.svg")));
        importFracturesAction->setEnabled(false);
        importFracturesAction->setToolTip(tr("Planned action; not implemented yet."));
        fracturePanel->addLargeAction(importFracturesAction);

        QAction* checkFracturesAction = new QAction(tr("Check Fractures"), m_category);
        checkFracturesAction->setIcon(QIcon(QStringLiteral(":/images/fracture-check.svg")));
        checkFracturesAction->setEnabled(false);
        checkFracturesAction->setToolTip(tr("Planned action; not implemented yet."));
        fracturePanel->addLargeAction(checkFracturesAction);

        SARibbonPanel* analysisPanel = m_category->addPanel(tr("Fracture Analysis"));
        QAction* reserveAction = new QAction(tr("Fracture-Controlled Reserve"), m_category);
        reserveAction->setIcon(QIcon(QStringLiteral(":/images/fracture-reserve.svg")));
        reserveAction->setEnabled(false);
        reserveAction->setToolTip(tr("Planned action; not implemented yet."));
        analysisPanel->addLargeAction(reserveAction);
    }
    m_initialized = m_category != nullptr;
}

void IDOSFractureModelingPlugin::unload()
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
    return new IDOSFractureModelingPlugin(interface);
}
