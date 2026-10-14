#ifndef IDOS_NATURAL_FRACTURE_SET_H
#define IDOS_NATURAL_FRACTURE_SET_H

#include <QVariantList>
#include <QVariantMap>

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

  private:
    QString m_sourceFile;
    QString m_lengthUnit;
    double m_coordinateScale;
    QVariantList m_fractures;
};

#endif // IDOS_NATURAL_FRACTURE_SET_H
