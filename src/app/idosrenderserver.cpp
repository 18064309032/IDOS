#include "idosrenderserver.h"
#include "idosdataobject.h"
#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idosgridrenderobjectprovider.h"
#include "idosproject.h"
#include "idosrendermesh.h"
#include "idosrenderobject.h"
#include "idosrenderobjectprovider.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"
#include "idoswell.h"

#include <QObject>
#include <QtAlgorithms>

IDOSRenderServer::IDOSRenderServer(QObject* parent)
    : QObject(parent)
    , m_project(nullptr)
    , m_mainScene(new IDOSRenderScene())
    , m_views()
    , m_activeViewId()
    , m_highlightedObjectId()
    , m_providers()
{
}

IDOSRenderServer::~IDOSRenderServer()
{
    qDeleteAll(m_providers);
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
    }

    m_project = project;
    if (m_project != nullptr)
    {
        connect(m_project, &IDOSProject::objectAdded, this, &IDOSRenderServer::onObjectAdded);
        connect(m_project, &IDOSProject::objectsAdded, this, &IDOSRenderServer::onObjectsAdded);
        connect(m_project, &IDOSProject::objectRemoved, this, &IDOSRenderServer::onObjectRemoved);
        connect(m_project, &IDOSProject::objectsRemoved, this, &IDOSRenderServer::onObjectsRemoved);
    }

    m_highlightedObjectId.clear();
    m_mainScene->clear();
    if (m_project != nullptr)
    {
        const QList<IDOSDataObject*> objects = m_project->objects();
        bool objectAdded = false;
        for (const IDOSDataObject* object : objects)
        {
            objectAdded = addWellToScene(object) || objectAdded;
        }
        if (objectAdded)
        {
            resetViews();
        }
    }
    refreshViews();
}

IDOSProject* IDOSRenderServer::project() const
{
    return m_project;
}

IDOSRenderScene* IDOSRenderServer::mainScene() const
{
    return m_mainScene;
}

void IDOSRenderServer::addView(const QString& viewId, IDOSRenderView* view)
{
    if (viewId.isEmpty() || view == nullptr)
    {
        return;
    }
    m_views.insert(viewId, view);
    view->setScene(m_mainScene);
    view->setHighlightedObjectId(m_highlightedObjectId);
    if (m_activeViewId.isEmpty())
    {
        m_activeViewId = viewId;
    }
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
        renderView->setScene(nullptr);
    }
    m_views.remove(viewId);
    if (m_activeViewId == viewId)
    {
        m_activeViewId.clear();
        if (!m_views.isEmpty())
        {
            m_activeViewId = m_views.firstKey();
        }
    }
}

IDOSRenderView* IDOSRenderServer::view(const QString& viewId) const
{
    return m_views.value(viewId, nullptr);
}

void IDOSRenderServer::setActiveView(const QString& viewId)
{
    if (!m_views.contains(viewId))
    {
        return;
    }
    m_activeViewId = viewId;
}

QString IDOSRenderServer::activeViewId() const
{
    return m_activeViewId;
}

IDOSRenderView* IDOSRenderServer::activeView() const
{
    return view(m_activeViewId);
}

void IDOSRenderServer::setHighlightedObjectId(const QString& objectId)
{
    if (m_highlightedObjectId == objectId)
    {
        return;
    }

    m_highlightedObjectId = objectId;
    QMap<QString, IDOSRenderView*>::const_iterator iterator = m_views.constBegin();
    while (iterator != m_views.constEnd())
    {
        IDOSRenderView* renderView = iterator.value();
        if (renderView != nullptr)
        {
            renderView->setHighlightedObjectId(m_highlightedObjectId);
        }
        ++iterator;
    }
}

void IDOSRenderServer::addProvider(IDOSRenderObjectProvider* provider)
{
    if (provider == nullptr)
    {
        return;
    }
    m_providers.append(provider);
}

