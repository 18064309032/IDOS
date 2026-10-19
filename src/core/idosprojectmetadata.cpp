#include <cmath>

#include <QCoreApplication>
#include <QDir>
#include <QList>
#include <QObject>
#include <QStringList>

#include <proj.h>

#include "idosprojectmetadata.h"

IDOSProjectMetadata::IDOSProjectMetadata()
    :
    m_unitSystem(UnitSystem::Metric)
    , m_coordinateType(CoordinateType::Unspecified)
    , m_lengthUnit(LengthUnit::Metres)
    , m_zDirection(ZDirection::Unspecified)
    , m_originX(0.0)
    , m_originY(0.0)
    , m_originZ(0.0)
    , m_rotationDegrees(0.0)
    , m_horizontalScale(1.0)
{
}

QString IDOSProjectMetadata::name() const
{
    return m_name;
}

void IDOSProjectMetadata::setName(const QString& name)
{
    m_name = name;
}

QString IDOSProjectMetadata::fieldBlock() const
{
    return m_fieldBlock;
}

void IDOSProjectMetadata::setFieldBlock(const QString& fieldBlock)
{
    m_fieldBlock = fieldBlock;
}

QString IDOSProjectMetadata::description() const
{
    return m_description;
}

void IDOSProjectMetadata::setDescription(const QString& description)
{
    m_description = description;
}

IDOSProjectMetadata::UnitSystem IDOSProjectMetadata::unitSystem() const
{
    return m_unitSystem;
}

void IDOSProjectMetadata::setUnitSystem(UnitSystem unitSystem)
{
    m_unitSystem = unitSystem;
}

IDOSProjectMetadata::CoordinateType IDOSProjectMetadata::coordinateType() const
{
    return m_coordinateType;
}

void IDOSProjectMetadata::setCoordinateType(CoordinateType coordinateType)
{
    m_coordinateType = coordinateType;
}

QString IDOSProjectMetadata::coordinateReference() const
{
    return m_coordinateReference;
}

void IDOSProjectMetadata::setCoordinateReference(const QString& coordinateReference)
{
    m_coordinateReference = coordinateReference;
}

QString IDOSProjectMetadata::verticalDatum() const
{
    return m_verticalDatum;
}

void IDOSProjectMetadata::setVerticalDatum(const QString& verticalDatum)
{
    m_verticalDatum = verticalDatum;
}

QVariantMap IDOSProjectMetadata::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("name"), QVariant::fromValue(m_name));
    data.insert(QStringLiteral("fieldBlock"), QVariant::fromValue(m_fieldBlock));
    data.insert(QStringLiteral("description"), QVariant::fromValue(m_description));
    data.insert(QStringLiteral("unitSystem"), static_cast<int>(m_unitSystem));
    data.insert(QStringLiteral("coordinateType"), static_cast<int>(m_coordinateType));
    data.insert(QStringLiteral("coordinateReference"), QVariant::fromValue(m_coordinateReference));
    data.insert(QStringLiteral("verticalDatum"), QVariant::fromValue(m_verticalDatum));
    data.insert(QStringLiteral("lengthUnit"), static_cast<int>(m_lengthUnit));
    data.insert(QStringLiteral("zDirection"), static_cast<int>(m_zDirection));
    data.insert(QStringLiteral("originX"), m_originX);
    data.insert(QStringLiteral("originY"), m_originY);
    data.insert(QStringLiteral("originZ"), m_originZ);
    data.insert(QStringLiteral("rotationDegrees"), m_rotationDegrees);
    data.insert(QStringLiteral("horizontalScale"), m_horizontalScale);
    return data;
}

