#ifndef IDOS_OBJECT_H
#define IDOS_OBJECT_H

#include "idos_core.h"
#include <QObject>
#include <QString>

/**
 * @brief 所有领域对象的基类。
 *
 * 继承 QObject 以支持信号槽和 Qt 父子所有权。
 * 构造时自动生成稳定 objectId，用于跨模块交互，替代裸指针。
 */
class CORE_EXPORT IDOSObject : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造领域对象，自动生成唯一 objectId。
     * @param parent Qt 父对象，用于父子所有权管理生命周期。
     */
    explicit IDOSObject(QObject* parent = nullptr);

    ~IDOSObject() override;

    /**
     * @brief 获取对象唯一标识。
     * @return 稳定的 objectId，构造时生成，对象生命周期内不变。
     */
    QString objectId() const;

    /**
     * @brief 获取对象名称。
     * @return 树和面板显示的名称。
     */
    QString name() const;

    /**
     * @brief 设置对象名称。
     * @param name 树和面板显示的名称。
     */
    void setName(const QString& name);

    /**
     * @brief objectId 变化时发射（仅恢复加载时触发）。
     * @param id 新的 objectId。
     */
    Q_SIGNAL void objectIdChanged(const QString& id);

    /**
     * @brief 对象名称变化时发射。
     * @param name 新的名称。
     */
    Q_SIGNAL void nameChanged(const QString& name);

protected:
    /**
     * @brief 设置对象唯一标识。
     * @details 仅在加载工程文件恢复对象时使用，正常构造由系统自动分配。
     * @param id 恢复的 objectId。
     */
    void setObjectId(const QString& id);

private:
    QString m_objectId;
    QString m_name;
};

#endif // IDOS_OBJECT_H
