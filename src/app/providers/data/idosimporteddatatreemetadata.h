#ifndef IDOS_IMPORTED_DATA_TREE_METADATA_H
#define IDOS_IMPORTED_DATA_TREE_METADATA_H

#include "idos_app.h"
#include "providers/idostreemetadata.h"

class APP_EXPORT IDOSImportedDataTreeMetadata : public IDOSTreeMetadata
{
  public:
    IDOSImportedDataTreeMetadata(const QString& dataTypeId, const QString& parentKey);
    QString typeId() const override;
    QString treeName() const override;
    QString groupKey() const override;
    IDOSTreeProvider* create(IDOSDataObject* data) const override;

  private:
    QString m_dataTypeId;
    QString m_parentKey;
};

#endif // IDOS_IMPORTED_DATA_TREE_METADATA_H
