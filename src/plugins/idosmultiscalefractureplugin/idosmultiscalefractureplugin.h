#ifndef IDOSMULTISCALEFRACTUREPLUGIN_H
#define IDOSMULTISCALEFRACTUREPLUGIN_H

#include <QList>
#include <QObject>
#include <QPointer>

#include "idosplugin.h"

class IDOSCommandRegistry;
class IDOSInterface;
class IDOSProject;
class QAction;
class QWidget;
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
    bool canUnload(QString& reason) const override;

  private:
    void uninstallTranslator();
    void closeWindows();
    void showWindow(QWidget* widget);

  private slots:
    void onProjectChanged(IDOSProject* project);
    void onUpdateActions();
    void onFieldDifference();
    void onCurveEvaluation();

  private:

    IDOSInterface* m_interface;
    SARibbonCategory* m_category;
    QTranslator* m_translator;
    bool m_initialized;
    IDOSCommandRegistry* m_commandRegistry;
    bool m_differenceAdded;
    bool m_curveAdded;
    QPointer<IDOSProject> m_project;
    QPointer<QAction> m_differenceAction;
    QPointer<QAction> m_curveAction;
    QList<QPointer<QWidget>> m_windows;
};

#endif // IDOSMULTISCALEFRACTUREPLUGIN_H
