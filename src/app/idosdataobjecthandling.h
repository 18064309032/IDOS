#ifndef IDOS_DATA_OBJECT_HANDLING_H
#define IDOS_DATA_OBJECT_HANDLING_H

#include "idos_app.h"
#include <QCoreApplication>
#include <QStringList>

class IDOSProject;
class QWidget;

/** 应用层数据对象操作；组织交互与解析，工程只负责管理数据。 */
class APP_EXPORT IDOSDataObjectHandling
{
    Q_DECLARE_TR_FUNCTIONS(IDOSDataObjectHandling)
  public:
    static bool newCase(IDOSProject* project, QWidget* parent);
    static bool importCase(IDOSProject* project, QWidget* parent);
    static bool importCase(IDOSProject* project, const QString& filePath, QWidget* parent);
    /** 输入井名并创建井，取消或目标工程失效时返回 false。 */
    static bool newWell(IDOSProject* project, QWidget* parent);
    /** 返回新增井数；取消、失败或无新增时返回零。支持井头与批量井轨迹。 */
    static int importWellData(IDOSProject* project, QWidget* parent);
    /** 为给定文件预览并导入轨迹，返回已更新的井数。 */
    static int importWellPaths(IDOSProject* project, const QStringList& files, QWidget* parent);

  private:
    static int importWellHeaders(IDOSProject* project, QWidget* parent);
    static bool caseNameExists(const IDOSProject* project, const QString& name);
};

#endif

