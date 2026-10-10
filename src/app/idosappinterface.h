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
    SARibbonCategory* insertCategoryPage(const QString& objectName,
                                         const QString& title,
                                         int index) override;
    // Removes and destroys only categories created through this interface.
    void removeRibbonCategory(SARibbonCategory* category) override;

  private:
    void reorderRibbonCategories();

    IDOSMainWindow* m_mainWindow;
    QList<QPointer<SARibbonCategory>> m_ownedRibbonCategories;
};

#endif // IDOS_APP_INTERFACE_H