bool IDOSProjectMetadata::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("name")) || data.value(QStringLiteral("name")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("name"));
        return false;
    }
    if (!data.contains(QStringLiteral("fieldBlock")) || data.value(QStringLiteral("fieldBlock")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("fieldBlock"));
        return false;
    }
    if (!data.contains(QStringLiteral("description")) || data.value(QStringLiteral("description")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("description"));
        return false;
    }
    if (!data.contains(QStringLiteral("unitSystem")) || data.value(QStringLiteral("unitSystem")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("unitSystem"));
        return false;
    }
    if (data.value(QStringLiteral("unitSystem")).toInt() < 0 || data.value(QStringLiteral("unitSystem")).toInt() >= 3)
    {
        error = QObject::tr("A stored enumeration value is invalid.");
        return false;
    }
    if (!data.contains(QStringLiteral("coordinateType")) || data.value(QStringLiteral("coordinateType")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("coordinateType"));
        return false;
    }
    if (data.value(QStringLiteral("coordinateType")).toInt() < 0 || data.value(QStringLiteral("coordinateType")).toInt() >= 3)
    {
        error = QObject::tr("A stored enumeration value is invalid.");
        return false;
    }
    if (!data.contains(QStringLiteral("coordinateReference")) || data.value(QStringLiteral("coordinateReference")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("coordinateReference"));
        return false;
    }
    if (!data.contains(QStringLiteral("verticalDatum")) || data.value(QStringLiteral("verticalDatum")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("verticalDatum"));
        return false;
    }
    const QStringList spatialNumbers{QStringLiteral("originX"), QStringLiteral("originY"),
        QStringLiteral("originZ"), QStringLiteral("rotationDegrees"), QStringLiteral("horizontalScale")};
    for (const QString& field : spatialNumbers)
    {
        if (data.contains(field) && (data.value(field).userType() != QMetaType::Double ||
                                    !std::isfinite(data.value(field).toDouble())))
        {
            error = QObject::tr("Invalid coordinate setting: %1.").arg(field);
            return false;
        }
    }
    const int lengthUnit = data.value(QStringLiteral("lengthUnit"), 0).toInt();
    const int zDirection = data.value(QStringLiteral("zDirection"), 0).toInt();
    if ((data.contains(QStringLiteral("lengthUnit")) && data.value(QStringLiteral("lengthUnit")).userType() != QMetaType::Int) ||
        (data.contains(QStringLiteral("zDirection")) && data.value(QStringLiteral("zDirection")).userType() != QMetaType::Int) ||
        lengthUnit < 0 || lengthUnit > 2 || zDirection < 0 || zDirection > 2 ||
        data.value(QStringLiteral("horizontalScale"), 1.0).toDouble() <= 0.0)
    {
        error = QObject::tr("Invalid coordinate units, direction or scale.");
        return false;
    }
    m_lengthUnit = static_cast<LengthUnit>(lengthUnit);
    m_zDirection = static_cast<ZDirection>(zDirection);
    m_originX = data.value(QStringLiteral("originX"), 0.0).toDouble();
    m_originY = data.value(QStringLiteral("originY"), 0.0).toDouble();
    m_originZ = data.value(QStringLiteral("originZ"), 0.0).toDouble();
    m_rotationDegrees = data.value(QStringLiteral("rotationDegrees"), 0.0).toDouble();
    m_horizontalScale = data.value(QStringLiteral("horizontalScale"), 1.0).toDouble();
    m_name = data.value(QStringLiteral("name")).value<QString>();
    m_fieldBlock = data.value(QStringLiteral("fieldBlock")).value<QString>();
    m_description = data.value(QStringLiteral("description")).value<QString>();
    m_unitSystem = static_cast<UnitSystem>(data.value(QStringLiteral("unitSystem")).toInt());
    m_coordinateType = static_cast<CoordinateType>(data.value(QStringLiteral("coordinateType")).toInt());
    m_coordinateReference = data.value(QStringLiteral("coordinateReference")).value<QString>();
    m_verticalDatum = data.value(QStringLiteral("verticalDatum")).value<QString>();
    return true;
}

IDOSProjectMetadata::LengthUnit IDOSProjectMetadata::lengthUnit() const
{
    return m_lengthUnit;
}

void IDOSProjectMetadata::setLengthUnit(LengthUnit value)
{
    m_lengthUnit = value;
}

IDOSProjectMetadata::ZDirection IDOSProjectMetadata::zDirection() const
{
    return m_zDirection;
}

void IDOSProjectMetadata::setZDirection(ZDirection value)
{
    m_zDirection = value;
}

