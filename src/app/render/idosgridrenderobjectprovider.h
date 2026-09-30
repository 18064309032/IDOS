#ifndef IDOS_GRID_RENDER_OBJECT_PROVIDER_H
#define IDOS_GRID_RENDER_OBJECT_PROVIDER_H

#include "idosrenderobjectprovider.h"

#include <QHash>
#include <QString>

class IDOSGrid;
class IDOSGridProperty;
class IDOSRenderMesh;
class QVector3D;

class APP_EXPORT IDOSGridRenderObjectProvider : public IDOSRenderObjectProvider
{
  public:
    IDOSGridRenderObjectProvider();
    ~IDOSGridRenderObjectProvider() override;

    QString providerId() const override;
    QString displayName() const override;
    bool canCreate(const IDOSDataObject* object) const override;
    IDOSRenderObject* createObject(const IDOSDataObject* object) const override;
    IDOSRenderObject* createObject(const IDOSGrid* grid, const IDOSGridProperty* property) const;

  private:
    IDOSRenderMesh* createMesh(const IDOSGrid* grid) const;
    void appendCell(IDOSRenderMesh* mesh, const IDOSGrid* grid, int i, int j, int k, QVector<double>* cellScalars,
                    const QVector<double>* propertyValues, QHash<QString, int>* pointMap) const;
    int appendPoint(IDOSRenderMesh* mesh, const QVector3D& point, QHash<QString, int>* pointMap) const;
    QString pointKey(const QVector3D& point) const;
};

#endif // IDOS_GRID_RENDER_OBJECT_PROVIDER_H

