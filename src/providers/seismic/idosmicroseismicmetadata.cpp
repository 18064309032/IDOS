#include <memory>

#include <QObject>

#include "idosmicroseismicprovider.h"

#include "idosmicroseismicmetadata.h"

QString IDOSMicroseismicMetadata::id() const
{
    return QStringLiteral("idos.microseismic.tsv");
}

QString IDOSMicroseismicMetadata::displayName() const
{
    return QObject::tr("Microseismic Events (TSV)");
}

QStringList IDOSMicroseismicMetadata::fileExtensions() const
{
    return QStringList{QStringLiteral("*.txt"), QStringLiteral("*.TXT")};
}

std::unique_ptr<IDOSDataProvider> IDOSMicroseismicMetadata::createProvider() const
{
    return std::unique_ptr<IDOSDataProvider>(new IDOSMicroseismicProvider());
}

bool IDOSMicroseismicMetadata::canHandle(const QString& filePath) const
{
    IDOSMicroseismicProvider provider;
    return provider.canRead(filePath);
}
