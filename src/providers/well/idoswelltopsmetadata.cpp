#include <QObject>

#include "idoswelltopsprovider.h"

#include "idoswelltopsmetadata.h"

QString IDOSWellTopsMetadata::id() const
{
    return QStringLiteral("idos.well.tops");
}

QString IDOSWellTopsMetadata::displayName() const
{
    return QObject::tr("Well Tops");
}

QStringList IDOSWellTopsMetadata::fileExtensions() const
{
    return {QStringLiteral("*.prn"), QStringLiteral("*.tops"), QStringLiteral("*.txt"),
            QStringLiteral("*.dat"), QStringLiteral("WELL TOP.txt")};
}

std::unique_ptr<IDOSDataProvider> IDOSWellTopsMetadata::create() const
{
    return std::make_unique<IDOSWellTopsProvider>();
}

bool IDOSWellTopsMetadata::canHandle(const QString& filePath) const
{
    IDOSWellTopsProvider provider;
    return provider.canRead(filePath);
}
