#include <QObject>

#include "idosdataobject.h"
#include "idosproject.h"
#include "idosrenderobject.h"
#include "idosrenderobjectprovider.h"
#include "idosrenderprovider.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"

#include "idosrenderserver.h"

IDOSRenderServer::IDOSRenderServer(QObject* parent)
    :
    QObject(parent)
    , m_project(nullptr)
    , m_mainScene(new IDOSRenderScene())
    , m_views()
    , m_viewContextObjectIds()
    , m_activeViewId()
    , m_highlightedObjectId()
    , m_selectedDataObjectId()
    , m_selectedRenderObjectId()
    , m_opacityPercentByDataId()
    , m_displayModeByDataId()
    , m_renderRegistry()
    , m_renderObjectIdsByDataId()
    , m_dataObjectIdsByRenderObjectId()
    , m_sceneUpdateDepth(0)
    , m_refreshPending(false)
    , m_resetViewsPending(false)
{
}

IDOSRenderServer::~IDOSRenderServer()
{
    delete m_mainScene;
}

void IDOSRenderServer::setProject(IDOSProject* project)
{
    if (m_project == project)
    {
        return;
    }

    if (m_project != nullptr)
    {
        disconnect(m_project, &IDOSProject::objectAdded, this, &IDOSRenderServer::onObjectAdded);
        disconnect(m_project, &IDOSProject::objectsAdded, this, &IDOSRenderServer::onObjectsAdded);
        disconnect(m_project, &IDOSProject::objectRemoved, this, &IDOSRenderServer::onObjectRemoved);
        disconnect(m_project, &IDOSProject::objectsRemoved, this, &IDOSRenderServer::onObjectsRemoved);
        disconnect(m_project, &IDOSProject::objectDataChanged, this, &IDOSRenderServer::onObjectDataChanged);
        disconnect(m_project, &IDOSProject::objectsDataChanged, this, &IDOSRenderServer::onObjectsDataChanged);
        disconnect(m_project, &IDOSProject::objectVisibilityChanged,
                   this, &IDOSRenderServer::onObjectVisibilityChanged);
        disconnect(m_project, &IDOSProject::objectsVisibilityChanged,
                   this, &IDOSRenderServer::onObjectsVisibilityChanged);
    }

    m_project = project;
    if (m_project != nullptr)
    {
        connect(m_project, &IDOSProject::objectAdded, this, &IDOSRenderServer::onObjectAdded);
        connect(m_project, &IDOSProject::objectsAdded, this, &IDOSRenderServer::onObjectsAdded);
        connect(m_project, &IDOSProject::objectRemoved, this, &IDOSRenderServer::onObjectRemoved);
        connect(m_project, &IDOSProject::objectsRemoved, this, &IDOSRenderServer::onObjectsRemoved);
        connect(m_project, &IDOSProject::objectDataChanged, this, &IDOSRenderServer::onObjectDataChanged);
        connect(m_project, &IDOSProject::objectsDataChanged, this, &IDOSRenderServer::onObjectsDataChanged);
        connect(m_project, &IDOSProject::objectVisibilityChanged,
                this, &IDOSRenderServer::onObjectVisibilityChanged);
        connect(m_project, &IDOSProject::objectsVisibilityChanged,
                this, &IDOSRenderServer::onObjectsVisibilityChanged);
    }

    beginSceneUpdate();
    setHighlightedObjectId(QString());
    m_viewContextObjectIds.clear();
    m_selectedDataObjectId.clear();
    m_selectedRenderObjectId.clear();
    m_opacityPercentByDataId.clear();
    m_displayModeByDataId.clear();
    m_renderObjectIdsByDataId.clear();
    m_dataObjectIdsByRenderObjectId.clear();
    emit selectedObjectOpacityChanged(100, false);
    emit selectedDisplayModeChanged(IDOSDisplayMode::Surface, false);
    m_mainScene->clear();
    synchronizeProviders(IDOSRenderObjectChange::ProjectReplaced, QStringList());
    applyRenderSettings();
    refreshViews();
    endSceneUpdate();
    emit activeViewContextObjectChanged(m_activeViewId, QString());
}

