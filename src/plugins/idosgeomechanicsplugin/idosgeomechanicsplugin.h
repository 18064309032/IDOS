#ifndef IDOSGEOMECHANICSPLUGIN_H
#define IDOSGEOMECHANICSPLUGIN_H

#include <QObject>

#include "plugin/idosplugin.h"

class IDOSInterface;
class SARibbonCategory;
class QTranslator;

class IDOSGeomechanicsPlugin : public QObject, public IDOSPlugin
{
    Q_OBJECT

  public:
    explicit IDOSGeomechanicsPlugin(IDOSInterface* interface);
    ~IDOSGeomechanicsPlugin() override;

    void initGui() override;
    void unload() override;

  private:
    void uninstallTranslator();

    IDOSInterface* m_interface;
    SARibbonCategory* m_category;
    QTranslator* m_translator;
    bool m_initialized;
};

#endif // IDOSGEOMECHANICSPLUGIN_H
