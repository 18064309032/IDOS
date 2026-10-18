#include <QAction>
#include <QIcon>
#include <QSignalBlocker>

#include "idosrenderview.h"
#include "tools/idosrenderinteractiontool.h"

#include "idospointzoomtoolprovider.h"

IDOSPointZoomToolProvider::IDOSPointZoomToolProvider(IDOSRenderView* view)
    :
    IDOSToolProvider(view)
    , m_action(new QAction(QIcon(QStringLiteral(":/images/render-zoom.svg")), tr("Point Zoom"), this))
    , m_tool(new IDOSRenderInteractionTool(view, IDOSRenderInteractionTool::Mode::PointZoom))
{
    m_tool->setParent(this);
    m_action->setObjectName(QStringLiteral("viewToolPointZoomAction"));
    m_action->setCheckable(true);
    connect(m_action, &QAction::toggled, this, &IDOSPointZoomToolProvider::onToggled);
    if (view != nullptr)
    {
        connect(view, &IDOSRenderView::toolEnabled, this, &IDOSPointZoomToolProvider::updateState);
        connect(view, &IDOSRenderView::toolDisabled, this, &IDOSPointZoomToolProvider::updateState);
        connect(view, &IDOSRenderView::viewModeChanged, this, &IDOSPointZoomToolProvider::updateState);
    }
    updateState();
}

QList<QAction*> IDOSPointZoomToolProvider::actions() const
{
    return QList<QAction*>{m_action};
}

QList<IDOSTool*> IDOSPointZoomToolProvider::tools() const
{
    return QList<IDOSTool*>{m_tool};
}

void IDOSPointZoomToolProvider::updateState()
{
    const QSignalBlocker blocker(m_action);
    m_action->setEnabled(view() != nullptr);
    m_action->setChecked(m_tool->isActive());
}

void IDOSPointZoomToolProvider::onToggled(bool enabled)
{
    if (view() == nullptr)
    {
        return;
    }
    if (enabled)
    {
        view()->enableTool(m_tool);
    }
    else
    {
        view()->disableTool(m_tool);
    }
    updateState();
}

QString IDOSPointZoomToolProviderMetadata::id() const
{
    return QStringLiteral("idos.view.PointZoom");
}

QString IDOSPointZoomToolProviderMetadata::name() const
{
    return IDOSPointZoomToolProvider::tr("Point Zoom");
}

QString IDOSPointZoomToolProviderMetadata::description() const
{
    return name();
}

QString IDOSPointZoomToolProviderMetadata::iconPath() const
{
    return QStringLiteral(":/images/render-zoom.svg");
}

QString IDOSPointZoomToolProviderMetadata::groupId() const
{
    return QStringLiteral("interaction");
}

int IDOSPointZoomToolProviderMetadata::order() const
{
    return 20;
}

bool IDOSPointZoomToolProviderMetadata::supports(const IDOSRenderView* view) const
{
    return view != nullptr;
}

IDOSToolProvider* IDOSPointZoomToolProviderMetadata::create(IDOSRenderView* view) const
{
    return view != nullptr ? new IDOSPointZoomToolProvider(view) : nullptr;
}
