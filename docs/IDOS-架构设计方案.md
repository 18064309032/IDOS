# IDOS 架构设计方案

适用仓库：`D:\CMakeCoder\IDOS`。
状态：当前有效方案。
目标：把 IDOS 做成可扩展的数据平台，基础平台负责工程、对象、树、窗口、加载和显示；业务能力通过明确的数据对象、树 Provider、数据 Provider、分析模块和后续插件扩展。

## 1. 总体原则

- 平台做通用能力，业务做可注册扩展。
- 工程中的所有业务数据统一由 `IDOSProject` 管理。
- 跨模块长期引用只使用稳定 `objectId`，不保存业务对象裸指针。
- 树只是展示结构，不决定对象生命周期。
- 数据树和工况树是两棵不同的业务树，不再使用 Input/Model 这种容易混淆的旧分类。
- 不使用自定义命名空间。
- 不引入万能 `Context`、`Center`、`Workflow`、`Bridge` 类。
- 能在具体模块解决的问题，不上升成通用框架。
- 根目录公共头面向二次开发；内部目录头文件默认只给模块内部使用。

## 2. 目标目录结构

```text
D:\CMakeCoder\IDOS
├─ src
│  ├─ core
│  │  ├─ idos_core.h
│  │  ├─ idosproject.h
│  │  ├─ idosobject.h
│  │  ├─ idosobjectregistry.h
│  │  ├─ idoscommand.h
│  │  ├─ idossession.h
│  │  ├─ case
│  │  ├─ command
│  │  ├─ data
│  │  ├─ object
│  │  ├─ project
│  │  └─ tree
│  ├─ gui
│  │  ├─ idos_gui.h
│  │  ├─ idosdockwidget.h
│  │  ├─ idospropertywidget.h
│  │  ├─ idosworkspacewidget.h
│  │  ├─ case
│  │  ├─ curve
│  │  ├─ grid
│  │  ├─ input
│  │  ├─ render
│  │  ├─ result
│  │  ├─ table
│  │  ├─ tree
│  │  └─ well
│  ├─ app
│  │  ├─ idos_app.h
│  │  ├─ idosapplication.h
│  │  ├─ idosmainwindow.h
│  │  ├─ idosappinterface.h
│  │  ├─ action
│  │  ├─ command
│  │  ├─ layout
│  │  └─ plugin
│  │     ├─ idos_plugin.h
│  │     ├─ idosplugin.h
│  │     ├─ idosinterface.h
│  │     ├─ idospluginmanager.*
│  │     ├─ idospluginloader.*
│  │     ├─ idospluginmetadata.*
│  │     └─ idospluginregistry.*
│  ├─ assistant
│  │  ├─ idos_assistant.h
│  │  ├─ idosassistantservice.h
│  │  ├─ idosassistantsession.h
│  │  ├─ idosassistantcontext.h
│  │  ├─ idosassistanttool.h
│  │  └─ idosassistanttoolregistry.h
│  ├─ python
│  │  ├─ idos_python.h
│  │  ├─ idospythonruntime.h
│  │  └─ idospythonmodule.h
│  ├─ providers
│  │  ├─ idos_providers.h
│  │  ├─ idosdataprovider.h
│  │  ├─ idosdataloadservice.h
│  │  ├─ eclipse
│  │  ├─ petrel
│  │  ├─ navigator
│  │  ├─ well
│  │  ├─ production
│  │  ├─ streamline
│  │  └─ seismic
│  ├─ analysis
│  │  ├─ idos_analysis.h
│  │  ├─ idosanalysisalgorithm.h
│  │  ├─ idosanalysisresult.h
│  │  ├─ curve
│  │  ├─ fracture
│  │  ├─ grid
│  │  └─ well
│  ├─ render_core
│  ├─ render_qt
│  └─ render_adapters
│     └─ idos
├─ python
├─ tests
├─ docs
├─ data
└─ thirdparty
```

说明：

