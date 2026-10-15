#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QColorDialog>
#include <QIcon>
#include <QInputDialog>
#include <QMenu>
#include <QSignalBlocker>

#include "data/grid/idosgrid.h"
#include "idosrenderinteractiontool.h"
#include "idosrenderobject.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"
#include "idostoolbar.h"
#include "providers/idosrenderprovider.h"

#include "idosrendertoolprovider.h"

IDOSRenderToolProvider::IDOSRenderToolProvider(IDOSRenderView* view)
    :
    IDOSToolProvider(view)
    , m_fitAllAction(new QAction(QIcon(QStringLiteral(":/images/render-fit.svg")),
                                 tr("Fit All"), this))
    , m_resetViewAction(new QAction(QIcon(QStringLiteral(":/images/render-view.svg")), tr("Reset View"), this))
    , m_legendAction(new QAction(QIcon(QStringLiteral(":/images/render-legend.svg")), tr("Legend"), this))
    , m_fitSelectionAction(new QAction(QIcon(QStringLiteral(":/images/render-fit-selection.svg")),
                                       tr("Zoom to Selection"), this))
    , m_rotateClockwiseAction(new QAction(QIcon(QStringLiteral(":/images/render-rotate-clockwise.svg")),
                                          tr("Rotate Clockwise"), this))
    , m_rotateCounterclockwiseAction(new QAction(QIcon(QStringLiteral(":/images/render-rotate-counterclockwise.svg")),
                                                 tr("Rotate Counterclockwise"), this))
    , m_projectionAction(new QAction(QIcon(QStringLiteral(":/images/render-view.svg")),
                                     tr("Orthographic Projection"), this))
    , m_cursorTrackingAction(new QAction(QIcon(QStringLiteral(":/images/render-select.svg")),
                                         tr("Cursor Tracking"), this))
    , m_gridXrayAction(new QAction(QIcon(QStringLiteral(":/images/render-transparency.svg")),
                                   tr("Grid X-ray"), this))
    , m_linkedViewAction(new QAction(QIcon(QStringLiteral(":/images/render-view.svg")),
                                     tr("Link View"), this))
    , m_pointZoomAction(new QAction(QIcon(QStringLiteral(":/images/render-zoom.svg")), tr("Point Zoom"), this))
    , m_pointProbeAction(new QAction(QIcon(QStringLiteral(":/images/render-select.svg")), tr("Point Probe"), this))
    , m_distanceMeasurementAction(new QAction(QIcon(QStringLiteral(":/images/render-measure.svg")),
                                               tr("Distance Measurement"), this))
    , m_selectionAction(new QAction(QIcon(QStringLiteral(":/images/render-select.svg")), tr("Select"), this))
    , m_clippingPlaneAction(new QAction(QIcon(QStringLiteral(":/images/render-clipping.svg")),
                                        tr("Clipping Plane"), this))
    , m_sliceByPointsAction(new QAction(QIcon(QStringLiteral(":/images/render-slice.svg")),
                                       tr("Slice by Points"), this))
    , m_boundsAction(new QAction(QIcon(QStringLiteral(":/images/render-bounds.svg")),
                                 tr("Show Bounding Box"), this))
    , m_backgroundColorAction(new QAction(QIcon(QStringLiteral(":/images/render-scene.svg")),
                                          tr("Background Color"), this))
    , m_screenshotAction(new QAction(QIcon(QStringLiteral(":/images/render-view.svg")), tr("Screenshot"), this))
    , m_scaleViewAction(new QAction(QIcon(QStringLiteral(":/images/render-zoom.svg")), tr("Scale View"), this))
    , m_zScaleAction(new QAction(QIcon(QStringLiteral(":/images/render-z-scale.svg")), tr("Z Scale"), this))
    , m_clearDisplayAction(new QAction(QIcon(QStringLiteral(":/images/render-hidden.svg")),
                                       tr("Hide All"), this))
    , m_standardViewsAction(new QAction(QIcon(QStringLiteral(":/images/render-standard-views.svg")),
                                       tr("Standard Views"), this))
    , m_axesAction(new QAction(QIcon(QStringLiteral(":/images/render-coordinate.svg")),
                               tr("Axes"), this))
    , m_standardViewsMenu(new QMenu(view != nullptr ? view->toolBar() : nullptr))
    , m_orientationActionGroup(new QActionGroup(this))
    , m_gridXrayOriginalOpacity()
    , m_gridXrayOriginalDisplayMode()
    , m_gridXrayEnabled(false)
    , m_interactionTools()
{
    m_fitAllAction->setObjectName(QStringLiteral("viewToolFitAllAction"));
    m_resetViewAction->setObjectName(QStringLiteral("viewToolResetAction"));
    m_legendAction->setObjectName(QStringLiteral("viewToolLegendAction"));
    m_legendAction->setCheckable(true);
    m_fitSelectionAction->setObjectName(QStringLiteral("viewToolFitSelectionAction"));
    m_rotateClockwiseAction->setObjectName(QStringLiteral("viewToolRotateClockwiseAction"));
    m_rotateCounterclockwiseAction->setObjectName(QStringLiteral("viewToolRotateCounterclockwiseAction"));
    m_projectionAction->setObjectName(QStringLiteral("viewToolProjectionAction"));
    m_cursorTrackingAction->setObjectName(QStringLiteral("viewToolCursorTrackingAction"));
    m_gridXrayAction->setObjectName(QStringLiteral("viewToolGridXrayAction"));
    m_linkedViewAction->setObjectName(QStringLiteral("viewToolLinkedViewAction"));
    m_pointZoomAction->setObjectName(QStringLiteral("viewToolPointZoomAction"));
    m_pointProbeAction->setObjectName(QStringLiteral("viewToolPointProbeAction"));
    m_distanceMeasurementAction->setObjectName(QStringLiteral("viewToolDistanceMeasurementAction"));
    m_selectionAction->setObjectName(QStringLiteral("viewToolSelectionAction"));
    m_clippingPlaneAction->setObjectName(QStringLiteral("viewToolClippingPlaneAction"));
    m_sliceByPointsAction->setObjectName(QStringLiteral("viewToolSliceByPointsAction"));
    m_boundsAction->setObjectName(QStringLiteral("viewToolBoundsAction"));
    m_backgroundColorAction->setObjectName(QStringLiteral("viewToolBackgroundColorAction"));
    m_screenshotAction->setObjectName(QStringLiteral("viewToolScreenshotAction"));
    m_scaleViewAction->setObjectName(QStringLiteral("viewToolScaleAction"));
    m_zScaleAction->setObjectName(QStringLiteral("viewToolZScaleAction"));
    m_clearDisplayAction->setObjectName(QStringLiteral("viewToolClearDisplayAction"));
    m_standardViewsAction->setObjectName(QStringLiteral("viewToolStandardViewsAction"));
    m_axesAction->setObjectName(QStringLiteral("viewToolAxesAction"));
    m_axesAction->setCheckable(true);
    m_projectionAction->setCheckable(true);
    m_cursorTrackingAction->setCheckable(true);
    m_gridXrayAction->setCheckable(true);
    m_linkedViewAction->setCheckable(true);
    m_pointZoomAction->setCheckable(true);
    m_pointProbeAction->setCheckable(true);
    m_distanceMeasurementAction->setCheckable(true);
    m_selectionAction->setCheckable(true);
    m_clippingPlaneAction->setCheckable(true);
    m_sliceByPointsAction->setCheckable(true);
    m_boundsAction->setCheckable(true);
    m_standardViewsAction->setMenu(m_standardViewsMenu);
    m_orientationActionGroup->setExclusive(true);

    QAction* topAction = m_standardViewsMenu->addAction(tr("Top"));
    topAction->setObjectName(QStringLiteral("viewToolTopAction"));
    topAction->setCheckable(true);
    topAction->setData(static_cast<int>(IDOSOrientation::Top));
    m_orientationActionGroup->addAction(topAction);

    QAction* bottomAction = m_standardViewsMenu->addAction(tr("Bottom"));
    bottomAction->setObjectName(QStringLiteral("viewToolBottomAction"));
    bottomAction->setCheckable(true);
    bottomAction->setData(static_cast<int>(IDOSOrientation::Bottom));
    m_orientationActionGroup->addAction(bottomAction);

    QAction* frontAction = m_standardViewsMenu->addAction(tr("Front"));
    frontAction->setObjectName(QStringLiteral("viewToolFrontAction"));
    frontAction->setCheckable(true);
    frontAction->setData(static_cast<int>(IDOSOrientation::Front));
    m_orientationActionGroup->addAction(frontAction);

    QAction* backAction = m_standardViewsMenu->addAction(tr("Back"));
    backAction->setObjectName(QStringLiteral("viewToolBackAction"));
    backAction->setCheckable(true);
    backAction->setData(static_cast<int>(IDOSOrientation::Back));
    m_orientationActionGroup->addAction(backAction);

    QAction* leftAction = m_standardViewsMenu->addAction(tr("Left"));
    leftAction->setObjectName(QStringLiteral("viewToolLeftAction"));
    leftAction->setCheckable(true);
    leftAction->setData(static_cast<int>(IDOSOrientation::Left));
    m_orientationActionGroup->addAction(leftAction);

    QAction* rightAction = m_standardViewsMenu->addAction(tr("Right"));
    rightAction->setObjectName(QStringLiteral("viewToolRightAction"));
    rightAction->setCheckable(true);
    rightAction->setData(static_cast<int>(IDOSOrientation::Right));
    m_orientationActionGroup->addAction(rightAction);

    QAction* isometricAction = m_standardViewsMenu->addAction(tr("Isometric"));
    isometricAction->setObjectName(QStringLiteral("viewToolIsometricAction"));
    isometricAction->setCheckable(true);
    isometricAction->setData(static_cast<int>(IDOSOrientation::Isometric));
    m_orientationActionGroup->addAction(isometricAction);

    connect(m_fitAllAction, &QAction::triggered,
            this, &IDOSRenderToolProvider::onFitAllTriggered);
    connect(m_resetViewAction, &QAction::triggered, this, &IDOSRenderToolProvider::onResetViewTriggered);
    connect(m_legendAction, &QAction::toggled, this, &IDOSRenderToolProvider::onLegendToggled);
    connect(m_fitSelectionAction, &QAction::triggered,
            this, &IDOSRenderToolProvider::onFitSelectionTriggered);
    connect(m_rotateClockwiseAction, &QAction::triggered,
            this, &IDOSRenderToolProvider::onRotateViewTriggered);
    connect(m_rotateCounterclockwiseAction, &QAction::triggered,
            this, &IDOSRenderToolProvider::onRotateViewTriggered);
    connect(m_projectionAction, &QAction::toggled,
            this, &IDOSRenderToolProvider::onProjectionToggled);
    connect(m_cursorTrackingAction, &QAction::toggled,
            this, &IDOSRenderToolProvider::onCursorTrackingToggled);
    connect(m_gridXrayAction, &QAction::toggled,
            this, &IDOSRenderToolProvider::onGridXrayToggled);
    connect(m_linkedViewAction, &QAction::toggled,
            this, &IDOSRenderToolProvider::onLinkedViewToggled);
    connect(m_pointZoomAction, &QAction::toggled,
            this, &IDOSRenderToolProvider::onInteractionToggled);
    connect(m_pointProbeAction, &QAction::toggled,
            this, &IDOSRenderToolProvider::onInteractionToggled);
    connect(m_distanceMeasurementAction, &QAction::toggled,
            this, &IDOSRenderToolProvider::onInteractionToggled);
    connect(m_selectionAction, &QAction::toggled,
            this, &IDOSRenderToolProvider::onInteractionToggled);
    connect(m_clippingPlaneAction, &QAction::toggled,
            this, &IDOSRenderToolProvider::onInteractionToggled);
    connect(m_sliceByPointsAction, &QAction::toggled,
            this, &IDOSRenderToolProvider::onInteractionToggled);
    connect(m_boundsAction, &QAction::toggled,
            this, &IDOSRenderToolProvider::onBoundsToggled);
    connect(m_backgroundColorAction, &QAction::triggered,
            this, &IDOSRenderToolProvider::onBackgroundColorTriggered);
    connect(m_screenshotAction, &QAction::triggered,
            this, &IDOSRenderToolProvider::onScreenshotTriggered);
    connect(m_scaleViewAction, &QAction::triggered,
            this, &IDOSRenderToolProvider::onScaleViewTriggered);
    connect(m_zScaleAction, &QAction::triggered,
            this, &IDOSRenderToolProvider::onZScaleTriggered);
    connect(m_clearDisplayAction, &QAction::triggered,
            this, &IDOSRenderToolProvider::onClearDisplayTriggered);
    connect(m_standardViewsMenu, &QMenu::triggered,
            this, &IDOSRenderToolProvider::onOrientationTriggered);
    connect(m_axesAction, &QAction::toggled,
            this, &IDOSRenderToolProvider::onAxesToggled);
    connect(view, &IDOSRenderView::viewStateChanged,
            this, &IDOSRenderToolProvider::updateState);
    connect(view, &IDOSRenderView::decorationsChanged, this, &IDOSRenderToolProvider::updateState);
    connect(view, &IDOSRenderView::viewModeChanged,
            this, &IDOSRenderToolProvider::updateState);
    connect(view, &IDOSRenderView::selectionChanged,
            this, &IDOSRenderToolProvider::updateState);
    connect(view, &IDOSRenderView::toolEnabled,
            this, &IDOSRenderToolProvider::updateState);
    connect(view, &IDOSRenderView::toolDisabled,
            this, &IDOSRenderToolProvider::updateState);
    m_interactionTools.append(new IDOSRenderInteractionTool(view, IDOSRenderInteractionTool::Mode::PointZoom));
    m_interactionTools.append(new IDOSRenderInteractionTool(view, IDOSRenderInteractionTool::Mode::PointProbe));
    m_interactionTools.append(new IDOSRenderInteractionTool(view, IDOSRenderInteractionTool::Mode::DistanceMeasurement));
    m_interactionTools.append(new IDOSRenderInteractionTool(view, IDOSRenderInteractionTool::Mode::Selection));
    m_interactionTools.append(new IDOSRenderInteractionTool(view, IDOSRenderInteractionTool::Mode::ClippingPlane));
    m_interactionTools.append(new IDOSRenderInteractionTool(view, IDOSRenderInteractionTool::Mode::SliceByPoints));
    for (IDOSTool* tool : m_interactionTools)
    {
        tool->setParent(this);
    }
    updateState();
}

