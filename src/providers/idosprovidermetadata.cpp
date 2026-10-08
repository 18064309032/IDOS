#include <QFileInfo>

#include "idosprovidermetadata.h"

bool IDOSProviderMetadata::canHandle(const QString& filePath) const
{
    QFileInfo fi(filePath);
    QString suffix = fi.suffix().toLower();
    if (suffix.isEmpty())
    {
        return false;
    }

    for (const QString& ext : fileExtensions())
    {
        QString pattern = ext.mid(ext.indexOf('.') + 1).toLower();
        if (pattern == suffix)
        {
            return true;
        }
    }
    return false;
}

IDOSProviderMetadata::~IDOSProviderMetadata()
{
}
