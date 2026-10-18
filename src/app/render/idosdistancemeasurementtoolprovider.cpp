#include <QAction>
#include <QIcon>
#include <QSignalBlocker>

#include "idosrenderview.h"
#include "tools/idosrenderinteractiontool.h"

#include "idosdistancemeasurementtoolprovider.h"

IDOSDistanceMeasurementToolProvider::IDOSDistanceMeasurementToolProvider(IDOSRenderView* view)
    :
    IDOSToolProvider(view)
    , m_action(new QAction(QIcon(QStringLiteral(":/images/render-measure.svg")), tr("Distance Measurement"), this))
    , m_tool(new IDOSRenderInteractionTool(view, IDOSRenderInteractionTool::Mode::DistanceMeasurement))
{
    m_tool->setParent(this);
    m_action->setObjectName(QStringLiteral("viewToolDistanceMeasurementAction"));
    m_action->setCheckable(true);
    connect(m_action, &QAction::toggled, this, &IDOSDistanceMeasurementToolProvider::onToggled);
    if (view != nullptr)
    {
        connect(view, &IDOSRenderView::toolEnabled, this, &IDOSDistanceMeasurementToolProvider::updateState);
        connect(view, &IDOSRenderView::toolDisabled, this, &IDOSDistanceMeasurementToolProvider::updateState);
        connect(view, &IDOSRenderView::viewModeChanged, this, &IDOSDistanceMeasurementToolProvider::updateState);
    }
    updateState();
}

QList<QAction*> IDOSDistanceMeasurementToolProvider::actions() const
{
    return QList<QAction*>{m_action};
}

QList<IDOSTool*> IDOSDistanceMeasurementToolProvider::tools() const
{
    return QList<IDOSTool*>{m_tool};
}

void IDOSDistanceMeasurementToolProvider::updateState()
{
    const QSignalBlocker blocker(m_action);
    m_action->setEnabled(view() != nullptr);
    m_action->setChecked(m_tool->isActive());
}

void IDOSDistanceMeasurementToolProvider::onToggled(bool enabled)
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

QString IDOSDistanceMeasurementToolProviderMetadata::id() const
{
    return QStringLiteral("idos.view.DistanceMeasurement");
}

QString IDOSDistanceMeasurementToolProviderMetadata::name() const
{
    return IDOSDistanceMeasurementToolProvider::tr("Distance Measurement");
}

QString IDOSDistanceMeasurementToolProviderMetadata::description() const
{
    return name();
}

QString IDOSDistanceMeasurementToolProviderMetadata::iconPath() const
{
    return QStringLiteral(":/images/render-measure.svg");
}

QString IDOSDistanceMeasurementToolProviderMetadata::groupId() const
{
    return QStringLiteral("interaction");
}

int IDOSDistanceMeasurementToolProviderMetadata::order() const
{
    return 22;
}

bool IDOSDistanceMeasurementToolProviderMetadata::supports(const IDOSRenderView* view) const
{
    return view != nullptr;
}

IDOSToolProvider* IDOSDistanceMeasurementToolProviderMetadata::create(IDOSRenderView* view) const
{
    return view != nullptr ? new IDOSDistanceMeasurementToolProvider(view) : nullptr;
}