void IDOSRenderServer::onItemCheckedChanged(const QString& objectId, bool checked)
{
    if (m_project != nullptr)
    {
        IDOSWell* well = qobject_cast<IDOSWell*>(m_project->objectById(objectId));
        if (well != nullptr)
        {
            well->setVisible(checked);
        }
    }

    onItemStateChanged(objectId, checked ? IDOSItemState::Shown : IDOSItemState::Hidden);
}

void IDOSRenderServer::onItemStateChanged(const QString& objectId, IDOSItemState state)
{
    switch (state)
    {
    case IDOSItemState::Shown:
        setItemShown(objectId);
        break;
    case IDOSItemState::Hidden:
        setItemHidden(objectId);
        break;
    case IDOSItemState::Selected:
        setItemSelectionState(objectId, state);
        break;
    case IDOSItemState::Unselected:
        setItemSelectionState(objectId, state);
        break;
    case IDOSItemState::Highlighted:
        setItemHighlightState(objectId, state);
        break;
    case IDOSItemState::Unhighlighted:
        setItemHighlightState(objectId, state);
        break;
    }
}

IDOSRenderObject* IDOSRenderServer::createObject(const IDOSDataObject* object) const
{
    if (object == nullptr)
    {
        return nullptr;
    }

    for (int index = 0; index < m_providers.size(); ++index)
    {
        IDOSRenderObjectProvider* provider = m_providers.at(index);
        if (provider->canCreate(object))
        {
            return provider->createObject(object);
        }
    }
    return nullptr;
}

bool IDOSRenderServer::addWellToScene(const IDOSDataObject* object)
{
    const IDOSWell* well = qobject_cast<const IDOSWell*>(object);
    if (well == nullptr || !well->isVisible())
    {
        return false;
    }

    IDOSRenderObject* renderObject = m_mainScene->object(well->objectId());
    if (renderObject != nullptr)
    {
        renderObject->setVisible(true);
        return false;
    }

    renderObject = createObject(well);
    if (renderObject == nullptr)
    {
        return false;
    }

    m_mainScene->addObject(renderObject);
    emit titleChanged(QObject::tr("3D View - %1").arg(well->name()));
    return true;
}

IDOSRenderView* IDOSRenderServer::firstView() const
{
    if (m_views.isEmpty())
    {
        return nullptr;
    }
    return m_views.first();
}