IDOSRenderToolProvider::~IDOSRenderToolProvider()
{
    if (m_gridXrayEnabled)
    {
        onGridXrayToggled(false);
    }
    if (view() != nullptr && view()->property("cameraLinkActive").toBool())
    {
        onLinkedViewToggled(false);
    }
    m_standardViewsAction->setMenu(nullptr);
    delete m_standardViewsMenu.data();
}

QList<QAction*> IDOSRenderToolProvider::actions() const
{
    return QList<QAction*>{m_axesAction, m_standardViewsAction, m_resetViewAction, m_legendAction, m_rotateCounterclockwiseAction,
                           m_rotateClockwiseAction, m_projectionAction,
                           m_fitSelectionAction, m_fitAllAction, m_scaleViewAction, m_zScaleAction,
                           m_cursorTrackingAction, m_linkedViewAction, m_pointZoomAction, m_pointProbeAction,
                           m_distanceMeasurementAction, m_selectionAction, m_clippingPlaneAction,
                           m_sliceByPointsAction, m_boundsAction, m_gridXrayAction, m_backgroundColorAction,
                           m_clearDisplayAction, m_screenshotAction};
}

QList<IDOSTool*> IDOSRenderToolProvider::tools() const
{
    return m_interactionTools;
}

void IDOSRenderToolProvider::updateState()
{
    IDOSRenderView* renderView = view();
    const bool hasView = renderView != nullptr;
    m_fitAllAction->setEnabled(hasView);
    m_resetViewAction->setEnabled(hasView);
    m_legendAction->setEnabled(hasView && renderView->legendAvailable());
    const QSignalBlocker legendBlocker(m_legendAction);
    m_legendAction->setChecked(hasView && renderView->legendVisible());
    m_fitSelectionAction->setEnabled(hasView && !renderView->selection().isEmpty());
    m_rotateClockwiseAction->setEnabled(hasView);
    m_rotateCounterclockwiseAction->setEnabled(hasView);
    m_standardViewsAction->setEnabled(hasView && !renderView->isTwoDimensional());
    m_axesAction->setEnabled(hasView);
    m_projectionAction->setEnabled(hasView && !renderView->isTwoDimensional());
    m_cursorTrackingAction->setEnabled(hasView);
    m_gridXrayAction->setEnabled(hasView);
    m_linkedViewAction->setEnabled(hasView && !renderView->isTwoDimensional());
    m_pointZoomAction->setEnabled(hasView);
    m_pointProbeAction->setEnabled(hasView);
    m_distanceMeasurementAction->setEnabled(hasView);
    m_selectionAction->setEnabled(hasView);
    const bool hasThreeDimensionalView = hasView && !renderView->isTwoDimensional();
    m_clippingPlaneAction->setEnabled(hasThreeDimensionalView);
    m_sliceByPointsAction->setEnabled(hasThreeDimensionalView);
    m_boundsAction->setEnabled(false);
    m_backgroundColorAction->setEnabled(hasView);
    m_screenshotAction->setEnabled(hasView);
    m_scaleViewAction->setEnabled(hasView);
    m_zScaleAction->setEnabled(hasView && !renderView->isTwoDimensional());
    m_clearDisplayAction->setEnabled(hasView && !renderView->scene()->objects().isEmpty());

    if (!hasView)
    {
        return;
    }

    bool hasVisibleObjects = false;
    for (IDOSRenderObject* object : renderView->scene()->objects())
    {
        hasVisibleObjects = hasVisibleObjects || object->visible();
    }
    m_boundsAction->setEnabled(hasVisibleObjects);
    m_clearDisplayAction->setText(hasVisibleObjects ? tr("Hide All") : tr("Show All"));
    m_clearDisplayAction->setToolTip(m_clearDisplayAction->text());
    m_zScaleAction->setToolTip(tr("Z Scale: %1x").arg(renderView->zAxisScale(), 0, 'f', 2));

    const QSignalBlocker axesBlocker(m_axesAction);
    const QSignalBlocker projectionBlocker(m_projectionAction);
    const QSignalBlocker cursorTrackingBlocker(m_cursorTrackingAction);
    const QSignalBlocker gridXrayBlocker(m_gridXrayAction);
    const QSignalBlocker linkedViewBlocker(m_linkedViewAction);
    const QSignalBlocker clippingPlaneBlocker(m_clippingPlaneAction);
    const QSignalBlocker sliceByPointsBlocker(m_sliceByPointsAction);
    const QSignalBlocker boundsBlocker(m_boundsAction);
    m_axesAction->setChecked(renderView->axesVisible());
    m_projectionAction->setChecked(renderView->parallelProjection());
    m_cursorTrackingAction->setChecked(renderView->cursorTrackingEnabled());
    m_gridXrayAction->setChecked(m_gridXrayEnabled);
    m_linkedViewAction->setChecked(renderView->isCameraLinked() ||
                                   renderView->property("cameraLinkActive").toBool());
    m_boundsAction->setChecked(renderView->boundsVisible());
    const QList<QAction*> interactionActions = {m_pointZoomAction, m_pointProbeAction,
                                                m_distanceMeasurementAction, m_selectionAction,
                                                m_clippingPlaneAction, m_sliceByPointsAction};
    for (int index = 0; index < m_interactionTools.size(); ++index)
    {
        const QSignalBlocker interactionBlocker(interactionActions.at(index));
        interactionActions.at(index)->setChecked(m_interactionTools.at(index)->isActive());
    }
    for (QAction* action : m_orientationActionGroup->actions())
    {
        action->setChecked(action->data().toInt() == static_cast<int>(renderView->orientation()));
    }
}

