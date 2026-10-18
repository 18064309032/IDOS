#ifndef IDOS_CLIPPING_PLANE_TOOL_PROVIDER_H
#define IDOS_CLIPPING_PLANE_TOOL_PROVIDER_H

#include "idos_app.h"
#include "tools/idostoolprovider.h"
#include "tools/idostoolprovidermetadata.h"

class IDOSRenderInteractionTool;

class APP_EXPORT IDOSClippingPlaneToolProvider : public IDOSToolProvider
{
    Q_OBJECT
public:
    explicit IDOSClippingPlaneToolProvider(IDOSRenderView* view);
    QList<QAction*> actions() const override;
    QList<IDOSTool*> tools() const override;
    void updateState() override;

private slots:
    void onToggled(bool enabled);

private:
    QAction* m_action;
    IDOSRenderInteractionTool* m_tool;
};

class APP_EXPORT IDOSClippingPlaneToolProviderMetadata : public IDOSToolProviderMetadata
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

#endif