double IDOSProjectMetadata::originX() const
{
    return m_originX;
}

double IDOSProjectMetadata::originY() const
{
    return m_originY;
}

double IDOSProjectMetadata::originZ() const
{
    return m_originZ;
}

double IDOSProjectMetadata::rotationDegrees() const
{
    return m_rotationDegrees;
}

void IDOSProjectMetadata::setRotationDegrees(double value)
{
    m_rotationDegrees = value;
}

double IDOSProjectMetadata::horizontalScale() const
{
    return m_horizontalScale;
}

void IDOSProjectMetadata::setHorizontalScale(double value)
{
    m_horizontalScale = value;
}

void IDOSProjectMetadata::setOrigin(double x, double y, double z)
{
    m_originX = x;
    m_originY = y;
    m_originZ = z;
}

double IDOSProjectMetadata::metresPerUnit() const
{
    if (m_lengthUnit == LengthUnit::Feet)
    {
        return 0.3048;
    }
    if (m_lengthUnit == LengthUnit::USSurveyFeet)
    {
        return 1200.0 / 3937.0;
    }
    return 1.0;
}

bool IDOSProjectMetadata::validateCrs(const QString& definition, bool projectedOnly, QString& error, bool horizontalOnly)
{
    PJ_CONTEXT* context = proj_context_create();
    if (context == nullptr)
    {
        error = QObject::tr("Cannot initialize the coordinate conversion library.");
        return false;
    }
    const QByteArray directory = QDir::toNativeSeparators(
        QCoreApplication::applicationDirPath() + QStringLiteral("/resources/proj")).toUtf8();
    const char* paths[] = {directory.constData()};
    proj_context_set_search_paths(context, 1, paths);
    proj_context_set_enable_network(context, 0);
    proj_log_level(context, PJ_LOG_NONE);
    PJ* crs = proj_create(context, definition.toUtf8().constData());
    bool valid = crs != nullptr && proj_is_crs(crs) != 0;
    if (valid && horizontalOnly)
    {
        PJ* axes = proj_crs_get_coordinate_system(context, crs);
        valid = (proj_get_type(crs) == PJ_TYPE_PROJECTED_CRS || proj_get_type(crs) == PJ_TYPE_GEOGRAPHIC_2D_CRS) &&
            axes != nullptr && proj_cs_get_axis_count(context, axes) == 2;
        proj_destroy(axes);
    }
    if (valid && projectedOnly)
    {
        valid = proj_get_type(crs) == PJ_TYPE_PROJECTED_CRS;
        PJ* axes = proj_crs_get_coordinate_system(context, crs);
        double factor = 0.0;
        valid = valid && axes != nullptr && proj_cs_get_axis_info(context, axes, 0, nullptr, nullptr,
            nullptr, &factor, nullptr, nullptr, nullptr) != 0 && std::abs(factor - 1.0) < 1.0e-12;
        proj_destroy(axes);
    }
    if (!valid)
    {
        error = horizontalOnly ? QObject::tr("Engineering coordinates require a 2D horizontal CRS. Use explicit CRS conversion for 3D or vertical operations.")
            : projectedOnly ? QObject::tr("A local coordinate anchor must be a projected CRS in metres.")
                              : QObject::tr("Invalid coordinate reference system: %1.").arg(definition);
    }
    proj_destroy(crs);
    proj_context_destroy(context);
    return valid;
}

bool IDOSProjectMetadata::validateCoordinates(QString& error) const
{
    error.clear();
    if (static_cast<int>(m_unitSystem) < 0 || static_cast<int>(m_unitSystem) > 2 ||
        static_cast<int>(m_coordinateType) < 0 || static_cast<int>(m_coordinateType) > 2 ||
        static_cast<int>(m_lengthUnit) < 0 || static_cast<int>(m_lengthUnit) > 2 ||
        static_cast<int>(m_zDirection) < 0 || static_cast<int>(m_zDirection) > 2 ||
        !std::isfinite(m_originX) || !std::isfinite(m_originY) || !std::isfinite(m_originZ) ||
        !std::isfinite(m_rotationDegrees) || !std::isfinite(m_horizontalScale) || m_horizontalScale <= 0.0)
    {
        error = QObject::tr("Invalid coordinate units, direction or scale.");
        return false;
    }
    if (m_coordinateType == CoordinateType::Custom ||
        (m_coordinateType == CoordinateType::Local && !m_coordinateReference.trimmed().isEmpty()))
    {
        return validateCrs(m_coordinateReference, m_coordinateType == CoordinateType::Local, error);
    }
    return true;
}

