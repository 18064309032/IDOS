#ifndef IDOS_PROJECT_H
#define IDOS_PROJECT_H

#include <QHash>
#include <QList>
#include <QObject>
#include <QSet>
#include <QString>
#include <QStringList>

#include "idos_core.h"
#include "idosdataobject.h"
#include "idosprojectmetadata.h"

class IDOSCommandManager;
class QUndoStack;

/**
 * @brief 工程根对象。
 *
 * 统一持有所有数据对象（网格、井、属性场等）的所有权，管理增删查，
 * 并统一转发变化信号。所有数据对象以 objectId 为键存入 QHash，
 * 具体类型由调用方通过 qobject_cast 自行区分。
 *
 * Project 不继承 IDOSObject，因为它是容器而非可被管理的数据对象。
 *
 * 批量事务：一次导入会增删大量对象，逐个发信号会让视图对每个对象
 * 重复重建引用分支（O(N²)）。beginUpdate/endUpdate 支持嵌套，
 * 事务期间只收集 objectId，结束时统一发一次批量信号。
 */
class CORE_EXPORT IDOSProject : public QObject
{
    Q_OBJECT

  public:
    explicit IDOSProject(QObject* parent = nullptr);
    ~IDOSProject() override;

    const IDOSProjectMetadata& metadata() const;
    bool setMetadata(const IDOSProjectMetadata& metadata);
    QUndoStack* undoStack() const;
    IDOSCommandManager* commandManager() const;

    /** 将数据对象加入工程，接管所有权。已存在同 objectId 则替换。 */
    void addObject(IDOSDataObject* object);

    /**
     * @brief Detach a data object without deleting it.
     * @return Detached object, or nullptr when no matching object exists.
     */
    IDOSDataObject* takeObject(const QString& objectId);

    /** 按 objectId 移除数据对象。成功返回 true。 */
    bool removeObject(const QString& objectId);

    /** 按 objectId 查找数据对象。找不到返回 nullptr。 */
    IDOSDataObject* objectById(const QString& objectId) const;

    /** 按对象名查找数据对象（provider 按井名汇合时用）。重名返回第一个；找不到返回 nullptr。 */
    IDOSDataObject* objectByName(const QString& name) const;

    /** 获取所有数据对象。 */
    QList<IDOSDataObject*> objects() const;

    /**
     * 开始批量更新事务（可嵌套）。事务内的增删改不发单个信号，
     * 仅在最外层 endUpdate 时统一发一次批量信号。
     */
    void beginUpdate();

    /** 结束一层批量事务；最外层结束时发射累积的批量信号。 */
    void endUpdate();

  signals:
    void metadataChanged();

    /** 新对象加入。 */
    void objectAdded(const QString& objectId);

    /** 对象移除。 */
    void objectRemoved(const QString& objectId);

    /** 对象业务数据变化，不包含可见性变化。 */
    void objectDataChanged(const QString& objectId);

    /** 对象可见性变化。 */
    void objectVisibilityChanged(const QString& objectId, bool visible);

    // ===== 批量信号（事务结束时发射，列表内 id 唯一） =====

    void objectsAdded(const QStringList& objectIds);
    void objectsRemoved(const QStringList& objectIds);
    void objectsDataChanged(const QStringList& objectIds);
    void objectsVisibilityChanged(const QStringList& objectIds);

  private slots:
    void onObjectDataChanged();
    void onObjectVisibilityChanged(bool visible);
    void onObjectNameChanged(const QString& name);

  private:
    void notifyAdded(const QString& objectId);
    void notifyRemoved(const QString& objectId);
    void notifyDataChanged(const QString& objectId);

    void connectObject(IDOSDataObject* object);
    void disconnectObject(IDOSDataObject* object);

    QHash<QString, IDOSDataObject*> m_objects; // All data objects use Qt parent-child ownership.
    IDOSProjectMetadata m_metadata;
    QUndoStack* m_undoStack;
    IDOSCommandManager* m_commandManager;
    int m_updateDepth;
    QSet<QString> m_pendingAdded;
    QSet<QString> m_pendingRemoved;
    QSet<QString> m_pendingDataChanged;
    QSet<QString> m_pendingVisibilityChanged;
};

#endif // IDOS_PROJECT_H
