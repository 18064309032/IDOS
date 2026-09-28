#ifndef IDOS_CASE_TREE_MENU_PROVIDER_H
#define IDOS_CASE_TREE_MENU_PROVIDER_H
#include "idos_app.h"
#include "idostreemenuprovider.h"
#include <QPair>
#include <QString>
class IDOSCaseTreeView;
class IDOSTreeGroupNode;
class APP_EXPORT IDOSCaseTreeMenuProvider : public IDOSTreeMenuProvider
{
    Q_OBJECT
  public:
    explicit IDOSCaseTreeMenuProvider(IDOSCaseTreeView* view);
    QMenu* createContextMenu() override;
  private slots:
    void onNewCase();
    void onImportCase();
    void onDeleteCase();
    void onDeleteProperty();
    void onImportProperty();
    void onImportGrid();
    void onDeleteGrid();

  private:
    IDOSCaseTreeView* m_view;
    QString m_pendingCaseId;     // 待删除工况的 objectId
    QString m_pendingPropertyId; // 待删除属性的 objectId
    QString m_pendingGridId;     // 待导入属性的目标网格 / 待删除网格 objectId
    QString m_pendingTargetCaseId; // 待导入属性/网格的目标工况 objectId

    bool isPropertyGroupKey(const QString& groupKey) const;
    QPair<QString, QString> resolvePropertyImportContext(IDOSTreeGroupNode* group) const;
    /** 上溯分组父节点链，取首个工况根的 objectId。 */
    QString resolveCaseIdFromGroup(IDOSTreeGroupNode* group) const;
};
#endif
