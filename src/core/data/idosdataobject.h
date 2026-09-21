#ifndef IDOS_DATA_OBJECT_H
#define IDOS_DATA_OBJECT_H

#include "../idosobject.h"
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
