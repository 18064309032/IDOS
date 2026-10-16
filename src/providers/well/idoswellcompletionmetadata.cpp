#include <QObject>

#include "idoswellcompletionprovider.h"

#include "idoswellcompletionmetadata.h"

QString IDOSWellCompletionMetadata::id() const
{
    return QStringLiteral("idos.well.completion");
}

QString IDOSWellCompletionMetadata::displayName() const
{
    return QObject::tr("Well Perforations");
}

QStringList IDOSWellCompletionMetadata::fileExtensions() const
{
    return {QStringLiteral("*.ev"), QStringLiteral("*.perf"), QStringLiteral("*.txt"), QStringLiteral("*.dat"),
            QStringLiteral("*.xlsx"), QStringLiteral("*.XLSX")};
}

std::unique_ptr<IDOSDataProvider> IDOSWellCompletionMetadata::createProvider() const
{
    return std::make_unique<IDOSWellCompletionProvider>();
}

bool IDOSWellCompletionMetadata::canHandle(const QString& filePath) const
{
    IDOSWellCompletionProvider provider;
    return provider.canRead(filePath);
}
