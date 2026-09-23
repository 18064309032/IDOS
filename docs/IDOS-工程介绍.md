# IDOS 工程介绍

> 适用仓库：`D:\CMakeCoder\IDOS`。日期：2026-09-23。
> 本文描述**当前源码的真实状态**（是什么、怎么构建、怎么组织、做到哪一步），面向新加入的开发者。
> 目标设计见 [IDOS-架构设计方案](IDOS-架构设计方案.md)，功能规划见 [IDOS平台功能清单](IDOS平台功能清单.md)。

---

## 1. 一句话定位

**IDOS 是一个面向油藏工程的桌面数据平台**：把井、油藏角点网格、网格属性、模拟工况等专业数据导入统一工程，通过两棵树组织浏览，并计划（尚未实现）用 VTK 做三维可视化。对标 Petrel 的数据组织与可视化能力，但不复刻其内部架构。

当前阶段：**架构骨架完整、核心闭环未通**——分层、扩展点、边界检查已就位；工程读写、三维渲染、撤销栈、后台任务尚未实现，启动后展示的是一份硬编码演示数据。

---

## 2. 技术栈

| 维度 | 选型 |
|---|---|
| 语言标准 | C++17 |
| GUI 框架 | Qt 5（Core / Widgets / Network），AUTOMOC / AUTOUIC / AUTORCC |
| 构建系统 | CMake ≥ 3.16（声明值，见 §9 已知问题）+ CMakePresets（VS 2022 x64） |
| 编译器 | MSVC（`/utf-8` 强制源文件按 UTF-8 解析） |
| 三维渲染 | VTK 9.6.0（**可选，`IDOS_ENABLE_RENDER` 默认 OFF，当前未启用**） |
| 专业格式解析 | OPM opm-common（静态库，ECLIPSE .DATA / .EGRID 解析） |
| 界面组件 | SARibbon（Ribbon 主窗口）、Qt Advanced Docking System（停靠面板） |
| 国际化 | Qt Linguist（`i18n/idos_zh_CN.ts`，启动时按 QLocale 加载） |
| 测试 | CTest（当前仅 `check_includes` 一项静态检查，无行为测试） |

---

## 3. 构建与运行

```powershell
# 1. 配置 Qt 路径（一次性）
copy CMakeUserPresets.example.json CMakeUserPresets.json
#    编辑其中的 IDOS_QT_DIR 指向本机 Qt5 安装根（含 lib/cmake/Qt5）

# 2. 配置 + 构建
cmake --preset win-x64-debug
cmake --build --preset debug

# 3. 运行（Debug 产物带 d 后缀）
..\build\win-x64-debug\bin\Debug\idosd.exe
```

构建期自动做的事：

- **架构边界检查**：`tools/check_includes.ps1` 作为 `ALL` 目标每次构建强制执行，违规即构建失败（详见 §7）。
- **产物归集**：所有 DLL/EXE 统一输出 `build/bin/<配置>/`，避免 `0xC0000135` DLL 找不到。
- **依赖部署**：POST_BUILD 自动 `windeployqt`、拷贝 SARibbon/ADS 的 Debug 或 Release 对应 DLL、拷贝 VC DebugCRT、（启用渲染时）拷贝 VTK 全部 DLL。

开启渲染模块：`cmake -DIDOS_ENABLE_RENDER=ON --preset win-x64-debug`。

---

## 4. 模块架构

### 4.1 依赖方向

```
                ┌────────────┐
                │  idos.exe  │  main.cpp（入口 + 演示数据装配）
                └─────┬──────┘
        ┌─────────────┼─────────────┬──────────────┐
        ▼             ▼             ▼              ▼
  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────────┐
  │ idos_app │  │ idos_gui │  │  (VTK*)  │  │  …           │
  │  [占位]  │  │ 主窗口+树 │  │  可选    │  │              │
  └────┬─────┘  └────┬─────┘  └──────────┘  └──────────────┘
       │             │
       └──────┬──────┘
              ▼
      ┌───────────────┐     ┌──────────────────────────────┐
      │ idos_providers│────▶│ OPM opmcommon（静态，PRIVATE）│
      │ 格式识别+解析 │     └──────────────────────────────┘
      └───────┬───────┘
              ▼
      ┌───────────────┐
      │   idos_core   │──── Qt5::Core Qt5::Gui
      │ 对象模型+专业数据│     （不链接 Widgets，禁止依赖界面）
      └───────────────┘

  占位模块（仅 dummy.cpp，零功能）：idos_render / idos_python /
                                     idos_assistant / idos_analysis
```

