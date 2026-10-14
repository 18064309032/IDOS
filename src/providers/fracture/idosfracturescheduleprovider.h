#ifndef IDOS_FRACTURE_SCHEDULE_PROVIDER_H
#define IDOS_FRACTURE_SCHEDULE_PROVIDER_H

#include "idosdataprovider.h"

class PROVIDERS_EXPORT IDOSFractureScheduleProvider : public IDOSDataProvider
{
  public:
    bool canRead(const QString& filePath) const;
    QList<IDOSDataObject*> read(const QString& filePath) override;

  private:
    bool supportsSpreadsheetColumns(const QStringList& columns) const;
    bool canReadSpreadsheet(const QString& filePath) const;
    QList<IDOSDataObject*> readSpreadsheet(const QString& filePath);
};

#endif // IDOS_FRACTURE_SCHEDULE_PROVIDER_H
