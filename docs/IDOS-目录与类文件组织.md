# IDOS 总目录与类文件组织

适用：`D:\CMakeCoder\IDOS`。状态：设计草案，2026-09-22。

## 1. 阅读约定

- `[保留]`：源码已有，保留其主要职责，不代表无需完善。
- `[调整]`：源码已有，建议移动、重命名或调整职责。
- `[新增]`：为第一阶段至交互闭环补齐的具体能力。
- `[后续]`：有实际功能需求时再建设。
- `.h/.cpp` 表示同一基名的两个真实文件；接口和简单值类型可以只用 `.h`。
- 类采用 `IDOS` 前缀与 UpperCamelCase，文件采用全小写类名，目录采用小写职责名。

## 2. 总目录

```text
IDOS/
├─ CMakeLists.txt
├─ CMakePresets.json
├─ CMakeUserPresets.example.json
├─ CMakeUserPresets.json                 本机配置，不作为可移植构建依据
├─ src/
│  ├─ CMakeLists.txt
│  ├─ main.cpp                          创建应用并启动
│  ├─ core/                             工程、专业数据与工程读写
│  ├─ providers/                        外部格式发现和解析
│  ├─ render/                           唯一渲染模块，包含内部适配器
│  ├─ gui/                              树、面板、专业窗口与工作区
│  ├─ app/                              程序装配、业务操作与任务调度
│  ├─ analysis/                         [后续] 有具体需求再加入算法
│  ├─ python/                           [后续] C++ Python 运行时与绑定
│  └─ assistant/                        [后续] 助手会话和工具调用
├─ python/                              [后续] 纯 Python SDK
├─ tests/
│  ├─ core/
│  ├─ providers/
│  ├─ render/
│  ├─ gui/
│  ├─ app/
│  └─ data/                             小型确定性测试数据
├─ docs/
├─ resources/                           图标、Qt 资源
├─ i18n/                                翻译资源
├─ tools/                               构建与架构检查
└─ thirdparty/                          外部依赖
```

`src/process` 当前为占位。目标设计不将它扩大为通用业务容器：算法进入 `analysis`，操作组织进入 `app`。迁移时确认没有实际能力后再移除其占位目标，本次文档不删除它。

## 3. Core：工程和专业数据

```text
src/core/
├─ CMakeLists.txt
├─ idos_core.h
├─ idosobject.h/.cpp                    IDOSObject [保留]
├─ idosproject.h/.cpp                   IDOSProject [调整]
├─ idosprojectmetadata.h                IDOSProjectMetadata [新增]
├─ idostyperegistry.h/.cpp              IDOSTypeRegistry [调整]
├─ idosobjecttypemetadata.h/.cpp        IDOSObjectTypeMetadata [调整，拆出独立文件]
├─ idosprojectio.h/.cpp                 IDOSProjectIO [新增]
│
├─ data/                               明确开放的专业数据 API
│  ├─ idosdataobject.h/.cpp             IDOSDataObject [调整]
│  ├─ well/
│  │  ├─ idoswell.h/.cpp                IDOSWell [调整]
│  │  ├─ idoswellhead.h/.cpp            IDOSWellHead [保留]
│  │  ├─ idoswellpath.h/.cpp            IDOSWellPath [保留]
│  │  ├─ idoswellpathpoint.h/.cpp       IDOSWellPathPoint [保留]
│  │  ├─ idoswelllogset.h/.cpp          IDOSWellLogSet [保留]
│  │  ├─ idoswelllogchannel.h/.cpp      IDOSWellLogChannel [保留]
│  │  ├─ idoswellmarkerset.h/.cpp       IDOSWellMarkerSet [保留]
│  │  └─ idoswellmarker.h/.cpp          IDOSWellMarker [保留]
│  ├─ grid/
│  │  ├─ idosgrid.h/.cpp                IDOSGrid [调整，完善几何契约]
│  │  ├─ idosgridcell.h/.cpp            IDOSGridCell [保留]
│  │  └─ idosgridproperty.h/.cpp        IDOSGridProperty [保留]
│  └─ simulation/
│     ├─ idossimulationcase.h/.cpp      IDOSSimulationCase [调整，原 IDOSModel]
│     ├─ idoscasewell.h/.cpp            IDOSCaseWell [新增]
│     ├─ idoswellcompletion.h/.cpp      IDOSWellCompletion [调整，迁入]
│     ├─ idoswellsegment.h/.cpp         IDOSWellSegment [调整，迁入]
│     └─ idoswellcontrol.h/.cpp         IDOSWellControl [调整，迁入]
│
└─ serialization/                      内部工程文件实现
   ├─ idosprojectreader.h/.cpp          IDOSProjectReader [新增]
   └─ idosprojectwriter.h/.cpp          IDOSProjectWriter [新增]
```

