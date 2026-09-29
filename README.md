# IDOS

IDOS 是一款面向油气储层数值模拟的桌面前处理工作站，基于 **Qt 5.15 + CMake + MSVC** 构建。
提供工程化的数据管理（井、网格、工况、测井曲线），并以 VTK 驱动的 3D 视图完成模型浏览与属性可视化。

- 左侧 **数据树**：管理原始输入数据（井组、井头、井轨迹、测井曲线、地震等输入分组）
- 底部 **工况树**：管理模型对象（工况 → 网格 → 属性场），勾选即渲染
- 中央 **3D 视图**：网格 / 井轨迹 / 井口 / 属性场着色显示，支持对象高亮与拾取
- 右侧 **属性面板**：展示当前选中对象的完整属性
- 顶部 **Ribbon 工具栏**（SARibbon）+ 可自由停靠/浮动的面板布局（Qt Advanced Docking System）

## 目录结构

```
IDOS/
├── CMakeLists.txt          # 根构建脚本：Qt/VTK/第三方库 + 架构边界守护
├── CMakePresets.json       # win-x64-debug / win-x64-release 预设
├── CMakeUserPresets.json   # 本机 Qt 路径（不入库，示例见 CMakeUserPresets.example.json）
├── AGENTS.md               # 编码规范（AI 与人类共同遵守）
├── .clang-format           # 格式化配置：Allman / 4 空格 / 120 列
├── src/                    # 全部业务源码（见"架构分层"）
├── tests/                  # Qt Test 单元/集成测试 + 样例数据
├── i18n/                   # 翻译文件 idos_zh_CN.ts / .qm
├── images/                 # SVG 图标（images.qrc 嵌入 idos_gui）
├── resources/              # 程序动态加载资源（预留）
├── thirdparty/             # 预编译第三方库（include/lib/bin 三段式）
└── tools/                  # check_includes.ps1 架构守护脚本 + 回归用例
```

## 架构分层

模块按严格单向依赖组织，越界 include 会在**每次构建时**被 `tools/check_includes.ps1`
拦截并直接导致构建失败（同时注册为 CTest 用例）。

```
core ──────────── 领域模型：IDOSObject / IDOSProject / IDOSWell / IDOSGrid / 类型注册表
 │                （仅依赖 Qt5::Core、Qt5::Gui）
 ├── providers ─── 数据解析：LAS / Petrel DEV / Wellheader / ECLIPSE DATA·GRID
 │                 （链 OPM opmcommon 静态库）
 ├── render ────── VTK 9.6 渲染：场景、视图、渲染对象、网格/井渲染体
 ├── gui ───────── 树控件体系：TreeModel / TreeView / TreeProvider / 右键菜单
 └── app ───────── 程序装配：主窗口、Ribbon、停靠面板、RenderServer、业务操作入口
analysis / python / assistant ── 扩展占位模块（INTERFACE 目标，尚无实现）

idos (exe) ────── src/main.cpp，只链接 idos_app + Qt
```

关键设计：

| 机制 | 说明 |
|---|---|
| 批量事务 | `IDOSProject::beginUpdate/endUpdate` + `IDOSProjectUpdateGuard`（RAII），一次导入只发一条批量信号，避免视图 O(N²) 重建 |
| 类型注册表 | `IDOSTypeRegistry`：typeId → 元数据 + 工厂，区分输入树 / 模型树，官方与插件类型同一注册通道 |
| Provider 注册表 | `IDOSProviderRegistry`（文件格式 → 解析器）、`IDOSRenderObjectProvider`（对象 → 渲染体）、`IDOSTreeProviderRegistry`（typeId → 树分支构建器） |
| 架构守护 | `check_includes` 构建目标：禁止跨目录回溯 include、禁止公开头引用他模块内部类型，并按 `tools/gui_layers.json` 校验 GUI 内部分层 |

## 构建

### 环境要求

- Windows x64，Visual Studio 2022（MSVC 工具集）
- CMake ≥ 3.16（VS 2022 自带版本即可）
- Qt 5.15.2 msvc2019_64（本机示例路径 `D:/Qt/Qt5.15.2/5.15.2/msvc2019_64`）
- 第三方库已随仓库提供在 `thirdparty/`：SARibbon、Qt Advanced Docking System、OPM(opm-common)、VTK 9.6.0

### 配置与编译

