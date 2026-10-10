#ifndef IDOS_GRID_RENDER_OBJECT_PROVIDER_H
#define IDOS_GRID_RENDER_OBJECT_PROVIDER_H

#include <QHash>
#include <QMap>
#include <QString>

#include "idosrenderobjectprovider.h"

class IDOSGrid;
class IDOSGridProperty;
class IDOSRenderMesh;
class QVector3D;

class APP_EXPORT IDOSGridRenderObjectProvider : public IDOSRenderObjectProvider
{
  public:
    IDOSGridRenderObjectProvider();
    ~IDOSGridRenderObjectProvider() override;

    QString dataTypeId() const override;
    IDOSRenderObject* createRenderObject(IDOSDataObject* object) const override;
    bool canConvert(const IDOSRenderObject* object) const override;
    QList<vtkActor*> toVtk(const IDOSRenderObject* object,
                           bool highlighted) const override;
    QList<vtkSmartPointer<vtkProp>> toVtkLegends(const IDOSRenderObject* object) const override;
    bool synchronize(IDOSRenderObjectChange change,
                     const QStringList& objectIds,
                     IDOSRenderProviderContext* context,
                     bool* resetCamera) override;
    bool canSetDisplayMode(const IDOSRenderObject* object) const override;
    bool setDisplayMode(IDOSRenderObject* object, IDOSDisplayMode mode) const override;
    IDOSDisplayMode displayMode(const IDOSRenderObject* object) const override;

  private:
    bool synchronizeGridProperty(const IDOSRenderProviderContext* context, const QString& gridId) const;
    bool applyGridProperty(IDOSRenderObject* renderObject, const IDOSGridProperty* property) const;
    IDOSRenderMesh* createMesh(const IDOSGrid* grid) const;
    void appendCell(IDOSRenderMesh* mesh, const IDOSGrid* grid, int i, int j, int k, QVector<double>* cellScalars,
                    const QVector<double>* propertyValues, QHash<QString, int>* pointMap) const;
    int appendPoint(IDOSRenderMesh* mesh, const QVector3D& point, QHash<QString, int>* pointMap) const;
    QString pointKey(const QVector3D& point) const;

    QMap<QString, QString> m_propertyGridIds;
};

#endif // IDOS_GRID_RENDER_OBJECT_PROVIDER_H

