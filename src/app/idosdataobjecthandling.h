#ifndef IDOS_DATA_OBJECT_HANDLING_H
#define IDOS_DATA_OBJECT_HANDLING_H

#include <QCoreApplication>
#include <QStringList>

#include "idos_app.h"

class IDOSProject;
class IDOSCaseObject;
class IDOSWell;
class QWidget;

/** 应用层数据对象操作；组织交互与解析，工程只负责管理数据。 */
class APP_EXPORT IDOSDataObjectHandling
{
    Q_DECLARE_TR_FUNCTIONS(IDOSDataObjectHandling)
  public:
    static bool newCase(IDOSProject* project, QWidget* parent);
    static bool importCase(IDOSProject* project, QWidget* parent);
    static bool importCase(IDOSProject* project, const QString& filePath, QWidget* parent);
    /**
     * 删除工况：网格归工况私有，连带删除该工况引用的网格及其属性本体；
     * 井等原始数据仅断引用、本体保留。确认取消或目标失效时返回 false。
     */
    static bool deleteCase(IDOSProject* project, const QString& caseId, QWidget* parent);
    /** 删除网格属性；删前先断开所有工况对该属性的引用。确认取消或目标失效时返回 false。 */
    static bool deleteProperty(IDOSProject* project, const QString& propertyId, QWidget* parent);
    /**
     * 定向导入网格到工况：网格以 "case.grid" 角色加入目标工况引用集。
     * 一个工况仅持有一个网格；已有网格时提示是否替换（旧网格及其属性连带删除）。
     * 取消或目标失效时返回 false。
     */
    static bool importGrid(IDOSProject* project, const QString& targetCaseId, QWidget* parent);
    /**
     * 定向导入网格属性：把文件中解析出的 IDOSGridProperty 挂到目标网格，并加入目标工况引用集。
     * @param targetGridId 目标网格 objectId；为空时退化为全局导入。
     * @param targetCaseId 目标工况 objectId；非空时新属性以 "case.gridProperty" 角色加入其引用集。
     * 取消或目标失效时返回 false。
     */
    static bool importProperty(IDOSProject* project, const QString& targetGridId,
                               const QString& targetCaseId, QWidget* parent);
    /** 输入井名并创建井，取消或目标工程失效时返回 false。 */
    static bool newWell(IDOSProject* project, QWidget* parent);
    /**
     * 井组批量导入：多选 *.txt/*.dev/*.las，按扩展名自动分派。
     * 井头、轨迹和测井按去空白且不区分大小写的井名汇合；任一类型可先导入。
     * 返回新建井数 + 已合并数据条数；取消或工程失效返回 0。
     */
    static int importWellData(IDOSProject* project, QWidget* parent);
    /**
     * 单井导入：单选 *.dev/*.las，按扩展名挂到目标井。
     * .dev 直接挂轨迹（mergeFrom，hasPath 已填充则覆盖）；
     * .las 合并测井通道（mergeFrom，按通道名覆盖）。
     * 返回 1 成功，0 取消或失败。
     */
    static int importWellData(IDOSProject* project, const QString& targetWellId, QWidget* parent);
    /** 删除井：先断开所有工况对该井的引用，再删本体。确认取消或目标失效时返回 false。 */
    static bool deleteWell(IDOSProject* project, const QString& wellId, QWidget* parent);
    /**
     * 删除网格：连带删除该网格下所有属性（逐个断工况引用 + 删本体），最后删网格本体。
     * 确认取消或目标失效时返回 false。
     */
    static bool deleteGrid(IDOSProject* project, const QString& gridId, QWidget* parent);

  private:
    /** 按去空白且不区分大小写的井名查找工程内的井。 */
    static IDOSWell* wellByName(const IDOSProject* project, const QString& name);
    static bool caseNameExists(const IDOSProject* project, const QString& name);
    /** 收集某网格下的所有属性 objectId（containerId 等于 gridId 的属性）。 */
    static QStringList collectGridPropertyIds(const IDOSProject* project, const QString& gridId);
    /** 取工况以 "case.grid" 角色引用的网格 objectId；无则返回空串。 */
    static QString referencedGridId(const IDOSCaseObject* caseObj);
};

#endif
