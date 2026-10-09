#ifndef IDOSMULTISCALEFRACTUREPLUGIN_H
#define IDOSMULTISCALEFRACTUREPLUGIN_H

#include <QObject>

#include "plugin/idosplugin.h"

class IDOSInterface;
class SARibbonCategory;
class QTranslator;

class IDOSMultiscaleFracturePlugin : public QObject, public IDOSPlugin
{
    Q_OBJECT

  public:
    explicit IDOSMultiscaleFracturePlugin(IDOSInterface* interface);
    ~IDOSMultiscaleFracturePlugin() override;

    void initGui() override;
    void unload() override;

  private:
    void uninstallTranslator();

    IDOSInterface* m_interface;
    SARibbonCategory* m_category;
    QTranslator* m_translator;
    bool m_initialized;
};

#endif // IDOSMULTISCALEFRACTUREPLUGIN_H
