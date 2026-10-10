#include <QList>
#include <QObject>
#include <QUuid>
#include <QtGlobal>
#include <QVector3D>

#ifdef IDOS_ENABLE_RENDER
#include <vtkActor.h>
#include <vtkCellData.h>
#include <vtkDataSetMapper.h>
#include <vtkDoubleArray.h>
#include <vtkHexahedron.h>
#include <vtkLookupTable.h>
#include <vtkPoints.h>
#include <vtkProperty.h>
#include <vtkScalarBarActor.h>
#include <vtkSmartPointer.h>
#include <vtkTextProperty.h>
#include <vtkUnstructuredGrid.h>
#endif

#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idosrendermesh.h"
#include "idosrenderobject.h"
#include "idosrenderprovider.h"

#include "idosgridrenderobjectprovider.h"

IDOSGridRenderObjectProvider::IDOSGridRenderObjectProvider()
{
}

IDOSGridRenderObjectProvider::~IDOSGridRenderObjectProvider()
{
}

QString IDOSGridRenderObjectProvider::dataTypeId() const
{
    return QStringLiteral("idos.grid");
}

IDOSRenderObject* IDOSGridRenderObjectProvider::createRenderObject(IDOSDataObject* object) const
{
    const IDOSGrid* grid = qobject_cast<const IDOSGrid*>(object);
    if (grid != nullptr)
    {
        IDOSRenderMesh* renderMesh = createMesh(grid);
        renderMesh->setRenderProvider(createRenderProvider(object, renderMesh));
        return renderMesh;
    }

    return nullptr;
}

bool IDOSGridRenderObjectProvider::canConvert(const IDOSRenderObject* object) const
{
    return dynamic_cast<const IDOSRenderMesh*>(object) != nullptr;
}

