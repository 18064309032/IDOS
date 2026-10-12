#include <algorithm>

#include <QAction>
#include <QEvent>
#include <QIcon>
#include <QMenu>
#include <QResizeEvent>
#include <QSize>
#include <QStringList>
#include <QThread>
#include <QToolButton>

#include <QVTKOpenGLNativeWidget.h>

#include "idosrenderview.h"
#include "idostool.h"
#include "idostoolmetadata.h"
#include "idostoolprovider.h"
#include "idostoolregistry.h"

#include "idostoolbar.h"

IDOSToolBar::IDOSToolBar(QWidget* parent)
    :
    QToolBar(parent)
    , m_overflowButton(new QToolButton(this))
    , m_overflowMenu(new QMenu(this))
    , m_updating(false)
    , m_internalVisibility(true)
    , m_userHidden(false)
{
    setMovable(false);
    setFloatable(false);
    setIconSize(QSize(20, 20));
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setObjectName(QStringLiteral("renderToolBar"));
    setStyleSheet(QStringLiteral("QToolBar#renderToolBar { background: palette(button); "
                                 "border: 1px solid palette(mid); border-radius: 6px; padding: 3px; }"));
    m_overflowButton->setText(QStringLiteral("»"));
    m_overflowButton->setToolTip(tr("More tools"));
    m_overflowButton->setMenu(m_overflowMenu);
    m_overflowButton->setPopupMode(QToolButton::InstantPopup);
    m_overflowButton->hide();
    installEventFilter(this);
    hide();
    m_internalVisibility = false;
}

void IDOSToolBar::setVisible(bool visible)
{
    if (!m_internalVisibility)
    {
        m_userHidden = !visible;
    }
    QToolBar::setVisible(visible);
}

IDOSToolBar::~IDOSToolBar()
{
    if (m_registry != nullptr)
    {
        disconnect(m_registry, nullptr, this, nullptr);
        m_registry->detach(this);
    }
    clearTools();
}

bool IDOSToolBar::setRegistry(IDOSToolRegistry* registry)
{
    if (QThread::currentThread() != thread() || !canChange() ||
        (registry != nullptr && registry->thread() != thread()))
    {
        return false;
    }
    if (m_registry == registry)
    {
        return true;
    }
    if (m_registry != nullptr)
    {
        disconnect(m_registry, nullptr, this, nullptr);
        m_registry->detach(this);
    }
    clearTools();
    m_registry = registry;
    if (registry != nullptr)
    {
        registry->attach(this);
        connect(registry, &IDOSToolRegistry::metadataAdded, this, &IDOSToolBar::onMetadataAdded);
        connect(registry, &IDOSToolRegistry::metadataAboutToBeRemoved, this, &IDOSToolBar::onMetadataAboutToBeRemoved);
        connect(registry, &QObject::destroyed, this, &IDOSToolBar::onRegistryDestroyed);
    }
    rebuildTools();
    return true;
}

IDOSToolRegistry* IDOSToolBar::registry() const
{
    return m_registry.data();
}

IDOSRenderView* IDOSToolBar::view() const
{
    return m_view.data();
}

bool IDOSToolBar::canRemoveTool(const QString& id) const
{
    IDOSToolProvider* provider = m_providers.value(id, nullptr);
    if (provider == nullptr)
    {
        return true;
    }
    for (IDOSTool* tool : provider->tools())
    {
        if (tool != nullptr && tool->isBusy())
        {
            return false;
        }
    }
    return m_view == nullptr || !m_view->isToolTransitioning();
}

bool IDOSToolBar::canChange() const
{
    for (const QString& id : m_providers.keys())
    {
        if (!canRemoveTool(id))
        {
            return false;
        }
    }
    return true;
}

bool IDOSToolBar::isBusy() const
{
    return !canChange();
}

