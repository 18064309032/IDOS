#ifndef IDOS_PLUGIN_INTERFACE_H
#define IDOS_PLUGIN_INTERFACE_H

#include <QList>
#include <QMainWindow>
#include <QObject>
#include <QString>

#include "idos_gui.h"

class SARibbonCategory;
class IDOSCommandRegistry;
class IDOSDataRegistry;
class IDOSDataMetadata;
class IDOSProject;
class IDOSRenderRegistry;
class IDOSTreeRegistry;

class GUI_EXPORT IDOSInterface : public QObject
{
    Q_OBJECT

  public:
    explicit IDOSInterface(QObject* parent = nullptr);
    ~IDOSInterface() override;

    virtual QMainWindow* mainWindow() const = 0;
    virtual IDOSProject* project() const = 0;
    virtual IDOSDataRegistry* dataRegistry() const = 0;
    virtual QList<const IDOSDataMetadata*> treeDataMetadata() const;
    virtual IDOSTreeRegistry* treeRegistry() const = 0;
    virtual IDOSRenderRegistry* renderRegistry() const = 0;
    virtual IDOSCommandRegistry* commandRegistry() const = 0;
    virtual SARibbonCategory* ribbonCategory(const QString& objectName) const = 0;
    virtual SARibbonCategory* addRibbonCategory(const QString& objectName,
                                                const QString& title) = 0;
    virtual SARibbonCategory* insertCategoryPage(const QString& objectName,
                                                 const QString& title,
                                                 int index) = 0;
    virtual void removeRibbonCategory(SARibbonCategory* category) = 0;

  signals:
    void projectChanged(IDOSProject* project);
};
#endif // IDOS_PLUGIN_INTERFACE_H