void IDOSRenderToolProvider::onBoundsToggled(bool enabled)
{
    IDOSRenderView* renderView = view();
    if (renderView != nullptr)
    {
        renderView->setBoundsVisible(enabled);
    }
}

void IDOSRenderToolProvider::onFitSelectionTriggered()
{
    IDOSRenderView* renderView = view();
    if (renderView != nullptr)
    {
        renderView->fitSelection();
    }
}

void IDOSRenderToolProvider::onRotateViewTriggered()
{
    IDOSRenderView* renderView = view();
    QAction* action = qobject_cast<QAction*>(sender());
    if (renderView != nullptr && action != nullptr)
    {
        renderView->rotateView(action == m_rotateClockwiseAction);
    }
}

void IDOSRenderToolProvider::onProjectionToggled(bool enabled)
{
    IDOSRenderView* renderView = view();
    if (renderView != nullptr && !renderView->isTwoDimensional())
    {
        renderView->setParallelProjection(enabled);
    }
}

void IDOSRenderToolProvider::onCursorTrackingToggled(bool enabled)
{
    IDOSRenderView* renderView = view();
    if (renderView != nullptr)
    {
        renderView->setCursorTrackingEnabled(enabled);
    }
}

void IDOSRenderToolProvider::onGridXrayToggled(bool enabled)
{
    IDOSRenderView* renderView = view();
    if (renderView == nullptr)
    {
        return;
    }

    const QList<IDOSRenderObject*> objects = renderView->scene()->objects();
    if (enabled)
    {
        bool changed = false;
        for (IDOSRenderObject* object : objects)
        {
            IDOSRenderProvider* gridProvider = object->provider();
            if (gridProvider == nullptr || qobject_cast<IDOSGrid*>(gridProvider->dataSource()) == nullptr)
            {
                continue;
            }
            m_gridXrayOriginalOpacity.insert(object->id(), object->opacity());
            m_gridXrayOriginalDisplayMode.insert(object->id(), gridProvider->displayMode());
            object->setOpacity(0.42);
            gridProvider->setDisplayMode(IDOSDisplayMode::SurfaceWithEdges);
            renderView->scene()->notifyChanged(object->id());
            changed = true;
        }
        m_gridXrayEnabled = changed;
    }
    else
    {
        for (IDOSRenderObject* object : objects)
        {
            if (!m_gridXrayOriginalOpacity.contains(object->id()))
            {
                continue;
            }
            IDOSRenderProvider* gridProvider = object->provider();
            object->setOpacity(m_gridXrayOriginalOpacity.take(object->id()));
            if (gridProvider != nullptr)
            {
                gridProvider->setDisplayMode(m_gridXrayOriginalDisplayMode.take(object->id()));
            }
            renderView->scene()->notifyChanged(object->id());
        }
        m_gridXrayEnabled = false;
    }
    updateState();
}

