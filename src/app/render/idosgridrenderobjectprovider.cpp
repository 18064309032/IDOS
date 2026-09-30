#include "idosgridrenderobjectprovider.h"
#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idosrendermesh.h"

#include <QObject>
#include <QVector3D>

IDOSGridRenderObjectProvider::IDOSGridRenderObjectProvider()
{
}

IDOSGridRenderObjectProvider::~IDOSGridRenderObjectProvider()
{
}

QString IDOSGridRenderObjectProvider::providerId() const
{
    return QStringLiteral("idos.render.grid");
}

QString IDOSGridRenderObjectProvider::displayName() const
{
    return QObject::tr("Grid Render Object Provider");
}

bool IDOSGridRenderObjectProvider::canCreate(const IDOSDataObject* object) const
{
    return qobject_cast<const IDOSGrid*>(object) != nullptr;
}

IDOSRenderObject* IDOSGridRenderObjectProvider::createObject(const IDOSDataObject* object) const
{
    const IDOSGrid* grid = qobject_cast<const IDOSGrid*>(object);
    if (grid != nullptr)
    {
        return createMesh(grid);
    }

    Q_UNUSED(object);
    return nullptr;
}

IDOSRenderObject* IDOSGridRenderObjectProvider::createObject(const IDOSGrid* grid,
                                                             const IDOSGridProperty* property) const
{
    if (grid == nullptr || property == nullptr || property->gridId() != grid->objectId())
    {
        return nullptr;
    }

    IDOSRenderMesh* mesh = new IDOSRenderMesh();
    mesh->setId(property->objectId());
    mesh->setName(QStringLiteral("%1 - %2").arg(grid->name(), property->name()));
    QVector<double> cellScalars;
    const QVector<double>& values = property->values();
    const bool showAllCells = property->keyword().compare(QStringLiteral("ACTNUM"), Qt::CaseInsensitive) == 0;
    for (int k = 0; k < grid->nz(); ++k)
    {
        for (int j = 0; j < grid->ny(); ++j)
        {
            for (int i = 0; i < grid->nx(); ++i)
            {
                if (showAllCells || grid->isActive(i, j, k))
                {
                    appendCell(mesh, grid, i, j, k, &cellScalars, &values, nullptr);
                }
            }
        }
    }
    mesh->setCellScalars(property->name(), cellScalars);
    return mesh;
}

IDOSRenderMesh* IDOSGridRenderObjectProvider::createMesh(const IDOSGrid* grid) const
{
    IDOSRenderMesh* mesh = new IDOSRenderMesh();
    mesh->setId(grid->objectId());
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



