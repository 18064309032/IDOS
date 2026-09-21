#ifndef IDOS_DATA_OBJECT_H
#define IDOS_DATA_OBJECT_H

#include "idosobject.h"
#include "idos_core.h"

/**
 * @brief 数据领域对象的基类。
 *
 * 继承 IDOSObject 的 objectId 和 name，增加可见性属性。
 * 所有具体数据类（Well/Grid/Property 等）的共同基类。
 */
class CORE_EXPORT IDOSDataObject : public IDOSObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造数据对象。
     * @param parent Qt 父对象，用于父子所有权管理生命周期。
     */
    explicit IDOSDataObject(QObject* parent = nullptr);

    ~IDOSDataObject() override;

    /**
     * @brief 获取稳定类型标识。
     * @return typeId 字符串（如 "idos.well"），用于注册表分发、序列化与插件扩展。
     * 纯虚强制每个具体子类实现，防止遗漏。
     */
    virtual QString typeId() const = 0;

    /**
     * @brief 容器对象的 objectId 引用（可选）。
     *
     * 挂在其他对象之下的子对象（如网格属性引用所属网格）返回容器
     * objectId；顶层对象返回空字符串。树模型据此分流：容器子对象
     * 不进顶层，由容器的 provider 子树展示。
     */
    virtual QString containerId() const { return QString(); }

    /**
     * @brief 获取对象可见性。
     * @return true 表示在渲染视图中显示，false 表示隐藏。
     */
    bool isVisible() const;

    /**
     * @brief 设置对象可见性。
     * @param visible true 显示，false 隐藏。
     */
    void setVisible(bool visible);

    /**
     * @brief 对象可见性变化时发射。
     * @param visible true 显示，false 隐藏。
     */
    Q_SIGNAL void visibilityChanged(bool visible);

    /**
     * @brief 对象任意业务数据变化时发射。
     *
     * 子类在属性（非 objectId/name/visible）发生变化时 emit 此信号，
     * 使 Project 等容器能统一感知并转发。
     */
    Q_SIGNAL void dataChanged();

private:
    bool m_visible;
};

#endif // IDOS_DATA_OBJECT_H