- `src/render` 是旧接口，最终移除；新增代码使用 `render_core`、`render_qt`、`render_adapters/idos`。
- `src/auto` 是待拆分旧模块，不作为目标架构保留。
- 根目录 `python` 只放纯 Python SDK 脚本，结构应对应 C++ 暴露出来的 Python 接口。
- `src/python` 是 C++ Python 模块，不放 `.py` 脚本。
- 当前暂不维护 `sdk` 目录。未来由构建把需要二次开发的公共头扁平导出到 SDK 目录。

## 3. 模块职责

| 模块 | 职责 |
|---|---|
| `core` | 工程、对象、会话、命令、业务状态、基础数据结构 |
| `gui` | 树、属性面板、工作区、业务窗口、对话框和交互控件 |
| `app` | 程序装配、主窗口、Action、布局、插件系统和应用级注册 |
| `assistant` | AI 助手服务、会话、上下文、工具和工具注册 |
| `python` | C++ Python 运行时、模块注入和控制台支撑 |
| `providers` | 数据源识别、文件解析、数据加载服务 |
| `analysis` | 独立分析算法和分析结果 |
| `render_core` | 渲染核心对象、场景、数据表达 |
| `render_qt` | Qt 渲染窗口和交互适配 |
| `render_adapters/idos` | IDOS 业务对象到渲染对象的适配 |

## 4. 公共 API 规则

- 模块根目录头文件视为公共 API。
- 模块子目录头文件默认是内部实现或业务分组。
- 公共 API 头文件不得 include 本模块子目录内部头。
- 公共 API 函数签名不得暴露内部目录类型。
- 如果某个子目录类型需要给二次开发使用，应提升到模块根目录。
- 聚合头只 include 同模块根目录公共头。
- 插件 SDK 头放在 `src/app/plugin`，对外头文件使用 `idos_plugin.h`。

## 5. 插件系统

插件系统放在 `src/app/plugin`，不放进 `core`。

目标类：

- `IDOSPlugin`
- `IDOSInterface`
- `IDOSPluginManager`
- `IDOSPluginLoader`
- `IDOSPluginMetadata`
- `IDOSPluginRegistry`
- `idos_plugin.h`

`idos_plugin.h` 是给外部插件开发者使用的导出头，内部只定义插件导出相关宏和必要入口约定。内部模块不依赖旧工程的 `idospluginapi.h`。

## 6. 数据加载

数据加载统一使用 `IDOSDataLoadService`。

职责：

- 根据文件路径识别合适的 Provider。
- 调用 Provider 解析文件。
- 把解析出的对象加入 `IDOSProject`。
- 返回加载成功的对象 ID。
- 保存最后一次错误信息。

命名上不再使用 `ImportCoordinator`，因为平台不应该把数据进入工程这件事只限定为“导入”。后续打开缓存、脚本加载、批量加载等都可以归入数据加载服务。

## 7. Provider 体系

Provider 分两类：

- 数据 Provider：负责识别和解析外部数据。
- 树 Provider：负责把业务对象组织成树节点。

数据 Provider 放在 `src/providers`。
树 Provider 放在 `src/gui` 对应业务目录或 `src/gui/tree` 的基础接口中。

Provider 注册尽量简单，不额外引入 `AppContext`、`BuiltinRegistrar`、`registerBuiltinTreeProviders` 这类间接层。当前阶段允许在 `IDOSMainWindow` 中直接 new 注册表并注册内置 Provider。

## 8. 两棵树设计

平台保留两棵业务树：

- 数据树：显示工程中的基础数据对象，例如井、网格、属性、生产数据等。
- 工况树：显示工况对象及其引用关系，例如某个模拟工况引用哪些井、网格、参数和结果。

两棵树共用基础树节点和基础 Model 能力，但各自有自己的 Model 和 Provider。

目标类：