IDOSProject* IDOSRenderServer::project() const
{
    return m_project;
}

IDOSDataObject* IDOSRenderServer::dataObject(const QString& objectId) const
{
    return m_project != nullptr ? m_project->objectById(objectId) : nullptr;
}

QList<IDOSDataObject*> IDOSRenderServer::dataObjects() const
{
    QList<IDOSDataObject*> result;
    if (m_project != nullptr)
    {
        const QList<IDOSDataObject*> objects = m_project->objects();
        for (IDOSDataObject* object : objects)
        {
            result.append(object);
        }
    }
    return result;
}

IDOSRenderObject* IDOSRenderServer::renderObject(const QString& objectId) const
{
    const QString renderObjectId = m_renderObjectIdsByDataId.value(objectId, objectId);
    return m_mainScene->object(renderObjectId);
}

QList<IDOSRenderObject*> IDOSRenderServer::renderObjects() const
{
    return m_mainScene->objects();
}

void IDOSRenderServer::addRenderObject(IDOSRenderObject* object)
{
    if (object == nullptr)
    {
        return;
    }
    IDOSDataRenderProvider* provider =
        dynamic_cast<IDOSDataRenderProvider*>(object->renderProvider());
    const QString dataObjectId = provider != nullptr && provider->dataObject() != nullptr
                                     ? provider->dataObject()->objectId()
                                     : object->id();
    const QString previousRenderObjectId = m_renderObjectIdsByDataId.value(dataObjectId);
    if (!previousRenderObjectId.isEmpty() && previousRenderObjectId != object->id())
    {
        m_dataObjectIdsByRenderObjectId.remove(previousRenderObjectId);
        m_mainScene->removeObject(previousRenderObjectId);
    }
    m_renderObjectIdsByDataId.insert(dataObjectId, object->id());
    m_dataObjectIdsByRenderObjectId.insert(object->id(), dataObjectId);
    if (m_selectedDataObjectId == dataObjectId)
    {
        m_selectedRenderObjectId = object->id();
    }
    m_mainScene->addObject(object);
}

void IDOSRenderServer::removeRenderObject(const QString& objectId)
{
    const QString renderObjectId = m_renderObjectIdsByDataId.take(objectId);
    if (renderObjectId.isEmpty())
    {
        return;
    }
    m_dataObjectIdsByRenderObjectId.remove(renderObjectId);
    m_mainScene->removeObject(renderObjectId);
}

QWidget* IDOSRenderServer::createView(const QString& viewId, bool parallelProjection)
{
    if (viewId.isEmpty() || m_views.contains(viewId))
    {
        return nullptr;
    }
    IDOSRenderView* view = new IDOSRenderView();
    m_views.insert(viewId, view);
    connect(view,
            &IDOSRenderView::activated,
            this,
            &IDOSRenderServer::onViewActivated,
            Qt::UniqueConnection);
    connect(view, &IDOSRenderView::decorationsChanged,
            this, &IDOSRenderServer::onViewDecorationsChanged, Qt::UniqueConnection);
    connect(view, &IDOSRenderView::objectActivated,
            this, &IDOSRenderServer::onViewObjectActivated, Qt::UniqueConnection);
    view->setScene(m_mainScene);
    view->setHighlightedObjectId(
        m_renderObjectIdsByDataId.value(m_highlightedObjectId, m_highlightedObjectId));
    view->setParallelProjection(parallelProjection);
    view->setOrientation(parallelProjection ? IDOSOrientation::Top : IDOSOrientation::Isometric);
    if (m_activeViewId.isEmpty())
    {
        setActiveView(viewId);
    }
    return view;
}

