#include <QAction>
#include <QIcon>
#include <QSignalBlocker>

#include "idosrenderview.h"
#include "tools/idosrenderinteractiontool.h"

#include "idosclippingplanetoolprovider.h"

IDOSClippingPlaneToolProvider::IDOSClippingPlaneToolProvider(IDOSRenderView* view)
    :
    IDOSToolProvider(view)
    , m_action(new QAction(QIcon(QStringLiteral(":/images/render-clipping.svg")), tr("Clipping Plane"), this))
    , m_tool(new IDOSRenderInteractionTool(view, IDOSRenderInteractionTool::Mode::ClippingPlane))
{
    m_tool->setParent(this);
    m_action->setObjectName(QStringLiteral("viewToolClippingPlaneAction"));
    m_action->setCheckable(true);
    connect(m_action, &QAction::toggled, this, &IDOSClippingPlaneToolProvider::onToggled);
    if (view != nullptr)
    {
        connect(view, &IDOSRenderView::toolEnabled, this, &IDOSClippingPlaneToolProvider::updateState);
        connect(view, &IDOSRenderView::toolDisabled, this, &IDOSClippingPlaneToolProvider::updateState);
        connect(view, &IDOSRenderView::viewModeChanged, this, &IDOSClippingPlaneToolProvider::updateState);
    }
    updateState();
}

QList<QAction*> IDOSClippingPlaneToolProvider::actions() const
{
    return QList<QAction*>{m_action};
}

QList<IDOSTool*> IDOSClippingPlaneToolProvider::tools() const
{
    return QList<IDOSTool*>{m_tool};
}

void IDOSClippingPlaneToolProvider::updateState()
{
    const QSignalBlocker blocker(m_action);
    m_action->setEnabled(view() != nullptr && !view()->isTwoDimensional());
    m_action->setChecked(m_tool->isActive());
}

void IDOSClippingPlaneToolProvider::onToggled(bool enabled)
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

QString IDOSClippingPlaneToolProviderMetadata::id() const
{
    return QStringLiteral("idos.view.ClippingPlane");
}

QString IDOSClippingPlaneToolProviderMetadata::name() const
{
    return IDOSClippingPlaneToolProvider::tr("Clipping Plane");
}

QString IDOSClippingPlaneToolProviderMetadata::description() const
{
    return name();
}

QString IDOSClippingPlaneToolProviderMetadata::iconPath() const
{
    return QStringLiteral(":/images/render-clipping.svg");
}

QString IDOSClippingPlaneToolProviderMetadata::groupId() const
{
    return QStringLiteral("interaction");
}

int IDOSClippingPlaneToolProviderMetadata::order() const
{
    return 24;
}

bool IDOSClippingPlaneToolProviderMetadata::supports(const IDOSRenderView* view) const
{
    return view != nullptr;
}

IDOSToolProvider* IDOSClippingPlaneToolProviderMetadata::create(IDOSRenderView* view) const
{
    return view != nullptr ? new IDOSClippingPlaneToolProvider(view) : nullptr;
}
