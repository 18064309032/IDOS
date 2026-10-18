#include <QtGlobal>

#include "data/grid/idosgridfilterobject.h"
#include "data/grid/idosgridobject.h"
#include "data/grid/idosgridpropertyobject.h"
#include "idosgridfilterrenderprovider.h"

#include "idosgridfilterrendermetadata.h"

QString IDOSGridFilterRenderMetadata::typeId() const
{
    return QStringLiteral("idos.grid.filter");
}

IDOSRenderProvider* IDOSGridFilterRenderMetadata::create(IDOSDataObject* data, const QList<IDOSDataObject*>& references) const
{
    IDOSGridFilterObject* filter = qobject_cast<IDOSGridFilterObject*>(data);
    if (filter == nullptr)
    {
        return nullptr;
    }
    IDOSGridObject* grid = nullptr;
    for (IDOSDataObject* reference : references)
    {
        if (reference != nullptr && reference->objectId() == filter->gridId())
        {
            grid = qobject_cast<IDOSGridObject*>(reference);
        }
    }
    if (grid == nullptr)
    {
        return nullptr;
    }
    IDOSGridFilterRenderProvider* provider = new IDOSGridFilterRenderProvider(filter, grid);
    if (!update(provider, references))
    {
        delete provider;
        return nullptr;
    }
    return provider;
}

QStringList IDOSGridFilterRenderMetadata::dependencies(const IDOSDataObject* object, const QVariantMap& state) const
{
    QStringList ids = IDOSGridFilterObject::propertyIds(state.value(QStringLiteral("filterCriteria")).toMap());
    ids.append(state.value(QStringLiteral("propertyId")).toString());
    const IDOSGridFilterObject* filter = qobject_cast<const IDOSGridFilterObject*>(object);
    if (filter != nullptr)
    {
        ids.append(filter->gridId());
        ids.append(IDOSGridFilterObject::propertyIds(filter->criteria()));
        ids.append(filter->criteria().value(QStringLiteral("displayPropertyId")).toString());
    }
    ids.removeAll(QString());
    ids.removeDuplicates();
    return ids;
}

bool IDOSGridFilterRenderMetadata::update(IDOSRenderProvider* provider, const QList<IDOSDataObject*>& references) const
{
    IDOSGridFilterRenderProvider* grid = dynamic_cast<IDOSGridFilterRenderProvider*>(provider);
    if (grid == nullptr)
    {
        return false;
    }
    QList<IDOSGridPropertyObject*> properties;
    bool hasGrid = false;
    for (IDOSDataObject* reference : references)
    {
        IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(reference);
        if (property != nullptr)
        {
            properties.append(property);
        }
        hasGrid = hasGrid || reference == grid->grid();
    }
    if (!hasGrid)
    {
        return false;
    }
    grid->setProperties(properties);
    grid->update();
    return true;
}