### 类职责与数据粒度

| 类 | 职责及约束 |
|---|---|
| IDOSProject | 拥有所有顶层数据对象，保证 ID 唯一，校验对象关系，发布增删改通知；不处理 VTK、文件选择框或树分组 |
| IDOSDataObject | 提供类型、业务数据变化与修订号；不保存窗口可见性或外部待导入路径 |
| IDOSTypeRegistry | 保存类型元数据、工厂和类型格式版本；不决定类型只能属于某一棵树 |
| IDOSWell | 保存井身份、井头、实测轨迹、曲线和分层；工况中的控制与网格定位不放在共享井上 |
| IDOSGrid | 完整网格几何、拓扑、有效单元映射；大数组不转换为逐单元 QObject |
| IDOSGridProperty | 引用 gridId，保存数值、单位、空值及索引语义；与网格版本匹配 |
| IDOSSimulationCase | 引用网格和属性，拥有工况配置及 IDOSCaseWell 值集合；不拥有共享井或网格对象 |
| IDOSCaseWell | 某工况中的井配置，保存 wellId、网格定位、井控、模拟完井及分段；由工况按值持有 |
| IDOSProjectIO | 无界面的工程读写入口，内部组合 Reader/Writer，不直接操作活动工程窗口 |

当前 `IDOSWellCompletion`、`IDOSWellSegment`、`IDOSWellControl` 面向 COMPDAT/WELSEGS 等模拟器语义，因此建议归入 simulation。未来若增加实测完井解释数据，应按其专业语义另建类型，不混为一套字段。

`IDOSWellHead`、`IDOSWellPath`、曲线集合、分层集合及其记录继续采用现有值语义。只有需要独立共享、版本化、删除或跨对象引用时，才提升为工程独立对象；不为了树中可展开而增加 QObject。

## 4. Providers：格式解析

```text
src/providers/
├─ CMakeLists.txt
├─ idos_providers.h
├─ idosdataprovider.h                   IDOSDataProvider [调整]
├─ idosprovidermetadata.h/.cpp          IDOSProviderMetadata [保留]
├─ idosproviderregistry.h/.cpp          IDOSProviderRegistry [保留]
├─ idosimportbatch.h/.cpp               IDOSImportBatch [新增]
├─ well/
│  ├─ idoswellheaderprovider.h/.cpp     IDOSWellHeaderProvider [保留]
│  ├─ idoswellheadermetadata.h/.cpp     IDOSWellHeaderMetadata [保留]
│  ├─ idoswelllasprovider.h/.cpp        IDOSWellLasProvider [完善]
│  └─ idoswelllasmetadata.h/.cpp        IDOSWellLasMetadata [保留]
├─ grid/
│  ├─ idosgrideclipseprovider.h/.cpp     IDOSGridEclipseProvider [完善]
│  └─ idosgrideclipsemetadata.h/.cpp     IDOSGridEclipseMetadata [保留]
└─ simulation/
   ├─ idossimulationeclipseprovider.h/.cpp   IDOSSimulationEclipseProvider [调整]
   └─ idossimulationeclipsemetadata.h/.cpp   IDOSSimulationEclipseMetadata [调整]
```

`IDOSImportBatch` 是有确定用途的导入数据包：暂存解析数据、外部标识与待解析引用。它不持有活动工程，不是通用 Result。记录类型可作为其嵌套值类型；跨线程数据不能包含带线程归属的 QObject。第一版只设计井头与网格实际需要的记录，不预建任意类型的万能对象系统。

当前返回 `QList<IDOSDataObject*>` 的同步接口可在迁移期暂时保留，但正式启用后台导入前应改为值数据或不可变数据块输出。不要直接把工作线程创建的 QObject 交给 GUI 线程设置 parent。

`IDOSImportCoordinator` 的活动工程协调职责迁到 `app/import/IDOSImportOperation`；解析器只解析，不按工程对象名称自行合并。

## 5. Render：单一渲染模块

```text
src/render/
├─ CMakeLists.txt
├─ idos_render.h
├─ idosrenderwidget.h/.cpp              IDOSRenderWidget [新增，公共画布]
├─ idosrenderstyle.h                    IDOSRenderStyle [新增，值类型]
├─ idosrendercamera.h                   IDOSRenderCamera [新增，值类型]
├─ idosrenderselection.h                IDOSRenderSelection [新增，值类型]
├─ scene/
│  ├─ idosrenderscene.h/.cpp            IDOSRenderScene [新增]
│  └─ idosrenderobject.h/.cpp           IDOSRenderObject [新增]
├─ adapters/
│  ├─ idosgridrenderadapter.h/.cpp      IDOSGridRenderAdapter [新增]
│  └─ idoswellrenderadapter.h/.cpp      IDOSWellRenderAdapter [新增]
├─ interaction/
│  ├─ idosrenderpicktool.h/.cpp         IDOSRenderPickTool [新增]
│  ├─ idosrendermeasuretool.h/.cpp      IDOSRenderMeasureTool [后续]
│  └─ idosrendercliptool.h/.cpp         IDOSRenderClipTool [后续]
└─ overlays/
   ├─ idosrenderlegend.h/.cpp           IDOSRenderLegend [新增]
   └─ idosrenderaxes.h/.cpp             IDOSRenderAxes [新增]
```

