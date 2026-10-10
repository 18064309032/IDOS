#include <QList>
#include <QtGlobal>

#ifdef IDOS_ENABLE_RENDER
#include <vtkActor.h>
#include <vtkProp.h>
#endif

#include "idosdataobject.h"
#include "idosrenderobject.h"
#include "idosrenderprovider.h"

#include "idosrenderobjectprovider.h"

class IDOSBoundRenderProvider : public IDOSDataRenderProvider
{
  public:
    IDOSBoundRenderProvider(IDOSDataObject* dataObject,
                            IDOSRenderObject* renderObject,
                            const IDOSRenderObjectProvider* renderer);
    ~IDOSBoundRenderProvider() override;

    void update() override;
    QList<vtkActor*> actors() const override;
    QList<vtkSmartPointer<vtkProp>> legends() const override;
    void setHighlighted(bool highlighted) override;
    bool visible() const override;
    void setVisible(bool visible) override;
    double opacity() const override;
    void setOpacity(double opacity) override;
    bool supportsDisplayMode() const override;
    IDOSDisplayMode displayMode() const override;
    bool setDisplayMode(IDOSDisplayMode mode) override;

  private:
    IDOSRenderObject* m_renderObject;
    const IDOSRenderObjectProvider* m_renderer;
    QList<vtkSmartPointer<vtkActor>> m_actors;
    QList<vtkSmartPointer<vtkProp>> m_legends;
    bool m_highlighted;
    bool m_visible;
    double m_opacity;
};

IDOSBoundRenderProvider::IDOSBoundRenderProvider(IDOSDataObject* dataObject,
                                                 IDOSRenderObject* renderObject,
                                                 const IDOSRenderObjectProvider* renderer)
    :
    IDOSDataRenderProvider(dataObject)
    , m_renderObject(renderObject)
    , m_renderer(renderer)
    , m_actors()
    , m_legends()
    , m_highlighted(false)
    , m_visible(dataObject != nullptr && dataObject->isVisible())
    , m_opacity(1.0)
{
}

IDOSBoundRenderProvider::~IDOSBoundRenderProvider()
{
}

void IDOSBoundRenderProvider::update()
{
    m_actors.clear();
    m_legends.clear();
    if (!visible() || m_renderer == nullptr || m_renderObject == nullptr ||
        !m_renderer->canConvert(m_renderObject))
    {
        return;
    }

#ifdef IDOS_ENABLE_RENDER
    const QList<vtkActor*> renderedActors = m_renderer->toVtk(m_renderObject, m_highlighted);
    for (vtkActor* actor : renderedActors)
    {
        if (actor != nullptr)
        {
            m_actors.append(actor);
        }
    }
    const QList<vtkSmartPointer<vtkProp>> renderedLegends =
        m_renderer->toVtkLegends(m_renderObject);
    for (int index = 0; index < renderedLegends.size(); ++index)
    {
        m_legends.append(renderedLegends.at(index));
    }
#endif
}

QList<vtkActor*> IDOSBoundRenderProvider::actors() const
{
    QList<vtkActor*> actors;
#ifdef IDOS_ENABLE_RENDER
    for (int index = 0; index < m_actors.size(); ++index)
    {
        actors.append(m_actors.at(index).GetPointer());
    }
#endif
    return actors;
}

QList<vtkSmartPointer<vtkProp>> IDOSBoundRenderProvider::legends() const
{
    return m_legends;
}

void IDOSBoundRenderProvider::setHighlighted(bool highlighted)
{
    if (m_highlighted != highlighted)
    {
        m_highlighted = highlighted;
        update();
    }
}

bool IDOSBoundRenderProvider::visible() const
{
    return m_visible;
}

void IDOSBoundRenderProvider::setVisible(bool visible)
{
    if (m_visible != visible)
    {
        m_visible = visible;
        update();
    }
}

double IDOSBoundRenderProvider::opacity() const
{
    return m_opacity;
}

void IDOSBoundRenderProvider::setOpacity(double opacity)
{
    const double boundedOpacity = qBound(0.0, opacity, 1.0);
    if (!qFuzzyCompare(m_opacity, boundedOpacity))
    {
        m_opacity = boundedOpacity;
        update();
    }
}

bool IDOSBoundRenderProvider::supportsDisplayMode() const
{
    return m_renderer != nullptr && m_renderObject != nullptr &&
           m_renderer->canSetDisplayMode(m_renderObject);
}

IDOSDisplayMode IDOSBoundRenderProvider::displayMode() const
{
    return m_renderer != nullptr && m_renderObject != nullptr
               ? m_renderer->displayMode(m_renderObject)
               : IDOSDisplayMode::Surface;
}

