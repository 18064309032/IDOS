# IDOS 源码目录

更新日期：2026-10-13。

本文是已确认的固定源码目录及扩展归属依据。主要框架、具体井/网格/工况实现和通用工具类已按此布局落地；清单不是整个仓库文件枚举，实施及未完成状态见工作进度。未经用户确认，不改变目录划分，不增加方案外类或包装层。

配套文档：[架构设计](IDOS架构设计.md)、[开发指南](开发指南.md)、[工作进度](工作进度-CJW.md)。

公共接口放在所属功能目录根部，不要求全部集中在链接模块根部。同名 `.h` 和 `.cpp` 必须同目录。功能目录不单独建立 CMake 模块，除非另行确认。

`IDOSProject` 放在 `core` 根目录，不建立 `core/project`；不设置 `app/render`，不保留 `IDOSRenderController`。通用树结构、模型和同步位于 `core/tree`，树视图和 GUI 扩展接口位于 `gui/tree`。插件公共访问接口位于 `gui`，入口契约位于 `plugins`，加载、描述、列表模型和管理界面均位于 `app` 根目录，不设置 `app/plugins`。

## 固定文件布局

2026-10-13 按用户要求补充并实现视图工具框架五组成对文件，位于现有 render 根目录，不改变目录划分。接口见架构设计第 13 节；具体业务工具待在 app 或插件中实现。

以下列出实际文件名；既有基础设施保留，新增文件仅限已经确认的架构类及具体实现。具体插件实现暂不讨论，原有插件不因本清单省略而删除。