**硬性规则**：core 不得反向依赖 providers/gui/render/app；这条同时被链接关系（core 不链 Widgets）和静态检查（core 禁止 include GUI 头）双重守护。

### 4.2 各模块职责

| 模块 | target | 职责 | 现状 |
|---|---|---|---|
| `src/core` | `idos_core.dll` | 工程容器、领域对象基类、类型注册表、专业数据（井/网格/属性/工况） | ✅ 有实现 |
| `src/providers` | `idos_providers.dll` | 格式识别（Metadata）、解析（Provider）、导入编排（Coordinator） | ✅ 有实现（LAS 仅骨架） |
| `src/gui` | `idos_gui.dll` | 主窗口、通用树机制、输入树/模型树两个面板 | ✅ 有实现 |
| `src/render` | `idos_render.dll` | 数据到 VTK 的转换、场景、交互 | ⬜ 占位 |
| `src/app` | `idos_app.dll` | 装配、会话、撤销命令、任务调度 | ⬜ 占位 |
| `src/python` / `assistant` / `analysis` | 各自 DLL | 脚本、AI 助手、专业算法 | ⬜ 占位 |

---

## 5. 核心机制（读代码先读这四条）

### 5.1 对象模型：稳定 ID + 弱引用

```
IDOSObject            objectId（构造自动生成、生命周期内不变）+ name
  └─ IDOSDataObject   typeId 纯虚 / containerId / mergeFrom / resolveReferences
       ├─ IDOSGrid         角点网格：扁平双精度 COORD + ZCORN + ACTNUM
       ├─ IDOSGridProperty 属性场：挂 gridId（containerId → 树上进网格子树）
       ├─ IDOSWell         井：井头/轨迹/曲线/分层/射孔/分段/井控 组合
       └─ IDOSModel        模拟工况：gridId + wellIds（按 objectId 弱引用）
```

- `IDOSProject` 是唯一容器：`QHash<objectId, IDOSDataObject*>` + Qt 父子所有权，统一转发 added/removed/changed 三个信号。
- **跨模块引用一律走 objectId 字符串，不持裸指针**；树节点只存 `m_objectId`，按需回 Project 查询。
- 工况对井/网格是"配料清单"式引用——数据本体只有一份，由 Project 持有。

### 5.2 导入流水线：五步编排

```
文件路径
  → ① IDOSProviderRegistry::metadataForFile()   扩展名 + canHandle() 嗅探排序
  → ② metadata->createProvider()                 每次 fresh 实例
  → ③ provider->read()                           解析出"碎片"对象（不碰 Project）
  → ④ project->objectByName() find-or-create     同名则 target->mergeFrom(碎片)
  → ⑤ target->resolveReferences(project)         把导入期的名字解析成 objectId
```

职责切分是这套设计最成功的部分：**解析在 Provider、合并语义在 `mergeFrom()` 多态、编排只在 Coordinator**——Coordinator 全文件 66 行，不含任何类型判断。三个多态钩子（`mergeFrom` / `resolveReferences` / `pendingImportPaths`）默认 no-op，具体类型按需 override。

### 5.3 树机制：机制层 / 业务层分离

```
IDOSTreeModel（QAbstractItemModel，机制层，不认识任何业务类型）
  ├─ TreeScope 过滤：All / Input / Model —— 两棵树是同一份 objects 的两种视图
  ├─ typeId → IDOSTreeItemProvider 查表分发（策略模式）
  ├─ 虚函数扩展点：shouldShow() / groupLabelOf()
  └─ rebuildContainerSubtree() / refreshReferencingSubtrees()

IDOSTreeItemProvider（按类型注册）
  ├─ buildChildren()    子树结构（井的井头/轨迹/曲线…）
  ├─ displayName()      显示名唯一裁决点
  ├─ needsRebuild()     objectChanged 时是否整体重建
  └─ addMenuActions()   右键菜单扩展
```

