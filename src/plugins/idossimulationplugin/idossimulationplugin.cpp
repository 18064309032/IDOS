#include <QCoreApplication>

#include "log/idoslogger.h"
#include "plugin/idosinterface.h"

#include "idossimulationplugin.h"

IDOSSimulationPlugin::IDOSSimulationPlugin(IDOSInterface* interface)
    :
    IDOSPlugin(QCoreApplication::translate("IDOSSimulationPlugin", "Reservoir Simulation"),
               QCoreApplication::translate("IDOSSimulationPlugin", "Reservoir simulation plugin"),
               QCoreApplication::translate("IDOSSimulationPlugin", "Reservoir Engineering"),
               QStringLiteral("0.1.0"))
    , m_interface(interface)
    , m_category(nullptr)
    , m_initialized(false)
{
}

void IDOSSimulationPlugin::initGui()
{
    IDOS_DEBUG(QCoreApplication::translate("IDOSSimulationPlugin", "Simulation plugin initGui entered: interface=0x%1, initialized=%2")
                   .arg(QString::number(reinterpret_cast<quintptr>(m_interface), 16))
                   .arg(m_initialized));
    if (m_interface == nullptr || m_initialized)
    {
        return;
    }

    m_category = m_interface->addRibbonCategory(
        QStringLiteral("idosSimulationCategory"),
        QCoreApplication::translate("IDOSSimulationPlugin", "Reservoir Simulation"));
    m_initialized = m_category != nullptr;
    IDOS_DEBUG(QCoreApplication::translate("IDOSSimulationPlugin", "Simulation plugin category initialization finished: category=0x%1, initialized=%2")
                   .arg(QString::number(reinterpret_cast<quintptr>(m_category), 16))
                   .arg(m_initialized));
}

void IDOSSimulationPlugin::unload()
{
    IDOS_DEBUG(QCoreApplication::translate("IDOSSimulationPlugin", "Simulation plugin unload entered: category=0x%1, initialized=%2")
                   .arg(QString::number(reinterpret_cast<quintptr>(m_category), 16))
                   .arg(m_initialized));
    if (m_interface != nullptr && m_category != nullptr)
    {
        m_interface->removeRibbonCategory(m_category);
    }

    m_category = nullptr;
    m_initialized = false;
    IDOS_DEBUG(QCoreApplication::translate("IDOSSimulationPlugin", "Simulation plugin unload finished."));
}

extern "C" IDOS_PLUGIN_EXPORT const QString* name()
{
    static const QString pluginName =
        QCoreApplication::translate("IDOSSimulationPlugin", "Reservoir Simulation");
    return &pluginName;
}

extern "C" IDOS_PLUGIN_EXPORT const QString* description()
{
    static const QString pluginDescription =
        QCoreApplication::translate("IDOSSimulationPlugin", "Reservoir simulation plugin");
    return &pluginDescription;
}

extern "C" IDOS_PLUGIN_EXPORT const QString* category()
{
    static const QString pluginCategory =
        QCoreApplication::translate("IDOSSimulationPlugin", "Reservoir Engineering");
    return &pluginCategory;
}

extern "C" IDOS_PLUGIN_EXPORT const QString* version()
{
    static const QString pluginVersion = QStringLiteral("0.1.0");
    return &pluginVersion;
}

extern "C" IDOS_PLUGIN_EXPORT IDOSPlugin* classFactory(IDOSInterface* interface)
{
    return new IDOSSimulationPlugin(interface);
}
