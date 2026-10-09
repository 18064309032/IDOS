#ifndef IDOS_SIMULATION_PLUGIN_H
#define IDOS_SIMULATION_PLUGIN_H

#include "plugin/idosplugin.h"

class IDOSInterface;
class SARibbonCategory;

class IDOSSimulationPlugin : public IDOSPlugin
{
  public:
    explicit IDOSSimulationPlugin(IDOSInterface* interface);

    void initGui() override;
    void unload() override;

  private:
    IDOSInterface* m_interface;
    SARibbonCategory* m_category;
    bool m_initialized;
};

#endif // IDOS_SIMULATION_PLUGIN_H