```text
src/
    CMakeLists.txt
    main.cpp

    core/
        CMakeLists.txt
        idos_core.h
        idosobject.h
        idosobject.cpp
        idosproject.h
        idosproject.cpp
        idosprojectmetadata.h
        idosprojectmetadata.cpp

    core/data/
        idosdataobject.h
        idosdataobject.cpp
        idosdatametadata.h
        idosdatametadata.cpp
        idosdataregistry.h
        idosdataregistry.cpp

    core/data/well/
        idoswell.h
        idoswell.cpp
        idoswellmetadata.h
        idoswellmetadata.cpp
        idoswellcompletion.h
        idoswellcompletion.cpp
        idoswellcontrol.h
        idoswellcontrol.cpp
        idoswellgeometryresolver.h
        idoswellgeometryresolver.cpp
        idoswellhead.h
        idoswellhead.cpp
        idoswelllogchannel.h
        idoswelllogchannel.cpp
        idoswelllogset.h
        idoswelllogset.cpp
        idoswellmarker.h
        idoswellmarker.cpp
        idoswellmarkerset.h
        idoswellmarkerset.cpp
        idoswellpath.h
        idoswellpath.cpp
        idoswellpathpoint.h
        idoswellpathpoint.cpp
        idoswellsegment.h
        idoswellsegment.cpp
        idoswellspatialreference.h
        idoswellspatialreference.cpp

    core/data/grid/
        idosgrid.h
        idosgrid.cpp
        idosgridmetadata.h
        idosgridmetadata.cpp
        idosgridcell.h
        idosgridcell.cpp
        idosgridproperty.h
        idosgridproperty.cpp

    core/data/case/
        idoscaseobject.h
        idoscaseobject.cpp
        idoscaseitemref.h
        idoscaseitemref.cpp
        idossimulationcaseobject.h
        idossimulationcaseobject.cpp
        idossimulationcasemetadata.h
        idossimulationcasemetadata.cpp

    core/tree/
        idostree.h
        idostree.cpp
        idostreeitem.h
        idostreeitem.cpp
        idostreeprovider.h
        idostreeprovider.cpp
        idostreemetadata.h
        idostreemetadata.cpp
        idostreeregistry.h
        idostreeregistry.cpp
        idostreemodel.h
        idostreemodel.cpp
        idostreeserver.h
        idostreeserver.cpp

    core/command/
        idoscommand.h
        idoscommand.cpp
        idoscommandmetadata.h
        idoscommandmetadata.cpp
        idoscommandregistry.h
        idoscommandregistry.cpp
        idoscommandmanager.h
        idoscommandmanager.cpp

    core/log/
        idoslogger.h
        idoslogger.cpp
        idosloglevel.h
        idosloglevel.cpp
        idoslogrecord.h
        idoslogrecord.cpp
        idoslogtarget.h
        idosconsoletarget.h
        idosconsoletarget.cpp
        idosfiletarget.h
        idosfiletarget.cpp
        idosmsvctarget.h
        idosmsvctarget.cpp

    providers/
        CMakeLists.txt
        idos_providers.h
        idosdataprovider.h
        idosdataprovider.cpp
        idosdataprovidermetadata.h
        idosdataprovidermetadata.cpp
        idosdataproviderregistry.h
        idosdataproviderregistry.cpp

    providers/well/
        idoswellheaderprovider.h
        idoswellheaderprovider.cpp
        idoswellheadermetadata.h
        idoswellheadermetadata.cpp
        idoswellpathprovider.h
        idoswellpathprovider.cpp
        idoswellpathmetadata.h
        idoswellpathmetadata.cpp

    providers/las/
        idoswelllasprovider.h
        idoswelllasprovider.cpp
        idoswelllasmetadata.h
        idoswelllasmetadata.cpp

    providers/eclipse/
        idosgrideclipseprovider.h
        idosgrideclipseprovider.cpp
        idosgrideclipsemetadata.h
        idosgrideclipsemetadata.cpp
        idossimulationcaseeclipseprovider.h
        idossimulationcaseeclipseprovider.cpp
        idossimulationcaseeclipsemetadata.h
        idossimulationcaseeclipsemetadata.cpp

    gui/
        CMakeLists.txt
        idos_gui.h
        idosinterface.h
        idosinterface.cpp
        idosnewprojectdialog.h
        idosnewprojectdialog.cpp
        idosdebuginfowidget.h
        idosdebuginfowidget.cpp
        idosruntimeinfowidget.h
        idosruntimeinfowidget.cpp

    gui/tree/
        idostreeview.h
        idostreeview.cpp
        idostreeguiprovider.h
        idostreeguiprovider.cpp

    render/
        CMakeLists.txt
        idos_render.h
        idosrenderobject.h
        idosrenderobject.cpp
        idosrenderprovider.h
        idosrenderprovider.cpp
        idosrendermetadata.h
        idosrendermetadata.cpp
        idosrenderregistry.h
        idosrenderregistry.cpp
        idosrenderscene.h
        idosrenderscene.cpp
        idosrenderview.h
        idosrenderview.cpp
        idosrenderserver.h
        idosrenderserver.cpp
        idostoolmetadata.h
        idostoolmetadata.cpp
        idostoolregistry.h
        idostoolregistry.cpp
        idostoolprovider.h
        idostoolprovider.cpp
        idostoolbar.h
        idostoolbar.cpp
        idostool.h
        idostool.cpp

    app/
        CMakeLists.txt
        idos_app.h
        idosapplication.h
        idosapplication.cpp
        idosappinterface.h
        idosappinterface.cpp
        idosmainwindow.h
        idosmainwindow.cpp
        idospluginmetadata.h
        idospluginmetadata.cpp
        idospluginregistry.h
        idospluginregistry.cpp
        idospluginlistmodel.h
        idospluginlistmodel.cpp
        idospluginmanagerwidget.h
        idospluginmanagerwidget.cpp

    app/well/
        idoswelltreeprovider.h
        idoswelltreeprovider.cpp
        idoswelltreemetadata.h
        idoswelltreemetadata.cpp
        idoswelltreeguiprovider.h
        idoswelltreeguiprovider.cpp
        idoswellrenderprovider.h
        idoswellrenderprovider.cpp
        idoswellrendermetadata.h
        idoswellrendermetadata.cpp
        idoswelllogtrackview.h
        idoswelllogtrackview.cpp
        idoscreatewellcommand.h
        idoscreatewellcommand.cpp
        idosdeletewellcommand.h
        idosdeletewellcommand.cpp

    app/grid/
        idosgridtreeprovider.h
        idosgridtreeprovider.cpp
        idosgridtreemetadata.h
        idosgridtreemetadata.cpp
        idosgridtreeguiprovider.h
        idosgridtreeguiprovider.cpp
        idosgridrenderprovider.h
        idosgridrenderprovider.cpp
        idosgridrendermetadata.h
        idosgridrendermetadata.cpp
        idosdeletegridcommand.h
        idosdeletegridcommand.cpp
        idosdeletepropertycommand.h
        idosdeletepropertycommand.cpp
        idosimportrestartresultscommand.h
        idosimportrestartresultscommand.cpp

    app/case/
        idossimulationcasetreeprovider.h
        idossimulationcasetreeprovider.cpp
        idossimulationcasetreemetadata.h
        idossimulationcasetreemetadata.cpp
        idossimulationcasetreeguiprovider.h
        idossimulationcasetreeguiprovider.cpp
        idoscasedialog.h
        idoscasedialog.cpp
        idoscreatecasecommand.h
        idoscreatecasecommand.cpp
        idosdeletecasecommand.h
        idosdeletecasecommand.cpp

    app/command/
        idosrenameobjectcommand.h
        idosrenameobjectcommand.cpp
        idosimportcommands.h
        idosimportcommands.cpp

    plugins/
        CMakeLists.txt
        idosplugin.h
        README.md
        idosexampleplugin/
            CMakeLists.txt
            README.md
            idosexampleplugin.def
            idosexampleplugin.h
            idosexampleplugin.cpp
            idospointset.h
            idospointset.cpp
            idospointsetmetadata.h
            idospointsetmetadata.cpp
            idospointsetcommands.h
            idospointsetcommands.cpp
            idospointsettreeprovider.h
            idospointsettreeprovider.cpp
            idospointsettreemetadata.h
            idospointsettreemetadata.cpp
            idospointsettreeguiprovider.h
            idospointsettreeguiprovider.cpp
            idospointsetrenderprovider.h
            idospointsetrenderprovider.cpp
            idospointsetrendermetadata.h
            idospointsetrendermetadata.cpp

    assistant/
        CMakeLists.txt
        idos_assistant.h
        idosassistantservice.h
        idosassistantservice.cpp
        idosassistantwidget.h
        idosassistantwidget.cpp
        idoslocalmodelprocess.h
        idoslocalmodelprocess.cpp
        idoslocalmodelservice.h
        idoslocalmodelservice.cpp

    analysis/
        CMakeLists.txt
        idos_analysis.h

    python/
        CMakeLists.txt
        idos_python.h
```

## 实施约束

- `core/data` 保存数据，不依赖界面或渲染；类型专属数据位于 `well`、`grid`、`case`。
- 内置树构建、菜单、渲染与业务命令按类型放在 `app/well`、`app/grid`、`app/case`；跨类型命令放在 `app/command`。
- 数据读取按已确定的来源或格式组织；井头与轨迹读取暂保留 `providers/well`，不擅自编造格式目录。
- 被替代的旧树模型、节点子类、菜单 Provider、类型注册和具体 RenderObject 等在接通新调用路径后删除。
- 具体命令 Metadata 的文件拆分尚未确定，实施相关功能前应明确清单，不据此自行增加新的抽象。
- `IDOSInterface` 已位于 gui，仅前置声明 RenderRegistry 和 ToolRegistry，不引入对 app 具体类型及 render 的链接依赖。
- 通用 Tool、ToolMetadata、ToolRegistry、ToolProvider、ToolBar 的头文件和实现放在 render 根部；内置具体工具放在 app 已有功能目录，插件工具放在插件内，不新增 tools 子目录。
- 目录固定不代表直接批准删除任何未审查的既有文件；迁移前核对其职责和完整调用路径。

