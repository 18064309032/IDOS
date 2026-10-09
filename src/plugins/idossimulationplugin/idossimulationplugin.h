#ifndef IDOS_SIMULATION_PLUGIN_H
#define IDOS_SIMULATION_PLUGIN_H

#include <QObject>

#include "plugin/idosplugin.h"

class IDOSInterface;
class SARibbonCategory;
class QTranslator;

class IDOSSimulationPlugin : public QObject, public IDOSPlugin
{
    Q_OBJECT

  public:
    explicit IDOSSimulationPlugin(IDOSInterface* interface);
    ~IDOSSimulationPlugin() override;

    void initGui() override;
    void unload() override;

  private:
    void uninstallTranslator();

    IDOSInterface* m_interface;
    SARibbonCategory* m_category;
    QTranslator* m_translator;
    bool m_initialized;
};

#endif // IDOS_SIMULATION_PLUGIN_H