QList<vtkActor*> IDOSGridRenderObjectProvider::toVtk(const IDOSRenderObject* object,
                                                     bool highlighted) const
{
    QList<vtkActor*> actors;
#ifdef IDOS_ENABLE_RENDER
    Q_UNUSED(highlighted);
    const IDOSRenderMesh* mesh = dynamic_cast<const IDOSRenderMesh*>(object);
    const IDOSRenderProvider* renderProvider = mesh != nullptr ? mesh->renderProvider() : nullptr;
    if (mesh == nullptr || renderProvider == nullptr || !renderProvider->visible())
    {
        return actors;
    }

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    const QVector<QVector3D>& meshPoints = mesh->points();
    for (int pointIndex = 0; pointIndex < meshPoints.size(); ++pointIndex)
    {
        const QVector3D& point = meshPoints.at(pointIndex);
        points->InsertNextPoint(point.x(), point.y(), point.z());
    }

    vtkSmartPointer<vtkUnstructuredGrid> grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    grid->SetPoints(points);
    const QVector<int>& hexahedra = mesh->hexahedra();
    for (int cellIndex = 0; cellIndex + 7 < hexahedra.size(); cellIndex += 8)
    {
        vtkIdType pointIds[8];
        for (int corner = 0; corner < 8; ++corner)
        {
            pointIds[corner] = static_cast<vtkIdType>(hexahedra.at(cellIndex + corner));
        }
        grid->InsertNextCell(VTK_HEXAHEDRON, 8, pointIds);
    }

    vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
    mapper->SetInputData(grid);
    if (!mesh->hasCellScalars())
    {
        mapper->ScalarVisibilityOff();
    }
    else
    {
        vtkSmartPointer<vtkDoubleArray> scalars = vtkSmartPointer<vtkDoubleArray>::New();
        scalars->SetName(mesh->cellScalarName().toUtf8().constData());
        const QVector<double>& values = mesh->cellScalars();
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
    actor->GetProperty()->SetOpacity(renderProvider->opacity());
    actor->GetProperty()->SetInterpolationToFlat();
    actor->GetProperty()->SetLineWidth(1.0);
    switch (mesh->displayMode())
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
    if (!mesh->hasCellScalars())
    {
        actor->GetProperty()->SetColor(0.35, 0.72, 1.0);
    }
    actors.append(actor.GetPointer());
#else
    Q_UNUSED(object);
    Q_UNUSED(highlighted);
#endif
    return actors;
}

QList<vtkSmartPointer<vtkProp>> IDOSGridRenderObjectProvider::toVtkLegends(
    const IDOSRenderObject* object) const
{
    QList<vtkSmartPointer<vtkProp>> legends;
#ifdef IDOS_ENABLE_RENDER
    const IDOSRenderMesh* mesh = dynamic_cast<const IDOSRenderMesh*>(object);
    const IDOSRenderProvider* renderProvider = mesh != nullptr ? mesh->renderProvider() : nullptr;
    if (mesh == nullptr || renderProvider == nullptr || !renderProvider->visible() ||
        !mesh->hasCellScalars())
    {
        return legends;
    }

    const QVector<double>& values = mesh->cellScalars();
    double minimumValue = values.first();
    double maximumValue = values.first();
    for (int index = 1; index < values.size(); ++index)
    {
        minimumValue = qMin(minimumValue, values.at(index));
        maximumValue = qMax(maximumValue, values.at(index));
    }
    if (minimumValue == maximumValue)
    {
        maximumValue = minimumValue + 1.0;
    }

    vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    lookupTable->SetHueRange(0.667, 0.0);
    lookupTable->SetSaturationRange(0.85, 0.85);
    lookupTable->SetValueRange(0.95, 0.95);
    lookupTable->SetRange(minimumValue, maximumValue);
    lookupTable->Build();

    vtkSmartPointer<vtkScalarBarActor> scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
    scalarBar->SetLookupTable(lookupTable);
    scalarBar->SetTitle(mesh->cellScalarName().toUtf8().constData());
    scalarBar->SetNumberOfLabels(5);
    scalarBar->SetPosition(0.85, 0.15);
    scalarBar->SetWidth(0.12);
    scalarBar->SetHeight(0.7);
    scalarBar->GetTitleTextProperty()->SetColor(1.0, 1.0, 1.0);
    scalarBar->GetLabelTextProperty()->SetColor(1.0, 1.0, 1.0);
    vtkSmartPointer<vtkProp> legend = scalarBar;
    legends.append(legend);
#else
    Q_UNUSED(object);
#endif
    return legends;
}

bool IDOSGridRenderObjectProvider::synchronize(IDOSRenderObjectChange change,
                                              const QStringList& objectIds,
                                              IDOSRenderProviderContext* context,
                                              bool* resetCamera)
{
    if (change == IDOSRenderObjectChange::ProjectReplaced)
    {
        m_propertyGridIds.clear();
        if (context != nullptr)
        {
        const QList<IDOSDataObject*> objects = context->dataObjects();
        for (IDOSDataObject* object : objects)
            {
                const IDOSGridProperty* property = qobject_cast<const IDOSGridProperty*>(object);
                if (property != nullptr)
                {
                    m_propertyGridIds.insert(property->objectId(), property->gridId());
                }
            }
        }

        bool changed = IDOSRenderObjectProvider::synchronize(change, objectIds, context, resetCamera);
        if (context != nullptr)
        {
            const QList<IDOSDataObject*> objects = context->dataObjects();
            for (IDOSDataObject* object : objects)
            {
                const IDOSGrid* grid = qobject_cast<const IDOSGrid*>(object);
                if (grid != nullptr)
                {
                    changed = synchronizeGridProperty(context, grid->objectId()) || changed;
                }
            }
        }
        return changed;
    }

    if (context == nullptr)
    {
        return false;
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
            const QString gridId = m_propertyGridIds.take(objectId);
            changed = IDOSRenderObjectProvider::synchronize(change, QStringList() << objectId,
                                                            context, resetCamera) || changed;
            if (!gridId.isEmpty())
            {
                changed = synchronizeGridProperty(context, gridId) || changed;
            }
            QMap<QString, QString>::iterator propertyIterator = m_propertyGridIds.begin();
            while (propertyIterator != m_propertyGridIds.end())
            {
                if (propertyIterator.value() == objectId)
                {
                    propertyIterator = m_propertyGridIds.erase(propertyIterator);
                }
                else
                {
                    ++propertyIterator;
                }
            }
            continue;
        }

        IDOSDataObject* object = context->dataObject(objectId);
        const IDOSGrid* grid = qobject_cast<const IDOSGrid*>(object);
        const IDOSGridProperty* property = qobject_cast<const IDOSGridProperty*>(object);
        if (property != nullptr)
        {
            m_propertyGridIds.insert(objectId, property->gridId());
            changed = synchronizeGridProperty(context, property->gridId()) || changed;
        }
        else if (grid != nullptr)
        {
            changed = IDOSRenderObjectProvider::synchronize(change, QStringList() << objectId,
                                                            context, resetCamera) || changed;
            changed = synchronizeGridProperty(context, grid->objectId()) || changed;
        }
        else
        {
            changed = IDOSRenderObjectProvider::synchronize(change, QStringList() << objectId,
                                                            context, resetCamera) || changed;
        }
    }
    return changed;
}

bool IDOSGridRenderObjectProvider::canSetDisplayMode(const IDOSRenderObject* object) const
{
    return dynamic_cast<const IDOSRenderMesh*>(object) != nullptr;
}

bool IDOSGridRenderObjectProvider::setDisplayMode(IDOSRenderObject* object, IDOSDisplayMode mode) const
{
    IDOSRenderMesh* mesh = dynamic_cast<IDOSRenderMesh*>(object);
    if (mesh == nullptr)
    {
        return false;
    }
    if (mesh->displayMode() == mode)
    {
        return true;
    }
    mesh->setDisplayMode(mode);
    return true;
}

