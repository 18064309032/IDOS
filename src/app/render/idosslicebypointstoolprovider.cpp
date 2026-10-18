#include <QAction>
#include <QIcon>
#include <QSignalBlocker>

#include "idosrenderview.h"
#include "tools/idosrenderinteractiontool.h"

#include "idosslicebypointstoolprovider.h"

IDOSSliceByPointsToolProvider::IDOSSliceByPointsToolProvider(IDOSRenderView* view)
    :
    IDOSToolProvider(view)
    , m_action(new QAction(QIcon(QStringLiteral(":/images/render-slice.svg")), tr("Slice by Points"), this))
    , m_tool(new IDOSRenderInteractionTool(view, IDOSRenderInteractionTool::Mode::SliceByPoints))
{
    m_tool->setParent(this);
    m_action->setObjectName(QStringLiteral("viewToolSliceByPointsAction"));
    m_action->setCheckable(true);
    connect(m_action, &QAction::toggled, this, &IDOSSliceByPointsToolProvider::onToggled);
    if (view != nullptr)
    {
        connect(view, &IDOSRenderView::toolEnabled, this, &IDOSSliceByPointsToolProvider::updateState);
        connect(view, &IDOSRenderView::toolDisabled, this, &IDOSSliceByPointsToolProvider::updateState);
        connect(view, &IDOSRenderView::viewModeChanged, this, &IDOSSliceByPointsToolProvider::updateState);
    }
    updateState();
}

QList<QAction*> IDOSSliceByPointsToolProvider::actions() const
{
    return QList<QAction*>{m_action};
}

QList<IDOSTool*> IDOSSliceByPointsToolProvider::tools() const
{
    return QList<IDOSTool*>{m_tool};
}

void IDOSSliceByPointsToolProvider::updateState()
{
    const QSignalBlocker blocker(m_action);
    m_action->setEnabled(view() != nullptr && !view()->isTwoDimensional());
    m_action->setChecked(m_tool->isActive());
}

void IDOSSliceByPointsToolProvider::onToggled(bool enabled)
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

QString IDOSSliceByPointsToolProviderMetadata::id() const
{
    return QStringLiteral("idos.view.SliceByPoints");
}

QString IDOSSliceByPointsToolProviderMetadata::name() const
{
    return IDOSSliceByPointsToolProvider::tr("Slice by Points");
}

QString IDOSSliceByPointsToolProviderMetadata::description() const
{
    return name();
}

QString IDOSSliceByPointsToolProviderMetadata::iconPath() const
{
    return QStringLiteral(":/images/render-slice.svg");
}

QString IDOSSliceByPointsToolProviderMetadata::groupId() const
{
    return QStringLiteral("interaction");
}

int IDOSSliceByPointsToolProviderMetadata::order() const
{
    return 25;
}

bool IDOSSliceByPointsToolProviderMetadata::supports(const IDOSRenderView* view) const
{
    return view != nullptr;
}

IDOSToolProvider* IDOSSliceByPointsToolProviderMetadata::create(IDOSRenderView* view) const
{
    return view != nullptr ? new IDOSSliceByPointsToolProvider(view) : nullptr;
}
