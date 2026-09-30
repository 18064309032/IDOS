#ifndef IDOS_WELL_RENDER_OBJECT_PROVIDER_H
#define IDOS_WELL_RENDER_OBJECT_PROVIDER_H

#include "idosrenderobjectprovider.h"
#include "idos_app.h"

class IDOSWell;

/**
 * @brief 井渲染对象提供器：按 typeId 分派，把 IDOSWell 转为 IDOSWellRenderObject。
 *
 * 遵循渲染对象注册制（IDOSRenderObjectProvider），与 IDOSGridRenderObjectProvider 同级。
 * 处理具有可显示井口或井轨迹的井，缺少可渲染几何时返回空指针。
 */
class APP_EXPORT IDOSWellRenderObjectProvider : public IDOSRenderObjectProvider
{
  public:
    IDOSWellRenderObjectProvider();
    ~IDOSWellRenderObjectProvider() override;

    QString providerId() const override;
    QString displayName() const override;
    bool canCreate(const IDOSDataObject* object) const override;
    IDOSRenderObject* createObject(const IDOSDataObject* object) const override;
};

#endif // IDOS_WELL_RENDER_OBJECT_PROVIDER_H