Render 可以依赖 Core 和 VTK，不依赖 GUI、App。场景与适配器均为内部类型，公共头通过 PIMPL 或前置声明隐藏它们。对外发送 `objectId + elementId` 拾取信息，不泄露 Actor 指针。

画布负责一个视图的显示状态与渲染资源；大数据转换缓存以对象 ID、数据修订号和转换参数为键。删除对象和关闭工程时先解除绑定再释放资源。

## 6. GUI：工作区、树与专业窗口

```text
src/gui/
├─ CMakeLists.txt
├─ idos_gui.h
├─ idosworkspace.h/.cpp                 IDOSWorkspace [新增]
├─ idosdocumentview.h/.cpp              IDOSDocumentView [新增]
├─ idosviewfactory.h/.cpp               IDOSViewFactory [新增]
├─ idosselection.h/.cpp                 IDOSSelection [新增]
├─ idospropertypanel.h/.cpp             IDOSPropertyPanel [新增]
├─ idosinputtreeview.h/.cpp             IDOSInputTreeView [调整，公共入口]
├─ idosmodeltreeview.h/.cpp             IDOSModelTreeView [调整，公共入口]
├─ tree/                               内部通用树机制
│  ├─ idostreemodel.h/.cpp              IDOSTreeModel [保留]
│  ├─ idostreeview.h/.cpp               IDOSTreeView [保留]
│  ├─ idostreenode.h/.cpp               IDOSTreeNode [保留]
│  ├─ idosobjecttreenode.h/.cpp         IDOSObjectTreeNode [保留]
│  ├─ idosvaluetreenode.h/.cpp          IDOSValueTreeNode [保留]
│  ├─ idostreeitemprovider.h            IDOSTreeItemProvider [保留，内部]
│  ├─ idostreemenuprovider.h/.cpp        IDOSTreeMenuProvider [保留]
│  └─ idostreemodelmenuprovider.h/.cpp   IDOSTreeModelMenuProvider [保留]
├─ inputtree/
│  ├─ idosinputtreemodel.h/.cpp          IDOSInputTreeModel [保留]
│  └─ idoswellitemprovider.h/.cpp        IDOSWellItemProvider [保留]
├─ modeltree/
│  ├─ idosmodeltreemodel.h/.cpp          IDOSModelTreeModel [保留]
│  ├─ idosgriditemprovider.h/.cpp        IDOSGridItemProvider [保留]
│  └─ idossimulationcaseitemprovider.h/.cpp  IDOSSimulationCaseItemProvider [调整]
├─ views/
│  ├─ idosrenderview.h/.cpp              IDOSRenderView [新增]
│  ├─ idostableview.h/.cpp               IDOSTableView [新增]
│  └─ idoswelllogview.h/.cpp             IDOSWellLogView [后续]
├─ properties/
│  ├─ idoswellpropertyeditor.h/.cpp     IDOSWellPropertyEditor [新增]
│  ├─ idosgridpropertyeditor.h/.cpp      IDOSGridPropertyEditor [新增]
│  └─ idosrenderstyleeditor.h/.cpp       IDOSRenderStyleEditor [新增]
└─ dialogs/
   ├─ idosnewprojectdialog.h/.cpp        IDOSNewProjectDialog [新增]
   └─ idosimportdialog.h/.cpp            IDOSImportDialog [新增]
```

公共 InputTreeView/ModelTreeView 改为组合内部树控件，不继承内部目录的基类；不提供内部 Model 指针访问器，改用对象 ID 信号与明确方法。

`IDOSViewFactory` 按窗口类型标识创建 `IDOSDocumentView*`，让 App 不必 include `gui/views`。窗口基类只规定 viewId、viewType、对象显示请求与状态保存恢复；相机等专属操作由渲染窗口实现，不要求表格实现无意义接口。

第一版树扩展接口保持内部。未来向插件开放时，需同时设计不泄露内部 Node/Model 的公共扩展协议，不能仅把现有 ItemProvider 头移动到根目录。

## 7. App：装配、命令和任务