void IDOSRenderServer::removeView(const QString& viewId)
{
    if (!m_views.contains(viewId))
    {
        return;
    }
    IDOSRenderView* renderView = m_views.value(viewId, nullptr);
    if (renderView != nullptr)
    {
        disconnect(renderView,
                   &IDOSRenderView::activated,
                   this,
                   &IDOSRenderServer::onViewActivated);
        disconnect(renderView, &IDOSRenderView::decorationsChanged,
                   this, &IDOSRenderServer::onViewDecorationsChanged);
        disconnect(renderView, &IDOSRenderView::objectActivated,
                   this, &IDOSRenderServer::onViewObjectActivated);
        renderView->setScene(nullptr);
    }
    m_views.remove(viewId);
    m_viewContextObjectIds.remove(viewId);
    if (m_activeViewId == viewId)
    {
        m_activeViewId.clear();
        if (!m_views.isEmpty())
        {
            setActiveView(m_views.firstKey());
        }
        else
        {
            emit currentViewChanged(m_activeViewId);
            emit activeViewContextObjectChanged(m_activeViewId, QString());
            emitActiveViewDecorationsChanged();
        }
    }
}

void IDOSRenderServer::setActiveView(const QString& viewId)
{
    if (!m_views.contains(viewId) || m_activeViewId == viewId)
    {
        return;
    }
    m_activeViewId = viewId;
    QMap<QString, IDOSRenderView*>::iterator viewIterator = m_views.begin();
    while (viewIterator != m_views.end())
    {
        if (viewIterator.value() != nullptr)
        {
            viewIterator.value()->setActive(viewIterator.key() == m_activeViewId);
        }
        ++viewIterator;
    }
    setHighlightedObjectId(activeViewContextObjectId());
    emit currentViewChanged(m_activeViewId);
    emit activeViewContextObjectChanged(m_activeViewId, activeViewContextObjectId());
    emitActiveViewDecorationsChanged();
}

QString IDOSRenderServer::activeViewId() const
{
    return m_activeViewId;
}

bool IDOSRenderServer::hasActiveView() const
{
    return m_views.contains(m_activeViewId) && m_views.value(m_activeViewId) != nullptr;
}

void IDOSRenderServer::setActiveViewOrientation(IDOSOrientation orientation)
{
    IDOSRenderView* renderView = m_views.value(m_activeViewId, nullptr);
    if (renderView != nullptr)
    {
        renderView->setOrientation(orientation);
    }
}

bool IDOSRenderServer::activeViewParallelProjection() const
{
    IDOSRenderView* renderView = m_views.value(m_activeViewId, nullptr);
    return renderView != nullptr && renderView->parallelProjection();
}

QColor IDOSRenderServer::activeViewBackgroundColor() const
{
    IDOSRenderView* renderView = m_views.value(m_activeViewId, nullptr);
    return renderView != nullptr ? renderView->backgroundColor() : QColor();
}

void IDOSRenderServer::setActiveViewBackgroundColor(const QColor& color)
{
    IDOSRenderView* renderView = m_views.value(m_activeViewId, nullptr);
    if (renderView != nullptr)
    {
        renderView->setBackgroundColor(color);
    }
}

QImage IDOSRenderServer::captureActiveViewImage()
{
    IDOSRenderView* renderView = m_views.value(m_activeViewId, nullptr);
    return renderView != nullptr ? renderView->captureImage() : QImage();
}

void IDOSRenderServer::flashActiveViewScreenshot()
{
    IDOSRenderView* renderView = m_views.value(m_activeViewId, nullptr);
    if (renderView != nullptr)
    {
        renderView->flashScreenshot();
    }
}

bool IDOSRenderServer::activeViewOrientationMarkerVisible() const
{
    IDOSRenderView* renderView = m_views.value(m_activeViewId, nullptr);
    return renderView != nullptr && renderView->orientationMarkerVisible();
}

bool IDOSRenderServer::activeViewLegendAvailable() const
{
    IDOSRenderView* renderView = m_views.value(m_activeViewId, nullptr);
    return renderView != nullptr && renderView->legendAvailable();
}

bool IDOSRenderServer::activeViewLegendVisible() const
{
    IDOSRenderView* renderView = m_views.value(m_activeViewId, nullptr);
    return renderView != nullptr && renderView->legendVisible();
}

