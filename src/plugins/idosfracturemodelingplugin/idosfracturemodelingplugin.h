#ifndef IDOSFRACTUREMODELINGPLUGIN_H
#define IDOSFRACTUREMODELINGPLUGIN_H

#include <QObject>

#include "plugin/idosplugin.h"

class IDOSInterface;
class SARibbonCategory;
class QTranslator;

class IDOSFractureModelingPlugin : public QObject, public IDOSPlugin
{
    Q_OBJECT

  public:
    explicit IDOSFractureModelingPlugin(IDOSInterface* interface);
    ~IDOSFractureModelingPlugin() override;

    void initGui() override;
    void unload() override;

  private:
    void uninstallTranslator();

    IDOSInterface* m_interface;
    SARibbonCategory* m_category;
    QTranslator* m_translator;
    bool m_initialized;
};

#endif // IDOSFRACTUREMODELINGPLUGIN_H
