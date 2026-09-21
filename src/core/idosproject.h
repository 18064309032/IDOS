#ifndef IDOS_PROJECT_H
#define IDOS_PROJECT_H

#include "idos_core.h"
#include "data/idosdataobject.h"
#include <QObject>
#include <QList>
#include <QHash>

/**
 * @brief 工程根对象。
 *
 * 统一持有所有数据对象（网格、井、属性场等）的所有权，管理增删查，
 * 并统一转发变化信号。所有数据对象以 objectId 为键存入 QHash，
 * 具体类型由调用方通过 qobject_cast 自行区分。
 *
 * Project 不继承 IDOSObject，因为它是容器而非可被管理的数据对象。
 */
class CORE_EXPORT IDOSProject : public QObject
{
    Q_OBJECT

public:
    explicit IDOSProject(QObject* parent = nullptr);
    ~IDOSProject() override;

    /** 将数据对象加入工程，接管所有权。已存在同 objectId 则替换。 */
    void addObject(IDOSDataObject* object);

    /** 按 objectId 移除数据对象。成功返回 true。 */
    bool removeObject(const QString& objectId);

    /** 按 objectId 查找数据对象。找不到返回 nullptr。 */
    IDOSDataObject* objectById(const QString& objectId) const;

    /** 获取所有数据对象。 */
    QList<IDOSDataObject*> objects() const;

    // ===== 信号 =====

    /** 新对象加入。 */
    Q_SIGNAL void objectAdded(const QString& objectId);

    /** 对象移除。 */
    Q_SIGNAL void objectRemoved(const QString& objectId);

    /** 对象任意属性变化。 */
    Q_SIGNAL void objectChanged(const QString& objectId);

private slots:
    void onObjectDataChanged();
    void onObjectVisibilityChanged(bool visible);
    void onObjectNameChanged(const QString& name);

private:
    void connectObject(IDOSDataObject* object);
    void disconnectObject(IDOSDataObject* object);

    QHash<QString, IDOSDataObject*> m_objects;   // 所有数据对象，Qt 父子所有权
};

#endif // IDOS_PROJECT_H
