#ifndef IDOS_PROJECT_METADATA_H
#define IDOS_PROJECT_METADATA_H

#include <QString>
#include <QVariantMap>
#include <QVector>

#include "idos_core.h"

// 工程元信息与显式坐标转换；修改设置不隐式改变已导入数据。
class CORE_EXPORT IDOSProjectMetadata
{
  public:
    QVariantMap projectData() const;
    bool restoreProjectData(const QVariantMap& data, QString& error);

    enum class UnitSystem
    {
        Metric,
        Field,
        SI
    };
    enum class CoordinateType
    {
        Unspecified,
        Local,
        Custom
    };
    enum class LengthUnit
    {
        Metres,
        Feet,
        USSurveyFeet
    };
    enum class ZDirection
    {
        Unspecified,
        PositiveUp,
        PositiveDown
    };

    IDOSProjectMetadata();

    QString name() const;
    void setName(const QString& name);

    QString fieldBlock() const;
    void setFieldBlock(const QString& fieldBlock);

    QString description() const;
    void setDescription(const QString& description);

    UnitSystem unitSystem() const;
    void setUnitSystem(UnitSystem unitSystem);

    CoordinateType coordinateType() const;
    void setCoordinateType(CoordinateType coordinateType);

    QString coordinateReference() const;
    void setCoordinateReference(const QString& coordinateReference);

    QString verticalDatum() const;
    void setVerticalDatum(const QString& verticalDatum);

    LengthUnit lengthUnit() const;
    void setLengthUnit(LengthUnit unit);
    ZDirection zDirection() const;
    void setZDirection(ZDirection direction);
    double originX() const;
    double originY() const;
    double originZ() const;
    void setOrigin(double x, double y, double z);
    double rotationDegrees() const;
    void setRotationDegrees(double degrees);
    double horizontalScale() const;
    void setHorizontalScale(double scale);
    bool validateCoordinates(QString& error) const;
    bool transformCoordinates(const IDOSProjectMetadata& source, QVector<double>& xyz, QString& error) const;
    static bool transformCrs(const QString& sourceCrs, const QString& targetCrs, QVector<double>& xyz,
                             QString& error, double decimalYear = 0.0);

  private:
    double metresPerUnit() const;
    static bool validateCrs(const QString& definition, bool projectedOnly, QString& error, bool horizontalOnly = false);

  private:
    QString m_name;
    QString m_fieldBlock;
    QString m_description;
    UnitSystem m_unitSystem;
    CoordinateType m_coordinateType;
    QString m_coordinateReference;
    QString m_verticalDatum;
    LengthUnit m_lengthUnit;
    ZDirection m_zDirection;
    double m_originX;
    double m_originY;
    double m_originZ;
    double m_rotationDegrees;
    double m_horizontalScale;
};

#endif // IDOS_PROJECT_METADATA_H
