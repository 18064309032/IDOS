#ifndef IDOS_COLORING_TREE_GUI_PROVIDER_H
#define IDOS_COLORING_TREE_GUI_PROVIDER_H

#include <QMap>
#include <QPointer>

#include "idostreeguiprovider.h"

class QAction;
class IDOSColoringWidget;
class IDOSInterface;
class IDOSProject;
class IDOSRenderObject;
class IDOSRenderView;

class IDOSColoringTreeGuiProvider : public IDOSTreeGuiProvider
{
    Q_OBJECT

  public:
    explicit IDOSColoringTreeGuiProvider(IDOSInterface* interface);
    ~IDOSColoringTreeGuiProvider() override;
    QString name() const override;
    bool supports(const IDOSTreeItem* item) const override;
    bool isCheckable(const IDOSTreeItem* item) const override;
    void populateMenu(QMenu* menu, IDOSTreeView* view) override;

  private slots:
    void onSettingsTriggered();
    void onDisplayTriggered();
    void onActionDestroyed(QObject* object);
    void onProjectChanged();

  private:
    QAction* addAction(QMenu* menu, IDOSTreeView* tree, const QString& text,
                       const QString& gridId, const QString& propertyId);
    IDOSRenderObject* renderObject(QAction* action) const;
    QPointer<IDOSInterface> m_interface;
    QPointer<IDOSColoringWidget> m_widget;
    QMap<QAction*, QPointer<IDOSProject>> m_projects;
    QMap<QAction*, QPointer<IDOSRenderView>> m_views;
};

#endif
