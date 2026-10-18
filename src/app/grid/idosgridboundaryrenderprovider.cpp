#include <QVector>
#include <QtGlobal>

#ifdef IDOS_ENABLE_RENDER
#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp.h>
#include <vtkProperty.h>
#endif

#include "data/grid/idosgridboundaryobject.h"

#include "idosgridboundaryrenderprovider.h"

IDOSGridBoundaryRenderProvider::IDOSGridBoundaryRenderProvider(IDOSGridBoundaryObject* boundary)
    :
    m_boundary(boundary)
    , m_actor()
    , m_visible(true)
    , m_highlighted(false)
    , m_opacity(1.0)
{
    update();
}

IDOSGridBoundaryRenderProvider::~IDOSGridBoundaryRenderProvider() = default;

IDOSGridBoundaryObject* IDOSGridBoundaryRenderProvider::boundary() const
{
    return m_boundary.data();
}

void IDOSGridBoundaryRenderProvider::update()
{
#ifdef IDOS_ENABLE_RENDER
    m_actor = nullptr;
    if (m_boundary.isNull())
    {
        return;
    }
    const QVector<double> xCoordinates = m_boundary->xCoordinates();
    const QVector<double> yCoordinates = m_boundary->yCoordinates();
    const QVector<double> zCoordinates = m_boundary->zCoordinates();
    const QVector<int> groupIds = m_boundary->groupIds();
    if (xCoordinates.size() < 2 || yCoordinates.size() != xCoordinates.size() ||
        zCoordinates.size() != xCoordinates.size() || groupIds.size() != xCoordinates.size())
    {
        return;
    }

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points->SetDataTypeToDouble();
    points->SetNumberOfPoints(xCoordinates.size());
    for (int index = 0; index < xCoordinates.size(); ++index)
    {
        points->SetPoint(index, xCoordinates.at(index), yCoordinates.at(index), zCoordinates.at(index));
    }
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    int segmentStart = 0;
    for (int index = 1; index <= groupIds.size(); ++index)
    {
        if (index != groupIds.size() && groupIds.at(index) == groupIds.at(segmentStart))
        {
            continue;
        }
        if (index - segmentStart >= 2)
        {
            lines->InsertNextCell(index - segmentStart);
            for (int pointIndex = segmentStart; pointIndex < index; ++pointIndex)
            {
                lines->InsertCellPoint(pointIndex);
            }
        }
        segmentStart = index;
    }
    vtkSmartPointer<vtkPolyData> boundaryData = vtkSmartPointer<vtkPolyData>::New();
    boundaryData->SetPoints(points);
    boundaryData->SetLines(lines);
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(boundaryData);
    m_actor = vtkSmartPointer<vtkActor>::New();
    m_actor->SetMapper(mapper);
    applyActorProperties();
#endif
}

QList<vtkProp*> IDOSGridBoundaryRenderProvider::props() const
{
    QList<vtkProp*> result;
#ifdef IDOS_ENABLE_RENDER
    if (m_actor != nullptr)
    {
        result.append(m_actor.GetPointer());
    }
#endif
    return result;
}


bool IDOSGridBoundaryRenderProvider::isVisible() const
{
    return m_visible;
}

void IDOSGridBoundaryRenderProvider::setVisible(bool visible)
{
    m_visible = visible;
    applyActorProperties();
}

bool IDOSGridBoundaryRenderProvider::isSelected() const
{
    return m_highlighted;
}

void IDOSGridBoundaryRenderProvider::setSelected(bool highlighted)
{
    m_highlighted = highlighted;
    applyActorProperties();
}

double IDOSGridBoundaryRenderProvider::opacity() const
{
    return m_opacity;
}

void IDOSGridBoundaryRenderProvider::setOpacity(double opacity)
{
    m_opacity = qBound(0.0, opacity, 1.0);
    applyActorProperties();
}

void IDOSGridBoundaryRenderProvider::applyActorProperties()
{
#ifdef IDOS_ENABLE_RENDER
    if (m_actor == nullptr)
    {
        return;
    }
    m_actor->SetVisibility(m_visible ? 1 : 0);
    m_actor->GetProperty()->SetColor(m_highlighted ? 1.0 : 0.96,
                                     m_highlighted ? 0.85 : 0.55,
                                     m_highlighted ? 0.0 : 0.15);
    m_actor->GetProperty()->SetLineWidth(m_highlighted ? 3.0 : 1.5);
    m_actor->GetProperty()->SetOpacity(m_opacity);
#endif
}

QVariantMap IDOSGridBoundaryRenderProvider::saveState() const
{
    QVariantMap state;
    state.insert(QStringLiteral("visible"), isVisible());
    state.insert(QStringLiteral("highlighted"), isSelected());
    state.insert(QStringLiteral("opacity"), opacity());
    return state;
}

bool IDOSGridBoundaryRenderProvider::restoreState(const QVariantMap& state)
{
    bool validOpacity = false;
    const double opacityValue = state.value(QStringLiteral("opacity")).toDouble(&validOpacity);
    if (!validOpacity || !(opacityValue >= 0.0 && opacityValue <= 1.0) ||
        state.value(QStringLiteral("visible")).type() != QVariant::Bool)
    {
        return false;
    }
    setVisible(state.value(QStringLiteral("visible")).toBool());
    setSelected(state.value(QStringLiteral("highlighted")).toBool());
    setOpacity(opacityValue);
    return true;
}
