#ifndef IDOS_POINT_SET_TREE_METADATA_H
#define IDOS_POINT_SET_TREE_METADATA_H

#include <QPointer>

#include "tree/idostreeitemprovidermetadata.h"

class IDOSExamplePlugin;

class IDOSPointSetTreeMetadata : public IDOSTreeItemProviderMetadata
{
  public:
    explicit IDOSPointSetTreeMetadata(IDOSExamplePlugin* plugin);
    QString id() const override;
    QString treeName() const override;
    IDOSTreeItemProvider* create(IDOSDataObject* data) const override;

  private:
    QPointer<IDOSExamplePlugin> m_plugin;
};

#endif