void IDOSRenderToolProvider::onLinkedViewToggled(bool enabled)
{
    IDOSRenderView* renderView = view();
    if (renderView == nullptr)
    {
        return;
    }
    if (enabled)
    {
        renderView->unlinkCamera();
        const QList<IDOSRenderView*> views = renderView->window()->findChildren<IDOSRenderView*>();
        for (IDOSRenderView* otherView : views)
        {
            if (otherView != renderView)
            {
                otherView->unlinkCamera();
            }
        }
        bool linked = false;
        for (IDOSRenderView* otherView : views)
        {
            if (otherView != renderView && otherView->linkCameraTo(renderView))
            {
                linked = true;
            }
        }
        renderView->setProperty("cameraLinkActive", linked);
        if (!linked)
        {
            const QSignalBlocker blocker(m_linkedViewAction);
            m_linkedViewAction->setChecked(false);
        }
    }
    else
    {
        if (renderView->property("cameraLinkActive").toBool())
        {
            const QList<IDOSRenderView*> views = renderView->window()->findChildren<IDOSRenderView*>();
            for (IDOSRenderView* otherView : views)
            {
                if (otherView != renderView)
                {
                    otherView->unlinkCamera();
                }
            }
            renderView->setProperty("cameraLinkActive", false);
        }
        renderView->unlinkCamera();
    }
    updateState();
}

