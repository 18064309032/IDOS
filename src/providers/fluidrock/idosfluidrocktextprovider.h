#ifndef IDOS_FLUID_ROCK_TEXT_PROVIDER_H
#define IDOS_FLUID_ROCK_TEXT_PROVIDER_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QVariantList>

#include "idos_providers.h"
#include "providers/idosdataprovider.h"

class IDOSDataObject;

class PROVIDERS_EXPORT IDOSFluidRockTextProvider : public IDOSDataProvider
{
  public:
    QList<IDOSDataObject*> read(const QString& filePath) override;

  private:
    QStringList supportedKeywords() const;
    QStringList columns(const QString& keyword) const;
    bool parseTableFile(const QString& filePath, QVariantList* tables, QString* error) const;
};

#endif // IDOS_FLUID_ROCK_TEXT_PROVIDER_H
