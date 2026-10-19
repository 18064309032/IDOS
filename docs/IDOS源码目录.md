# IDOS 源码目录

更新日期：2026-10-16。

> 下节“目标目录草案”对应 2026-10-16 的新设计，尚未整体移动源码；其后的文件树记录实际布局。树的新框架不设 TreeServer，工程同步由一个 AppTreeAccess 对接两个 TreeModel；新旧接口迁移状态见工作进度。

本文为用户确认并已实施的固定源码目录，以当前仓库的实际文件位置为准，已同步 2026-10-14 用户手工调整；保留实际源码、导出头、构建文件及资源。同名 `.h` 和 `.cpp` 必须同目录；后续改变目录划分须经用户确认。

配套文档：[架构设计](IDOS架构设计.md)、[开发指南](开发指南.md)、[工作进度](工作进度-CJW.md)。

- `core` 根部放对象、工程和命令；`core/data` 按业务保存数据及对应 DataMetadata，通用数据对象位于 core 根部。
- `core/providers` 放 DataProvider、TreeProvider 及其 Metadata/Registry，同时保存通用 DataMetadata/Registry；`core/tree` 放 TreeItem、TreeModel、数据树/工况树专用 ItemModel 和 QObject 基类 TreeAccess；日志位于 `core/logger`。
- `gui` 保留 tree、well、grid、simulation 业务目录；通用 TreeView、IDOSDataTreeView、IDOSCaseTreeView 与树菜单接口放在 tree。旧 TreeGuiProvider 在迁移完成前仍位于该目录。工程启动页、工况对话框、新建工程、调试信息、运行信息和 Python 控制台位于 gui 根部。公共平台接口 IDOSInterface 位于 gui 根部，使用 GUI_EXPORT 并由 idos_gui 编译。
- `render` 根部放通用渲染框架、Registry 和交互工具；`render/providers` 放 RenderProvider、ToolProvider 及其 Metadata。
- `app` 根部负责应用装配、模拟服务和插件 Metadata/Registry；`app/command` 放具体命令，`app/plugin` 放插件列表和管理界面，`app/providers` 按 well、grid、case、fluidrock、seismic、data 分组，集中同一业务的树、菜单、渲染实现和 Metadata；内置视图工具放 `app/providers/render`。
- 顶层 `providers` 只放具体数据格式实现，按业务分类；其他模块及示例插件保留。

物理目录与链接目标分别管理：gui 根部及业务目录中的 20 组业务界面（模拟窗口、工况对话框、井/网格显示等）仍由 `src/app/CMakeLists.txt` 显式列入 `idos_app`，使用 APP_EXPORT；它们需要具体业务 Provider。通用界面由 `idos_gui` 编译，不依赖 app 或 render；core 不依赖 gui、render、app 或插件。边界检查按这份明确的 CMake 文件清单识别业务界面，禁止通用模块反向引用它们。此次不增加接口或包装层来拆分已有界面绑定。

## 2026-10-16 目标目录草案（待迁移）

