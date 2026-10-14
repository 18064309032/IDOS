#ifndef IDOS_WELL_TOPS_PROVIDER_H
#define IDOS_WELL_TOPS_PROVIDER_H

#include "idosdataprovider.h"

class PROVIDERS_EXPORT IDOSWellTopsProvider : public IDOSDataProvider
{
  public:
    QList<IDOSDataObject*> read(const QString& filePath) override;
    bool canRead(const QString& filePath) const;

  private:
    bool readTabularWellTops(const QString& filePath, QList<IDOSDataObject*>* result);
};

#endif
