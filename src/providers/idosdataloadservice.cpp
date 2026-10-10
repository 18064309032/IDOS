#include <memory>

#include <QObject>

#include "idoscaseitemref.h"
#include "idoscaseobject.h"
#include "idosdataobject.h"
#include "idosdataprovider.h"
#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idosproject.h"
#include "idosprovidermetadata.h"
#include "idosproviderregistry.h"

#include "idosdataloadservice.h"

QStringList IDOSDataLoadService::loadFile(const QString& filePath, IDOSProject* project,
                                          const QString& targetGridId, const QString& targetCaseId)
{
    QStringList loadedObjectIds;
    m_lastError.clear();
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

    // 属性定向导入模式：targetGridId 非空时只保留 provider 返回的 IDOSGridProperty，
    // 把它挂到指定网格；provider 顺带创建的网格/工况对象丢弃不导入。
    // 网格定向导入模式：targetGridId 为空且 targetCaseId 非空时，只接收网格及其属性，
    // 网格以 "case.grid" 角色挂入目标工况（网格归工况私有），其余附带对象丢弃。
    const bool propertyMode = !targetGridId.isEmpty();
    IDOSCaseObject* targetCase = nullptr;
    if (!targetCaseId.isEmpty())
    {
        targetCase = qobject_cast<IDOSCaseObject*>(project->objectById(targetCaseId));
    }
    const bool gridMode = !propertyMode && targetCase != nullptr;
    QList<IDOSCaseItemRef> pendingCaseRefs;
    QSet<QString> seenRefKeys;
    if (targetCase != nullptr)
    {
        const QList<IDOSCaseItemRef> existingRefs = targetCase->itemRefs();
        for (const IDOSCaseItemRef& ref : existingRefs)
        {
            seenRefKeys.insert(ref.role() + QLatin1Char(':') + ref.objectId());
        }
    }

    // 一次文件导入可能产生数十上百个对象：包批量事务，
    // 视图等订阅者只在全部对象落位后收到一次批量信号
    project->beginUpdate();
    for (IDOSDataObject* object : objects)
    {
        if (object == nullptr)
        {
            continue;
        }
        IDOSGrid* grid = qobject_cast<IDOSGrid*>(object);
        IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(object);
        if (propertyMode)
        {
            if (property == nullptr)
            {
                // 非属性对象（网格/工况）在属性定向导入模式下丢弃
                delete object;
                continue;
            }
            property->setGridId(targetGridId);
        }
        else if (gridMode && grid == nullptr && property == nullptr)
        {
            // 网格定向导入模式下，工况等非网格/非属性附带对象丢弃
            delete object;
            continue;
        }
        project->addObject(object);
        object->resolveReferences(project);
        loadedObjectIds.append(object->objectId());
        if (targetCase != nullptr)
        {
            const QString key = QStringLiteral("case.gridProperty") + QLatin1Char(':') + object->objectId();
            if (property != nullptr)
            {
                const QString refKey = QStringLiteral("case.gridProperty") + QLatin1Char(':') + object->objectId();
                if (!seenRefKeys.contains(refKey))
                {
                    pendingCaseRefs.append(IDOSCaseItemRef(QStringLiteral("case.gridProperty"), object->objectId()));
                    seenRefKeys.insert(refKey);
                }
            }
            else if (grid != nullptr)
            {
                const QString refKey = QStringLiteral("case.grid") + QLatin1Char(':') + object->objectId();
                if (!seenRefKeys.contains(refKey))
                {
                    pendingCaseRefs.append(IDOSCaseItemRef(QStringLiteral("case.grid"), object->objectId()));
                    seenRefKeys.insert(refKey);
                }
            }
        }
    }

    // 一次性把新属性引用追加到目标工况，避免逐条 dataChanged 信号
    if (targetCase != nullptr && !pendingCaseRefs.isEmpty())
    {
        QList<IDOSCaseItemRef> refs = targetCase->itemRefs();
        refs.append(pendingCaseRefs);
        targetCase->setItemRefs(refs);
    }

    m_lastError.clear();
    project->endUpdate();
    return loadedObjectIds;
}

QString IDOSDataLoadService::lastError() const
{
    return m_lastError;
}

IDOSDataLoadService::IDOSDataLoadService()
{
}