void IDOSRenderServer::setActiveViewOrientationMarkerVisible(bool visible)
{
    IDOSRenderView* renderView = m_views.value(m_activeViewId, nullptr);
    if (renderView != nullptr)
    {
        renderView->setOrientationMarkerVisible(visible);
    }
}

void IDOSRenderServer::setActiveViewLegendVisible(bool visible)
{
    IDOSRenderView* renderView = m_views.value(m_activeViewId, nullptr);
    if (renderView != nullptr)
    {
        renderView->setLegendVisible(visible);
    }
}

QString IDOSRenderServer::activeViewContextObjectId() const
{
    return m_viewContextObjectIds.value(m_activeViewId);
}

void IDOSRenderServer::setActiveViewContextObjectId(const QString& objectId)
{
    if (!m_views.contains(m_activeViewId) ||
        m_viewContextObjectIds.value(m_activeViewId) == objectId)
    {
        return;
    }
    m_viewContextObjectIds.insert(m_activeViewId, objectId);
    setHighlightedObjectId(objectId);
    emit activeViewContextObjectChanged(m_activeViewId, objectId);
}

void IDOSRenderServer::onViewActivated()
{
    IDOSRenderView* renderView = qobject_cast<IDOSRenderView*>(sender());
    if (renderView == nullptr)
    {
        return;
    }

    QMap<QString, IDOSRenderView*>::const_iterator iterator = m_views.constBegin();
    while (iterator != m_views.constEnd())
    {
        if (iterator.value() == renderView)
        {
            setActiveView(iterator.key());
            return;
        }
        ++iterator;
    }
}

void IDOSRenderServer::setHighlightedObjectId(const QString& objectId)
{
    m_highlightedObjectId = objectId;
    const QString renderObjectId = m_renderObjectIdsByDataId.value(objectId, objectId);
    QMap<QString, IDOSRenderView*>::const_iterator iterator = m_views.constBegin();
    while (iterator != m_views.constEnd())
    {
        IDOSRenderView* renderView = iterator.value();
        if (renderView != nullptr)
        {
            renderView->setHighlightedObjectId(renderObjectId);
        }
        ++iterator;
    }
    refreshViews();
}

void IDOSRenderServer::setSelectedObjectId(const QString& objectId)
{
    QString renderObjectId;
    QString highlightedObjectId;
    if (m_project != nullptr && !objectId.isEmpty())
    {
        IDOSDataObject* object = m_project->objectById(objectId);
        IDOSRenderMetadata* metadata = object != nullptr
                                           ? m_renderRegistry.metadata(object->typeId())
                                           : nullptr;
        if (metadata != nullptr && object != nullptr)
        {
            renderObjectId = m_renderObjectIdsByDataId.value(object->objectId());
            highlightedObjectId = object->objectId();
        }
    }
    m_selectedRenderObjectId = renderObjectId;
    m_selectedDataObjectId = highlightedObjectId;
    setHighlightedObjectId(highlightedObjectId);
    emit selectedObjectOpacityChanged(selectedObjectOpacityPercent(), !m_selectedRenderObjectId.isEmpty());
    const IDOSRenderObject* renderObject = m_mainScene->object(m_selectedRenderObjectId);
    IDOSRenderProvider* renderProvider =
        renderObject != nullptr ? renderObject->renderProvider() : nullptr;
    const bool displayModeEnabled = renderProvider != nullptr && renderProvider->supportsDisplayMode();
    emit selectedDisplayModeChanged(selectedDisplayMode(), displayModeEnabled);
}

int IDOSRenderServer::selectedObjectOpacityPercent() const
{
    const QString dataObjectId = m_dataObjectIdsByRenderObjectId.value(m_selectedRenderObjectId);
    return m_opacityPercentByDataId.value(dataObjectId, 100);
}