void IDOSRenderToolProvider::onInteractionToggled(bool enabled)
{
    IDOSRenderView* renderView = view();
    QAction* action = qobject_cast<QAction*>(sender());
    if (renderView == nullptr || action == nullptr)
    {
        return;
    }
    int index = -1;
    if (action == m_pointZoomAction)
    {
        index = 0;
    }
    else if (action == m_pointProbeAction)
    {
        index = 1;
    }
    else if (action == m_distanceMeasurementAction)
    {
        index = 2;
    }
    else if (action == m_selectionAction)
    {
        index = 3;
    }
    else if (action == m_clippingPlaneAction)
    {
        index = 4;
    }
    else if (action == m_sliceByPointsAction)
    {
        index = 5;
    }
    if (index < 0 || index >= m_interactionTools.size())
    {
        return;
    }
    if (enabled)
    {
        renderView->enableTool(m_interactionTools.at(index));
    }
    else
    {
        renderView->disableTool(m_interactionTools.at(index));
    }
    updateState();
}

void IDOSRenderToolProvider::onBackgroundColorTriggered()
{
    IDOSRenderView* renderView = view();
    if (renderView == nullptr)
    {
        return;
    }
    const QColor color = QColorDialog::getColor(renderView->background(), renderView, tr("Background Color"));
    if (color.isValid())
    {
        renderView->setBackground(color);
    }
}