bool IDOSBoundRenderProvider::setDisplayMode(IDOSDisplayMode mode)
{
    if (!supportsDisplayMode())
    {
        return false;
    }
    const bool changed = m_renderer->setDisplayMode(m_renderObject, mode);
    if (changed)
    {
        update();
    }
    return changed;
}

IDOSDataRenderProvider::IDOSDataRenderProvider(IDOSDataObject* dataObject)
    :
    m_dataObject(dataObject)
{
}

IDOSDataRenderProvider::~IDOSDataRenderProvider()
{
}

IDOSDataObject* IDOSDataRenderProvider::dataObject() const
{
    return m_dataObject;
}

IDOSRenderObjectProvider::IDOSRenderObjectProvider()
{
}

IDOSRenderObjectProvider::~IDOSRenderObjectProvider()
{
}

bool IDOSRenderObjectProvider::canConvert(const IDOSRenderObject* object) const
{
    Q_UNUSED(object);
    return false;
}

QList<vtkActor*> IDOSRenderObjectProvider::toVtk(const IDOSRenderObject* object,
                                                bool highlighted) const
{
    Q_UNUSED(object);
    Q_UNUSED(highlighted);
    return QList<vtkActor*>();
}

QList<vtkSmartPointer<vtkProp>> IDOSRenderObjectProvider::toVtkLegends(
    const IDOSRenderObject* object) const
{
    Q_UNUSED(object);
    return QList<vtkSmartPointer<vtkProp>>();
}

IDOSRenderProviderContext::~IDOSRenderProviderContext()
{
}

bool IDOSRenderObjectProvider::synchronize(IDOSRenderObjectChange change,
                                           const QStringList& objectIds,
                                           IDOSRenderProviderContext* context,
                                           bool* resetCamera)
{
    if (context == nullptr)
    {
        return false;
    }

    if (change == IDOSRenderObjectChange::ProjectReplaced)
    {
        bool changed = false;
        const QList<IDOSDataObject*> objects = context->dataObjects();
        for (IDOSDataObject* object : objects)
        {
            if (object != nullptr && object->isVisible() && object->typeId() == dataTypeId())
            {
                IDOSRenderObject* renderObject = createRenderObject(object);
                if (renderObject != nullptr)
                {
                    context->addRenderObject(renderObject);
                    changed = true;
                    if (resetCamera != nullptr)
                    {
                        *resetCamera = true;
                    }
                }
            }
        }
        return changed;
    }

    bool changed = false;
    for (const QString& objectId : objectIds)
    {
        if (objectId.isEmpty())
        {
            continue;
        }

        if (change == IDOSRenderObjectChange::Removed)
        {
            if (context->renderObject(objectId) != nullptr)
            {
                context->removeRenderObject(objectId);
                changed = true;
            }
            continue;
        }

        IDOSDataObject* object = context->dataObject(objectId);
        if (object == nullptr || object->typeId() != dataTypeId())
        {
            continue;
        }

        IDOSRenderObject* existingObject = context->renderObject(objectId);
        if (!object->isVisible())
        {
            IDOSRenderProvider* renderProvider =
                existingObject != nullptr ? existingObject->renderProvider() : nullptr;
            if (renderProvider != nullptr && renderProvider->visible())
            {
                renderProvider->setVisible(false);
                changed = true;
            }
            continue;
        }

        if (change == IDOSRenderObjectChange::VisibilityChanged && existingObject != nullptr)
        {
            IDOSRenderProvider* renderProvider = existingObject->renderProvider();
            if (renderProvider != nullptr && !renderProvider->visible())
            {
                renderProvider->setVisible(true);
                changed = true;
            }
            continue;
        }

        IDOSRenderObject* renderObject = createRenderObject(object);
        if (renderObject == nullptr)
        {
            continue;
        }
        if (existingObject != nullptr)
        {
            context->removeRenderObject(objectId);
        }
        context->addRenderObject(renderObject);
        changed = true;
        if (resetCamera != nullptr && existingObject == nullptr)
        {
            *resetCamera = true;
        }
    }
    return changed;
}

bool IDOSRenderObjectProvider::canSetDisplayMode(const IDOSRenderObject* object) const
{
    Q_UNUSED(object);
    return false;
}

bool IDOSRenderObjectProvider::setDisplayMode(IDOSRenderObject* object, IDOSDisplayMode mode) const
{
    Q_UNUSED(object);
    Q_UNUSED(mode);
    return false;
}

IDOSDisplayMode IDOSRenderObjectProvider::displayMode(const IDOSRenderObject* object) const
{
    Q_UNUSED(object);
    return IDOSDisplayMode::Surface;
}

IDOSRenderProvider* IDOSRenderObjectProvider::createRenderProvider(
    IDOSDataObject* dataObject,
    IDOSRenderObject* renderObject) const
{
    return new IDOSBoundRenderProvider(dataObject, renderObject, this);
}
