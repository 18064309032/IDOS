#ifndef IDOS_DATA_OBJECT_H
#define IDOS_DATA_OBJECT_H

#include <QVariantMap>

#include "idos_core.h"
#include "idosobject.h"

class IDOSProject;

/**
 * @brief 数据领域对象的基类。
 *
 * 继承 IDOSObject 的 objectId 和 name，提供业务数据通知。
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
    virtual QVariantMap projectData() const;
    virtual bool restoreProjectData(const QVariantMap& data, QString& error);
    virtual bool validateProjectReferences(const IDOSProject* project, QString& error) const;
    virtual bool canClone() const;
    virtual IDOSDataObject* clone() const;

    /**
     * @brief 容器对象的 objectId 引用（可选）。
     *
     * 挂在其他对象之下的子对象（如网格属性引用所属网格）返回容器
     * objectId；顶层对象返回空字符串。树模型据此分流：容器子对象
     * 不进顶层，由容器的 provider 子树展示。
     */
    virtual QString containerId() const;

    /**
     * @brief 把同类的数据合并进本对象（导入协调者多态分派用）。
     *
     * 各子类按自己语义合并：井按井名汇合 header/path/logs/markers，
     * 网格按网格名等。类型不符应 no-op（子类用 typeId 校验）。
     * 调用方通过数据接口处理加载结果，不写类型判断、不强转。
     *
     * 基类默认 no-op 容错，具体子类应 override 实现自身合并语义。
     */
    virtual void mergeFrom(const IDOSDataObject* other);

    /**
     * @brief 导入完成后用 Project 解析对象内引用（按名→objectId）。
     *
     * 适用场景：provider 从外部文件只能拿到"名字"（如 ECLIPSE .DATA 的
     * 井名），但数据对象需存 objectId 弱引用。导入命令将数据加入工程后，
     * 调用本接口，使用 Project 把名字解析成 objectId。
     *
     * 基类默认 no-op（多数对象无引用需要解析）；工况等带弱引用的
     * 子类 override 实现。子类自己保证重复调用幂等（清空 pending）。
     */
    virtual void resolveReferences(IDOSProject* project);

  signals:
    /**
     * @brief 对象任意业务数据变化时发射。
     *
     * 子类在属性（非 objectId/name）发生变化时 emit 此信号，
     * 使 Project 等容器能统一感知并转发。
     */
    void dataChanged();

};

#endif // IDOS_DATA_OBJECT_H