void IDOSRenderToolProvider::onScreenshotTriggered()
{
    IDOSRenderView* renderView = view();
    if (renderView == nullptr)
    {
        return;
    }
    const QImage image = renderView->captureImage();
    if (!image.isNull())
    {
        QApplication::clipboard()->setImage(image);
        renderView->flashScreenshot();
    }
}

void IDOSRenderToolProvider::onScaleViewTriggered()
{
    IDOSRenderView* renderView = view();
    if (renderView == nullptr)
    {
        return;
    }
    bool accepted = false;
    const double factor = QInputDialog::getDouble(renderView, tr("Scale View"), tr("Scale factor:"),
                                                   2.0, 0.01, 100.0, 2, &accepted);
    if (accepted)
    {
        renderView->scaleView(factor);
    }
}

void IDOSRenderToolProvider::onZScaleTriggered()
{
    IDOSRenderView* renderView = view();
    if (renderView == nullptr || renderView->isTwoDimensional())
    {
        return;
    }
    bool accepted = false;
    const double scale = QInputDialog::getDouble(renderView, tr("Z Scale"), tr("Z scale factor:"),
                                                renderView->zAxisScale(), 0.01, 100.0, 2, &accepted);
    if (accepted)
    {
        renderView->setZAxisScale(scale);
        updateState();
    }
}

