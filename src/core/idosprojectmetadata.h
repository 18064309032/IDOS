#ifndef IDOS_PROJECT_METADATA_H
#define IDOS_PROJECT_METADATA_H

#include "idos_core.h"

#include <QString>

// 工程元信息。单位制为工程偏好，不表示已转换导入数据。
class CORE_EXPORT IDOSProjectMetadata
{
  public:
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

  private:
    QString m_name;
    QString m_fieldBlock;
    QString m_description;
    UnitSystem m_unitSystem;
    CoordinateType m_coordinateType;
    QString m_coordinateReference;
    QString m_verticalDatum;
};

#endif // IDOS_PROJECT_METADATA_H
