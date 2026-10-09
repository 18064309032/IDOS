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

#include "idosgeomechanicsplugin.h"

static const QString sName = QObject::tr("Geomechanics");
static const QString sDescription = QObject::tr("Geomechanics plugin");
static const QString sCategory = QObject::tr("Geomechanics");
static const QString sPluginVersion = QStringLiteral("0.1.0");
static const IDOSPlugin::PluginType sPluginType = IDOSPlugin::UI;
static const QString sPluginIcon = QStringLiteral(":/images/geomechanics-stress.svg");

IDOSGeomechanicsPlugin::IDOSGeomechanicsPlugin(IDOSInterface* interface)
    :
    QObject(nullptr)
    , IDOSPlugin(sName, sDescription, sCategory, sPluginVersion, sPluginType)
    , m_interface(interface)
    , m_category(nullptr)
    , m_translator(nullptr)
    , m_initialized(false)
{
}

IDOSGeomechanicsPlugin::~IDOSGeomechanicsPlugin()
{
    uninstallTranslator();
}

void IDOSGeomechanicsPlugin::uninstallTranslator()
{
    if (m_translator == nullptr)
    {
        return;
    }

    qApp->removeTranslator(m_translator);
    delete m_translator;
    m_translator = nullptr;
}

void IDOSGeomechanicsPlugin::initGui()
{
    if (m_interface == nullptr || m_initialized)
    {
        return;
    }

    m_translator = new QTranslator(this);
    const QString translationPath = qApp->applicationDirPath()
        + QStringLiteral("/i18n/idosgeomechanicsplugin_") + QLocale().name()
        + QStringLiteral(".qm");
    if (!m_translator->load(translationPath) || !qApp->installTranslator(m_translator))
    {
        delete m_translator;
        m_translator = nullptr;
    }

    m_category = m_interface->addRibbonCategory(
        QStringLiteral("idosgeomechanicsCategory"),
        tr("Geomechanics"));
    if (m_category != nullptr)
    {
        SARibbonPanel* calculationPanel = m_category->addPanel(tr("Stress Calculation"));
        QAction* inSituStressAction = new QAction(tr("In-Situ Stress"), m_category);
        inSituStressAction->setIcon(QIcon(QStringLiteral(":/images/geomechanics-stress.svg")));
        inSituStressAction->setEnabled(false);
        inSituStressAction->setToolTip(tr("Planned action; not implemented yet."));
        calculationPanel->addLargeAction(inSituStressAction);

        SARibbonPanel* stressPanel = m_category->addPanel(tr("Stress Analysis"));
        QAction* stressFieldsAction = new QAction(tr("Stress Fields"), m_category);
        stressFieldsAction->setIcon(QIcon(QStringLiteral(":/images/geomechanics-fields.svg")));
        stressFieldsAction->setEnabled(false);
        stressFieldsAction->setToolTip(tr("Planned action; not implemented yet."));
        stressPanel->addLargeAction(stressFieldsAction);

        SARibbonPanel* fieldPanel = m_category->addPanel(tr("Field Analysis"));
        QAction* threeFieldAction = new QAction(tr("Three-Field Comparison"), m_category);
        threeFieldAction->setIcon(QIcon(QStringLiteral(":/images/geomechanics-three-field.svg")));
        threeFieldAction->setEnabled(false);
        threeFieldAction->setToolTip(tr("Planned action; not implemented yet."));
        fieldPanel->addLargeAction(threeFieldAction);

        SARibbonPanel* diagnosticsPanel = m_category->addPanel(tr("Diagnostics"));
        QAction* stageEnergyAction = new QAction(tr("Stage Energy"), m_category);
        stageEnergyAction->setIcon(QIcon(QStringLiteral(":/images/geomechanics-energy.svg")));
        stageEnergyAction->setEnabled(false);
        stageEnergyAction->setToolTip(tr("Planned action; not implemented yet."));
        diagnosticsPanel->addLargeAction(stageEnergyAction);
    }
    m_initialized = m_category != nullptr;
}

void IDOSGeomechanicsPlugin::unload()
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
    return new IDOSGeomechanicsPlugin(interface);
}
