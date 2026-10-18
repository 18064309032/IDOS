#include <QAction>
#include <QIcon>
#include <QSignalBlocker>

#include "idosrenderview.h"
#include "tools/idosrenderinteractiontool.h"

#include "idospointprobetoolprovider.h"

IDOSPointProbeToolProvider::IDOSPointProbeToolProvider(IDOSRenderView* view)
    :
    IDOSToolProvider(view)
    , m_action(new QAction(QIcon(QStringLiteral(":/images/render-select.svg")), tr("Point Probe"), this))
    , m_tool(new IDOSRenderInteractionTool(view, IDOSRenderInteractionTool::Mode::PointProbe))
{
    m_tool->setParent(this);
    m_action->setObjectName(QStringLiteral("viewToolPointProbeAction"));
    m_action->setCheckable(true);
    connect(m_action, &QAction::toggled, this, &IDOSPointProbeToolProvider::onToggled);
    if (view != nullptr)
    {
        connect(view, &IDOSRenderView::toolEnabled, this, &IDOSPointProbeToolProvider::updateState);
        connect(view, &IDOSRenderView::toolDisabled, this, &IDOSPointProbeToolProvider::updateState);
        connect(view, &IDOSRenderView::viewModeChanged, this, &IDOSPointProbeToolProvider::updateState);
    }
    updateState();
}

QList<QAction*> IDOSPointProbeToolProvider::actions() const
{
    return QList<QAction*>{m_action};
}

QList<IDOSTool*> IDOSPointProbeToolProvider::tools() const
{
    return QList<IDOSTool*>{m_tool};
}

void IDOSPointProbeToolProvider::updateState()
{
    const QSignalBlocker blocker(m_action);
    m_action->setEnabled(view() != nullptr);
    m_action->setChecked(m_tool->isActive());
}

void IDOSPointProbeToolProvider::onToggled(bool enabled)
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

QString IDOSPointProbeToolProviderMetadata::id() const
{
    return QStringLiteral("idos.view.PointProbe");
}

QString IDOSPointProbeToolProviderMetadata::name() const
{
    return IDOSPointProbeToolProvider::tr("Point Probe");
}

QString IDOSPointProbeToolProviderMetadata::description() const
{
    return name();
}

QString IDOSPointProbeToolProviderMetadata::iconPath() const
{
    return QStringLiteral(":/images/render-select.svg");
}

QString IDOSPointProbeToolProviderMetadata::groupId() const
{
    return QStringLiteral("interaction");
}

int IDOSPointProbeToolProviderMetadata::order() const
{
    return 21;
}

bool IDOSPointProbeToolProviderMetadata::supports(const IDOSRenderView* view) const
{
    return view != nullptr;
}

IDOSToolProvider* IDOSPointProbeToolProviderMetadata::create(IDOSRenderView* view) const
{
    return view != nullptr ? new IDOSPointProbeToolProvider(view) : nullptr;
}
