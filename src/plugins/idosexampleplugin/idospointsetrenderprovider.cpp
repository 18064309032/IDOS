#include <QVector>
#include <QVector3D>

#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp.h>
#include <vtkProperty.h>

#include "idospointset.h"

#include "idospointsetrenderprovider.h"

IDOSPointSetRenderProvider::IDOSPointSetRenderProvider(IDOSPointSet* data)
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
    m_actor->SetVisibility(data != nullptr && data->isVisible());
    update();
}

IDOSPointSetRenderProvider::~IDOSPointSetRenderProvider()
{
}

IDOSDataObject* IDOSPointSetRenderProvider::dataSource() const
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

QList<vtkActor*> IDOSPointSetRenderProvider::actors() const
{
    return QList<vtkActor*>{m_actor.GetPointer()};
}

QList<vtkSmartPointer<vtkProp>> IDOSPointSetRenderProvider::legends() const
{
    return QList<vtkSmartPointer<vtkProp>>();
}

bool IDOSPointSetRenderProvider::visible() const
{
    return m_actor->GetVisibility() != 0;
}

void IDOSPointSetRenderProvider::setVisible(bool visible)
{
    m_actor->SetVisibility(visible);
}

bool IDOSPointSetRenderProvider::highlighted() const
{
    return m_highlighted;
}

void IDOSPointSetRenderProvider::setHighlighted(bool highlighted)
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
