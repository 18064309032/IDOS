#include "idosimporteddatatreeprovider.h"

#include "idosimporteddatatreemetadata.h"

IDOSImportedDataTreeMetadata::IDOSImportedDataTreeMetadata(const QString& dataTypeId,
                                                           const QString& parentKey)
    :
    m_dataTypeId(dataTypeId)
    , m_parentKey(parentKey)
{
}

QString IDOSImportedDataTreeMetadata::id() const
{
    return m_dataTypeId;
}

QString IDOSImportedDataTreeMetadata::treeName() const
{
    return QStringLiteral("data");
}

QString IDOSImportedDataTreeMetadata::groupKey() const
{
    return m_parentKey;
}

IDOSTreeItemProvider* IDOSImportedDataTreeMetadata::create(IDOSDataObject* data) const
{
    return new IDOSImportedDataTreeProvider(data);
}