bool IDOSProjectMetadata::transformCrs(const QString& sourceCrs, const QString& targetCrs,
                                     QVector<double>& xyz, QString& error, double decimalYear)
{
    error.clear();
    if (xyz.size() % 3 != 0 || !std::isfinite(decimalYear))
    {
        error = QObject::tr("Coordinates must contain finite XYZ triples and a valid epoch.");
        return false;
    }
    for (double value : xyz)
    {
        if (!std::isfinite(value))
        {
            error = QObject::tr("Coordinates must contain finite XYZ triples and a valid epoch.");
            return false;
        }
    }
    PJ_CONTEXT* context = proj_context_create();
    if (context == nullptr)
    {
        error = QObject::tr("Cannot initialize the coordinate conversion library.");
        return false;
    }
    const QByteArray directory = QDir::toNativeSeparators(
        QCoreApplication::applicationDirPath() + QStringLiteral("/resources/proj")).toUtf8();
    const char* paths[] = {directory.constData()};
    proj_context_set_search_paths(context, 1, paths);
    proj_context_set_enable_network(context, 0);
    proj_log_level(context, PJ_LOG_NONE);
    PJ* source = proj_create(context, sourceCrs.toUtf8().constData());
    PJ* target = proj_create(context, targetCrs.toUtf8().constData());
    bool epochRequired = false;
    const QList<PJ*> systems{source, target};
    for (PJ* system : systems)
    {
        if (system != nullptr && proj_is_crs(system))
        {
            PJ* datum = proj_crs_get_datum_forced(context, system);
            if (datum != nullptr)
            {
                const PJ_TYPE type = proj_get_type(datum);
                epochRequired = epochRequired || type == PJ_TYPE_DYNAMIC_GEODETIC_REFERENCE_FRAME ||
                    type == PJ_TYPE_DYNAMIC_VERTICAL_REFERENCE_FRAME;
            }
            proj_destroy(datum);
        }
    }
    const char* options[] = {"ALLOW_BALLPARK=NO", "ONLY_BEST=YES", nullptr};
    PJ* operation = source != nullptr && target != nullptr && proj_is_crs(source) && proj_is_crs(target)
        ? proj_create_crs_to_crs_from_pj(context, source, target, nullptr, options) : nullptr;
    PJ* normalized = operation != nullptr ? proj_normalize_for_visualization(context, operation) : nullptr;
    QVector<double> result = xyz;
    bool success = normalized != nullptr && (!epochRequired || decimalYear > 0.0);
    if (success)
    {
        for (int index = 0; index < result.size(); index += 3)
        {
            const PJ_COORD point = proj_trans(normalized, PJ_FWD,
                proj_coord(xyz.at(index), xyz.at(index + 1), xyz.at(index + 2), decimalYear));
            if (proj_errno(normalized) != 0 || !std::isfinite(point.xyz.x) ||
                !std::isfinite(point.xyz.y) || !std::isfinite(point.xyz.z))
            {
                success = false;
                break;
            }
            result[index] = point.xyz.x;
            result[index + 1] = point.xyz.y;
            result[index + 2] = point.xyz.z;
        }
    }
    if (!success)
    {
        const int code = normalized != nullptr ? proj_errno(normalized) : proj_context_errno(context);
        const char* detail = proj_context_errno_string(context, code);
        error = epochRequired && decimalYear <= 0.0 ? QObject::tr("A decimal-year epoch is required for dynamic reference frames.")
            : QObject::tr("Coordinate conversion failed; check CRS definitions, coverage and required grids: %1")
                    .arg(detail != nullptr ? QString::fromUtf8(detail) : QStringLiteral("PROJ"));
    }
    proj_destroy(normalized);
    proj_destroy(operation);
    proj_destroy(source);
    proj_destroy(target);
    proj_context_destroy(context);
    if (success)
    {
        xyz.swap(result);
    }
    return success;
}

