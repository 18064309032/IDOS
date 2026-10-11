#ifndef IDOS_POINT_SET_H
#define IDOS_POINT_SET_H

#include <QVector>
#include <QVector3D>

#include "idosdataobject.h"

class IDOSPointSet : public IDOSDataObject
{
    Q_OBJECT

  public:
    explicit IDOSPointSet(QObject* parent = nullptr);
    QString typeId() const override;
    QVector<QVector3D> points() const;
    void setPoints(const QVector<QVector3D>& points);

  private:
    QVector<QVector3D> m_points;
};

#endif