bool IDOSToolBar::setView(IDOSRenderView* view)
{
    if (view == nullptr || view->toolBar() != this || !canChange() ||
        (m_view != nullptr && m_view != view))
    {
        return false;
    }
    if (m_view == view)
    {
        return true;
    }
    m_internalVisibility = true;
    setParent(view->vtkWidget());
    m_internalVisibility = false;
    clearTools();
    if (m_view != nullptr)
    {
        disconnect(m_view, nullptr, this, nullptr);
    }
    m_view = view;
    if (view != nullptr)
    {
        connect(view, &IDOSRenderView::viewStateChanged, this, &IDOSToolBar::onViewStateChanged);
        connect(view, &IDOSRenderView::viewModeChanged, this, &IDOSToolBar::onViewModeChanged);
    }
    rebuildTools();
    return true;
}

void IDOSToolBar::addTool(const QString& id)
{
    if (m_view == nullptr || m_registry == nullptr || m_providers.contains(id))
    {
        return;
    }
    IDOSToolMetadata* metadata = m_registry->metadata(id);
    if (metadata == nullptr || !metadata->supports(m_view))
    {
        return;
    }
    IDOSToolProvider* provider = metadata->create(m_view);
    if (provider == nullptr)
    {
        return;
    }
    bool valid = provider->view() == m_view && provider->parent() == nullptr;
    for (IDOSTool* tool : provider->tools())
    {
        valid = valid && tool != nullptr && tool->view() == m_view && tool->parent() == provider;
    }
    for (QAction* action : provider->actions())
    {
        valid = valid && action != nullptr && action->parent() == provider;
    }
    if (!valid)
    {
        delete provider;
        return;
    }
    provider->setParent(this);
    m_providers.insert(id, provider);
    const QList<QAction*> entries = provider->actions();
    if (entries.size() == 1)
    {
        QAction* entry = entries.first();
        if (entry->text().isEmpty())
        {
            entry->setText(metadata->name());
        }
        if (entry->toolTip().isEmpty())
        {
            entry->setToolTip(metadata->description());
        }
        if (entry->icon().isNull() && !metadata->iconPath().isEmpty())
        {
            entry->setIcon(QIcon(metadata->iconPath()));
        }
    }
    for (QAction* action : provider->actions())
    {
        connect(action, &QAction::changed, this, &IDOSToolBar::onActionChanged);
    }
    provider->updateState();
}

void IDOSToolBar::removeTool(const QString& id)
{
    IDOSToolProvider* provider = m_providers.take(id);
    if (provider == nullptr)
    {
        return;
    }
    const QList<QMenu*> menus = provider->findChildren<QMenu*>();
    for (QMenu* menu : menus)
    {
        menu->close();
    }
    m_overflowMenu->close();
    m_overflowMenu->clear();
    for (IDOSTool* tool : provider->tools())
    {
        if (m_view != nullptr)
        {
            m_view->disableTool(tool);
        }
    }
    for (QAction* action : provider->actions())
    {
        if (action->menu() != nullptr)
        {
            action->menu()->close();
            const QList<QMenu*> nested = action->menu()->findChildren<QMenu*>();
            for (QMenu* menu : nested)
            {
                menu->close();
            }
        }
        disconnect(action, nullptr, this, nullptr);
        removeAction(action);
    }
    delete provider;
}

void IDOSToolBar::clearTools()
{
    const QList<QString> ids = m_providers.keys();
    for (const QString& id : ids)
    {
        removeTool(id);
    }
    for (QAction* separator : m_separators)
    {
        delete separator;
    }
    m_separators.clear();
    m_internalVisibility = true;
    hide();
    m_internalVisibility = false;
}

void IDOSToolBar::rebuildTools()
{
    if (!canChange())
    {
        return;
    }
    clearTools();
    if (m_registry != nullptr && m_view != nullptr)
    {
        for (IDOSToolMetadata* metadata : m_registry->metadata())
        {
            addTool(metadata->id());
        }
    }
    arrangeActions();
}

bool IDOSToolBar::metadataBefore(const IDOSToolMetadata* first, const IDOSToolMetadata* second)
{
    const QStringList groups{QStringLiteral("navigation"), QStringLiteral("display"),
                             QStringLiteral("interaction"), QStringLiteral("output")};
    int firstGroup = groups.indexOf(first->groupId());
    int secondGroup = groups.indexOf(second->groupId());
    firstGroup = firstGroup < 0 ? groups.size() : firstGroup;
    secondGroup = secondGroup < 0 ? groups.size() : secondGroup;
    if (firstGroup != secondGroup)
    {
        return firstGroup < secondGroup;
    }
    if (first->groupId() != second->groupId())
    {
        return first->groupId() < second->groupId();
    }
    if (first->order() != second->order())
    {
        return first->order() < second->order();
    }
    return first->id() < second->id();
}

