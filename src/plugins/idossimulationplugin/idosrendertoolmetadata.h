#ifndef IDOS_RENDER_TOOL_METADATA_H
#define IDOS_RENDER_TOOL_METADATA_H

#include <QString>

#include "idostoolmetadata.h"

class IDOSRenderToolMetadata : public IDOSToolMetadata
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
