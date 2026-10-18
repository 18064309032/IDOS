#ifndef IDOS_POINT_SET_OBJECT_H
#define IDOS_POINT_SET_OBJECT_H

#include <QVector>
#include <QVector3D>

#include "idosdataobject.h"

class IDOSPointSetObject : public IDOSDataObject
{
    Q_OBJECT

  public:
    explicit IDOSPointSetObject(QObject* parent = nullptr);
    QString typeId() const override;
    QVariantMap projectData() const override;
    bool restoreProjectData(const QVariantMap& data, QString& error) override;
    QVector<QVector3D> points() const;
    void setPoints(const QVector<QVector3D>& points);

  private:
    QVector<QVector3D> m_points;
};

#endif
