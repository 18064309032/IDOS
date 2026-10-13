#ifndef IDOS_TOOL_BAR_H
#define IDOS_TOOL_BAR_H

#include <QList>
#include <QMap>
#include <QPoint>
#include <QPointer>
#include <QString>
#include <QToolBar>

#include "idos_render.h"

class QAction;
class IDOSRenderView;
class IDOSToolMetadata;
class IDOSToolProvider;
class IDOSToolRegistry;
class QLabel;
class QWidgetAction;

class RENDER_EXPORT IDOSToolBar : public QToolBar
{
    Q_OBJECT
public:
    explicit IDOSToolBar(QWidget* parent = nullptr);
    ~IDOSToolBar() override;
    bool setRegistry(IDOSToolRegistry* registry);
    IDOSToolRegistry* registry() const;
    IDOSRenderView* view() const;
    bool setView(IDOSRenderView* view);
    bool isBusy() const;
    bool isUserPositioned() const;
    void setVisible(bool visible) override;
signals:
    void layoutChanged();
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
private slots:
    void onMetadataAdded(const QString& id);
    void onMetadataAboutToBeRemoved(const QString& id);
    void onRegistryDestroyed();
    void onViewStateChanged();
    void onViewModeChanged();
    void onActionChanged();
private:
    bool canRemoveTool(const QString& id) const;
    bool canChange() const;
    void rebuildTools();
    void addTool(const QString& id);
    void removeTool(const QString& id);
    void removeActionWidget(QAction* action);
    void clearTools();
    void arrangeActions();
    void updateToolStates();
    void updateVisibility();
    static bool metadataBefore(const IDOSToolMetadata* first, const IDOSToolMetadata* second);
    QPointer<IDOSRenderView> m_view;
    QPointer<IDOSToolRegistry> m_registry;
    QMap<QString, IDOSToolProvider*> m_providers;
    QList<QAction*> m_separators;
    QWidgetAction* m_dragHandleAction;
    QLabel* m_dragHandle;
    QPoint m_dragStartGlobalPosition;
    QPoint m_dragStartPosition;
    bool m_updating;
    bool m_internalVisibility;
    bool m_userHidden;
    bool m_dragging;
    bool m_userPositioned;
};

#endif // IDOS_TOOL_BAR_H