bool IDOSProjectMetadata::transformCoordinates(const IDOSProjectMetadata& source,
                                               QVector<double>& xyz, QString& error) const
{
    if (!source.validateCoordinates(error) || !validateCoordinates(error))
    {
        return false;
    }
    if ((!source.m_coordinateReference.trimmed().isEmpty() && !validateCrs(source.m_coordinateReference, false, error, true)) ||
        (!m_coordinateReference.trimmed().isEmpty() && !validateCrs(m_coordinateReference, false, error, true)))
    {
        return false;
    }
    if (m_coordinateType == CoordinateType::Unspecified || source.m_coordinateType == CoordinateType::Unspecified ||
        m_zDirection == ZDirection::Unspecified || source.m_zDirection == ZDirection::Unspecified || xyz.size() % 3 != 0)
    {
        error = QObject::tr("Specify source and project coordinate systems and Z directions before converting data.");
        return false;
    }
    if (source.m_verticalDatum.trimmed().compare(m_verticalDatum.trimmed(), Qt::CaseInsensitive) != 0)
    {
        error = QObject::tr("Vertical datums differ. Supply an explicit vertical CRS transformation first.");
        return false;
    }
    QVector<double> result = xyz;
    const double sourceAngle = source.m_rotationDegrees * std::acos(-1.0) / 180.0;
    const double targetAngle = m_rotationDegrees * std::acos(-1.0) / 180.0;
    for (int index = 0; index < result.size(); index += 3)
    {
        if (!std::isfinite(result.at(index)) || !std::isfinite(result.at(index + 1)) ||
            !std::isfinite(result.at(index + 2)))
        {
            error = QObject::tr("Coordinates must contain finite XYZ triples and a valid epoch.");
            return false;
        }
        if (source.m_coordinateType == CoordinateType::Local)
        {
            const double x = result.at(index) * source.metresPerUnit() * source.m_horizontalScale;
            const double y = result.at(index + 1) * source.metresPerUnit() * source.m_horizontalScale;
            result[index] = source.m_originX + x * std::cos(sourceAngle) - y * std::sin(sourceAngle);
            result[index + 1] = source.m_originY + x * std::sin(sourceAngle) + y * std::cos(sourceAngle);
        }
        result[index + 2] = source.m_originZ + result.at(index + 2) * source.metresPerUnit() *
            (source.m_zDirection == ZDirection::PositiveDown ? -1.0 : 1.0);
    }
    if (source.m_coordinateReference.trimmed() != m_coordinateReference.trimmed())
    {
        if (source.m_coordinateReference.trimmed().isEmpty() || m_coordinateReference.trimmed().isEmpty())
        {
            error = QObject::tr("A local coordinate system needs an anchor CRS before geographic conversion.");
            return false;
        }
        if (!transformCrs(source.m_coordinateReference, m_coordinateReference, result, error))
        {
            return false;
        }
    }
    for (int index = 0; index < result.size(); index += 3)
    {
        if (m_coordinateType == CoordinateType::Local)
        {
            const double x = result.at(index) - m_originX;
            const double y = result.at(index + 1) - m_originY;
            result[index] = (x * std::cos(targetAngle) + y * std::sin(targetAngle)) / (metresPerUnit() * m_horizontalScale);
            result[index + 1] = (-x * std::sin(targetAngle) + y * std::cos(targetAngle)) / (metresPerUnit() * m_horizontalScale);
        }
        result[index + 2] = (result.at(index + 2) - m_originZ) / metresPerUnit() *
            (m_zDirection == ZDirection::PositiveDown ? -1.0 : 1.0);
        if (!std::isfinite(result.at(index)) || !std::isfinite(result.at(index + 1)) || !std::isfinite(result.at(index + 2)))
        {
            error = QObject::tr("Coordinate conversion overflowed.");
            return false;
        }
    }
    xyz.swap(result);
    return true;
}
