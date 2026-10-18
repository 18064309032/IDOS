#ifndef IDOS_TOOL_PROVIDER_METADATA_H
#define IDOS_TOOL_PROVIDER_METADATA_H

#include <QString>

#include "idos_render.h"

class IDOSRenderView;
class IDOSToolProvider;

class RENDER_EXPORT IDOSToolProviderMetadata
{
public:
    virtual ~IDOSToolProviderMetadata();
    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QString iconPath() const = 0;
    virtual QString groupId() const = 0;
    virtual int order() const = 0;
    virtual bool supports(const IDOSRenderView* view) const = 0;
    virtual IDOSToolProvider* create(IDOSRenderView* view) const = 0;
};

#endif // IDOS_TOOL_PROVIDER_METADATA_H
