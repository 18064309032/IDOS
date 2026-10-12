#include <QList>
#include <QObject>
#include <QtGlobal>
#include <QVector>
#include <QVector3D>

#ifdef IDOS_ENABLE_RENDER
#include <vtkActor.h>
#include <vtkCellData.h>
#include <vtkDataSetMapper.h>
#include <vtkDoubleArray.h>
#include <vtkHexahedron.h>
#include <vtkLookupTable.h>
#include <vtkPoints.h>
#include <vtkProp.h>
#include <vtkProperty.h>
#include <vtkScalarBarActor.h>
#include <vtkSmartPointer.h>
#include <vtkTextProperty.h>
#include <vtkUnstructuredGrid.h>
#endif

#include "idosgrid.h"
#include "idosgridproperty.h"

#include "idosgridrenderprovider.h"

IDOSGridRenderProvider::IDOSGridRenderProvider(IDOSGrid* dataObject)
    :
    m_dataObject(dataObject)
    , m_property(nullptr)
    , m_displayMode(IDOSDisplayMode::Surface)
    , m_actors()
    , m_legends()
    , m_highlighted(false)
    , m_visible(dataObject != nullptr && dataObject->isVisible())
    , m_opacity(1.0)
{
}

IDOSGridRenderProvider::~IDOSGridRenderProvider()
{
}

IDOSDataObject* IDOSGridRenderProvider::dataSource() const
{
    return m_dataObject.data();
}

void IDOSGridRenderProvider::update()
{
    m_actors.clear();
    m_legends.clear();
    if (m_dataObject.isNull())
    {
        return;
    }
#ifdef IDOS_ENABLE_RENDER
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkUnstructuredGrid> grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    grid->SetPoints(points);
    QVector<double> values;
    const IDOSGrid* sourceGrid = m_dataObject.data();
    const bool hasScalars = !m_property.isNull() && !m_property->values().isEmpty();
    for (int k = 0; k < sourceGrid->nz(); ++k)
    {
        for (int j = 0; j < sourceGrid->ny(); ++j)
        {
            for (int i = 0; i < sourceGrid->nx(); ++i)
            {
                if (!sourceGrid->isActive(i, j, k))
                {
                    continue;
                }
                vtkIdType pointIds[8];
                for (int corner = 0; corner < 8; ++corner)
                {
                    const QVector3D point = sourceGrid->cornerPosition(i, j, k, corner);
                    pointIds[corner] = points->InsertNextPoint(point.x(), point.y(), point.z());
                }
                grid->InsertNextCell(VTK_HEXAHEDRON, 8, pointIds);
                if (hasScalars)
                {
                    const int globalIndex = IDOSGridProperty::cellIndex(i, j, k, sourceGrid->nx(), sourceGrid->ny());
                    values.append(globalIndex < m_property->values().size() ? m_property->values().at(globalIndex)
                                                                            : 0.0);
                }
            }
        }
    }
    const bool hasCellScalars = hasScalars && !values.isEmpty();
    vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
    mapper->SetInputData(grid);
    if (!hasCellScalars)
    {
        mapper->ScalarVisibilityOff();
    }
    else
    {
        vtkSmartPointer<vtkDoubleArray> scalars = vtkSmartPointer<vtkDoubleArray>::New();
        scalars->SetName(m_property->name().toUtf8().constData());
        double minimumValue = values.first();
        double maximumValue = values.first();
        for (int index = 0; index < values.size(); ++index)
        {
            const double value = values.at(index);
            scalars->InsertNextValue(value);
            minimumValue = qMin(minimumValue, value);
            maximumValue = qMax(maximumValue, value);
        }
        if (minimumValue == maximumValue)
        {
            maximumValue = minimumValue + 1.0;
        }
        grid->GetCellData()->SetScalars(scalars);
        vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
        lookupTable->SetHueRange(0.667, 0.0);
        lookupTable->SetSaturationRange(0.85, 0.85);
        lookupTable->SetValueRange(0.95, 0.95);
        lookupTable->SetRange(minimumValue, maximumValue);
        lookupTable->Build();
        mapper->SetLookupTable(lookupTable);
        mapper->SetScalarRange(minimumValue, maximumValue);
        mapper->SetScalarModeToUseCellData();
        mapper->SetInterpolateScalarsBeforeMapping(false);
        mapper->ScalarVisibilityOn();
    }

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetOpacity(m_opacity);
    actor->GetProperty()->SetInterpolationToFlat();
    actor->GetProperty()->SetLineWidth(1.0);
    m_actors.append(actor);
    if (hasCellScalars)
    {
        vtkSmartPointer<vtkScalarBarActor> scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
        scalarBar->SetLookupTable(mapper->GetLookupTable());
        scalarBar->SetTitle(m_property->name().toUtf8().constData());
        scalarBar->SetNumberOfLabels(5);
        scalarBar->SetPosition(0.92, 0.32);
        scalarBar->SetWidth(0.05);
        scalarBar->SetHeight(0.36);
        scalarBar->SetUnconstrainedFontSize(true);
        scalarBar->GetTitleTextProperty()->SetColor(1.0, 1.0, 1.0);
        scalarBar->GetTitleTextProperty()->SetFontSize(18);
        scalarBar->GetLabelTextProperty()->SetColor(1.0, 1.0, 1.0);
        scalarBar->GetLabelTextProperty()->SetFontSize(14);
        m_legends.append(scalarBar);
    }
#endif
    applyActorProperties();
}

