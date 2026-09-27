#include <QObject>
#include "idosdataloadservice.h"
#include "idosdataprovider.h"
#include "idosprovidermetadata.h"
#include "idosproviderregistry.h"
#include "idosdataobject.h"
#include "idosproject.h"

#include <memory>

QStringList IDOSDataLoadService::loadFile(const QString& filePath, IDOSProject* project)
{
    QStringList loadedObjectIds;
    if (project == nullptr)
    {
        m_lastError = QObject::tr("Target project is null");
        return loadedObjectIds;
    }

    QList<IDOSProviderMetadata*> metadataList = IDOSProviderRegistry::instance().metadataForFile(filePath);
    if (metadataList.isEmpty())
    {
        m_lastError = QObject::tr("No provider matches file: %1").arg(filePath);
        return loadedObjectIds;
    }

    std::unique_ptr<IDOSDataProvider> provider = metadataList.first()->createProvider();
    if (provider == nullptr)
    {
        m_lastError = QObject::tr("Failed to create provider for: %1").arg(filePath);
        return loadedObjectIds;
    }

    QList<IDOSDataObject*> objects = provider->read(filePath);
    if (objects.isEmpty())
    {
        m_lastError = provider->lastError();
        return loadedObjectIds;
    }

    // 一次文件导入可能产生数十上百个对象：包批量事务，
    // 视图等订阅者只在全部对象落位后收到一次批量信号
    IDOSProjectUpdateGuard updateGuard(project);
    for (IDOSDataObject* object : objects)
    {
        if (object == nullptr)
        {
            continue;
        }
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

IDOSDataLoadService::IDOSDataLoadService()
{
}
