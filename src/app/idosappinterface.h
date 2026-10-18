#ifndef IDOS_APP_INTERFACE_H
#define IDOS_APP_INTERFACE_H

#include <QList>
#include <QPointer>

#include "idos_app.h"
#include "idosinterface.h"

class IDOSMainWindow;
class SARibbonCategory;

class APP_EXPORT IDOSAppInterface : public IDOSInterface
{
    Q_OBJECT

  public:
    explicit IDOSAppInterface(IDOSMainWindow* mainWindow);

    QMainWindow* mainWindow() const override;
    IDOSProject* project() const override;
    IDOSDataRegistry* dataRegistry() const override;
    IDOSTreeItemProviderRegistry* treeRegistry() const override;
    IDOSRenderProviderRegistry* renderRegistry() const override;
    IDOSRenderServer* renderServer() const override;
    IDOSToolProviderRegistry* toolRegistry() const override;
    IDOSCommandRegistry* commandRegistry() const override;
    SARibbonCategory* addRibbonCategory(const QString& objectName,
                                        const QString& title) override;
    SARibbonCategory* insertCategoryPage(const QString& objectName,
                                         const QString& title,
                                         int index) override;

    void removeRibbonCategory(SARibbonCategory* category) override;
    SARibbonPanel* homeRibbonPanel(const QString& groupId) const override;

  private:
    void reorderRibbonCategories();

    IDOSMainWindow* m_mainWindow;
    QList<QPointer<SARibbonCategory>> m_ownedRibbonCategories;
};

#endif // IDOS_APP_INTERFACE_H
