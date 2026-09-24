#include "idosdataloadservice.h"
#include "idosdataprovider.h"
#include "idosprovidermetadata.h"
#include "idosproviderregistry.h"
#include "data/idosdataobject.h"
#include "idosproject.h"

#include <memory>

QStringList IDOSDataLoadService::loadFile(const QString& filePath, IDOSProject* project)
{
    QStringList loadedObjectIds;
    if (project == nullptr)
    {
        m_lastError = QStringLiteral("Target project is null");
        return loadedObjectIds;
    }

    QList<IDOSProviderMetadata*> metadataList = IDOSProviderRegistry::instance().metadataForFile(filePath);
    if (metadataList.isEmpty())
    {
        m_lastError = QStringLiteral("No provider matches file: %1").arg(filePath);
        return loadedObjectIds;
    }

    std::unique_ptr<IDOSDataProvider> provider = metadataList.first()->createProvider();
    if (provider == nullptr)
    {
        m_lastError = QStringLiteral("Failed to create provider for: %1").arg(filePath);
        return loadedObjectIds;
    }

    QList<IDOSDataObject*> objects = provider->read(filePath);
    if (objects.isEmpty())
    {
        m_lastError = provider->lastError();
        return loadedObjectIds;
    }

    for (IDOSDataObject* object : objects)
    {
        if (object == nullptr) continue;
        project->addObject(object);
        object->resolveReferences(project);
        loadedObjectIds.append(object->objectId());
    }

    m_lastError.clear();
    return loadedObjectIds;
}

QString IDOSDataLoadService::lastError() const
{
    return m_lastError;
}
