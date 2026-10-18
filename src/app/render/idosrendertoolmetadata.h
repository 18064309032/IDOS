#ifndef IDOS_RENDER_TOOL_METADATA_H
#define IDOS_RENDER_TOOL_METADATA_H

#include <QString>

#include "idos_app.h"
#include "tools/idostoolprovidermetadata.h"

class APP_EXPORT IDOSRenderToolMetadata : public IDOSToolProviderMetadata
{
public:
    QString id() const override;
    QString name() const override;
    QString description() const override;
    QString iconPath() const override;
    QString groupId() const override;
    int order() const override;
    bool supports(const IDOSRenderView* view) const override;
    IDOSToolProvider* create(IDOSRenderView* view) const override;
};

#endif // IDOS_RENDER_TOOL_METADATA_H
