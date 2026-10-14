#ifndef IDOS_GRID_SURFACE_H
#define IDOS_GRID_SURFACE_H

#include <QString>
#include <QVector>

#include "idosdataobject.h"

class CORE_EXPORT IDOSGridSurface : public IDOSDataObject
{
    Q_OBJECT

  public:
    explicit IDOSGridSurface(QObject* parent = nullptr);

    QString typeId() const override;
    QString sourceFile() const;
    void setSourceFile(const QString& sourceFile);
    int nx() const;
    void setNx(int nx);
    int ny() const;
    void setNy(int ny);
    double originX() const;
    void setOriginX(double originX);
    double originY() const;
    void setOriginY(double originY);
    double spacingX() const;
    void setSpacingX(double spacingX);
    double spacingY() const;
    void setSpacingY(double spacingY);
    double missingValue() const;
    void setMissingValue(double missingValue);
    QVector<double> values() const;
    void setValues(const QVector<double>& values);
    QString coordinateReferenceSystem() const;
    void setCoordinateReferenceSystem(const QString& coordinateReferenceSystem);

  private:
    QString m_sourceFile;
    int m_nx;
    int m_ny;
    double m_originX;
    double m_originY;
    double m_spacingX;
    double m_spacingY;
    double m_missingValue;
    QVector<double> m_values;
    QString m_coordinateReferenceSystem;
};

#endif // IDOS_GRID_SURFACE_H
