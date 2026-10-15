#ifndef IDOS_NATURAL_FRACTURE_SET_H
#define IDOS_NATURAL_FRACTURE_SET_H

#include <QVariantList>
#include <QVariantMap>
#include <QVector>

#include "idosdataobject.h"

class CORE_EXPORT IDOSNaturalFractureSet : public IDOSDataObject
{
    Q_OBJECT

  public:
    QVariantMap projectData() const override;
    bool restoreProjectData(const QVariantMap& data, QString& error) override;

    explicit IDOSNaturalFractureSet(QObject* parent = nullptr);

    QString typeId() const override;
    bool canClone() const override;
    IDOSDataObject* clone() const override;
    QString sourceFile() const;
    void setSourceFile(const QString& sourceFile);
    QString lengthUnit() const;
    void setLengthUnit(const QString& lengthUnit);
    double coordinateScale() const;
    void setCoordinateScale(double coordinateScale);
    QVariantList fractures() const;
    void setFractures(const QVariantList& fractures);
    QVariantList geometryReport() const;

  private:
    QVariantMap inspectFracture(int index) const;
    void appendGeometryIssue(QVariantList* issues, const QString& code,
                             const QString& message, bool error) const;
    double projectedOrientation(const QVector<double>& coordinates, int first,
                                int second, int third, int xAxis, int yAxis) const;
    bool edgesIntersect(const QVector<double>& coordinates, int first, int second,
                        int xAxis, int yAxis, double lengthTolerance, double areaTolerance) const;
    QString m_sourceFile;
    QString m_lengthUnit;
    double m_coordinateScale;
    QVariantList m_fractures;
};

#endif // IDOS_NATURAL_FRACTURE_SET_H