```text
src/app/
├─ CMakeLists.txt
├─ idos_app.h
├─ idosapplication.h/.cpp               IDOSApplication [新增]
├─ idosmainwindow.h/.cpp                IDOSMainWindow [调整，从 gui 移入]
├─ idossession.h/.cpp                   IDOSSession [新增]
├─ idosappinterface.h/.cpp              IDOSAppInterface [后续，稳定操作入口]
├─ action/
│  ├─ idosprojectactions.h/.cpp          IDOSProjectActions [新增]
│  ├─ idosimportactions.h/.cpp           IDOSImportActions [新增]
│  ├─ idosobjectactions.h/.cpp           IDOSObjectActions [新增]
│  └─ idosviewactions.h/.cpp             IDOSViewActions [新增]
├─ import/
│  └─ idosimportoperation.h/.cpp         IDOSImportOperation [调整]
├─ command/
│  ├─ idosrenameobjectcommand.h/.cpp     IDOSRenameObjectCommand [新增]
│  ├─ idosremoveobjectscommand.h/.cpp    IDOSRemoveObjectsCommand [新增]
│  └─ idosimportobjectscommand.h/.cpp    IDOSImportObjectsCommand [新增]
├─ task/
│  ├─ idostask.h/.cpp                    IDOSTask [新增]
│  ├─ idostaskqueue.h/.cpp               IDOSTaskQueue [新增]
│  └─ idosimporttask.h/.cpp              IDOSImportTask [新增]
├─ automation/                          [后续] 应用操作到脚本/助手的注册
└─ plugin/                              [后续] 插件加载、注册与生命周期
```

`IDOSSession` 仅负责当前工程、对应撤销栈、工程切换代次及关闭时任务协调，不承担数据查询、渲染或全局服务定位。第一版一个会话对应一个打开的工程。

Action 负责 QAction、具名槽和状态刷新；ImportOperation 负责业务过程；Command 负责一次可撤销的提交。三者不重复解析文件或维护另一份业务数据。

## 8. 公共头与内部头的明确规则

本方案建议采用“模块根目录 + 显式专业数据目录”的公共 API 规则。与前面对话引用的 QtCoder 根目录限定规则不同，实施前应将这一选择写入 CMakeCoder 自己的工作约定，不沿用含糊规则。

| 范围 | API 地位 |
|---|---|
| 各模块根目录明确导出的头 | 公共 |
| `core/data/idosdataobject.h`、`core/data/well/*.h`、`grid/*.h`、`simulation/*.h` | 明确允许跨模块使用的专业数据 API |
| `core/serialization`、Render 所有子目录 | 内部 |
| GUI 子目录、App 子目录、具体 Provider 子目录 | 第一版内部 |

公共头不得 include 内部头，public/protected 签名及继承关系不得暴露内部类型。公开值类型的头也必须属于公共 API 集合。CMake 的 include 搜索目录不等于 API 承诺；检查脚本应依据明确头文件清单验证。

```cpp
#include "idosproject.h"
#include "data/well/idoswell.h"
#include "data/grid/idosgrid.h"
#include "idosrenderwidget.h"
```

未来 SDK 导出时保留 `data/well` 等公开路径，以免源码和 SDK 使用不同 include 路径；不要求扁平导出。本次不创建 SDK 目录。

## 9. 主要迁移映射

| 现有位置/类 | 目标 | 原因 |
|---|---|---|
| `gui/idosmainwindow.*` | `app/idosmainwindow.*` | 主窗口属于应用装配 |
| `core/data/model/IDOSModel` | `core/data/simulation/IDOSSimulationCase` | 当前语义是工况，不是所有模型的基类 |
| `well/IDOSWellControl` 等模拟配置 | `simulation`，由 IDOSCaseWell 持有 | 防止工况间误改共享井 |
| `providers/IDOSImportCoordinator` | `app/import/IDOSImportOperation` | 工程提交不是格式解析职责 |
| `providers/model/IDOSModelEclipseProvider` | `providers/simulation/IDOSSimulationEclipseProvider` | 与工况语义统一 |
| `gui/modeltree/IDOSModelItemProvider` | IDOSSimulationCaseItemProvider | 名称与实际对象一致 |
| `IDOSDataObject::visible` | 每个 RenderWidget 的显示配置 | 多窗口独立显示 |
| `IDOSDataObject` 中 pending 路径/引用解析 | ImportBatch + ImportOperation | 去除领域对象对导入流程的依赖 |
| 类型注册中的 Input/Model 分组 | GUI 树展示规则 | 类型与界面分类解耦 |

所有迁移均需同步 CMake、include、翻译上下文及相关测试。重命名公开类型不要求同时更换已持久化的 typeId；如果更换，必须有明确格式版本策略。
