#include "idosfracturematerialcatalog.h"

IDOSFractureMaterialCatalog::IDOSFractureMaterialCatalog(QObject* parent)
    :
    IDOSDataObject(parent)
    , m_kind(Kind::Fluid)
{
}

QString IDOSFractureMaterialCatalog::typeId() const
{
    if (m_kind == Kind::Proppant)
    {
        return QStringLiteral("idos.fracture.proppantCatalog");
    }
    return QStringLiteral("idos.fracture.fluidCatalog");
}

IDOSFractureMaterialCatalog::Kind IDOSFractureMaterialCatalog::kind() const
{
    return m_kind;
}

void IDOSFractureMaterialCatalog::setKind(Kind kind)
{
    if (m_kind == kind)
    {
        return;
    }
    m_kind = kind;
    emit dataChanged();
}

QString IDOSFractureMaterialCatalog::sourceFile() const
{
    return m_sourceFile;
}

void IDOSFractureMaterialCatalog::setSourceFile(const QString& sourceFile)
{
    if (m_sourceFile == sourceFile)
    {
        return;
    }
    m_sourceFile = sourceFile;
    emit dataChanged();
}

QStringList IDOSFractureMaterialCatalog::columns() const
{
    return m_columns;
}

void IDOSFractureMaterialCatalog::setColumns(const QStringList& columns)
{
    if (m_columns == columns)
    {
        return;
    }
    m_columns = columns;
    emit dataChanged();
}

QVariantList IDOSFractureMaterialCatalog::records() const
{
    return m_records;
}

void IDOSFractureMaterialCatalog::setRecords(const QVariantList& records)
{
    if (m_records == records)
    {
        return;
    }
    m_records = records;
    emit dataChanged();
}
