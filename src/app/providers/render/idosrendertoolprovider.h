#ifndef IDOS_RENDER_TOOL_PROVIDER_H
#define IDOS_RENDER_TOOL_PROVIDER_H

#include <QList>
#include <QMap>
#include <QPointer>

#include "idos_app.h"
#include "idosrendertypes.h"
#include "providers/idostoolprovider.h"

class QAction;
class QActionGroup;
class QMenu;

class APP_EXPORT IDOSRenderToolProvider : public IDOSToolProvider
{
    Q_OBJECT
public:
    explicit IDOSRenderToolProvider(IDOSRenderView* view);
    ~IDOSRenderToolProvider() override;
    QList<QAction*> actions() const override;
    QList<IDOSTool*> tools() const override;
    void updateState() override;

private slots:
    void onFitAllTriggered();
    void onResetViewTriggered();
    void onLegendToggled(bool visible);
    void onFitSelectionTriggered();
    void onRotateViewTriggered();
    void onProjectionToggled(bool enabled);
    void onCursorTrackingToggled(bool enabled);
    void onGridXrayToggled(bool enabled);
    void onLinkedViewToggled(bool enabled);
    void onInteractionToggled(bool enabled);
    void onBackgroundColorTriggered();
    void onScreenshotTriggered();
    void onScaleViewTriggered();
    void onClearDisplayTriggered();
    void onOrientationTriggered(QAction* action);
    void onAxesToggled(bool visible);

private:
    QAction* m_fitAllAction;
    QAction* m_resetViewAction;
    QAction* m_legendAction;
    QAction* m_fitSelectionAction;
    QAction* m_rotateClockwiseAction;
    QAction* m_rotateCounterclockwiseAction;
    QAction* m_projectionAction;
    QAction* m_cursorTrackingAction;
    QAction* m_gridXrayAction;
    QAction* m_linkedViewAction;
    QAction* m_pointZoomAction;
    QAction* m_pointProbeAction;
    QAction* m_distanceMeasurementAction;
    QAction* m_selectionAction;
    QAction* m_backgroundColorAction;
    QAction* m_screenshotAction;
    QAction* m_scaleViewAction;
    QAction* m_clearDisplayAction;
    QAction* m_standardViewsAction;
    QAction* m_axesAction;
    QPointer<QMenu> m_standardViewsMenu;
    QActionGroup* m_orientationActionGroup;
    QMap<QString, double> m_gridXrayOriginalOpacity;
    QMap<QString, IDOSDisplayMode> m_gridXrayOriginalDisplayMode;
    bool m_gridXrayEnabled;
    QList<IDOSTool*> m_interactionTools;
};

#endif // IDOS_RENDER_TOOL_PROVIDER_H