新增一种数据类型**不需要改** `IDOSTreeModel`、`IDOSMainWindow`——注册一个 ItemProvider 即可上树，这为将来插件扩展预留了接入点。`IDOSInputTreeModel` 构造函数有效代码只有 4 行，正说明机制层抽取干净。

### 5.4 类型注册表

`IDOSTypeRegistry`（单例，跨 DLL 共享）保存 `typeId → IDOSObjectTypeMetadata`（显示名、类别、工厂函数）：

- `IDOSObjectCategory::Input` → 进输入树（可配 `inputGroup` 分组，如"井组"）
- `IDOSObjectCategory::Model` → 进模型树
- 未知类型兜底按 Model 处理

官方类型与未来插件类型走同一注册通道。

---

## 6. 目录结构

```
IDOS/
├─ CMakeLists.txt              根构建：Qt 查找、三方库 IMPORTED target、边界检查
├─ CMakePresets.json           win-x64-debug / win-x64-release 预设
├─ CMakeUserPresets.json       本机 Qt 路径（不入库，有 example 模板）
├─ src/
│  ├─ main.cpp                 入口：加载翻译 → 造演示数据 → 开主窗口
│  ├─ core/                    idos_core：工程 + 领域对象 + 注册表
│  │   ├─ idosobject / idosproject / idostyperegistry
│  │   └─ data/
│  │      ├─ idosdataobject.h  领域对象基类（扩展钩子都在这）
│  │      ├─ grid/             IDOSGrid / IDOSGridProperty / IDOSGridCell
│  │      ├─ well/             11 个类：井头/轨迹/曲线/分层/射孔/分段/井控…
│  │      └─ model/            IDOSModel（模拟工况）
│  ├─ providers/               idos_providers：Registry + Coordinator + 格式实现
│  │   ├─ well/                井头文本 + LAS（骨架）
│  │   ├─ grid/                ECLIPSE .EGRID（OPM EclipseGrid）
│  │   └─ model/               ECLIPSE .DATA（OPM Parser）
│  ├─ gui/                     idos_gui：主窗口 + 树
│  │   ├─ tree/                通用机制：Node / Model / View / ItemProvider / MenuProvider
│  │   ├─ inputtree/           输入树（TreeScope::Input + 井 Provider）
│  │   └─ modeltree/           模型树（TreeScope::Model + 网格/工况 Provider）
│  ├─ render|app|python|assistant|analysis/   占位模块（analysis 职责判据见其 CMakeLists）
├─ tools/check_includes.ps1    include 架构边界守护（构建期强制）
├─ i18n/                       翻译（idos_zh_CN.ts/.qm）
├─ resources/                  Qt 资源
├─ thirdparty/                 预编译三方库（include/lib/bin 三段式）
│   ├─ SARibbon / AdvancedDockingSystem / opm / vtk9.6.0
├─ docs/                       本组文档
└─ .vscode/、.box-agent*/      编辑器与工具目录
```

---

## 7. 质量保障机制

**`tools/check_includes.ps1`**（每次构建 + CTest 双入口，违规退出码 1 → 构建失败）：

1. 全项目禁止 `../` 目录穿越式 include
2. `core/` 禁止 include **任何其他模块**的头（gui/providers/render/app/扩展侧；core↔gui 这条另有"core 不链接 Widgets"兜底，双保险）
3. `gui/tree/`（机制层）禁止 include 业务层（inputtree / modeltree / 主窗口）
4. `gui` 业务层互不依赖（inputtree ⇄ modeltree 隔离）
5. `providers/` 禁止 include gui 侧与 app 侧头（解析层不认识界面与装配）
6. `render/` 禁止 include gui / providers / app / 扩展侧头（渲染只依赖 core + VTK）
7. `gui/` 禁止 include providers 侧、app 侧与扩展模块侧头（允许 render 侧：设计约定 GUI→Render）
8. `analysis/` `python/` `assistant/` 只允许 include core 侧与自身根头（不得互相依赖、不得反向依赖 app）
9. `app/` 禁止 include gui / providers / render 的内部子目录（app 只走公共头与工厂；app 是唯一可认识所有模块公共面的装配层）

**其他约定**（散落在代码注释中，属于隐性规范）：

- 模块只暴露根目录为 PUBLIC include，内部头一律从根写全路径（如 `#include "data/grid/idosgrid.h"`）
- 类注释用 `@brief` 中文，UI 文本用可翻译的英文/`tr()`，槽函数具名
- Registry 类不自注册，启动时集中注册，避免跨 DLL 静态初始化顺序问题