void IDOSRenderServer::setSelectedObjectOpacityPercent(int opacityPercent)
{
    if (m_selectedRenderObjectId.isEmpty())
    {
        return;
    }
    const int clampedOpacity = qBound(0, opacityPercent, 100);
    const QString dataObjectId = m_dataObjectIdsByRenderObjectId.value(m_selectedRenderObjectId);
    m_opacityPercentByDataId.insert(dataObjectId, clampedOpacity);
    IDOSRenderObject* renderObject = m_mainScene->object(m_selectedRenderObjectId);
    IDOSRenderProvider* provider = renderObject != nullptr ? renderObject->renderProvider() : nullptr;
    if (provider != nullptr)
    {
        provider->setOpacity(static_cast<double>(clampedOpacity) / 100.0);
        refreshViews();
    }
}

void IDOSRenderServer::resetActiveViewCamera()
{
    IDOSRenderView* renderView = m_views.value(m_activeViewId, nullptr);
    if (renderView != nullptr)
    {
        renderView->resetCamera();
    }
}

void IDOSRenderServer::onViewDecorationsChanged()
{
    IDOSRenderView* renderView = qobject_cast<IDOSRenderView*>(sender());
    if (renderView != nullptr && m_views.value(m_activeViewId, nullptr) == renderView)
    {
        emitActiveViewDecorationsChanged();
    }
}

void IDOSRenderServer::onViewObjectActivated(const QString& objectId)
{
    IDOSRenderView* renderView = qobject_cast<IDOSRenderView*>(sender());
    const QString viewId = viewIdFor(renderView);
    if (!viewId.isEmpty())
    {
        setActiveView(viewId);
        emit renderViewObjectActivated(viewId,
                                       m_dataObjectIdsByRenderObjectId.value(objectId, objectId));
    }
}

QString IDOSRenderServer::viewIdFor(const IDOSRenderView* renderView) const
{
    QMap<QString, IDOSRenderView*>::const_iterator viewIterator = m_views.constBegin();
    while (viewIterator != m_views.constEnd())
    {
        if (viewIterator.value() == renderView)
        {
            return viewIterator.key();
        }
        ++viewIterator;
    }
    return QString();
}

void IDOSRenderServer::emitActiveViewDecorationsChanged()
{
    emit activeViewDecorationsChanged(activeViewOrientationMarkerVisible(),
                                      activeViewLegendAvailable(),
                                      activeViewLegendVisible());
}

IDOSDisplayMode IDOSRenderServer::selectedDisplayMode() const
{
    const IDOSRenderObject* renderObject = m_mainScene->object(m_selectedRenderObjectId);
    IDOSRenderProvider* renderProvider =
        renderObject != nullptr ? renderObject->renderProvider() : nullptr;
    return renderProvider != nullptr && renderProvider->supportsDisplayMode()
               ? renderProvider->displayMode()
               : IDOSDisplayMode::Surface;
}

void IDOSRenderServer::setSelectedDisplayMode(IDOSDisplayMode mode)
{
    IDOSRenderObject* renderObject = m_mainScene->object(m_selectedRenderObjectId);
    if (renderObject == nullptr)
    {
        return;
    }
    IDOSRenderProvider* renderProvider = renderObject->renderProvider();
    if (renderProvider != nullptr && renderProvider->setDisplayMode(mode))
    {
        const QString dataObjectId = m_dataObjectIdsByRenderObjectId.value(m_selectedRenderObjectId);
        m_displayModeByDataId.insert(dataObjectId, mode);
        refreshViews();
    }
}

void IDOSRenderServer::registerMetadata(IDOSRenderMetadata* metadata)
{
    if (metadata == nullptr)
    {
        return;
    }
    const QString dataTypeId = metadata->dataTypeId();
    if (!m_renderRegistry.registerMetadata(metadata) &&
        m_renderRegistry.metadata(dataTypeId) != metadata)
    {
        delete metadata;
    }
}

