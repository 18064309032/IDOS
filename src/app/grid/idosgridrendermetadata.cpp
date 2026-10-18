#include <QObject>
#include <QtGlobal>

#include "data/grid/idosgridfilterobject.h"
#include "data/grid/idosgridobject.h"
#include "data/grid/idosgridpropertyobject.h"
#include "idosgridrenderprovider.h"
#include "idosrenderprovider.h"

#include "idosgridrendermetadata.h"

QString IDOSGridRenderMetadata::typeId() const
{
    return QStringLiteral("idos.grid");
}

IDOSRenderProvider* IDOSGridRenderMetadata::create(IDOSDataObject* dataObject, const QList<IDOSDataObject*>& references) const
{
    IDOSGridObject* source = qobject_cast<IDOSGridObject*>(dataObject);
    if (source == nullptr)
    {
        return nullptr;
    }
    IDOSGridRenderProvider* provider = new IDOSGridRenderProvider(source);
    if (!update(provider, references))
    {
        delete provider;
        return nullptr;
    }
    return provider;
}

QStringList IDOSGridRenderMetadata::dependencies(const IDOSDataObject* object, const QVariantMap& state) const
{
    Q_UNUSED(object)
    QStringList ids = IDOSGridFilterObject::propertyIds(state.value(QStringLiteral("filterCriteria")).toMap());
    ids.append(state.value(QStringLiteral("propertyId")).toString());
    ids.append(state.value(QStringLiteral("previewFilterId")).toString());
    ids.removeAll(QString());
    ids.removeDuplicates();
    return ids;
}

bool IDOSGridRenderMetadata::update(IDOSRenderProvider* provider, const QList<IDOSDataObject*>& references) const
{
    IDOSGridRenderProvider* grid = dynamic_cast<IDOSGridRenderProvider*>(provider);
    if (grid == nullptr)
    {
        return false;
    }
    QList<IDOSGridPropertyObject*> properties;
    bool hasPreview = grid->previewFilterId().isEmpty();
    for (IDOSDataObject* reference : references)
    {
        IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(reference);
        if (property != nullptr)
        {
            properties.append(property);
        }
        hasPreview = hasPreview || (reference != nullptr && reference->objectId() == grid->previewFilterId());
    }
    if (!hasPreview)
    {
        grid->setFilterCriteria(QVariantMap());
    }
    grid->setProperties(properties);
    grid->update();
    return true;
}
