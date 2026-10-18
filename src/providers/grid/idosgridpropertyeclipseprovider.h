#ifndef IDOS_GRID_PROPERTY_ECLIPSE_PROVIDER_H
#define IDOS_GRID_PROPERTY_ECLIPSE_PROVIDER_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QVector>

#include "idos_providers.h"
#include "providers/idosdataprovider.h"

namespace Opm
{
class DeckKeyword;
}

class IDOSDataObject;
class IDOSGridObject;

class PROVIDERS_EXPORT IDOSGridPropertyEclipseProvider : public IDOSDataProvider
{
  public:
    QList<IDOSDataObject*> read(const QString& filePath) override;
    QList<IDOSDataObject*> readRestart(const QString& filePath, const IDOSGridObject* grid);
    QList<int> restartReportSteps() const;

  private:
    QStringList propertyKeywords() const;
    QVector<double> values(const Opm::DeckKeyword& keyword) const;
    void appendProperty(QList<IDOSDataObject*>* result, const QString& keyword,
                        const QVector<double>& values) const;
    bool isSupportedRestartKeyword(const QString& keyword) const;
    QList<int> m_restartReportSteps;
};

#endif // IDOS_GRID_PROPERTY_ECLIPSE_PROVIDER_H
