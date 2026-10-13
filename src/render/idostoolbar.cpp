#include <algorithm>

#include <QAction>
#include <QColor>
#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QIcon>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QPointF>
#include <QSize>
#include <QStringList>
#include <QThread>
#include <QToolButton>
#include <QWidgetAction>

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
    , m_dragHandleAction(new QWidgetAction(this))
    , m_dragHandle(new QLabel())
    , m_dragStartGlobalPosition()
    , m_dragStartPosition()
    , m_updating(false)
    , m_internalVisibility(true)
    , m_userHidden(false)
    , m_dragging(false)
    , m_userPositioned(false)
{
    setMovable(false);
    setFloatable(false);
    setIconSize(QSize(20, 20));
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setObjectName(QStringLiteral("renderToolBar"));
    setStyleSheet(QStringLiteral("QToolBar#renderToolBar { background: #F7F9FC; "
                                 "border: 1px solid #D7DFE8; border-radius: 9px; padding: 7px; "
                                 "padding-left: 0px; }"
                                 "QToolBar#renderToolBar QToolButton { background: transparent; "
                                 "border: 1px solid transparent; border-radius: 5px; "
                                 "margin: 3px; padding: 3px; }"
                                 "QToolBar#renderToolBar QToolButton:hover { background: #EAF3FD; "
                                 "border-color: #C9DCF4; }"
                                 "QToolBar#renderToolBar QToolButton:pressed, "
                                 "QToolBar#renderToolBar QToolButton:checked { background: #DCEBFA; "
                                 "border-color: #B9D2EF; }"
                                 "QLabel#renderToolBarDragHandle { color: #94A3B8; border-radius: 4px; }"
                                 "QLabel#renderToolBarDragHandle:hover { background: #EAF3FD; }"));
    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(14.0);
    shadowEffect->setOffset(0.0, 3.0);
    shadowEffect->setColor(QColor(25, 45, 68, 42));
    setGraphicsEffect(shadowEffect);
    m_dragHandle->setAlignment(Qt::AlignCenter);
    m_dragHandle->setFixedSize(10, 24);
    m_dragHandle->setCursor(Qt::SizeAllCursor);
    m_dragHandle->setAttribute(Qt::WA_Hover, true);
    m_dragHandle->setToolTip(tr("Drag to move toolbar"));
    m_dragHandle->setObjectName(QStringLiteral("renderToolBarDragHandle"));
    const qreal devicePixelRatio = m_dragHandle->devicePixelRatioF();
    QPixmap gripPixmap(QSize(qRound(10.0 * devicePixelRatio),
                             qRound(24.0 * devicePixelRatio)));
    gripPixmap.setDevicePixelRatio(devicePixelRatio);
    gripPixmap.fill(Qt::transparent);
    QPainter gripPainter(&gripPixmap);
    gripPainter.setRenderHint(QPainter::Antialiasing);
    gripPainter.setPen(Qt::NoPen);
    gripPainter.setBrush(palette().color(QPalette::Mid));
    for (int row = 0; row < 4; ++row)
    {
        const qreal y = 3.0 + static_cast<qreal>(row) * 6.0;
        gripPainter.drawEllipse(QPointF(5.0, y), 1.0, 1.0);
        gripPainter.drawEllipse(QPointF(9.0, y), 1.0, 1.0);
    }
    gripPainter.end();
    m_dragHandle->setPixmap(gripPixmap);
    m_dragHandleAction->setDefaultWidget(m_dragHandle);
    m_dragHandle->installEventFilter(this);
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
    if (m_view != nullptr)
    {
        disconnect(m_view, nullptr, this, nullptr);
    }
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

bool IDOSToolBar::isUserPositioned() const
{
    return m_userPositioned;
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
        removeActionWidget(action);
    }
    delete provider;
}

void IDOSToolBar::removeActionWidget(QAction* action)
{
    if (qobject_cast<QWidgetAction*>(action) != nullptr)
    {
        removeAction(action);
        return;
    }
    QPointer<QToolButton> button = qobject_cast<QToolButton*>(widgetForAction(action));
    removeAction(action);
    // QToolBar defers button deletion; release plugin icons before unloading its DLL.
    delete button.data();
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
        removeActionWidget(action);
    }
    for (QAction* separator : m_separators)
    {
        delete separator;
    }
    m_separators.clear();
    addAction(m_dragHandleAction);
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
        available = available || (action != m_dragHandleAction &&
                                 !action->isSeparator() && action->isVisible());
    }
    m_internalVisibility = true;
    setVisible(m_view != nullptr && available && !m_userHidden);
    m_internalVisibility = false;
    emit layoutChanged();
}

bool IDOSToolBar::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_dragHandle)
    {
        if (event->type() == QEvent::UngrabMouse || event->type() == QEvent::Hide)
        {
            m_dragging = false;
        }
        if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseMove ||
            event->type() == QEvent::MouseButtonRelease)
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (event->type() == QEvent::MouseButtonPress &&
                mouseEvent->button() == Qt::LeftButton)
            {
                m_dragging = true;
                m_dragStartGlobalPosition = mouseEvent->globalPos();
                m_dragStartPosition = pos();
                if (m_view != nullptr)
                {
                    emit m_view->focused();
                }
                return true;
            }
            if (event->type() == QEvent::MouseMove && m_dragging)
            {
                m_userPositioned = true;
                const QPoint globalDelta = mouseEvent->globalPos() - m_dragStartGlobalPosition;
                const QPoint requestedPosition = m_dragStartPosition + globalDelta;
                QWidget* toolbarParent = this->parentWidget();
                if (toolbarParent != nullptr)
                {
                    const int maximumX = qMax(0, toolbarParent->width() - width());
                    const int maximumY = qMax(0, toolbarParent->height() - height());
                    move(qBound(0, requestedPosition.x(), maximumX),
                         qBound(0, requestedPosition.y(), maximumY));
                }
                emit layoutChanged();
                return true;
            }
            if (event->type() == QEvent::MouseButtonRelease &&
                mouseEvent->button() == Qt::LeftButton && m_dragging)
            {
                m_dragging = false;
                return true;
            }
        }
    }
    if (m_view != nullptr && (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::FocusIn))
    {
        emit m_view->focused();
    }
    return QToolBar::eventFilter(watched, event);
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