IDOSDisplayMode IDOSGridRenderObjectProvider::displayMode(const IDOSRenderObject* object) const
{
    const IDOSRenderMesh* mesh = dynamic_cast<const IDOSRenderMesh*>(object);
    return mesh != nullptr ? mesh->displayMode() : IDOSDisplayMode::Surface;
}

bool IDOSGridRenderObjectProvider::synchronizeGridProperty(const IDOSRenderProviderContext* context,
                                                           const QString& gridId) const
{
    if (context == nullptr || gridId.isEmpty())
    {
        return false;
    }

    const IDOSGridProperty* visibleProperty = nullptr;
    const QList<IDOSDataObject*> objects = context->dataObjects();
    for (IDOSDataObject* object : objects)
    {
        const IDOSGridProperty* property = qobject_cast<const IDOSGridProperty*>(object);
        if (property != nullptr && property->isVisible() && property->gridId() == gridId)
        {
            visibleProperty = property;
            break;
        }
    }

    IDOSRenderObject* renderObject = context->renderObject(gridId);
    IDOSRenderMesh* mesh = dynamic_cast<IDOSRenderMesh*>(renderObject);
    if (mesh == nullptr)
    {
        return false;
    }

    if (visibleProperty == nullptr)
    {
        mesh->clearCellScalars();
        return true;
    }
    return applyGridProperty(mesh, visibleProperty);
}

bool IDOSGridRenderObjectProvider::applyGridProperty(IDOSRenderObject* renderObject,
                                                     const IDOSGridProperty* property) const
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
        cellScalars.append(globalIndex >= 0 && globalIndex < values.size() ? values.at(globalIndex) : 0.0);
    }
    mesh->setCellScalars(property->name(), cellScalars);
    return true;
}

IDOSRenderMesh* IDOSGridRenderObjectProvider::createMesh(const IDOSGrid* grid) const
{
    IDOSRenderMesh* mesh = new IDOSRenderMesh();
    mesh->setId(QUuid::createUuid().toString(QUuid::WithoutBraces));
    mesh->setName(grid->name());

    for (int k = 0; k < grid->nz(); ++k)
    {
        for (int j = 0; j < grid->ny(); ++j)
        {
            for (int i = 0; i < grid->nx(); ++i)
            {
                if (grid->isActive(i, j, k))
                {
                    appendCell(mesh, grid, i, j, k, nullptr, nullptr, nullptr);
                }
            }
        }
    }

    return mesh;
}

void IDOSGridRenderObjectProvider::appendCell(IDOSRenderMesh* mesh, const IDOSGrid* grid, int i, int j, int k,
                                             QVector<double>* cellScalars, const QVector<double>* propertyValues,
                                             QHash<QString, int>* pointMap) const
{
    const int point0 = appendPoint(mesh, grid->cornerPosition(i, j, k, 0), pointMap);
    const int point1 = appendPoint(mesh, grid->cornerPosition(i, j, k, 1), pointMap);
    const int point2 = appendPoint(mesh, grid->cornerPosition(i, j, k, 2), pointMap);
    const int point3 = appendPoint(mesh, grid->cornerPosition(i, j, k, 3), pointMap);
    const int point4 = appendPoint(mesh, grid->cornerPosition(i, j, k, 4), pointMap);
    const int point5 = appendPoint(mesh, grid->cornerPosition(i, j, k, 5), pointMap);
    const int point6 = appendPoint(mesh, grid->cornerPosition(i, j, k, 6), pointMap);
    const int point7 = appendPoint(mesh, grid->cornerPosition(i, j, k, 7), pointMap);
    const int globalIndex = IDOSGridProperty::cellIndex(i, j, k, grid->nx(), grid->ny());
    mesh->addHexahedron(point0, point1, point2, point3, point4, point5, point6, point7, globalIndex);
    if (cellScalars != nullptr)
    {
        double scalarValue = 0.0;
        if (propertyValues != nullptr)
        {
            const int cellIndex = globalIndex;
            if (cellIndex >= 0 && cellIndex < propertyValues->size())
            {
                scalarValue = propertyValues->at(cellIndex);
            }
        }
        cellScalars->append(scalarValue);
    }
}

int IDOSGridRenderObjectProvider::appendPoint(IDOSRenderMesh* mesh, const QVector3D& point,
                                             QHash<QString, int>* pointMap) const
{
    if (pointMap == nullptr)
    {
        return mesh->addPoint(point);
    }

    const QString key = pointKey(point);
    if (pointMap->contains(key))
    {
        return pointMap->value(key);
    }

    const int pointIndex = mesh->addPoint(point);
    pointMap->insert(key, pointIndex);
    return pointIndex;
}

QString IDOSGridRenderObjectProvider::pointKey(const QVector3D& point) const
{
    return QStringLiteral("%1,%2,%3")
        .arg(static_cast<double>(point.x()), 0, 'g', 17)
        .arg(static_cast<double>(point.y()), 0, 'g', 17)
        .arg(static_cast<double>(point.z()), 0, 'g', 17);
}



