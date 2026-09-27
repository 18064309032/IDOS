#ifndef IDOS_RENDER_MESH_H
#define IDOS_RENDER_MESH_H

#include "idosrenderobject.h"

#include <QString>
#include <QVector>
#include <QVector3D>

class RENDER_EXPORT IDOSRenderMesh : public IDOSRenderObject
{
  public:
    IDOSRenderMesh();
    ~IDOSRenderMesh() override;

    const QVector<QVector3D>& points() const;
    void setPoints(const QVector<QVector3D>& points);

    const QVector<int>& hexahedra() const;
    void setHexahedra(const QVector<int>& hexahedra);

    int addPoint(const QVector3D& point);
    void addHexahedron(int point0, int point1, int point2, int point3, int point4, int point5, int point6, int point7);
    void addHexahedron(int point0, int point1, int point2, int point3, int point4, int point5, int point6, int point7,
                       int globalIndex);

    int hexahedronCount() const;
    const QVector<int>& cellGlobalIndices() const;

    QString cellScalarName() const;
    const QVector<double>& cellScalars() const;
    void setCellScalars(const QString& name, const QVector<double>& values);
    bool hasCellScalars() const;

    void clear();

  private:
    QVector<QVector3D> m_points;
    QVector<int> m_hexahedra;
    QVector<int> m_cellGlobalIndices;
    QString m_cellScalarName;
    QVector<double> m_cellScalars;
};

#endif // IDOS_RENDER_MESH_H

