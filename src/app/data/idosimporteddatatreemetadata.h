#ifndef IDOS_IMPORTED_DATA_TREE_METADATA_H
#define IDOS_IMPORTED_DATA_TREE_METADATA_H

#include "idos_app.h"
#include "tree/idostreeitemprovidermetadata.h"

class APP_EXPORT IDOSImportedDataTreeMetadata : public IDOSTreeItemProviderMetadata
{
  public:
    IDOSImportedDataTreeMetadata(const QString& dataTypeId, const QString& parentKey);
    QString id() const override;
    QString treeName() const override;
    QString groupKey() const override;
    IDOSTreeItemProvider* create(IDOSDataObject* data) const override;

  private:
    QString m_dataTypeId;
    QString m_parentKey;
};

#endif // IDOS_IMPORTED_DATA_TREE_METADATA_H
