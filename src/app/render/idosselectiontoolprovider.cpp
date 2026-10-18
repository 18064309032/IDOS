#include <QAction>
#include <QIcon>
#include <QSignalBlocker>

#include "idosrenderview.h"
#include "tools/idosrenderinteractiontool.h"

#include "idosselectiontoolprovider.h"

IDOSSelectionToolProvider::IDOSSelectionToolProvider(IDOSRenderView* view)
    :
    IDOSToolProvider(view)
    , m_action(new QAction(QIcon(QStringLiteral(":/images/render-select.svg")), tr("Select"), this))
    , m_tool(new IDOSRenderInteractionTool(view, IDOSRenderInteractionTool::Mode::Selection))
{
    m_tool->setParent(this);
    m_action->setObjectName(QStringLiteral("viewToolSelectionAction"));
    m_action->setCheckable(true);
    connect(m_action, &QAction::toggled, this, &IDOSSelectionToolProvider::onToggled);
    if (view != nullptr)
    {
        connect(view, &IDOSRenderView::toolEnabled, this, &IDOSSelectionToolProvider::updateState);
        connect(view, &IDOSRenderView::toolDisabled, this, &IDOSSelectionToolProvider::updateState);
        connect(view, &IDOSRenderView::viewModeChanged, this, &IDOSSelectionToolProvider::updateState);
    }
    updateState();
}

QList<QAction*> IDOSSelectionToolProvider::actions() const
{
    return QList<QAction*>{m_action};
}

QList<IDOSTool*> IDOSSelectionToolProvider::tools() const
{
    return QList<IDOSTool*>{m_tool};
}

void IDOSSelectionToolProvider::updateState()
{
    const QSignalBlocker blocker(m_action);
    m_action->setEnabled(view() != nullptr);
    m_action->setChecked(m_tool->isActive());
}

void IDOSSelectionToolProvider::onToggled(bool enabled)
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

QString IDOSSelectionToolProviderMetadata::id() const
{
    return QStringLiteral("idos.view.Selection");
}

QString IDOSSelectionToolProviderMetadata::name() const
{
    return IDOSSelectionToolProvider::tr("Select");
}

QString IDOSSelectionToolProviderMetadata::description() const
{
    return name();
}

QString IDOSSelectionToolProviderMetadata::iconPath() const
{
    return QStringLiteral(":/images/render-select.svg");
}

QString IDOSSelectionToolProviderMetadata::groupId() const
{
    return QStringLiteral("interaction");
}

int IDOSSelectionToolProviderMetadata::order() const
{
    return 23;
}

bool IDOSSelectionToolProviderMetadata::supports(const IDOSRenderView* view) const
{
    return view != nullptr;
}

IDOSToolProvider* IDOSSelectionToolProviderMetadata::create(IDOSRenderView* view) const
{
    return view != nullptr ? new IDOSSelectionToolProvider(view) : nullptr;
}
