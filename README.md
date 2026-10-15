# IDOS

IDOS 是面向油气储层建模与数值模拟的 Windows 桌面工程平台，提供井、网格和工况数据管理、VTK 可视化、可撤销业务命令，以及可扩展的 C++ 插件接口。平台通过 WSL2 调用 OPM Flow，支持计算输出跟踪和动态属性结果导入。

项目处于持续开发阶段。本文描述当前代码提供的能力；待实现功能单独列出，不将界面占位入口视为可用功能。

## 功能

- **数据与工况**：井头、井轨迹、LAS 测井数据、ECLIPSE DATA 与网格导入；数据、工况、网格和属性支持独立副本、复制粘贴、拖动复制及重命名；网格和属性支持兼容目标间移动，已有井/表另通过“添加到工况…”建立关联。
- **工程文件**：单个 HDF5 二进制 `.idos` 文件保存工程信息、全部平台业务数据及对象关联，支持打开、保存、另存为和未保存修改提示；恢复业务数据不依赖导入源文件。
- **可视化**：独立二维、三维窗口，网格与井展示、属性着色、选择高亮、标准视角、坐标轴、图例和截图。
- **数值模拟**：DATA 浏览与生成、运行预检、输入初始化、WSL Flow 启停、实时输出、UNRST 动态属性导入及报告步播放。
- **业务操作**：命令注册、参数验证、撤销与重做；界面和助手共用业务入口。
- **插件扩展**：数据、树、渲染、命令和视图工具注册；加载管理与卸载保护。
- **本地助手**：模型进程、HTTP 请求和命令调用，工程切换时隔离旧请求；Enter 发送、Shift+Enter 换行。
- **视图工具框架**：每窗口独立实例、悬浮工具条、多操作菜单、Qt 输入过滤器和 VTK 成员函数事件监听。

### 当前限制

| 范围 | 状态 |
|---|---|
| 工程文件及设置 | 打开、保存、另存为及视图状态恢复已接入；本机保存窗口和停靠布局；工程设置编辑仍待实施 |
| 内置悬浮工具按钮 | 已提供主视图、适配全部对象、缩放至选中对象、三维标准视角、旋转、正交投影、坐标追踪、视图联动、选择、点缩放、点探针、距离测量、网格透视、缩放、背景、清空显示、截图和坐标轴操作 |
| Python | 控制台界面存在，运行时未接入，不能执行 Python 代码 |
| 通用分析模块与拟合评价 | 待实现，不代表专业插件已经具备完整计算能力 |
| Flow 运行环境 | 需要预先安装 WSL2、Linux 发行版和 Flow，未提供离线环境包 |
| 支持平台 | 当前构建和验证针对 Windows x64、Qt 5.15.2、MSVC，其他平台未完成验证 |

最新验收范围和计划见 [工作进度](docs/工作进度-CJW.md)。

## 快速开始

### 环境要求

| 依赖 | 要求 |
|---|---|
| 编译器 | Visual Studio 2022，MSVC x64 C++ 工具链及 Windows SDK |
| CMake | 建议 3.23 或更高，支持仓库 version 4 预设；代码使用 C++17 |
| Qt | Qt 5.15.2 `msvc2019_64`，包含 Widgets、Network、LinguistTools；测试需要 Qt Test |
| 第三方依赖 | `thirdparty/` 下的 VTK 9.6.0、SARibbon、Advanced Docking System、OPM opm-common |
| 助手资源 | 当前构建部署依赖 `resources/assistant/` 的运行程序、配置及 GGUF 模型 |

部分依赖为预编译二进制，模型文件体积较大；只复制源码不足以完成当前构建。Flow 求解器独立于 opm-common 解析库，不是启动普通数据查看功能的必要条件。

### 配置与构建

在仓库根目录运行。当前工作约定使用唯一构建目录 `E:/CJW-Archive/IDOS-Build`，禁止源码内构建。

```powershell
cmake -S . -B E:/CJW-Archive/IDOS-Build `
    -G "Visual Studio 17 2022" -A x64 `
    -DIDOS_QT_DIR="D:/Qt/Qt5.15.2/5.15.2/msvc2019_64" `
    -DIDOS_ENABLE_RENDER=ON -DIDOS_BUILD_TESTS=ON

