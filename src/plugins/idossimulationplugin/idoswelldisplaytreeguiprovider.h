#ifndef IDOS_WELL_DISPLAY_TREE_GUI_PROVIDER_H
#define IDOS_WELL_DISPLAY_TREE_GUI_PROVIDER_H

#include <QMap>
#include <QPointer>

#include "idostreeguiprovider.h"

class QAction;
class IDOSInterface;
class IDOSProject;
class IDOSRenderObject;
class IDOSRenderView;
class IDOSWellDisplayWidget;

class IDOSWellDisplayTreeGuiProvider : public IDOSTreeGuiProvider
{
    Q_OBJECT
  public:
    explicit IDOSWellDisplayTreeGuiProvider(IDOSInterface* interface);
    ~IDOSWellDisplayTreeGuiProvider() override;
    QString name() const override;
    bool supports(const IDOSTreeItem* item) const override;
    bool isCheckable(const IDOSTreeItem* item) const override;
    void populateMenu(QMenu* menu, IDOSTreeView* view) override;
  private slots:
    void onSettingsTriggered();
    void onActionDestroyed(QObject* object);
    void onProjectChanged();
  private:
    IDOSRenderObject* renderObject(QAction* action) const;
    QPointer<IDOSInterface> m_interface;
    QPointer<IDOSWellDisplayWidget> m_widget;
    QMap<QAction*, QPointer<IDOSProject>> m_projects;
    QMap<QAction*, QPointer<IDOSRenderView>> m_views;
};

#endif