最新职责和接口见 [架构设计定稿记录](IDOS架构设计.md#2026-10-16-框架设计定稿记录待实施)。本节不是已完成迁移清单；未执行移动、类重命名或 CMake 改造。同名 .h/.cpp 成对同目录。

| 目标位置 | 目标内容与迁移说明 |
|---|---|
| `src/core` | Object/DataObject、Project、Command/CommandMetadata/CommandRegistry/CommandManager、领域基础设施；不依赖 gui/render/app/插件 |
| `src/core/data/{grid,well,case,fracture,fluidrock,seismic,production}` | 平台业务实体统一 Object 后缀，保留所属类型 Metadata；值类型不机械加后缀 |
| `src/core/providers` | 数据类型及格式读取的公共接口、Metadata、Registry；树契约拟随树整体迁出，不先零散搬文件 |
| `src/providers` 各业务子目录 | 实际文件格式解析，继续依赖 core，不改工程或展示 |
| `src/app/tree`（拟调整） | 早期目录草案，当前树新框架不设置 TreeServer；树类仍按固定源码目录逐步迁移，未确认目录变更前不移动。目标移除旧 TreeGuiProvider，改由节点 Provider 与 IDOSTreeMenuProvider 分工 |
| `src/app/providers/data` | IDOSDataTreeRootProvider，以及本业务具体树/渲染实现 |
| `src/app/providers/case` | IDOSCaseTreeRootProvider，以及工况具体树实现 |
| `src/app/providers/{well,grid,fluidrock,seismic}` | 各业务的具体 TreeProvider/RenderProvider 及 Metadata，不搬到通用模块 |
| `src/render` | RenderObject/Scene/View/Server、RenderProviderRegistry、Tool/ToolBar/ToolProviderRegistry；一个 RenderServer 同步多个视图 |
| `src/render/providers` | RenderProvider/RenderProviderMetadata、ToolProvider/ToolProviderMetadata |
| `src/app/providers/render` | 内置具体视图工具及其 Provider/Metadata |
| `src/gui` | 通用界面和 IDOSInterface；业务窗口继续物理归档 well/grid/simulation 及根部并由 app 目标显式编译，不因整理树再整体搬动 |
| `src/app/command` | 一个业务操作一组命令 .h/.cpp，对应 Metadata 可同文件对；拆除 idosimportcommands 中的大 Mode 命令 |
| `src/app`、`src/app/plugin` | 应用、主窗口、插件 Registry/Metadata 和插件管理界面；MainWindow 装配公共服务 |
| `src/plugins/<插件目录>` | 该插件的算法、非模态 Widget、命令、Metadata 和具体扩展；不把插件算法推入通用框架 |

目标改名：IDOSTreeMetadata/IDOSTreeRegistry → IDOSTreeProviderMetadata/IDOSTreeProviderRegistry；IDOSRenderMetadata/IDOSRenderRegistry → IDOSRenderProviderMetadata/IDOSRenderProviderRegistry；IDOSToolMetadata/IDOSToolRegistry → IDOSToolProviderMetadata/IDOSToolProviderRegistry。命令保留 IDOSCommandMetadata/IDOSCommandRegistry，方法改为 registerMetadata/unregisterMetadata/metadata/createCommand。

插件窗口目标名称：IDOSDfnGenerationWidget、IDOSFractureCheckWidget、IDOSCurveFitEvaluationWidget、IDOSFieldDifferenceWidget。统一 QWidget 非模态窗口，构造接收 IDOSInterface*；算法独立。当前文件名仍以实际文件树和工作进度为准。

## 当前实际文件布局

以下为迁移后的实际文件树（排除空目录），包括源码、CMake 与资源：

```text
src/
├── analysis/
│   ├── CMakeLists.txt
│   └── idos_analysis.h
├── app/
│   ├── command/
│   │   ├── idostransferobjectscommand.cpp
│   │   ├── idostransferobjectscommand.h
│   │   ├── idoscasereferencecommand.cpp
│   │   ├── idoscasereferencecommand.h
│   │   ├── idoscreatecasecommand.cpp
│   │   ├── idoscreatecasecommand.h
│   │   ├── idoscreatewellcommand.cpp
│   │   ├── idoscreatewellcommand.h
│   │   ├── idosdeletecasecommand.cpp
│   │   ├── idosdeletecasecommand.h
│   │   ├── idosdeletegridcommand.cpp
│   │   ├── idosdeletegridcommand.h
│   │   ├── idosdeletedatacommand.cpp
│   │   ├── idosdeletedatacommand.h
│   │   ├── idosdeletepropertycommand.cpp
│   │   ├── idosdeletepropertycommand.h
│   │   ├── idosdeletewellcommand.cpp
│   │   ├── idosdeletewellcommand.h
│   │   ├── idosgridfiltercommand.cpp
│   │   ├── idosgridfiltercommand.h
│   │   ├── idosimportcommands.cpp
│   │   ├── idosimportcommands.h
│   │   ├── idosimportrestartresultscommand.cpp
│   │   ├── idosimportrestartresultscommand.h
│   │   ├── idosrenameobjectcommand.cpp
│   │   └── idosrenameobjectcommand.h
│   ├── plugin/
│   │   ├── idospluginlistmodel.cpp
│   │   ├── idospluginlistmodel.h
│   │   ├── idospluginmanagerwidget.cpp
│   │   └── idospluginmanagerwidget.h
│   ├── providers/
│   │   ├── case/
│   │   │   ├── idoscasetreerootprovider.cpp
│   │   │   ├── idoscasetreerootprovider.h
│   │   │   ├── idoscasetreemenuprovider.cpp
│   │   │   ├── idoscasetreemenuprovider.h
│   │   │   ├── idossimulationcasetreemetadata.cpp
│   │   │   ├── idossimulationcasetreemetadata.h
│   │   │   ├── idossimulationcasetreeprovider.cpp
│   │   │   └── idossimulationcasetreeprovider.h
│   │   ├── data/
│   │   │   ├── idosdatatreerootprovider.cpp
│   │   │   ├── idosdatatreerootprovider.h
│   │   │   ├── idosdatatreemenuprovider.cpp
│   │   │   ├── idosdatatreemenuprovider.h
│   │   │   ├── idosimporteddatatreemetadata.cpp
│   │   │   ├── idosimporteddatatreemetadata.h
│   │   │   ├── idosimporteddatatreeprovider.cpp
│   │   │   ├── idosimporteddatatreeprovider.h
│   │   │   ├── idosnaturalfracturerendermetadata.cpp
│   │   │   ├── idosnaturalfracturerendermetadata.h
│   │   │   ├── idosnaturalfracturerenderprovider.cpp
│   │   │   └── idosnaturalfracturerenderprovider.h
│   │   ├── fluidrock/
│   │   │   ├── idosfluidrocktabletreemetadata.cpp
│   │   │   ├── idosfluidrocktabletreemetadata.h
│   │   │   ├── idosfluidrocktabletreeprovider.cpp
│   │   │   └── idosfluidrocktabletreeprovider.h
│   │   ├── grid/
│   │   │   ├── idosgridboundaryrendermetadata.cpp
│   │   │   ├── idosgridboundaryrendermetadata.h
│   │   │   ├── idosgridboundaryrenderprovider.cpp
│   │   │   ├── idosgridboundaryrenderprovider.h
│   │   │   ├── idosgridfilterrendermetadata.cpp
│   │   │   ├── idosgridfilterrendermetadata.h
│   │   │   ├── idosgridfilterrenderprovider.cpp
│   │   │   ├── idosgridfilterrenderprovider.h
│   │   │   ├── idosgridrendermetadata.cpp
│   │   │   ├── idosgridrendermetadata.h
│   │   │   ├── idosgridrenderprovider.cpp
│   │   │   ├── idosgridrenderprovider.h
│   │   │   ├── idosgridsurfacerendermetadata.cpp
│   │   │   ├── idosgridsurfacerendermetadata.h
│   │   │   ├── idosgridsurfacerenderprovider.cpp
│   │   │   ├── idosgridsurfacerenderprovider.h
│   │   │   ├── idosgridtreemetadata.cpp
│   │   │   ├── idosgridtreemetadata.h
│   │   │   ├── idosgridtreeprovider.cpp
│   │   │   └── idosgridtreeprovider.h
│   │   ├── render/
│   │   │   ├── idosrendertoolmetadata.cpp
│   │   │   ├── idosrendertoolmetadata.h
│   │   │   ├── idosrendertoolprovider.cpp
│   │   │   └── idosrendertoolprovider.h
│   │   ├── seismic/
│   │   │   ├── idosseismicvolumerendermetadata.cpp
│   │   │   ├── idosseismicvolumerendermetadata.h
│   │   │   ├── idosseismicvolumerenderprovider.cpp
│   │   │   └── idosseismicvolumerenderprovider.h
│   │   └── well/
│   │       ├── idoswellrendermetadata.cpp
│   │       ├── idoswellrendermetadata.h
│   │       ├── idoswellrenderprovider.cpp
│   │       ├── idoswellrenderprovider.h
│   │       ├── idoswelltreemetadata.cpp
│   │       ├── idoswelltreemetadata.h
│   │       ├── idoswelltreeprovider.cpp
│   │       └── idoswelltreeprovider.h
│   ├── CMakeLists.txt
│   ├── idos_app.h
│   ├── idosapptreeaccess.cpp
│   ├── idosapptreeaccess.h
│   ├── idosappinterface.cpp
│   ├── idosappinterface.h
│   ├── idosapplication.cpp
│   ├── idosapplication.h
│   ├── idosmainwindow.cpp
│   ├── idosmainwindow.h
│   ├── idospluginmetadata.cpp
│   ├── idospluginmetadata.h
│   ├── idospluginregistry.cpp
│   ├── idospluginregistry.h
│   ├── idossimulation.cpp
│   └── idossimulation.h
├── assistant/
│   ├── CMakeLists.txt
│   ├── idos_assistant.h
│   ├── idosassistantservice.cpp
│   ├── idosassistantservice.h
│   ├── idosassistantwidget.cpp
│   ├── idosassistantwidget.h
│   ├── idoslocalmodelprocess.cpp
│   ├── idoslocalmodelprocess.h
│   ├── idoslocalmodelservice.cpp
│   └── idoslocalmodelservice.h
├── core/
│   ├── data/
│   │   ├── case/
│   │   │   ├── idoscaseitemref.cpp
│   │   │   ├── idoscaseitemref.h
│   │   │   ├── idoscaseobject.cpp
│   │   │   ├── idoscaseobject.h
│   │   │   ├── idossimulationcasemetadata.cpp
│   │   │   ├── idossimulationcasemetadata.h
│   │   │   ├── idossimulationcaseobject.cpp
│   │   │   └── idossimulationcaseobject.h
│   │   │   ├── idossimulationsummaryresults.cpp
│   │   │   ├── idossimulationsummaryresults.h
│   │   │   ├── idossimulationsummaryresultsmetadata.cpp
│   │   │   └── idossimulationsummaryresultsmetadata.h
│   │   ├── fluidrock/
│   │   │   ├── idosfluidrocktable.cpp
│   │   │   ├── idosfluidrocktable.h
│   │   │   ├── idosfluidrocktablemetadata.cpp
│   │   │   └── idosfluidrocktablemetadata.h
│   │   ├── fracture/
│   │   │   ├── idosfracturematerialcatalog.cpp
│   │   │   ├── idosfracturematerialcatalog.h
│   │   │   ├── idosfracturematerialcatalogdatametadata.cpp
│   │   │   ├── idosfracturematerialcatalogdatametadata.h
│   │   │   ├── idosfractureschedule.cpp
│   │   │   ├── idosfractureschedule.h
│   │   │   ├── idosfracturescheduledatametadata.cpp
│   │   │   ├── idosfracturescheduledatametadata.h
│   │   │   ├── idosnaturalfractureset.cpp
│   │   │   ├── idosnaturalfractureset.h
│   │   │   ├── idosnaturalfracturesetdatametadata.cpp
│   │   │   └── idosnaturalfracturesetdatametadata.h
│   │   ├── grid/
│   │   │   ├── idosgrid.cpp
│   │   │   ├── idosgrid.h
│   │   │   ├── idosgridboundary.cpp
│   │   │   ├── idosgridboundary.h
│   │   │   ├── idosgridboundarydatametadata.cpp
│   │   │   ├── idosgridboundarydatametadata.h
│   │   │   ├── idosgridcell.cpp
│   │   │   ├── idosgridcell.h
│   │   │   ├── idosgridfilter.cpp
│   │   │   ├── idosgridfilter.h
│   │   │   ├── idosgridfiltermetadata.cpp
│   │   │   ├── idosgridfiltermetadata.h
│   │   │   ├── idosgridmetadata.cpp
│   │   │   ├── idosgridmetadata.h
│   │   │   ├── idosgridproperty.cpp
│   │   │   ├── idosgridproperty.h
│   │   │   ├── idosgridpropertymetadata.cpp
│   │   │   ├── idosgridpropertymetadata.h
│   │   │   ├── idosgridsurface.cpp
│   │   │   ├── idosgridsurface.h
│   │   │   ├── idosgridsurfacedatametadata.cpp
│   │   │   └── idosgridsurfacedatametadata.h
│   │   ├── production/
│   │   │   ├── idosproductionhistory.cpp
│   │   │   ├── idosproductionhistory.h
│   │   │   ├── idosproductionhistorydatametadata.cpp
│   │   │   └── idosproductionhistorydatametadata.h
│   │   ├── seismic/
│   │   │   ├── idosmicroseismicdatametadata.cpp
│   │   │   ├── idosmicroseismicdatametadata.h
│   │   │   ├── idosmicroseismicset.cpp
│   │   │   ├── idosmicroseismicset.h
│   │   │   ├── idosseismicvolume.cpp
│   │   │   ├── idosseismicvolume.h
│   │   │   ├── idosseismicvolumedatametadata.cpp
│   │   │   └── idosseismicvolumedatametadata.h
│   │   └── well/
│   │       ├── idoswell.cpp
│   │       ├── idoswell.h
│   │       ├── idoswellcheckshot.cpp
│   │       ├── idoswellcheckshot.h
│   │       ├── idoswellcompletion.cpp
│   │       ├── idoswellcompletion.h
│   │       ├── idoswellcontrol.cpp
│   │       ├── idoswellcontrol.h
│   │       ├── idoswellgeometryresolver.cpp
│   │       ├── idoswellgeometryresolver.h
│   │       ├── idoswellhead.cpp
│   │       ├── idoswellhead.h
│   │       ├── idoswelllogchannel.cpp
│   │       ├── idoswelllogchannel.h
│   │       ├── idoswelllogset.cpp
│   │       ├── idoswelllogset.h
│   │       ├── idoswellmarker.cpp
│   │       ├── idoswellmarker.h
│   │       ├── idoswellmarkerset.cpp
│   │       ├── idoswellmarkerset.h
│   │       ├── idoswellmetadata.cpp
│   │       ├── idoswellmetadata.h
│   │       ├── idoswellpath.cpp
│   │       ├── idoswellpath.h
│   │       ├── idoswellpathpoint.cpp
│   │       ├── idoswellpathpoint.h
│   │       ├── idoswellsegment.cpp
│   │       ├── idoswellsegment.h
│   │       ├── idoswellspatialreference.cpp
│   │       └── idoswellspatialreference.h
│   ├── logger/
│   │   ├── idosconsoletarget.cpp
│   │   ├── idosconsoletarget.h
│   │   ├── idosfiletarget.cpp
│   │   ├── idosfiletarget.h
│   │   ├── idoslogger.cpp
│   │   ├── idoslogger.h
│   │   ├── idosloglevel.cpp
│   │   ├── idosloglevel.h
│   │   ├── idoslogrecord.cpp
│   │   ├── idoslogrecord.h
│   │   ├── idoslogtarget.h
│   │   ├── idosmsvctarget.cpp
│   │   └── idosmsvctarget.h
│   ├── providers/
│   │   ├── idosdatametadata.cpp
│   │   ├── idosdatametadata.h
│   │   ├── idosdataprovider.cpp
│   │   ├── idosdataprovider.h
│   │   ├── idosdataprovidermetadata.cpp
│   │   ├── idosdataprovidermetadata.h
│   │   ├── idosdataproviderregistry.cpp
│   │   ├── idosdataproviderregistry.h
│   │   ├── idosdataregistry.cpp
│   │   ├── idosdataregistry.h
│   │   ├── idostreeitemprovidermetadata.cpp
│   │   ├── idostreeitemprovidermetadata.h
│   │   ├── idostreeitemprovider.cpp
│   │   ├── idostreeitemprovider.h
│   │   ├── idostreeitemproviderregistry.cpp
│   │   └── idostreeitemproviderregistry.h
│   ├── tree/
│   │   ├── idostreeaccess.cpp
│   │   ├── idostreeaccess.h
│   │   ├── idostreeitem.cpp
│   │   ├── idostreeitem.h
│   │   ├── idosdatatreeitemmodel.cpp
│   │   ├── idosdatatreeitemmodel.h
│   │   ├── idoscasetreeitemmodel.cpp
│   │   ├── idoscasetreeitemmodel.h
│   │   ├── idostreemodel.cpp
│   │   └── idostreemodel.h
│   ├── CMakeLists.txt
│   ├── idos_core.h
│   ├── idoscommand.cpp
│   ├── idoscommand.h
│   ├── idoscommandmanager.cpp
│   ├── idoscommandmanager.h
│   ├── idoscommandmetadata.cpp
│   ├── idoscommandmetadata.h
│   ├── idoscommandregistry.cpp
│   ├── idoscommandregistry.h
│   ├── idosdataobject.cpp
│   ├── idosdataobject.h
│   ├── idosobject.cpp
│   ├── idosobject.h
│   ├── idosproject.cpp
│   ├── idosproject.h
│   ├── idosprojectmetadata.cpp
│   ├── idosprojectmetadata.h
├── gui/
│   ├── grid/
│   │   ├── idosgridfilterwidget.cpp
│   │   ├── idosgridfilterwidget.h
│   │   ├── idosgridsettingwidget.cpp
│   │   └── idosgridsettingwidget.h
│   ├── simulation/
│   │   ├── idossimulationcaseinventoryreportwidget.cpp
│   │   ├── idossimulationcaseinventoryreportwidget.h
│   │   ├── idossimulationcasetablewidget.cpp
│   │   ├── idossimulationcasetablewidget.h
│   │   ├── idossimulationcurvecomparisonwidget.cpp
│   │   ├── idossimulationcurvecomparisonwidget.h
│   │   ├── idossimulationdatagenerationwidget.cpp
│   │   ├── idossimulationdatagenerationwidget.h
│   │   ├── idossimulationinitializewidget.cpp
│   │   ├── idossimulationinitializewidget.h
│   │   ├── idossimulationpreflightwidget.cpp
│   │   ├── idossimulationpreflightwidget.h
│   │   ├── idossimulationpropertycurvewidget.cpp
│   │   ├── idossimulationpropertycurvewidget.h
│   │   ├── idossimulationsummarycurveswidget.cpp
│   │   ├── idossimulationsummarycurveswidget.h
│   │   ├── idossimulationresultfileswidget.cpp
│   │   ├── idossimulationresultfileswidget.h
│   │   ├── idossimulationresultplaybackwidget.cpp
│   │   ├── idossimulationresultplaybackwidget.h
│   │   ├── idossimulationrunarchivewidget.cpp
│   │   ├── idossimulationrunarchivewidget.h
│   │   ├── idossimulationrunstatuswidget.cpp
│   │   ├── idossimulationrunstatuswidget.h
│   │   ├── idossimulationwellcontrolswidget.cpp
│   │   └── idossimulationwellcontrolswidget.h
│   ├── tree/
│   │   ├── idostreemenuprovider.cpp
│   │   ├── idostreemenuprovider.h
│   │   ├── idosdatatreeview.cpp
│   │   ├── idosdatatreeview.h
│   │   ├── idoscasetreeview.cpp
│   │   ├── idoscasetreeview.h
│   │   ├── idostreeview.cpp
│   │   └── idostreeview.h
│   ├── well/
│   │   ├── idoswelldisplaywidget.cpp
│   │   ├── idoswelldisplaywidget.h
│   │   ├── idoswelllogtrackview.cpp
│   │   └── idoswelllogtrackview.h
│   ├── CMakeLists.txt
│   ├── idos_gui.h
│   ├── idoscasedialog.cpp
│   ├── idoscasedialog.h
│   ├── idosdebuginfowidget.cpp
│   ├── idosdebuginfowidget.h
│   ├── idosinterface.cpp
│   ├── idosinterface.h
│   ├── idosnewprojectdialog.cpp
│   ├── idosnewprojectdialog.h
│   ├── idosprojectstartwidget.cpp
│   ├── idosprojectstartwidget.h
│   ├── idospythonconsolewidget.cpp
│   ├── idospythonconsolewidget.h
│   ├── idosruntimeinfowidget.cpp
│   └── idosruntimeinfowidget.h
├── plugins/
│   ├── idosexampleplugin/
│   │   ├── CMakeLists.txt
│   │   ├── idosexampleplugin.cpp
│   │   ├── idosexampleplugin.def
│   │   ├── idosexampleplugin.h
│   │   ├── idospointset.cpp
│   │   ├── idospointset.h
│   │   ├── idospointsetcommands.cpp
│   │   ├── idospointsetcommands.h
│   │   ├── idospointsetmetadata.cpp
│   │   ├── idospointsetmetadata.h
│   │   ├── idospointsetrendermetadata.cpp
│   │   ├── idospointsetrendermetadata.h
│   │   ├── idospointsetrenderprovider.cpp
│   │   ├── idospointsetrenderprovider.h
│   │   ├── idospointsettreemetadata.cpp
│   │   ├── idospointsettreemetadata.h
│   │   ├── idospointsettreeprovider.cpp
│   │   └── idospointsettreeprovider.h
│   ├── idosfracturemodelingplugin/
│   │   ├── images/
│   │   │   ├── app-import.svg
│   │   │   ├── fracture-azimuth.svg
│   │   │   ├── fracture-check.svg
│   │   │   ├── fracture-import.svg
│   │   │   ├── fracture-reconstruct.svg
│   │   │   └── fracture-reserve.svg
│   │   ├── CMakeLists.txt
│   │   ├── idosdfngeneratecommand.cpp
│   │   ├── idosdfngeneratecommand.h
│   │   ├── idosdfngenerationwidget.cpp
│   │   ├── idosdfngenerationwidget.h
│   │   ├── idosdfngenerator.cpp
│   │   ├── idosdfngenerator.h
│   │   ├── idosfracturegeometryreportdialog.cpp
│   │   ├── idosfracturegeometryreportdialog.h
│   │   ├── idosfracturemodelingplugin.cpp
│   │   ├── idosfracturemodelingplugin.h
│   │   └── idosfracturemodelingplugin.qrc
│   ├── idosgeomechanicsplugin/
│   │   ├── images/
│   │   │   ├── geomechanics-energy.svg
│   │   │   ├── geomechanics-fields.svg
│   │   │   ├── geomechanics-stress.svg
│   │   │   └── geomechanics-three-field.svg
│   │   ├── CMakeLists.txt
│   │   ├── idosgeomechanicsplugin.cpp
│   │   ├── idosgeomechanicsplugin.h
│   │   └── idosgeomechanicsplugin.qrc
│   ├── idosmultiscalefractureplugin/
│   │   ├── images/
│   │   │   ├── gui-grid-geometry.svg
│   │   │   ├── multiscale-coarsen.svg
│   │   │   ├── multiscale-connection-check.svg
│   │   │   ├── multiscale-dfn.svg
│   │   │   ├── multiscale-edfm.svg
│   │   │   ├── multiscale-equivalent-mechanics.svg
│   │   │   └── multiscale-equivalent.svg
│   │   ├── CMakeLists.txt
│   │   ├── idosmultiscalefractureplugin.cpp
│   │   ├── idosmultiscalefractureplugin.h
│   │   └── idosmultiscalefractureplugin.qrc
│   ├── CMakeLists.txt
│   └── idosplugin.h
├── providers/
│   ├── case/
│   │   ├── idossimulationcaseeclipsemetadata.cpp
│   │   ├── idossimulationcaseeclipsemetadata.h
│   │   ├── idossimulationcaseeclipseprovider.cpp
│   │   └── idossimulationcaseeclipseprovider.h
│   │   ├── idossimulationsummaryprovider.cpp
│   │   ├── idossimulationsummaryprovider.h
│   │   ├── idossimulationsummaryprovidermetadata.cpp
│   │   └── idossimulationsummaryprovidermetadata.h
│   ├── fluidrock/
│   │   ├── idosfluidrocktextprovider.cpp
│   │   └── idosfluidrocktextprovider.h
│   ├── fracture/
│   │   ├── idosfracturematerialcatalogmetadata.cpp
│   │   ├── idosfracturematerialcatalogmetadata.h
│   │   ├── idosfracturematerialcatalogprovider.cpp
│   │   ├── idosfracturematerialcatalogprovider.h
│   │   ├── idosfractureschedulemetadata.cpp
│   │   ├── idosfractureschedulemetadata.h
│   │   ├── idosfracturescheduleprovider.cpp
│   │   ├── idosfracturescheduleprovider.h
│   │   ├── idosnaturalfracturemetadata.cpp
│   │   ├── idosnaturalfracturemetadata.h
│   │   ├── idosnaturalfractureprovider.cpp
│   │   └── idosnaturalfractureprovider.h
│   ├── grid/
│   │   ├── idosgridboundarymetadata.cpp
│   │   ├── idosgridboundarymetadata.h
│   │   ├── idosgridboundaryprovider.cpp
│   │   ├── idosgridboundaryprovider.h
│   │   ├── idosgrideclipsemetadata.cpp
│   │   ├── idosgrideclipsemetadata.h
│   │   ├── idosgrideclipseprovider.cpp
│   │   ├── idosgrideclipseprovider.h
│   │   ├── idosgridpropertyeclipsemetadata.cpp
│   │   ├── idosgridpropertyeclipsemetadata.h
│   │   ├── idosgridpropertyeclipseprovider.cpp
│   │   ├── idosgridpropertyeclipseprovider.h
│   │   ├── idosgridsurfacemetadata.cpp
│   │   ├── idosgridsurfacemetadata.h
│   │   ├── idosgridsurfaceprovider.cpp
│   │   └── idosgridsurfaceprovider.h
│   ├── las/
│   │   ├── idoswelllasmetadata.cpp
│   │   ├── idoswelllasmetadata.h
│   │   ├── idoswelllasprovider.cpp
│   │   └── idoswelllasprovider.h
│   ├── production/
│   │   ├── idosproductionhistorymetadata.cpp
│   │   ├── idosproductionhistorymetadata.h
│   │   ├── idosproductionhistoryprovider.cpp
│   │   └── idosproductionhistoryprovider.h
│   ├── seismic/
│   │   ├── idosmicroseismicmetadata.cpp
│   │   ├── idosmicroseismicmetadata.h
│   │   ├── idosmicroseismicprovider.cpp
│   │   ├── idosmicroseismicprovider.h
│   │   ├── idossegyprovider.cpp
│   │   ├── idossegyprovider.h
│   │   ├── idossegyprovidermetadata.cpp
│   │   └── idossegyprovidermetadata.h
│   ├── well/
│   │   ├── idoswellcheckshotmetadata.cpp
│   │   ├── idoswellcheckshotmetadata.h
│   │   ├── idoswellcheckshotprovider.cpp
│   │   ├── idoswellcheckshotprovider.h
│   │   ├── idoswellcompletionmetadata.cpp
│   │   ├── idoswellcompletionmetadata.h
│   │   ├── idoswellcompletionprovider.cpp
│   │   ├── idoswellcompletionprovider.h
│   │   ├── idoswellheadermetadata.cpp
│   │   ├── idoswellheadermetadata.h
│   │   ├── idoswellheaderprovider.cpp
│   │   ├── idoswellheaderprovider.h
│   │   ├── idoswellpathmetadata.cpp
│   │   ├── idoswellpathmetadata.h
│   │   ├── idoswellpathprovider.cpp
│   │   ├── idoswellpathprovider.h
│   │   ├── idoswelltopsmetadata.cpp
│   │   ├── idoswelltopsmetadata.h
│   │   ├── idoswelltopsprovider.cpp
│   │   └── idoswelltopsprovider.h
│   ├── CMakeLists.txt
│   └── idos_providers.h
├── python/
│   ├── CMakeLists.txt
│   └── idos_python.h
├── render/
│   ├── providers/
│   │   ├── idosrendermetadata.cpp
│   │   ├── idosrendermetadata.h
│   │   ├── idosrenderprovider.cpp
│   │   ├── idosrenderprovider.h
│   │   ├── idostoolmetadata.cpp
│   │   ├── idostoolmetadata.h
│   │   ├── idostoolprovider.cpp
│   │   └── idostoolprovider.h
│   ├── CMakeLists.txt
│   ├── idos_render.h
│   ├── idosrenderinteractiontool.cpp
│   ├── idosrenderinteractiontool.h
│   ├── idosrenderobject.cpp
│   ├── idosrenderobject.h
│   ├── idosrenderregistry.cpp
│   ├── idosrenderregistry.h
│   ├── idosrenderscene.cpp
│   ├── idosrenderscene.h
│   ├── idosrenderserver.cpp
│   ├── idosrenderserver.h
│   ├── idosrendertypes.cpp
│   ├── idosrendertypes.h
│   ├── idosrenderview.cpp
│   ├── idosrenderview.h
│   ├── idostool.cpp
│   ├── idostool.h
│   ├── idostoolbar.cpp
│   ├── idostoolbar.h
│   ├── idostoolregistry.cpp
│   └── idostoolregistry.h
├── CMakeLists.txt
└── main.cpp
```


### 多尺度裂缝分析窗口（2026-10-16）

以下同名 `.h/.cpp` 对均位于 `src/plugins/idosmultiscalefractureplugin`：

- `idosfielddifferencewidget`：属性场和报告步选择、差异结果提示。
- `idosfielddifferencecommand`：活动单元差异、输入校验、撤销与缓存重做，含命令 Metadata。
- `idoscurveevaluationwidget`：曲线来源、评价指标、样本表格及报告 CSV 导出。
- `idoscurveevaluation`：CSV/汇总曲线读取、时间对齐和误差评价。
- `idoscurveevaluationcommand`：只读评价命令及 Metadata。

`src/core/data/grid/idosgridproperty` 增加指定报告步的只读数值访问；不包含插件计算或界面逻辑。
