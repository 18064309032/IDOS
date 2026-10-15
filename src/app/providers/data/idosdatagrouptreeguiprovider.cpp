#include <memory>

#include "idosdataobject.h"
#include "idosrenderobject.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"
#include "providers/idosrenderprovider.h"
#include "providers/idostreeregistry.h"
#include "tree/idostree.h"
#include "tree/idostreeitem.h"
#include "tree/idostreeview.h"

#include "idosdatagrouptreeguiprovider.h"

IDOSDataGroupTreeGuiProvider::IDOSDataGroupTreeGuiProvider(IDOSTreeRegistry* registry)
    :
    IDOSTreeGuiProvider(nullptr)
    , m_registry(registry)
{
}

QString IDOSDataGroupTreeGuiProvider::name() const
{
    return QStringLiteral("idos.dataGroups");
}

bool IDOSDataGroupTreeGuiProvider::supports(const IDOSTreeItem* item) const
{
    return item != nullptr && item->tree() != nullptr
        && item->tree()->name() == QStringLiteral("data") && item->isGroup()
        && item->key().startsWith(QStringLiteral("data."));
}

void IDOSDataGroupTreeGuiProvider::populateMenu(QMenu* menu, IDOSTreeView* view)
{
    Q_UNUSED(menu)
    Q_UNUSED(view)
}

void IDOSDataGroupTreeGuiProvider::collectDataIds(const IDOSTreeItem* item, QSet<QString>* ids) const
{
    if (item == nullptr || m_registry.isNull())
    {
        return;
    }
    for (IDOSTreeItem* child : item->children())
    {
        bool display = false;
        if (!child->isGroup() && !child->dataId().isEmpty())
        {
            for (const std::shared_ptr<IDOSTreeGuiProvider>& provider : m_registry->guiProviders())
            {
                if (provider.get() != this && provider->supports(child) && provider->isCheckable(child))
                {
                    display = true;
                    break;
                }
            }
        }
        if (display)
        {
            ids->insert(child->dataId());
        }
        else
        {
            collectDataIds(child, ids);
        }
    }
}

bool IDOSDataGroupTreeGuiProvider::isCheckable(const IDOSTreeItem* item) const
{
    if (!supports(item))
    {
        return false;
    }
    QSet<QString> ids;
    collectDataIds(item, &ids);
    return !ids.isEmpty();
}

Qt::CheckState IDOSDataGroupTreeGuiProvider::checkState(const IDOSTreeItem* item, const IDOSTreeView* view) const
{
    IDOSRenderView* target = view != nullptr ? qobject_cast<IDOSRenderView*>(view->targetView()) : nullptr;
    if (!supports(item) || target == nullptr)
    {
        return Qt::Unchecked;
    }
    QSet<QString> ids;
    collectDataIds(item, &ids);
    bool visible = false;
    bool hidden = false;
    for (IDOSRenderObject* object : target->scene()->objects())
    {
        IDOSDataObject* source = object->provider()->dataSource();
        if (source != nullptr && ids.contains(source->objectId()))
        {
            visible = visible || object->visible();
            hidden = hidden || !object->visible();
        }
    }
    return visible ? (hidden ? Qt::PartiallyChecked : Qt::Checked) : Qt::Unchecked;
}

bool IDOSDataGroupTreeGuiProvider::isChecked(const IDOSTreeItem* item, const IDOSTreeView* view) const
{
    return checkState(item, view) == Qt::Checked;
}

bool IDOSDataGroupTreeGuiProvider::setChecked(IDOSTreeItem* item, bool checked, IDOSTreeView* view)
{
    IDOSRenderView* target = view != nullptr ? qobject_cast<IDOSRenderView*>(view->targetView()) : nullptr;
    if (!supports(item) || target == nullptr)
    {
        return false;
    }
    QSet<QString> ids;
    collectDataIds(item, &ids);
    bool matched = false;
    IDOSRenderScene* scene = target->scene();
    scene->beginUpdate();
    for (IDOSRenderObject* object : scene->objects())
    {
        IDOSDataObject* source = object->provider()->dataSource();
        if (source != nullptr && ids.contains(source->objectId()))
        {
            matched = true;
            if (object->visible() != checked)
            {
                object->setVisible(checked);
                scene->notifyChanged(object->id());
            }
        }
    }
    scene->endUpdate();
    if (matched && checked)
    {
        target->resetCamera();
    }
    emit checkStateChanged();
    return matched;
}
