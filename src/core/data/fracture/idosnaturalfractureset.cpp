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