bool IDOSRenderServer::unregisterMetadata(const QString& dataTypeId)
{
    IDOSDataObject* selectedDataObject = dataObject(m_selectedDataObjectId);
    if (selectedDataObject != nullptr && selectedDataObject->typeId() == dataTypeId)
    {
        setSelectedObjectId(QString());
    }

    const QList<QString> dataObjectIds = m_renderObjectIdsByDataId.keys();
    for (int index = 0; index < dataObjectIds.size(); ++index)
    {
        IDOSRenderObject* object = renderObject(dataObjectIds.at(index));
        IDOSDataRenderProvider* provider = object != nullptr
                                               ? dynamic_cast<IDOSDataRenderProvider*>(
                                                     object->renderProvider())
                                               : nullptr;
        IDOSDataObject* sourceDataObject = provider != nullptr ? provider->dataObject() : nullptr;
        if (sourceDataObject != nullptr && sourceDataObject->typeId() == dataTypeId)
        {
            removeRenderObject(sourceDataObject->objectId());
        }
    }
    const bool removed = m_renderRegistry.unregisterMetadata(dataTypeId);
    if (removed)
    {
        refreshViews();
    }
    return removed;
}

bool IDOSRenderServer::synchronizeProviders(IDOSRenderObjectChange change, const QStringList& objectIds)
{
    bool changed = false;
    bool resetCamera = false;
    const QList<IDOSRenderMetadata*> metadataList = m_renderRegistry.metadataList();
    for (int index = 0; index < metadataList.size(); ++index)
    {
        IDOSRenderMetadata* metadata = metadataList.at(index);
        IDOSRenderObjectProvider* provider =
            dynamic_cast<IDOSRenderObjectProvider*>(metadata);
        if (provider != nullptr)
        {
            changed = provider->synchronize(change, objectIds, this, &resetCamera) || changed;
            continue;
        }

        if (change == IDOSRenderObjectChange::ProjectReplaced)
        {
            if (m_project == nullptr)
            {
                continue;
            }
            const QList<IDOSDataObject*> dataObjects = m_project->objects();
            for (IDOSDataObject* dataObject : dataObjects)
            {
                if (dataObject == nullptr || dataObject->typeId() != metadata->dataTypeId() ||
                    !dataObject->isVisible())
                {
                    continue;
                }
                IDOSRenderObject* renderObject = metadata->createRenderObject(dataObject);
                if (renderObject != nullptr)
                {
                    addRenderObject(renderObject);
                    changed = true;
                    resetCamera = true;
                }
            }
            continue;
        }

        for (const QString& objectId : objectIds)
        {
            if (objectId.isEmpty())
            {
                continue;
            }
            if (change == IDOSRenderObjectChange::Removed)
            {
                if (renderObject(objectId) != nullptr)
                {
                    removeRenderObject(objectId);
                    changed = true;
                }
                continue;
            }

            IDOSDataObject* sourceDataObject = dataObject(objectId);
            if (sourceDataObject == nullptr ||
                sourceDataObject->typeId() != metadata->dataTypeId())
            {
                continue;
            }
            IDOSRenderObject* existingObject = renderObject(objectId);
            IDOSRenderProvider* existingProvider =
                existingObject != nullptr ? existingObject->renderProvider() : nullptr;
            if (!sourceDataObject->isVisible())
            {
                if (existingProvider != nullptr && existingProvider->visible())
                {
                    existingProvider->setVisible(false);
                    changed = true;
                }
                continue;
            }
            if (change == IDOSRenderObjectChange::VisibilityChanged && existingProvider != nullptr)
            {
                if (!existingProvider->visible())
                {
                    existingProvider->setVisible(true);
                    changed = true;
                }
                continue;
            }

            IDOSRenderObject* renderObject = metadata->createRenderObject(sourceDataObject);
            if (renderObject == nullptr)
            {
                continue;
            }
            if (existingObject != nullptr)
            {
                removeRenderObject(objectId);
            }
            addRenderObject(renderObject);
            changed = true;
            if (existingObject == nullptr)
            {
                resetCamera = true;
            }
        }
    }
    if (changed && !m_highlightedObjectId.isEmpty())
    {
        setHighlightedObjectId(m_highlightedObjectId);
    }
    applyRenderSettings();
    if (!m_selectedDataObjectId.isEmpty())
    {
        const IDOSRenderObject* selectedObject = m_mainScene->object(m_selectedRenderObjectId);
        IDOSRenderProvider* selectedProvider =
            selectedObject != nullptr ? selectedObject->renderProvider() : nullptr;
        emit selectedObjectOpacityChanged(selectedObjectOpacityPercent(),
                                          selectedProvider != nullptr);
        emit selectedDisplayModeChanged(
            selectedDisplayMode(),
            selectedProvider != nullptr && selectedProvider->supportsDisplayMode());
    }
    if (resetCamera)
    {
        resetViews();
    }
    return changed;
}