cmake --build E:/CJW-Archive/IDOS-Build --config Debug --parallel 6
```

Qt 路径按本机安装位置调整，也可以通过 `QTDIR` 设置。已有本地预设时可使用 `cmake --preset win-x64-debug-local -B E:/CJW-Archive/IDOS-Build`；显式指定目录以确保配置、编译和测试使用同一位置。

Debug 程序位于 `E:/CJW-Archive/IDOS-Build/bin/Debug/idosd.exe`。构建部署 Qt 与项目 DLL、第三方运行库、插件、翻译及助手资源，应从当前产物目录启动程序。

`IDOS_BUILD_TESTS` 默认 `OFF`，开发验证应打开。`IDOS_ENABLE_RENDER` 默认 `ON`；当前源码仍使用 VTK 控件，不能将 `OFF` 视为已支持的无渲染构建模式。

## 使用

1. 启动程序并新建工程。
2. 从数据树或工况入口导入井数据、网格或 DATA 工况。
3. 选择对象并勾选支持展示的节点，在目标视图查看属性。
4. 数值模拟先运行预检和初始化，再在运行状态窗口启动 Flow。
5. 成功计算后检查动态属性，选择报告步或播放结果。

运行信息进入底部输出面板，调试日志使用独立面板。完整步骤与常见问题见 [使用指南](docs/使用指南.md)。

## 架构

| 模块 | 职责 |
|---|---|
| `core` | 工程、领域数据、类型工厂、树结构/模型/同步、业务命令与历史 |
| `providers` | 井数据、LAS、ECLIPSE 输入和结果等格式解析 |
| `render` | 通用展示对象、Scene、View、Provider、注册与同步，以及工具框架 |
| `gui` | 树界面、节点交互、公共平台接口和通用面板 |
| `app` | 主窗口、数值模拟、服务装配、插件加载、内置具体展示和业务命令 |
| `assistant` | 本地模型进程、请求服务、命令调用及助手界面 |
| `plugins` | 裂缝建模、地质力学等专业扩展及点集示例 |
| `analysis` / `python` | 保留的接口目标，通用算法及 Python 运行时待实现 |

物理目录和链接目标分别管理：`gui` 按业务目录组织界面源码，其中业务窗口由 `app/CMakeLists.txt` 显式编入 `idos_app`；通用 `idos_gui` 保持独立。通用 Provider/Metadata/Registry 位于 `core/providers`，具体展示实现位于 `app/providers`。完整文件树见 [源码目录](docs/IDOS源码目录.md)。

树和渲染分别通过 Server 同步工程；每个渲染窗口拥有独立 Scene 和显示状态。注册表拥有 Metadata，工厂创建具体 Provider，通用框架不判断业务子类。数据修改通过命令执行，局部显示操作作用于明确的目标视图。

`gui` 与 `render` 不互相链接，插件通过 `IDOSInterface` 注册能力。工具遵循 `Registry → Metadata → ToolBar → Provider → Action / Tool`，Qt 处理输入，VTK 通过成员函数指针提供状态通知。

详细接口与所有权见 [架构设计](docs/IDOS架构设计.md)，固定目录见 [源码目录](docs/IDOS源码目录.md)。

## 测试

```powershell
ctest --test-dir E:/CJW-Archive/IDOS-Build -C Debug --output-on-failure
```

测试包括 `command_registry`、`assistant_widget`、`project_dialog`、`well_import`、`well_path`、`seismic_import`、`case_workflow`、`tree_incremental`，以及两项 include 边界检查。`idos_render_header_checks` 构建目标验证公共头文件自包含。

实际 Flow 验证需准备可运行的 SPE1 输入包，并设置路径：

```powershell
$env:IDOS_FLOW_SMOKE_DATA = 'E:/CJW-Archive/IDOS-Build/flow-smoke/SPE1CASE1.DATA'
ctest --test-dir E:/CJW-Archive/IDOS-Build -C Debug -R '^case_workflow$' --output-on-failure
```

未设置该变量时，本机 Flow 用例跳过，不能将普通回归通过解释为求解器计算已验收。验证与翻译流程见 [开发指南](docs/开发指南.md)。

## 文档

- [使用指南](docs/使用指南.md)：工程、数据、视图、Flow 和助手操作。
- [开发指南](docs/开发指南.md)：依赖、构建、验证、翻译和贡献流程。
- [架构设计](docs/IDOS架构设计.md)：模块边界、公共接口与生命周期。
- [源码目录](docs/IDOS源码目录.md)：固定目录及类归属。
- [工作进度](docs/工作进度-CJW.md)：状态、验收记录和后续计划。
- [插件开发](src/plugins/README.md)与[点集示例](src/plugins/idosexampleplugin/README.md)。
- [图标规范](images/README.md)。

## 贡献与授权

修改前阅读 [AGENTS.md](AGENTS.md)。新增功能沿用注册及命令路径，同步测试、翻译和进度；第三方库不作批量风格改写。

当前仓库未提供顶层 `LICENSE` 文件。第三方库、运行程序和模型的授权以各自随附文件为准；本文不新增或声明项目许可证。
