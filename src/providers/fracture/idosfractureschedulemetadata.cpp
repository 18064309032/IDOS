#include <memory>

#include <QObject>

#include "idosfracturescheduleprovider.h"

#include "idosfractureschedulemetadata.h"

QString IDOSFractureScheduleMetadata::id() const
{
    return QStringLiteral("idos.fracture.schedule.tsv");
}

QString IDOSFractureScheduleMetadata::displayName() const
{
    return QObject::tr("Fracture Pump Schedule");
}

QStringList IDOSFractureScheduleMetadata::fileExtensions() const
{
    return QStringList{QStringLiteral("*.txt"), QStringLiteral("*.TXT"), QStringLiteral("*.xlsx"),
                       QStringLiteral("*.XLSX")};
}

std::unique_ptr<IDOSDataProvider> IDOSFractureScheduleMetadata::createProvider() const
{
    return std::unique_ptr<IDOSDataProvider>(new IDOSFractureScheduleProvider());
}

bool IDOSFractureScheduleMetadata::canHandle(const QString& filePath) const
{
    IDOSFractureScheduleProvider provider;
    return provider.canRead(filePath);
}
