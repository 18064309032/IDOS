#ifndef IDOS_WELL_COMPLETION_PROVIDER_H
#define IDOS_WELL_COMPLETION_PROVIDER_H

#include "idos_providers.h"
#include "providers/idosdataprovider.h"

class PROVIDERS_EXPORT IDOSWellCompletionProvider : public IDOSDataProvider
{
  public:
    QList<IDOSDataObject*> read(const QString& filePath) override;
    bool canRead(const QString& filePath) const;

  private:
    bool canReadSpreadsheet(const QString& filePath) const;
    QList<IDOSDataObject*> readSpreadsheet(const QString& filePath);
};

#endif
