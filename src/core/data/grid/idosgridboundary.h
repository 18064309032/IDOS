#ifndef IDOS_GRID_BOUNDARY_H
#define IDOS_GRID_BOUNDARY_H

#include <QString>
#include <QVariantMap>
#include <QVector>

#include "idosdataobject.h"

class CORE_EXPORT IDOSGridBoundary : public IDOSDataObject
{
    Q_OBJECT

  public:
    bool validateProjectReferences(const IDOSProject* project, QString& error) const override;
    QVariantMap projectData() const override;
    bool restoreProjectData(const QVariantMap& data, QString& error) override;

    explicit IDOSGridBoundary(QObject* parent = nullptr);

    QString typeId() const override;
    bool canClone() const override;
    IDOSDataObject* clone() const override;
    QString sourceFile() const;
    void setSourceFile(const QString& sourceFile);
    QVector<double> xCoordinates() const;
    void setXCoordinates(const QVector<double>& xCoordinates);
    QVector<double> yCoordinates() const;
    void setYCoordinates(const QVector<double>& yCoordinates);
    QVector<double> zCoordinates() const;
    void setZCoordinates(const QVector<double>& zCoordinates);
    QVector<int> groupIds() const;
    void setGroupIds(const QVector<int>& groupIds);
    QString coordinateReferenceSystem() const;
    void setCoordinateReferenceSystem(const QString& coordinateReferenceSystem);

  private:
    QString m_sourceFile;
    QVector<double> m_xCoordinates;
    QVector<double> m_yCoordinates;
    QVector<double> m_zCoordinates;
    QVector<int> m_groupIds;
    QString m_coordinateReferenceSystem;
};

#endif // IDOS_GRID_BOUNDARY_H
