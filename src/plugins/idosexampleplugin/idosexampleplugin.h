#ifndef IDOS_EXAMPLE_PLUGIN_H
#define IDOS_EXAMPLE_PLUGIN_H

#include <memory>

#include <QList>
#include <QObject>
#include <QPointer>

#include "idosplugin.h"

class IDOSCommandRegistry;
class IDOSDataRegistry;
class IDOSInterface;
class IDOSProject;
class IDOSRenderRegistry;
class IDOSTreeGuiProvider;
class IDOSTreeRegistry;
class QAction;
class QTranslator;
class SARibbonCategory;

class IDOS_PLUGIN_EXPORT IDOSExamplePlugin : public QObject, public IDOSPlugin
{
    Q_OBJECT

  public:
    explicit IDOSExamplePlugin(IDOSInterface* interface);
    ~IDOSExamplePlugin() override;
    void initGui() override;
    void unload() override;
    bool canUnload(QString& reason) const override;
    static IDOSPlugin* create(IDOSInterface* interface);
    static const QString* pluginName();
    static const QString* pluginDescription();
    static const QString* pluginCategory();
    static const QString* pluginVersion();
    static const QString* pluginIcon();
    static int pluginType();

  private slots:
    void onCreatePointSet();
    void onProjectChanged(IDOSProject* project);

  private:
    QPointer<IDOSInterface> m_interface;
    QPointer<IDOSDataRegistry> m_dataRegistry;
    QPointer<IDOSTreeRegistry> m_treeRegistry;
    QPointer<IDOSRenderRegistry> m_renderRegistry;
    IDOSCommandRegistry* m_commandRegistry;
    QPointer<SARibbonCategory> m_category;
    QPointer<QAction> m_createAction;
    QTranslator* m_translator;
    std::shared_ptr<IDOSTreeGuiProvider> m_treeGui;
    QList<QPointer<IDOSProject>> m_projects;
    bool m_dataAdded;
    bool m_treeAdded;
    bool m_renderAdded;
    bool m_createAdded;
    bool m_deleteAdded;
};
#endif