void IDOSRenderServer::applyRenderSettings()
{
    const QList<IDOSRenderObject*> renderObjects = m_mainScene->objects();
    for (IDOSRenderObject* renderObject : renderObjects)
    {
        if (renderObject == nullptr)
        {
            continue;
        }
        IDOSRenderProvider* renderProvider = renderObject->renderProvider();
        IDOSDataRenderProvider* dataProvider =
            dynamic_cast<IDOSDataRenderProvider*>(renderProvider);
        IDOSDataObject* sourceDataObject =
            dataProvider != nullptr ? dataProvider->dataObject() : nullptr;
        const QString dataObjectId =
            sourceDataObject != nullptr ? sourceDataObject->objectId() : renderObject->id();
        if (m_opacityPercentByDataId.contains(dataObjectId))
        {
            if (renderProvider != nullptr)
            {
                renderProvider->setOpacity(
                    static_cast<double>(m_opacityPercentByDataId.value(dataObjectId)) / 100.0);
            }
        }
        if (renderProvider != nullptr && renderProvider->supportsDisplayMode() &&
            m_displayModeByDataId.contains(dataObjectId))
        {
            renderProvider->setDisplayMode(m_displayModeByDataId.value(dataObjectId));
        }
    }
}

void IDOSRenderServer::beginSceneUpdate()
{
    ++m_sceneUpdateDepth;
}

void IDOSRenderServer::endSceneUpdate()
{
    if (m_sceneUpdateDepth <= 0)
    {
        return;
    }

    --m_sceneUpdateDepth;
    if (m_sceneUpdateDepth > 0)
    {
        return;
    }

    const bool refreshPending = m_refreshPending;
    const bool resetViewsPending = m_resetViewsPending;
    m_refreshPending = false;
    m_resetViewsPending = false;

    if (refreshPending)
    {
        refreshViews();
    }
    if (resetViewsPending)
    {
        resetViews();
    }
}

void IDOSRenderServer::refreshViews()
{
    if (m_sceneUpdateDepth > 0)
    {
        m_refreshPending = true;
        return;
    }

    QMap<QString, IDOSRenderView*>::const_iterator iterator = m_views.constBegin();
    while (iterator != m_views.constEnd())
    {
        IDOSRenderView* renderView = iterator.value();
        if (renderView != nullptr)
        {
            renderView->refresh();
        }
        ++iterator;
    }
}

void IDOSRenderServer::resetViews()
{
    if (m_sceneUpdateDepth > 0)
    {
        m_resetViewsPending = true;
        return;
    }

    QMap<QString, IDOSRenderView*>::const_iterator iterator = m_views.constBegin();
    while (iterator != m_views.constEnd())
    {
        IDOSRenderView* renderView = iterator.value();
        if (renderView != nullptr)
        {
            renderView->resetCamera();
        }
        ++iterator;
    }
}

void IDOSRenderServer::onObjectAdded(const QString& objectId)
{
    if (m_project == nullptr || objectId.isEmpty())
    {
        return;
    }

    beginSceneUpdate();
    if (synchronizeProviders(IDOSRenderObjectChange::Added, QStringList() << objectId))
    {
        refreshViews();
        IDOSDataObject* object = m_project->objectById(objectId);
        if (object != nullptr)
        {
            emit titleChanged(QObject::tr("3D View - %1").arg(object->name()));
        }
    }
    endSceneUpdate();
}