void IDOSToolBar::arrangeActions()
{
    m_updating = true;
    const QList<QAction*> oldActions = actions();
    for (QAction* action : oldActions)
    {
        removeAction(action);
    }
    for (QAction* separator : m_separators)
    {
        delete separator;
    }
    m_separators.clear();
    if (m_registry != nullptr)
    {
        QList<IDOSToolMetadata*> definitions = m_registry->metadata();
        std::sort(definitions.begin(), definitions.end(), &IDOSToolBar::metadataBefore);
        QString previousGroup;
        bool hasActions = false;
        for (IDOSToolMetadata* metadata : definitions)
        {
            IDOSToolProvider* provider = m_providers.value(metadata->id(), nullptr);
            if (provider == nullptr || provider->actions().isEmpty())
            {
                continue;
            }
            if (hasActions && previousGroup != metadata->groupId())
            {
                m_separators.append(addSeparator());
            }
            for (QAction* action : provider->actions())
            {
                addAction(action);
                QWidget* button = widgetForAction(action);
                if (button != nullptr)
                {
                    button->installEventFilter(this);
                }
                QToolButton* toolButton = qobject_cast<QToolButton*>(button);
                if (toolButton != nullptr && action->menu() != nullptr)
                {
                    toolButton->setPopupMode(QToolButton::InstantPopup);
                }
            }
            hasActions = true;
            previousGroup = metadata->groupId();
        }
    }
    m_updating = false;
    updateVisibility();
}

void IDOSToolBar::updateToolStates()
{
    if (m_updating)
    {
        return;
    }
    m_updating = true;
    for (IDOSToolProvider* provider : m_providers)
    {
        provider->updateState();
    }
    m_updating = false;
    updateVisibility();
}

void IDOSToolBar::updateVisibility()
{
    bool available = false;
    for (QAction* action : actions())
    {
        available = available || (!action->isSeparator() && action->isVisible());
    }
    m_internalVisibility = true;
    setVisible(m_view != nullptr && available && !m_userHidden);
    m_internalVisibility = false;
    emit layoutChanged();
    updateOverflow();
}

void IDOSToolBar::updateOverflow()
{
    m_overflowMenu->clear();
    for (QAction* action : actions())
    {
        if (!action->isSeparator() && action->isVisible() &&
            (actionGeometry(action).right() > width() - 28 || actionGeometry(action).isEmpty()))
        {
            m_overflowMenu->addAction(action);
        }
    }
    const bool hasOverflow = !m_overflowMenu->actions().isEmpty();
    m_overflowButton->setGeometry(qMax(0, width() - 26), 2, 24, qMax(24, height() - 4));
    m_overflowButton->setVisible(hasOverflow);
    m_overflowButton->raise();
}

bool IDOSToolBar::eventFilter(QObject* watched, QEvent* event)
{
    if (m_view != nullptr && (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::FocusIn))
    {
        emit m_view->focused();
    }
    return QToolBar::eventFilter(watched, event);
}

void IDOSToolBar::resizeEvent(QResizeEvent* event)
{
    QToolBar::resizeEvent(event);
    updateOverflow();
}

void IDOSToolBar::onMetadataAdded(const QString& id)
{
    addTool(id);
    arrangeActions();
}

void IDOSToolBar::onMetadataAboutToBeRemoved(const QString& id)
{
    removeTool(id);
    arrangeActions();
}

void IDOSToolBar::onRegistryDestroyed()
{
    m_registry = nullptr;
    clearTools();
}

void IDOSToolBar::onViewStateChanged()
{
    updateToolStates();
}

void IDOSToolBar::onViewModeChanged()
{
    rebuildTools();
}

void IDOSToolBar::onActionChanged()
{
    if (!m_updating)
    {
        updateVisibility();
    }
}
