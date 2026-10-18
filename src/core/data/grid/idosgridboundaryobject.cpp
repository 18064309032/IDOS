#include <QObject>

#include "idosproject.h"

#include "idosgridboundaryobject.h"

IDOSGridBoundaryObject::IDOSGridBoundaryObject(QObject* parent)
    :
    IDOSDataObject(parent)
    , m_sourceFile()
    , m_xCoordinates()
    , m_yCoordinates()
    , m_zCoordinates()
    , m_groupIds()
    , m_coordinateReferenceSystem()
{
}

QString IDOSGridBoundaryObject::typeId() const
{
    return QStringLiteral("idos.grid.boundary");
}

QString IDOSGridBoundaryObject::sourceFile() const
{
    return m_sourceFile;
}

void IDOSGridBoundaryObject::setSourceFile(const QString& sourceFile)
{
    m_sourceFile = sourceFile;
}

QVector<double> IDOSGridBoundaryObject::xCoordinates() const
{
    return m_xCoordinates;
}

void IDOSGridBoundaryObject::setXCoordinates(const QVector<double>& xCoordinates)
{
    m_xCoordinates = xCoordinates;
}

QVector<double> IDOSGridBoundaryObject::yCoordinates() const
{
    return m_yCoordinates;
}

void IDOSGridBoundaryObject::setYCoordinates(const QVector<double>& yCoordinates)
{
    m_yCoordinates = yCoordinates;
}

QVector<double> IDOSGridBoundaryObject::zCoordinates() const
{
    return m_zCoordinates;
}

void IDOSGridBoundaryObject::setZCoordinates(const QVector<double>& zCoordinates)
{
    m_zCoordinates = zCoordinates;
}

QVector<int> IDOSGridBoundaryObject::groupIds() const
{
    return m_groupIds;
}

void IDOSGridBoundaryObject::setGroupIds(const QVector<int>& groupIds)
{
    m_groupIds = groupIds;
}

QString IDOSGridBoundaryObject::coordinateReferenceSystem() const
{
    return m_coordinateReferenceSystem;
}

void IDOSGridBoundaryObject::setCoordinateReferenceSystem(const QString& coordinateReferenceSystem)
{
    m_coordinateReferenceSystem = coordinateReferenceSystem;
}

bool IDOSGridBoundaryObject::canClone() const
{
    return true;
}

IDOSDataObject* IDOSGridBoundaryObject::clone() const
{
    IDOSGridBoundaryObject* copy = new IDOSGridBoundaryObject();
    copy->setName(name());
    copy->m_sourceFile = m_sourceFile;
    copy->m_xCoordinates = m_xCoordinates;
    copy->m_yCoordinates = m_yCoordinates;
    copy->m_zCoordinates = m_zCoordinates;
    copy->m_groupIds = m_groupIds;
    copy->m_coordinateReferenceSystem = m_coordinateReferenceSystem;
    return copy;
}

QVariantMap IDOSGridBoundaryObject::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("sourceFile"), QVariant::fromValue(m_sourceFile));
    data.insert(QStringLiteral("xCoordinates"), QVariant::fromValue(m_xCoordinates));
    data.insert(QStringLiteral("yCoordinates"), QVariant::fromValue(m_yCoordinates));
    data.insert(QStringLiteral("zCoordinates"), QVariant::fromValue(m_zCoordinates));
    data.insert(QStringLiteral("groupIds"), QVariant::fromValue(m_groupIds));
    data.insert(QStringLiteral("coordinateReferenceSystem"), QVariant::fromValue(m_coordinateReferenceSystem));
    return data;
}

bool IDOSGridBoundaryObject::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("sourceFile")) || data.value(QStringLiteral("sourceFile")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("sourceFile"));
        return false;
    }
    if (!data.contains(QStringLiteral("xCoordinates")) || data.value(QStringLiteral("xCoordinates")).userType() != qMetaTypeId<QVector<double>>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("xCoordinates"));
        return false;
    }
    if (!data.contains(QStringLiteral("yCoordinates")) || data.value(QStringLiteral("yCoordinates")).userType() != qMetaTypeId<QVector<double>>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("yCoordinates"));
        return false;
    }
    if (!data.contains(QStringLiteral("zCoordinates")) || data.value(QStringLiteral("zCoordinates")).userType() != qMetaTypeId<QVector<double>>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("zCoordinates"));
        return false;
    }
    if (!data.contains(QStringLiteral("groupIds")) || data.value(QStringLiteral("groupIds")).userType() != qMetaTypeId<QVector<int>>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("groupIds"));
        return false;
    }
    if (!data.contains(QStringLiteral("coordinateReferenceSystem")) || data.value(QStringLiteral("coordinateReferenceSystem")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("coordinateReferenceSystem"));
        return false;
    }
    m_sourceFile = data.value(QStringLiteral("sourceFile")).value<QString>();
    m_xCoordinates = data.value(QStringLiteral("xCoordinates")).value<QVector<double>>();
    m_yCoordinates = data.value(QStringLiteral("yCoordinates")).value<QVector<double>>();
    m_zCoordinates = data.value(QStringLiteral("zCoordinates")).value<QVector<double>>();
    m_groupIds = data.value(QStringLiteral("groupIds")).value<QVector<int>>();
    m_coordinateReferenceSystem = data.value(QStringLiteral("coordinateReferenceSystem")).value<QString>();
    return true;
}
bool IDOSGridBoundaryObject::validateProjectReferences(const IDOSProject* project, QString& error) const
{
    Q_UNUSED(project);
    if (m_xCoordinates.size() != m_yCoordinates.size() || m_xCoordinates.size() != m_zCoordinates.size() ||
        (!m_groupIds.isEmpty() && m_groupIds.size() != m_xCoordinates.size()))
    {
        error = tr("Stored boundary coordinate arrays have inconsistent lengths.");
        return false;
    }
    return true;
}
