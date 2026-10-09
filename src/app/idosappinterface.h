#ifndef IDOS_APP_INTERFACE_H
#define IDOS_APP_INTERFACE_H

#include <QList>
#include <QPointer>

#include "idos_app.h"
#include "plugin/idosinterface.h"

class IDOSMainWindow;
class SARibbonCategory;

class APP_EXPORT IDOSAppInterface : public IDOSInterface
{
    Q_OBJECT

  public:
    explicit IDOSAppInterface(IDOSMainWindow* mainWindow);

    QMainWindow* mainWindow() const override;
    SARibbonCategory* ribbonCategory(const QString& objectName) const override;
    SARibbonCategory* addRibbonCategory(const QString& objectName,
                                        const QString& title) override;
    // Removes and destroys only categories created through addRibbonCategory().
    void removeRibbonCategory(SARibbonCategory* category) override;

  private:
    IDOSMainWindow* m_mainWindow;
    QList<QPointer<SARibbonCategory>> m_ownedRibbonCategories;
};

#endif // IDOS_APP_INTERFACE_H
