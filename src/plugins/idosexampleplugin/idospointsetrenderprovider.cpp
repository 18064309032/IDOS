#include <QVector>
#include <QVector3D>

#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp.h>
#include <vtkProperty.h>

#include "idospointsetobject.h"

#include "idospointsetrenderprovider.h"

IDOSPointSetRenderProvider::IDOSPointSetRenderProvider(IDOSPointSetObject* data)
    :
    m_data(data)
    , m_actor(vtkSmartPointer<vtkActor>::New())
    , m_polyData(vtkSmartPointer<vtkPolyData>::New())
    , m_highlighted(false)
{
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(m_polyData);
    m_actor->SetMapper(mapper);
    m_actor->GetProperty()->SetColor(0.2, 0.8, 0.9);
    m_actor->GetProperty()->SetPointSize(9.0);
    m_actor->SetVisibility(false);
    update();
}

IDOSPointSetRenderProvider::~IDOSPointSetRenderProvider()
{
}

IDOSPointSetObject* IDOSPointSetRenderProvider::pointSet() const
{
    return m_data.data();
}

void IDOSPointSetRenderProvider::update()
{
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
    if (m_data != nullptr)
    {
        for (const QVector3D& point : m_data->points())
        {
            const vtkIdType id = points->InsertNextPoint(point.x(), point.y(), point.z());
            vertices->InsertNextCell(1, &id);
        }
    }
    m_polyData->SetPoints(points);
    m_polyData->SetVerts(vertices);
    m_polyData->Modified();
}

QList<vtkProp*> IDOSPointSetRenderProvider::props() const
{
    QList<vtkProp*> result;
    if (m_actor != nullptr)
    {
        result.append(m_actor.GetPointer());
    }
    return result;
}


bool IDOSPointSetRenderProvider::isVisible() const
{
    return m_actor->GetVisibility() != 0;
}

void IDOSPointSetRenderProvider::setVisible(bool visible)
{
    m_actor->SetVisibility(visible);
}

bool IDOSPointSetRenderProvider::isSelected() const
{
    return m_highlighted;
}

void IDOSPointSetRenderProvider::setSelected(bool highlighted)
{
    m_highlighted = highlighted;
    if (highlighted)
    {
        m_actor->GetProperty()->SetColor(1.0, 0.8, 0.2);
    }
    else
    {
        m_actor->GetProperty()->SetColor(0.2, 0.8, 0.9);
    }
}

double IDOSPointSetRenderProvider::opacity() const
{
    return m_actor->GetProperty()->GetOpacity();
}

void IDOSPointSetRenderProvider::setOpacity(double opacity)
{
    m_actor->GetProperty()->SetOpacity(opacity);
}

QVariantMap IDOSPointSetRenderProvider::saveState() const
{
    QVariantMap state;
    state.insert(QStringLiteral("visible"), isVisible());
    state.insert(QStringLiteral("highlighted"), isSelected());
    state.insert(QStringLiteral("opacity"), opacity());
    return state;
}

bool IDOSPointSetRenderProvider::restoreState(const QVariantMap& state)
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
