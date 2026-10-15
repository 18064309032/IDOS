#ifndef IDOSFRACTUREMODELINGPLUGIN_H
#define IDOSFRACTUREMODELINGPLUGIN_H

#include <QList>
#include <QObject>
#include <QPointer>
#include <QVariant>

#include "idosplugin.h"

class IDOSCommandRegistry;
class IDOSInterface;
class IDOSProject;
class QAction;
class QWidget;
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
    bool canUnload(QString& reason) const override;

  private:
    void uninstallTranslator();
    void closeWindows();
    void showWindow(QWidget* widget);

  private slots:
    void onProjectChanged(IDOSProject* project);
    void onUpdateCheckAction();
    void onCheckFractures();
    void onGenerateDfn();
    void onIssueActivated(const QString& objectId, const QVariantList& center);

  private:

    IDOSInterface* m_interface;
    SARibbonCategory* m_category;
    QTranslator* m_translator;
    bool m_initialized;
    IDOSCommandRegistry* m_commandRegistry;
    bool m_commandAdded;
    QPointer<IDOSProject> m_project;
    QPointer<QAction> m_checkAction;
    QPointer<QAction> m_generateAction;
    QList<QPointer<QWidget>> m_windows;
};

#endif // IDOSFRACTUREMODELINGPLUGIN_H
