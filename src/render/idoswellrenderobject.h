#ifndef IDOS_WELL_RENDER_OBJECT_H
#define IDOS_WELL_RENDER_OBJECT_H

#include "idos_render.h"
#include "idosrenderobject.h"

#include <QVector>
#include <QVector3D>

/**
 * @brief 井渲染对象。
 *
 * 存储井口位置和可选的井眼轨迹点列（X/Y/Z），由渲染视图层构建为
 * vtkPolyData 井口标记与轨迹折线。
 * 与 IDOSRenderMesh（网格六面体）同级，均继承 IDOSRenderObject。
 */
class RENDER_EXPORT IDOSWellRenderObject : public IDOSRenderObject
{
  public:
    IDOSWellRenderObject();
    ~IDOSWellRenderObject() override;

    bool hasWellHead() const;
    QVector3D wellHeadPosition() const;
    void setWellHeadPosition(const QVector3D& position);

    const QVector<QVector3D>& points() const;
    void setPoints(const QVector<QVector3D>& points);
    void appendPoint(const QVector3D& point);

    int pointCount() const;
    bool isEmpty() const;
    void clear();

  private:
    QVector3D m_wellHeadPosition;
    bool m_hasWellHead;
    QVector<QVector3D> m_points;
};

#endif // IDOS_WELL_RENDER_OBJECT_H