QList<vtkActor*> IDOSGridRenderProvider::actors() const
{
    QList<vtkActor*> result;
#ifdef IDOS_ENABLE_RENDER
    for (int index = 0; index < m_actors.size(); ++index)
    {
        result.append(m_actors.at(index).GetPointer());
    }
#endif
    return result;
}

QList<vtkSmartPointer<vtkProp>> IDOSGridRenderProvider::legends() const
{
    return m_visible ? m_legends : QList<vtkSmartPointer<vtkProp>>();
}

void IDOSGridRenderProvider::setHighlighted(bool highlighted)
{
    m_highlighted = highlighted;
    applyActorProperties();
}

bool IDOSGridRenderProvider::visible() const
{
    return m_visible;
}

void IDOSGridRenderProvider::setVisible(bool visible)
{
    m_visible = visible;
    applyActorProperties();
}

double IDOSGridRenderProvider::opacity() const
{
    return m_opacity;
}

void IDOSGridRenderProvider::setOpacity(double opacity)
{
    m_opacity = qBound(0.0, opacity, 1.0);
    applyActorProperties();
}

void IDOSGridRenderProvider::setProperty(IDOSGridProperty* property)
{
    m_property = property;
    update();
}

IDOSGridProperty* IDOSGridRenderProvider::property() const
{
    return m_property.data();
}

IDOSDisplayMode IDOSGridRenderProvider::displayMode() const
{
    return m_displayMode;
}

bool IDOSGridRenderProvider::setDisplayMode(IDOSDisplayMode mode)
{
    m_displayMode = mode;
    applyActorProperties();
    return true;
}

bool IDOSGridRenderProvider::highlighted() const
{
    return m_highlighted;
}

void IDOSGridRenderProvider::applyActorProperties()
{
#ifdef IDOS_ENABLE_RENDER
    for (int index = 0; index < m_actors.size(); ++index)
    {
        vtkActor* actor = m_actors.at(index).GetPointer();
        switch (m_displayMode)
        {
        case IDOSDisplayMode::Surface:
            actor->GetProperty()->SetRepresentationToSurface();
            actor->GetProperty()->EdgeVisibilityOff();
            break;
        case IDOSDisplayMode::Wireframe:
            actor->GetProperty()->SetRepresentationToWireframe();
            actor->GetProperty()->EdgeVisibilityOn();
            break;
        case IDOSDisplayMode::SurfaceWithEdges:
            actor->GetProperty()->SetRepresentationToSurface();
            actor->GetProperty()->EdgeVisibilityOn();
            break;
        case IDOSDisplayMode::Points:
            actor->GetProperty()->SetRepresentationToPoints();
            actor->GetProperty()->EdgeVisibilityOff();
            break;
        }
        if (actor->GetMapper()->GetScalarVisibility() == 0)
        {
            actor->GetProperty()->SetColor(0.35, 0.72, 1.0);
        }
        actor->GetProperty()->SetEdgeColor(m_highlighted ? 1.0 : 0.0, m_highlighted ? 0.95 : 0.0,
                                           m_highlighted ? 0.15 : 0.0);
        actor->GetProperty()->SetLineWidth(m_highlighted ? 3.0 : 1.0);
        if (m_highlighted && m_displayMode != IDOSDisplayMode::Points)
        {
            actor->GetProperty()->EdgeVisibilityOn();
        }
        actor->GetProperty()->SetOpacity(m_opacity);
        actor->SetVisibility(m_visible);
    }
    for (int index = 0; index < m_legends.size(); ++index)
    {
        m_legends.at(index)->SetVisibility(m_visible);
    }
#endif
}