void IDOSRenderToolProvider::onClearDisplayTriggered()
{
    IDOSRenderView* renderView = view();
    if (renderView != nullptr)
    {
        bool hasVisibleObjects = false;
        for (IDOSRenderObject* object : renderView->scene()->objects())
        {
            hasVisibleObjects = hasVisibleObjects || object->visible();
        }
        renderView->setDisplayVisible(!hasVisibleObjects);
    }
}

void IDOSRenderToolProvider::onFitAllTriggered()
{
    IDOSRenderView* renderView = view();
    if (renderView != nullptr)
    {
        renderView->resetCamera();
    }
}

void IDOSRenderToolProvider::onResetViewTriggered()
{
    IDOSRenderView* renderView = view();
    if (renderView != nullptr)
    {
        renderView->setOrientation(renderView->isTwoDimensional() ? renderView->orientation() : IDOSOrientation::Isometric);
    }
}

void IDOSRenderToolProvider::onLegendToggled(bool visible)
{
    if (view() != nullptr)
    {
        view()->setLegendVisible(visible);
    }
}

void IDOSRenderToolProvider::onOrientationTriggered(QAction* action)
{
    IDOSRenderView* renderView = view();
    if (renderView == nullptr || action == nullptr || renderView->isTwoDimensional())
    {
        return;
    }
    renderView->setOrientation(static_cast<IDOSOrientation>(action->data().toInt()));
}

void IDOSRenderToolProvider::onAxesToggled(bool visible)
{
    IDOSRenderView* renderView = view();
    if (renderView != nullptr)
    {
        renderView->setAxesVisible(visible);
    }
}
