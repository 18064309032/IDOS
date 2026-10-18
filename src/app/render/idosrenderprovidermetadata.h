#ifndef IDOS_RENDER_PROVIDER_METADATA_H
#define IDOS_RENDER_PROVIDER_METADATA_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include "idos_app.h"

class IDOSDataObject;
class IDOSRenderProvider;

class APP_EXPORT IDOSRenderProviderMetadata
{
public:
    virtual ~IDOSRenderProviderMetadata();
    virtual QString typeId() const = 0;
    virtual QStringList dependencies(const IDOSDataObject* object, const QVariantMap& state) const;
    virtual bool update(IDOSRenderProvider* provider, const QList<IDOSDataObject*>& references) const;
    virtual IDOSRenderProvider* create(IDOSDataObject* object,
                                       const QList<IDOSDataObject*>& references) const = 0;
};

#endif
