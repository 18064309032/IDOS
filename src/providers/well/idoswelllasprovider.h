#ifndef IDOS_WELL_LAS_PROVIDER_H
#define IDOS_WELL_LAS_PROVIDER_H

#include <QString>
#include <QStringList>

#include "idosdataprovider.h"

/**
 * @brief LAS 2.0 / 3.0 well log provider.
 *
 * Parses LAS well metadata and curve samples into an IDOSWell. The first
 * curve is used as the depth index and every following curve becomes an
 * IDOSWellLogChannel.
 */
class PROVIDERS_EXPORT IDOSWellLasProvider : public IDOSDataProvider
{
  public:
    QList<IDOSDataObject*> read(const QString& filePath) override;

  private:
    enum class Section
    {
        None,
        Version,
        Well,
        Curve,
        Ascii,
        Other
    };

    class CurveDefinition
    {
      public:
        CurveDefinition();
        CurveDefinition(const QString& name, const QString& unit);

        QString name;
        QString unit;
    };

    Section sectionForLine(const QString& line) const;
    bool parseEntry(const QString& line, QString* mnemonic, QString* unit, QString* value) const;
    QStringList splitValues(const QString& line) const;
    bool parseNumber(const QString& text, double* value) const;
};

#endif // IDOS_WELL_LAS_PROVIDER_H