```powershell
# 1) 指定 Qt 路径（一次性）：复制 CMakeUserPresets.example.json 为 CMakeUserPresets.json
#    或设置环境变量 QTDIR

# 2) 配置
cmake --preset win-x64-debug-local

# 3) 编译（VS 多配置生成器）
cmake --build ../IDOS-Build/win-x64-debug-local --config Debug

# 或直接用 VS 内置 cmake（PATH 中无 cmake 时）
# "D:/Microsoft Visual Studio 2022/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe"
```

产物统一输出到 `<构建目录>/bin/<配置>/`，`idosd.exe`（Debug）/ `idos.exe`（Release）与全部依赖 DLL 同目录。
POST_BUILD 步骤会自动完成 Qt 部署（windeployqt）、VC Debug 运行库、VTK / SARibbon / ADS DLL 与 `.qm` 翻译的拷贝。

### 常用选项

| 选项 | 默认 | 说明 |
|---|---|---|
| `IDOS_ENABLE_RENDER` | `ON` | 关闭后不链接 VTK，可无渲染模块编译 |
| `IDOS_BUILD_TESTS` | `OFF` | 打开后构建 `tests/` 下的 Qt Test 目标 |

```powershell
cmake --preset win-x64-debug-local -DIDOS_BUILD_TESTS=ON -DIDOS_ENABLE_RENDER=ON
```

### 架构检查

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/check_includes.ps1 -SourceRoot src
ctest --test-dir ../IDOS-Build/win-x64-debug-local -C Debug -R check_includes
```

## 测试

打开 `IDOS_BUILD_TESTS=ON` 后，`ctest` 提供：

| 用例 | 内容 |
|---|---|
| `project_dialog` | 新建工程对话框流程 |
| `well_import` | 井头 / 井轨迹 / 测井导入与井名汇合 |
| `well_path` | Petrel `.dev` 轨迹解析、空间基准、井口几何解析 |
| `case_workflow` | 工况创建、网格与属性导入、级联删除（依赖 OPM 解析 `case.DATA`） |
| `tree_incremental` | 数据树/工况树增量刷新与批量事务 |

```powershell
ctest --test-dir ../IDOS-Build/win-x64-debug-local -C Debug --output-on-failure
```

样例数据位于 `tests/data/`（`case.DATA`、`Wellheader.txt`、`A10.dev`、`case-wells.inc`）。

## 国际化

所有用户可见文案使用 `tr()`（源文本英文），中文写入 `i18n/idos_zh_CN.ts`：

```powershell
cmake --build <构建目录> --target lupdate       # 扫描 src 提取 tr() 到 .ts
linguist i18n/idos_zh_CN.ts                      # 翻译
cmake --build <构建目录> --target translations   # lrelease 生成 .qm（随 ALL 目标自动执行）
```

程序启动时按 `QLocale` 从 exe 目录自动加载 `idos_zh_CN.qm`。

## 代码规范

完整规则见 [AGENTS.md](AGENTS.md)，要点：

- 同名 `.h` / `.cpp` 必须同目录；公共类放模块根目录，内部类在子目录成对放置
- 禁止 `auto`、禁止 `struct`（类型一律 `class`）、禁止 lambda、禁止文件作用域静态函数
- 头文件只放声明，函数实现（含一行函数）一律写在 `.cpp`
- 控制语句与函数体大括号独占一行（Allman），`if`/`for` 等即使单条语句也必须加括号
- 槽函数以 `on` 开头；构造函数初始化列表每项独占一行
- 代码与测试用英文，中文仅出现在翻译文件；界面文案用 `tr()` 并同步更新 `i18n/idos_zh_CN.ts`

格式化：`clang-format`（配置见 `.clang-format`，基于 Microsoft 风格）。

## 第三方库

| 库 | 用途 | 形式 |
|---|---|---|
| Qt 5.15.2 | UI / 信号槽 / 模型视图 | 外部安装，`IDOS_QT_DIR` 或 `QTDIR` 指定 |
| SARibbon | Ribbon 主窗 | 预编译，`SARibbon::SARibbonBar` IMPORTED |
| Qt Advanced Docking System | 可停靠面板 | 预编译，`ADS::ADS` IMPORTED |
| OPM (opm-common) | ECLIPSE `.DATA` 输入文件解析 | 静态库，`OPM::opmcommon`，仅 providers 使用 |
| VTK 9.6.0 | 3D 渲染 | `find_package(VTK)`，仅 render 模块持有 |

第三方库不做批量风格改写；Qt 固定签名的事件重写函数保持框架要求的名称。
