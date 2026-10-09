#ifndef IDOS_PLUGIN_INTERFACE_H
#define IDOS_PLUGIN_INTERFACE_H

#include <QMainWindow>
#include <QObject>
#include <QString>

#include "../idos_app.h"

class SARibbonCategory;

class APP_EXPORT IDOSInterface : public QObject
{
    Q_OBJECT

  public:
    explicit IDOSInterface(QObject* parent = nullptr);
    ~IDOSInterface() override;

    virtual QMainWindow* mainWindow() const = 0;
    virtual SARibbonCategory* ribbonCategory(const QString& objectName) const = 0;
    virtual SARibbonCategory* addRibbonCategory(const QString& objectName,
                                                const QString& title) = 0;
    virtual void removeRibbonCategory(SARibbonCategory* category) = 0;
};
#endif // IDOS_PLUGIN_INTERFACE_H
