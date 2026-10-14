#ifndef IDOS_POINT_SET_METADATA_H
#define IDOS_POINT_SET_METADATA_H

#include <QList>
#include <QPointer>

#include "providers/idosdatametadata.h"

class IDOSPointSetMetadata : public IDOSDataMetadata
{
  public:
    QString typeId() const override;
    QString displayName() const override;
    IDOSDataObject* create() const override;
    bool isInUse() const;

  private:
    mutable QList<QPointer<IDOSDataObject>> m_objects;
};

#endif