---

## 8. 当前状态

### 8.1 已实现

- ✅ 对象模型：objectId 身份、Project 容器、信号汇聚
- ✅ 专业数据类型：网格（双精度角点数组）、井（11 类值对象组合）、属性场、工况弱引用
- ✅ 导入链路：EGRID（OPM，含 COORD/ZCORN 布局转换与维度校验）、.DATA（工况名 + WELSPECS 井名）、井头文本、LAS 骨架
- ✅ 双树界面：SARibbon 主窗 + ADS 停靠（输入树/模型树上下分割），机制层可插拔
- ✅ 构建边界守护、Debug/Release 双配置产物部署、中文翻译加载

### 8.2 明确缺失（对比设计文档 P0–P5）

| 能力 | 状态 |
|---|---|
| 工程保存/打开（.idos） | ❌ 不存在，启动数据硬编码在 `main.cpp` |
| 三维渲染（VTK） | ❌ Render 模块占位，主窗口中央为 QLabel 占位 |
| 撤销/重做、命令模式 | ❌ 无 QUndoStack |
| 后台解析、取消、进度 | ❌ 全同步主线程，大文件会阻塞 UI |
| 行为测试 | ❌ 仅一条静态检查 |
| Python / 插件 / AI 助手 | ❌ 占位 |

### 8.3 已知问题（写文档时核对代码确认，修复前请先看架构方案对应章节）

1. **同名异类静默丢数据**：Coordinator 按 `name` 找目标，类型不符时 `mergeFrom` no-op 但碎片仍被 `delete`，用户看到"成功"实则未导入（`idosimportcoordinator.cpp`）。
2. **递归导入未实现**：`pendingImportPaths()` 头注释承诺 Coordinator 会递归导入，实际 `import()` 未调用它 → .DATA 的 INCLUDE 网格链路不通。
3. **同 ID 直接替换**：`IDOSProject::addObject` 遇同 objectId 会 `deleteLater` 旧对象，与设计文档"默认拒绝"冲突，存在悬垂指针风险。
4. **删除无引用检查**：`IDOSModel::references()` 写了"删除保护查询用"但全工程无调用点；删除被引用对象会留死 ID。
5. **共享井混装工况字段**：井控/射孔/网格定位在共享 `IDOSWell` 上，多工况会互相污染（目标是拆出 `IDOSCaseWell`）。
6. **CMake 版本声明不实**：声明 3.16 但 `src/CMakeLists.txt` 使用了需 3.20 的 `cmake_path`。
7. **LAS 仅解析 Well Section**；`.GRID` ASCII 格式不支持；EGRID 转换正确性未经真实非方形/斜柱线数据验证。

---

## 9. 阅读文档索引

| 文档 | 回答的问题 |
|---|---|
| **本文** | 这个工程现在长什么样？怎么构建？代码怎么组织？ |
| [功能规格](功能规格.md) | 产品最终要做哪些界面功能？（按主页/导入/视图/工具分区） |
| [IDOS平台功能清单](IDOS平台功能清单.md) | 功能全集 + P0–P3 优先级与验收标准 |
| [IDOS-架构设计方案](IDOS-架构设计方案.md) | 目标架构、关键取舍、P0–P5 迁移阶段（**现状问题的修复依据**） |
| [IDOS-目录与类文件组织](IDOS-目录与类文件组织.md) | 目标目录/类名/公共 API 白名单、现有文件迁移映射 |
| [IDOS-UML设计图](IDOS-UML设计图.md) | 模块依赖、领域类、窗口类、导入时序（Mermaid） |
| [README](README.md) | 文档组的阅读顺序与设计约束 |

---

## 10. 变更记录

| 版本 | 日期 | 说明 |
|---|---|---|
| v1.0 | 2026-09-23 | 初版：基于当前源码的工程介绍 |
| v1.1 | 2026-09-23 | `src/process` 更名为 `src/analysis`（职责判据写入其 CMakeLists 头部注释） |
| v1.2 | 2026-09-23 | check_includes 由 4 条规则扩为 9 条（模块级依赖矩阵）；VTK 改 PRIVATE、exe 解除 VTK 链接 |
