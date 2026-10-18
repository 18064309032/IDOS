#include <QObject>

#include "idosproject.h"

#include "idosgridsurfaceobject.h"

IDOSGridSurfaceObject::IDOSGridSurfaceObject(QObject* parent)
    :
    IDOSDataObject(parent)
    , m_sourceFile()
    , m_nx(0)
    , m_ny(0)
    , m_originX(0.0)
    , m_originY(0.0)
    , m_spacingX(0.0)
    , m_spacingY(0.0)
    , m_missingValue(0.0)
    , m_values()
    , m_coordinateReferenceSystem()
{
}

QString IDOSGridSurfaceObject::typeId() const
{
    return QStringLiteral("idos.grid.surface");
}

QString IDOSGridSurfaceObject::sourceFile() const
{
    return m_sourceFile;
}

void IDOSGridSurfaceObject::setSourceFile(const QString& sourceFile)
{
    m_sourceFile = sourceFile;
}

int IDOSGridSurfaceObject::nx() const
{
    return m_nx;
}

void IDOSGridSurfaceObject::setNx(int nx)
{
    m_nx = nx;
}

int IDOSGridSurfaceObject::ny() const
{
    return m_ny;
}

void IDOSGridSurfaceObject::setNy(int ny)
{
    m_ny = ny;
}

double IDOSGridSurfaceObject::originX() const
{
    return m_originX;
}

void IDOSGridSurfaceObject::setOriginX(double originX)
{
    m_originX = originX;
}

double IDOSGridSurfaceObject::originY() const
{
    return m_originY;
}

void IDOSGridSurfaceObject::setOriginY(double originY)
{
    m_originY = originY;
}

double IDOSGridSurfaceObject::spacingX() const
{
    return m_spacingX;
}

void IDOSGridSurfaceObject::setSpacingX(double spacingX)
{
    m_spacingX = spacingX;
}

double IDOSGridSurfaceObject::spacingY() const
{
    return m_spacingY;
}

void IDOSGridSurfaceObject::setSpacingY(double spacingY)
{
    m_spacingY = spacingY;
}

double IDOSGridSurfaceObject::missingValue() const
{
    return m_missingValue;
}

void IDOSGridSurfaceObject::setMissingValue(double missingValue)
{
    m_missingValue = missingValue;
}

QVector<double> IDOSGridSurfaceObject::values() const
{
    return m_values;
}

void IDOSGridSurfaceObject::setValues(const QVector<double>& values)
{
    m_values = values;
}

QString IDOSGridSurfaceObject::coordinateReferenceSystem() const
{
    return m_coordinateReferenceSystem;
}

void IDOSGridSurfaceObject::setCoordinateReferenceSystem(const QString& coordinateReferenceSystem)
{
    m_coordinateReferenceSystem = coordinateReferenceSystem;
}

bool IDOSGridSurfaceObject::canClone() const
{
    return true;
}

IDOSDataObject* IDOSGridSurfaceObject::clone() const
{
    IDOSGridSurfaceObject* copy = new IDOSGridSurfaceObject();
    copy->setName(name());
    copy->m_sourceFile = m_sourceFile;
    copy->m_nx = m_nx;
    copy->m_ny = m_ny;
    copy->m_originX = m_originX;
    copy->m_originY = m_originY;
    copy->m_spacingX = m_spacingX;
    copy->m_spacingY = m_spacingY;
    copy->m_missingValue = m_missingValue;
    copy->m_values = m_values;
    copy->m_coordinateReferenceSystem = m_coordinateReferenceSystem;
    return copy;
}

QVariantMap IDOSGridSurfaceObject::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("sourceFile"), QVariant::fromValue(m_sourceFile));
    data.insert(QStringLiteral("nx"), QVariant::fromValue(m_nx));
    data.insert(QStringLiteral("ny"), QVariant::fromValue(m_ny));
    data.insert(QStringLiteral("originX"), QVariant::fromValue(m_originX));
    data.insert(QStringLiteral("originY"), QVariant::fromValue(m_originY));
    data.insert(QStringLiteral("spacingX"), QVariant::fromValue(m_spacingX));
    data.insert(QStringLiteral("spacingY"), QVariant::fromValue(m_spacingY));
    data.insert(QStringLiteral("missingValue"), QVariant::fromValue(m_missingValue));
    data.insert(QStringLiteral("values"), QVariant::fromValue(m_values));
    data.insert(QStringLiteral("coordinateReferenceSystem"), QVariant::fromValue(m_coordinateReferenceSystem));
    return data;
}

bool IDOSGridSurfaceObject::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("sourceFile")) || data.value(QStringLiteral("sourceFile")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("sourceFile"));
        return false;
    }
    if (!data.contains(QStringLiteral("nx")) || data.value(QStringLiteral("nx")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("nx"));
        return false;
    }
    if (!data.contains(QStringLiteral("ny")) || data.value(QStringLiteral("ny")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("ny"));
        return false;
    }
    if (!data.contains(QStringLiteral("originX")) || data.value(QStringLiteral("originX")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("originX"));
        return false;
    }
    if (!data.contains(QStringLiteral("originY")) || data.value(QStringLiteral("originY")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("originY"));
        return false;
    }
    if (!data.contains(QStringLiteral("spacingX")) || data.value(QStringLiteral("spacingX")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("spacingX"));
        return false;
    }
    if (!data.contains(QStringLiteral("spacingY")) || data.value(QStringLiteral("spacingY")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("spacingY"));
        return false;
    }
    if (!data.contains(QStringLiteral("missingValue")) || data.value(QStringLiteral("missingValue")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("missingValue"));
        return false;
    }
    if (!data.contains(QStringLiteral("values")) || data.value(QStringLiteral("values")).userType() != qMetaTypeId<QVector<double>>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("values"));
        return false;
    }
    if (!data.contains(QStringLiteral("coordinateReferenceSystem")) || data.value(QStringLiteral("coordinateReferenceSystem")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("coordinateReferenceSystem"));
        return false;
    }
    m_sourceFile = data.value(QStringLiteral("sourceFile")).value<QString>();
    m_nx = data.value(QStringLiteral("nx")).value<int>();
    m_ny = data.value(QStringLiteral("ny")).value<int>();
    m_originX = data.value(QStringLiteral("originX")).value<double>();
    m_originY = data.value(QStringLiteral("originY")).value<double>();
    m_spacingX = data.value(QStringLiteral("spacingX")).value<double>();
    m_spacingY = data.value(QStringLiteral("spacingY")).value<double>();
    m_missingValue = data.value(QStringLiteral("missingValue")).value<double>();
    m_values = data.value(QStringLiteral("values")).value<QVector<double>>();
    m_coordinateReferenceSystem = data.value(QStringLiteral("coordinateReferenceSystem")).value<QString>();
    return true;
}
bool IDOSGridSurfaceObject::validateProjectReferences(const IDOSProject* project, QString& error) const
{
    Q_UNUSED(project);
    if (m_nx < 0 || m_ny < 0 || static_cast<qint64>(m_nx) * m_ny != m_values.size())
    {
        error = tr("Stored surface dimensions do not match its values.");
        return false;
    }
    return true;
}
