#include "idosimportcoordinator.h"
#include "idosproviderregistry.h"
#include "idosprovidermetadata.h"
#include "idosdataprovider.h"
#include "data/idosdataobject.h"
#include "idosproject.h"

#include <memory>

QStringList IDOSImportCoordinator::import(const QString& filePath, IDOSProject* project)
{
    QStringList affected;

    if (project == nullptr)
    {
        m_lastError = QStringLiteral("Target project is null");
        return affected;
    }

    // ① 路由 provider
    QList<IDOSProviderMetadata*> metas =
        IDOSProviderRegistry::instance().metadataForFile(filePath);
    if (metas.isEmpty())
    {
        m_lastError = QStringLiteral("No provider matches file: %1").arg(filePath);
        return affected;
    }

    // ② 解析产碎片
    std::unique_ptr<IDOSDataProvider> provider = metas.first()->createProvider();
    if (provider == nullptr)
    {
        m_lastError = QStringLiteral("Failed to create provider for: %1").arg(filePath);
        return affected;
    }

    QList<IDOSDataObject*> fragments = provider->read(filePath);
    if (fragments.isEmpty())
    {
        m_lastError = provider->lastError();
        return affected;
    }

    // ③④ find-or-create + 多态合并
    for (IDOSDataObject* frag : fragments)
    {
        if (frag == nullptr) continue;

        IDOSDataObject* target = project->objectByName(frag->name());
        if (target != nullptr)
        {
            target->mergeFrom(frag);          // 同名已有：多态合并（well/grid/model 各自语义）
            delete frag;
            target->resolveReferences(project); // 引用名→objectId（IDOSModel 等待解析 pending）
            affected << target->objectId();
        }
        else
        {
            project->addObject(frag);        // 首次出现：碎片直接上岗
            frag->resolveReferences(project); // 同上
            affected << frag->objectId();
        }
    }

    return affected;
}