- `IDOSTreeNode`
- `IDOSObjectTreeNode`
- `IDOSTreeGroupNode`
- `IDOSTreePartNode`
- `IDOSTreeReferenceNode`
- `IDOSTreePartKey`
- `IDOSTreeBuilder`
- `IDOSTreeProvider`
- `IDOSDataTreeProvider`
- `IDOSCaseTreeProvider`
- `IDOSTreeProviderRegistry`
- `IDOSDataTreeModel`
- `IDOSDataTreeView`
- `IDOSCaseTreeModel`
- `IDOSCaseTreeView`

设计约束：

- 不使用 `domain` 字符串区分树，因为数据树和工况树已经是不同 Model。
- 不使用 `IDOSTreeBuildContext`。
- Builder 只负责创建树节点，不保存业务上下文。
- 树节点通过 `objectId`、part key 或 reference 表达含义。
- 删除对象和移除引用必须区分。

## 9. 工况对象

工况对象必须有基类。

目标类：

- `IDOSCaseItemRef`
- `IDOSCaseObject`
- `IDOSSimulationCaseObject`

`IDOSCaseObject` 继承业务数据对象，表示一个可保存、可显示、可引用数据的工况。
`IDOSCaseItemRef` 用于表示工况树中对数据树对象或对象内部子项的引用。

工况树不是数据树的复制。它可以引用数据树中的井、网格、属性、结果和其他业务节点，但不拥有这些基础数据。

## 10. Python 设计

Python 分成两部分：

- `src/python`：C++ Python 运行时、模块注入、控制台支撑。
- 根目录 `python`：纯 Python SDK 脚本。

根目录 `python` 目录只放 `.py` 相关脚本，不作为 VS C++ 工程里的源码筛选器展示。
Python SDK 的目录结构应接近 C++ 暴露出来的接口，例如未来运行时可形成 `idos.core`、`idos.gui`、`idos.app` 等 Python 包。

## 11. Assistant 设计

AI 助手作为独立模块，模块名使用 `assistant`。

目标职责：

- 助手服务。
- 助手会话。
- 上下文整理。
- 工具抽象。
- 工具注册。

助手 UI 不放在 `src/assistant`，而是放在 `src/gui` 对应界面位置。`src/assistant` 只负责能力，不负责窗口。

## 12. auto 模块处理

`src/auto` 是旧模块，目标架构中不保留。

拆分方向：

- Action 相关能力进入 `src/app/action`。
- Python 脚本校验和运行相关能力进入 `src/python`。
- Assistant 相关能力进入 `src/assistant`。

拆分完成后删除 `src/auto`。

## 13. MainWindow 装配原则

当前阶段可以在 `IDOSMainWindow` 中直接创建并持有注册表裸指针。

例如：

- 创建 `IDOSTreeProviderRegistry`。
- 注册内置数据树 Provider。
- 注册内置工况树 Provider。
- 创建数据树 Model/View。
- 创建工况树 Model/View。
- 把当前 Project 设置给两棵树。

不引入额外 AppContext，不增加不必要的 Registrar。

## 14. 后续实施顺序

1. 清理旧目录和旧命名。
2. 固化两棵树和 Provider 注册机制。
3. 使用 `IDOSDataLoadService` 接入真实数据加载。
4. 补齐工况对象和工况树引用显示。
5. 整理 `src/python` 与根目录 `python`。
6. 拆分并移除 `src/auto`。
7. 接入插件系统。
8. 再补 Assistant、Python 控制台和高级业务扩展。

## 15. 验收标准

- 新增一种数据类型时，只需要新增数据对象、数据 Provider 和树 Provider，不需要在主窗口里写大量类型判断。
- 新增一种工况时，只需要新增工况对象和工况树 Provider。
- 数据树和工况树职责清楚，不混用 Input/Model 旧概念。
- `src/python` 不出现纯 Python 脚本。
- 根目录 `python` 不参与 C++ 工程源码筛选器。
- 新代码不依赖旧 `src/render`。
- 旧 `src/auto` 能逐步清空并删除。
- 公共 API 不暴露内部目录类型。
