#include <QObject>

#include "idosnaturalfractureset.h"

IDOSNaturalFractureSet::IDOSNaturalFractureSet(QObject* parent)
    :
    IDOSDataObject(parent)
    , m_coordinateScale(1.0)
{
}

QString IDOSNaturalFractureSet::typeId() const
{
    return QStringLiteral("idos.naturalFracture.set");
}

QString IDOSNaturalFractureSet::sourceFile() const
{
    return m_sourceFile;
}

void IDOSNaturalFractureSet::setSourceFile(const QString& sourceFile)
{
    if (m_sourceFile == sourceFile)
    {
        return;
    }
    m_sourceFile = sourceFile;
    emit dataChanged();
}

QString IDOSNaturalFractureSet::lengthUnit() const
{
    return m_lengthUnit;
}

void IDOSNaturalFractureSet::setLengthUnit(const QString& lengthUnit)
{
    if (m_lengthUnit == lengthUnit)
    {
        return;
    }
    m_lengthUnit = lengthUnit;
    emit dataChanged();
}

double IDOSNaturalFractureSet::coordinateScale() const
{
    return m_coordinateScale;
}

void IDOSNaturalFractureSet::setCoordinateScale(double coordinateScale)
{
    if (m_coordinateScale == coordinateScale)
    {
        return;
    }
    m_coordinateScale = coordinateScale;
    emit dataChanged();
}

QVariantList IDOSNaturalFractureSet::fractures() const
{
    return m_fractures;
}

void IDOSNaturalFractureSet::setFractures(const QVariantList& fractures)
{
    if (m_fractures == fractures)
    {
        return;
    }
    m_fractures = fractures;
    emit dataChanged();
}

bool IDOSNaturalFractureSet::canClone() const
{
    return true;
}

IDOSDataObject* IDOSNaturalFractureSet::clone() const
{
    IDOSNaturalFractureSet* copy = new IDOSNaturalFractureSet();
    copy->setName(name());
    copy->setVisible(isVisible());
    copy->m_sourceFile = m_sourceFile;
    copy->m_lengthUnit = m_lengthUnit;
    copy->m_coordinateScale = m_coordinateScale;
    copy->m_fractures = m_fractures;
    return copy;
}

QVariantMap IDOSNaturalFractureSet::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("sourceFile"), QVariant::fromValue(m_sourceFile));
    data.insert(QStringLiteral("lengthUnit"), QVariant::fromValue(m_lengthUnit));
    data.insert(QStringLiteral("coordinateScale"), QVariant::fromValue(m_coordinateScale));
    data.insert(QStringLiteral("fractures"), QVariant::fromValue(m_fractures));
    return data;
}

bool IDOSNaturalFractureSet::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("sourceFile")) || data.value(QStringLiteral("sourceFile")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("sourceFile"));
        return false;
    }
    if (!data.contains(QStringLiteral("lengthUnit")) || data.value(QStringLiteral("lengthUnit")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("lengthUnit"));
        return false;
    }
    if (!data.contains(QStringLiteral("coordinateScale")) || data.value(QStringLiteral("coordinateScale")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("coordinateScale"));
        return false;
    }
    if (!data.contains(QStringLiteral("fractures")) || data.value(QStringLiteral("fractures")).userType() != qMetaTypeId<QVariantList>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("fractures"));
        return false;
    }
    m_sourceFile = data.value(QStringLiteral("sourceFile")).value<QString>();
    m_lengthUnit = data.value(QStringLiteral("lengthUnit")).value<QString>();
    m_coordinateScale = data.value(QStringLiteral("coordinateScale")).value<double>();
    m_fractures = data.value(QStringLiteral("fractures")).value<QVariantList>();
    return true;
}