void IDOSRenderServer::onObjectsAdded(const QStringList& objectIds)
{
    if (m_project == nullptr)
    {
        return;
    }

    beginSceneUpdate();
    if (synchronizeProviders(IDOSRenderObjectChange::Added, objectIds))
    {
        refreshViews();
        for (const QString& objectId : objectIds)
        {
            const IDOSDataObject* object = m_project->objectById(objectId);
            if (object != nullptr && object->isVisible())
            {
                emit titleChanged(QObject::tr("3D View - %1").arg(object->name()));
            }
        }
    }
    endSceneUpdate();
}

void IDOSRenderServer::onObjectDataChanged(const QString& objectId)
{
    if (m_project == nullptr || objectId.isEmpty())
    {
        return;
    }

    beginSceneUpdate();
    if (synchronizeProviders(IDOSRenderObjectChange::DataChanged, QStringList() << objectId))
    {
        refreshViews();
    }
    endSceneUpdate();
}

void IDOSRenderServer::onObjectsDataChanged(const QStringList& objectIds)
{
    beginSceneUpdate();
    if (synchronizeProviders(IDOSRenderObjectChange::DataChanged, objectIds))
    {
        refreshViews();
    }
    endSceneUpdate();
}

void IDOSRenderServer::onObjectVisibilityChanged(const QString& objectId, bool visible)
{
    Q_UNUSED(visible);
    if (objectId.isEmpty())
    {
        return;
    }
    beginSceneUpdate();
    if (synchronizeProviders(IDOSRenderObjectChange::VisibilityChanged, QStringList() << objectId))
    {
        refreshViews();
    }
    endSceneUpdate();
}

void IDOSRenderServer::onObjectsVisibilityChanged(const QStringList& objectIds)
{
    beginSceneUpdate();
    if (synchronizeProviders(IDOSRenderObjectChange::VisibilityChanged, objectIds))
    {
        refreshViews();
    }
    endSceneUpdate();
}

void IDOSRenderServer::onObjectRemoved(const QString& objectId)
{
    if (objectId.isEmpty())
    {
        return;
    }

    beginSceneUpdate();
    clearViewContextForObject(objectId);
    if (m_highlightedObjectId == objectId)
    {
        setHighlightedObjectId(QString());
    }
    if (m_selectedRenderObjectId == m_renderObjectIdsByDataId.value(objectId, objectId))
    {
        m_selectedDataObjectId.clear();
        m_selectedRenderObjectId.clear();
        emit selectedObjectOpacityChanged(100, false);
        emit selectedDisplayModeChanged(IDOSDisplayMode::Surface, false);
    }
    if (synchronizeProviders(IDOSRenderObjectChange::Removed, QStringList() << objectId))
    {
        refreshViews();
    }
    endSceneUpdate();
}

void IDOSRenderServer::onObjectsRemoved(const QStringList& objectIds)
{
    beginSceneUpdate();
    for (const QString& objectId : objectIds)
    {
        if (!objectId.isEmpty())
        {
            clearViewContextForObject(objectId);
            if (m_highlightedObjectId == objectId)
            {
                setHighlightedObjectId(QString());
            }
            if (m_selectedRenderObjectId ==
                m_renderObjectIdsByDataId.value(objectId, objectId))
            {
                m_selectedDataObjectId.clear();
                m_selectedRenderObjectId.clear();
                emit selectedObjectOpacityChanged(100, false);
                emit selectedDisplayModeChanged(IDOSDisplayMode::Surface, false);
            }
        }
    }
    if (synchronizeProviders(IDOSRenderObjectChange::Removed, objectIds))
    {
        refreshViews();
    }
    endSceneUpdate();
}

void IDOSRenderServer::clearViewContextForObject(const QString& objectId)
{
    QMap<QString, QString>::iterator contextIterator = m_viewContextObjectIds.begin();
    while (contextIterator != m_viewContextObjectIds.end())
    {
        if (contextIterator.value() == objectId)
        {
            const QString viewId = contextIterator.key();
            contextIterator = m_viewContextObjectIds.erase(contextIterator);
            if (viewId == m_activeViewId)
            {
                emit activeViewContextObjectChanged(viewId, QString());
            }
        }
        else
        {
            ++contextIterator;
        }
    }
}
