#include <QObject>

#include "idosproject.h"

#include "idosgridboundary.h"

IDOSGridBoundary::IDOSGridBoundary(QObject* parent)
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

QString IDOSGridBoundary::typeId() const
{
    return QStringLiteral("idos.grid.boundary");
}

QString IDOSGridBoundary::sourceFile() const
{
    return m_sourceFile;
}

void IDOSGridBoundary::setSourceFile(const QString& sourceFile)
{
    m_sourceFile = sourceFile;
}

QVector<double> IDOSGridBoundary::xCoordinates() const
{
    return m_xCoordinates;
}

void IDOSGridBoundary::setXCoordinates(const QVector<double>& xCoordinates)
{
    m_xCoordinates = xCoordinates;
}

QVector<double> IDOSGridBoundary::yCoordinates() const
{
    return m_yCoordinates;
}

void IDOSGridBoundary::setYCoordinates(const QVector<double>& yCoordinates)
{
    m_yCoordinates = yCoordinates;
}

QVector<double> IDOSGridBoundary::zCoordinates() const
{
    return m_zCoordinates;
}

void IDOSGridBoundary::setZCoordinates(const QVector<double>& zCoordinates)
{
    m_zCoordinates = zCoordinates;
}

QVector<int> IDOSGridBoundary::groupIds() const
{
    return m_groupIds;
}

void IDOSGridBoundary::setGroupIds(const QVector<int>& groupIds)
{
    m_groupIds = groupIds;
}

QString IDOSGridBoundary::coordinateReferenceSystem() const
{
    return m_coordinateReferenceSystem;
}

void IDOSGridBoundary::setCoordinateReferenceSystem(const QString& coordinateReferenceSystem)
{
    m_coordinateReferenceSystem = coordinateReferenceSystem;
}

bool IDOSGridBoundary::canClone() const
{
    return true;
}

IDOSDataObject* IDOSGridBoundary::clone() const
{
    IDOSGridBoundary* copy = new IDOSGridBoundary();
    copy->setName(name());
    copy->setVisible(isVisible());
    copy->m_sourceFile = m_sourceFile;
    copy->m_xCoordinates = m_xCoordinates;
    copy->m_yCoordinates = m_yCoordinates;
    copy->m_zCoordinates = m_zCoordinates;
    copy->m_groupIds = m_groupIds;
    copy->m_coordinateReferenceSystem = m_coordinateReferenceSystem;
    return copy;
}

QVariantMap IDOSGridBoundary::projectData() const
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

bool IDOSGridBoundary::restoreProjectData(const QVariantMap& data, QString& error)
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
bool IDOSGridBoundary::validateProjectReferences(const IDOSProject* project, QString& error) const
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