void IDOSRenderServer::refreshViews()
{
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

bool IDOSRenderServer::showGridProperty(const IDOSGridProperty* property)
{
    if (property == nullptr || m_project == nullptr)
    {
        return false;
    }

    const IDOSGrid* grid = qobject_cast<const IDOSGrid*>(m_project->objectById(property->gridId()));
    if (grid == nullptr)
    {
        return false;
    }

    IDOSRenderObject* renderObject = m_mainScene->object(grid->objectId());
    bool objectCreated = false;
    if (renderObject == nullptr)
    {
        renderObject = createObject(grid);
        if (renderObject == nullptr)
        {
            return false;
        }
        m_mainScene->addObject(renderObject);
        objectCreated = true;
    }

    renderObject->setVisible(true);
    if (!applyGridProperty(renderObject, property))
    {
        return false;
    }

    emit titleChanged(QObject::tr("3D View - %1").arg(property->name()));
    refreshViews();
    if (objectCreated)
    {
        resetViews();
    }
    return true;
}

bool IDOSRenderServer::hideGridProperty(const IDOSGridProperty* property)
{
    if (property == nullptr)
    {
        return false;
    }

    // 清除属性着色，网格保持可见并回退到默认色；
    // 不隐藏整个 grid renderObject（取消属性 ≠ 取消网格）
    IDOSRenderObject* renderObject = m_mainScene->object(property->gridId());
    IDOSRenderMesh* mesh = dynamic_cast<IDOSRenderMesh*>(renderObject);
    if (mesh != nullptr)
    {
        mesh->clearCellScalars();
        refreshViews();
    }
    return true;
}

bool IDOSRenderServer::applyGridProperty(IDOSRenderObject* renderObject, const IDOSGridProperty* property)
{
    IDOSRenderMesh* mesh = dynamic_cast<IDOSRenderMesh*>(renderObject);
    if (mesh == nullptr || property == nullptr)
    {
        return false;
    }

    const QVector<double>& values = property->values();
    const QVector<int>& globalIndices = mesh->cellGlobalIndices();
    QVector<double> cellScalars;
    cellScalars.reserve(globalIndices.size());
    for (int index = 0; index < globalIndices.size(); ++index)
    {
        const int globalIndex = globalIndices.at(index);
        if (globalIndex >= 0 && globalIndex < values.size())
        {
            cellScalars.append(values.at(globalIndex));
        }
        else
        {
            cellScalars.append(0.0);
        }
    }
    mesh->setCellScalars(property->name(), cellScalars);
    return true;
}

void IDOSRenderServer::setItemShown(const QString& objectId)
{
    if (m_project == nullptr || objectId.isEmpty())
    {
        return;
    }

    const IDOSDataObject* object = m_project->objectById(objectId);
    const IDOSGridProperty* property = qobject_cast<const IDOSGridProperty*>(object);
    if (property != nullptr)
    {
        showGridProperty(property);
        return;
    }

    IDOSRenderObject* renderObject = m_mainScene->object(objectId);
    bool objectCreated = false;
    if (renderObject == nullptr)
    {
        renderObject = createObject(object);
        if (renderObject == nullptr)
        {
            return;
        }
        m_mainScene->addObject(renderObject);
        emit titleChanged(QObject::tr("3D View - %1").arg(object->name()));
        objectCreated = true;
    }
    else
    {
        renderObject->setVisible(true);
    }
    refreshViews();
    if (objectCreated)
    {
        resetViews();
    }
}

void IDOSRenderServer::setItemHidden(const QString& objectId)
{
    if (objectId.isEmpty())
    {
        return;
    }

    if (m_project != nullptr)
    {
        const IDOSDataObject* object = m_project->objectById(objectId);
        const IDOSGridProperty* property = qobject_cast<const IDOSGridProperty*>(object);
        if (property != nullptr)
        {
            hideGridProperty(property);
            return;
        }
    }

    m_mainScene->setObjectVisible(objectId, false);
    refreshViews();
}

void IDOSRenderServer::onObjectAdded(const QString& objectId)
{
    if (m_project == nullptr || objectId.isEmpty())
    {
        return;
    }

    if (addWellToScene(m_project->objectById(objectId)))
    {
        refreshViews();
        resetViews();
    }
}

void IDOSRenderServer::onObjectsAdded(const QStringList& objectIds)
{
    if (m_project == nullptr)
    {
        return;
    }

    bool objectAdded = false;
    for (const QString& objectId : objectIds)
    {
        if (!objectId.isEmpty())
        {
            objectAdded = addWellToScene(m_project->objectById(objectId)) || objectAdded;
        }
    }

    if (objectAdded)
    {
        refreshViews();
        resetViews();
    }
}

void IDOSRenderServer::onObjectRemoved(const QString& objectId)
{
    if (objectId.isEmpty())
    {
        return;
    }

    m_mainScene->removeObject(objectId);
    refreshViews();
}

void IDOSRenderServer::onObjectsRemoved(const QStringList& objectIds)
{
    for (const QString& objectId : objectIds)
    {
        if (!objectId.isEmpty())
        {
            m_mainScene->removeObject(objectId);
        }
    }
    refreshViews();
}

void IDOSRenderServer::setItemSelectionState(const QString& objectId, IDOSItemState state)
{
    Q_UNUSED(objectId);
    Q_UNUSED(state);
}

void IDOSRenderServer::setItemHighlightState(const QString& objectId, IDOSItemState state)
{
    if (state == IDOSItemState::Highlighted)
    {
        setHighlightedObjectId(objectId);
        return;
    }

    if (state == IDOSItemState::Unhighlighted && m_highlightedObjectId == objectId)
    {
        setHighlightedObjectId(QString());
    }
}


