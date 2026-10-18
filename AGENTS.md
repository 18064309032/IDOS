# 编码规范

- 禁止使用友元类或普通友元函数；仅在必要时允许运算符友元函数。

- 同名的 `.h` 和 `.cpp` 必须放在同一目录。固定源码目录以 `docs/IDOS源码目录.md` 为准；公共接口及实现放在所属功能目录根部，具体业务实现按确认的目录归属放置。未经用户确认不得改变目录划分。
- 项目自有 C++ 代码（含测试）禁止使用 `auto`，必须显式写出类型。
- 项目自有 C++ 代码（含测试）禁止使用 `struct`，类型定义和前置声明统一使用 `class`，并显式声明所需访问权限。
- 禁止使用匿名函数（lambda），使用具名函数、成员函数或槽函数。
- 禁止在 `.cpp` 中定义文件作用域静态函数；辅助函数必须在对应类中声明，非公开辅助函数声明为 `private`。
- 自定义槽函数统一以 `on` 开头，声明和信号连接保持一致。
- 构造函数初始化列表中，每个基类或成员初始化项独占一行；冒号单独起行引出首项，后续项的逗号放在行首，与冒号对齐，不在同一行并列多个初始化项。
- 头文件和源文件的 `#include` 均按以下分组顺序排列：C/C++ 标准库及平台系统头文件、Qt、VTK、其他第三方库、项目内部库。不同分组之间空一行；同一分组内按头文件名升序排列。项目内部库包括本模块及其他 IDOS 模块的头文件。
- 头文件的 `#define` 防卫宏与首个 `#include` 之间空一行；全部 `#include` 结束后也空一行，再开始写声明和定义。
- `.cpp` 文件先按上述顺序列出标准库、Qt、VTK、其他第三方库和项目内部库，最后包含本 `.cpp` 对应的头文件；该头文件与前一组之间空一行。随后再空一行，开始写源文件定义。若没有其他头文件，对应头文件仍单独成组。
- `.cpp` 对应头文件放在最后是本项目的固定排序约定，不代表可以依赖间接包含；每个头文件必须自包含，并应通过独立包含/编译检查验证。
- 所有控制语句（如 `if`、`else`、`for`、`while`、`do`、`switch`）的语句体必须使用大括号，即使只有一条语句也不得省略；左、右大括号各自独占一行，不与控制语句或语句体同行。
- 函数实现统一放在 `.cpp` 文件中，头文件只保留声明；简单的一行函数也不能在头文件内实现。函数左、右大括号各自独占一行，函数体另起行书写，不使用单行函数实现。
- 除注释外，项目自有代码全部使用英文；`tr()` 的源文本使用英文，中文仅放在中文翻译文件中。测试输入和断言同样遵循此规则。
- 用户可见的界面文字、格式名称和错误提示使用 `tr()`（非 QObject 类可使用 `QObject::tr()`），更新 `i18n/idos_zh_CN.ts` 并完成中文翻译。内部标识、协议关键字和文件路径不翻译。
- 第三方库不作批量风格改写；Qt 固定签名的事件重写函数保持框架要求的名称。

# 架构约定

- 当前功能状态、未完成事项和后续计划统一记录在 [docs/工作进度-CJW.md](docs/工作进度-CJW.md)；实施或验收功能后同步更新。
- 目标架构以 `docs/IDOS架构设计.md` 的新方案为准。该文档描述待实施架构，不表示当前代码已完成；不受旧类划分约束。
- 树与渲染独立但职责一致：通用展示对象管理身份与所有权，Provider 提供展示，GuiProvider 提供菜单与交互，树由 AppTreeAccess 对接工程并驱动 Model、渲染由 Server 同步工程。通用 Model、View、Scene、Server 不判断具体数据或 Provider 子类。
- 仅允许增加设计方案中的类及对应具体数据、Provider、GuiProvider、Metadata、命令实现；禁止新增方案外的类、结构体、命名空间、匿名命名空间和自由函数。类内接口与成员可以按需要增删调整，非公开辅助方法放在所属类 private 区域。
- 源码按已确认的新目录组织：通用树结构、Model 和 TreeAccess 放在 src/core/tree，AppTreeAccess 放在 src/app 根部，不设置独立 IDOSTree 或 TreeServer；树及数据读取的 Provider、Metadata、Registry 放在 src/core/providers，TreeView 与 TreeMenuProvider 均放在 src/gui/tree。渲染及工具 Provider、Metadata 放在 src/render/providers，其他通用渲染框架放在 render 根部。具体展示实现按 well、grid、case、fluidrock、seismic、data 分入 src/app/providers 的业务子目录，视图工具放在其 render 子目录；gui 按 tree、well、grid、simulation 分类；工况对话框、新建工程、调试、运行信息及 Python 控制台位于 gui 根部，公共平台接口 IDOSInterface 位于 gui 根部并由 gui 导出。业务命令放在 src/app/command，具体文件解析放在顶层 src/providers。
- 物理源码目录与链接模块分别管理：平台业务窗口统一归档在 src/gui，但由 src/app/CMakeLists.txt 显式列入 idos_app，保持 APP_EXPORT；通用界面编入 idos_gui。依赖检查按实际目标归属执行，禁止通用 idos_gui 反向依赖 idos_app 或其业务窗口。idos_gui 与 idos_render 不互相依赖，均可依赖 core；core 不依赖 gui、render、app 或插件。
- RenderObject 为通用具体类，管理独立 ID、名称和 Provider 所有权；RenderMetadata 创建 RenderProvider，由 RenderServer 装入通用 RenderObject。删除仅用于装配 Provider 的具体 RenderObject 子类，不增加数据 Provider 包装层。
- 每个 RenderView 拥有独立 Scene，Scene 拥有 RenderObject，RenderObject 拥有 Provider；源数据关联非拥有，各视图显示状态独立。数据 ID、节点 ID、渲染 ID 分别管理。
- 树工程同步由 AppTreeAccess 处理，渲染工程同步由 RenderServer 处理；AppTreeAccess 监听工程、两个 Model 和 RenderServer 的信号，再调用其他模块接口；Model、Item、Provider 不持有 Project、Access 或 RenderServer，不委托给 Metadata、Provider 或 GuiProvider；不引入 Provider Context、synchronize()、统一交互结果协议或已确认 TreeAccess/AppTreeAccess 之外的额外适配层。
- TreeMenuProvider 只关联所属 View/Model；右键节点由 Item Provider 创建菜单并连接自身具名槽，右键空白由树 MenuProvider 创建菜单并连接自身具名槽；Provider 校验动作上下文后调用 Model 操作接口，由 Model 发业务信号，AppTreeAccess 再调用应用业务或渲染接口。View 仅按命中位置选择 Provider、发菜单扩展信号并弹出菜单，不统一绑定业务 QAction。菜单与 TreeView 均不得保存或访问 Project、MainWindow、RenderView、RenderServer；菜单保存创建时的 Model sessionId，多选动作固定业务数据 ID；Provider 直接调用 Model，Model 校验 session 和所属节点归属，命令校验数据目标；不以 QAction 的 operation 属性统一解析代替具体 Provider 槽。根 Provider 不作为其他节点 Provider 的菜单工具类。
- AppTreeAccess 负责连接、路由和回写，不实现网格属性/过滤器等具体渲染算法；这些操作放在所属业务 RenderProvider 的成员接口中。
- 数据树与工况树分别使用 IDOSDataTreeItemModel/IDOSCaseTreeItemModel 和 IDOSDataTreeView/IDOSCaseTreeView，分别继承通用 Model/View；Model 位于 src/core/tree，View 位于 src/gui/tree。具体 Model 提供明确参数的业务函数及对应信号，AppTreeAccess 用独立具名槽对接业务；禁止恢复操作字符串加参数字典的树菜单统一分发。
- 节点结构、分类、勾选联动和专属菜单由具体实现提供，不写入通用模型。View 类内组合菜单，不新增菜单组织器类；对象、空白、多选以及实际目标视图必须明确。
- 业务修改统一调用命令，菜单、工具栏、脚本和助手共用同一逻辑；局部显示操作调用目标 Provider 或 View。失败命令不进入历史，批量通知不代替回滚。
- 直接改造现有框架并删除被替代的类、接口及适配层，不保留两套调用链。改动前梳理完整路径；保留未被替代的日志、应用、插件和领域基础设施。

# 编译目录

- 后续所有 CMake 配置、编译和构建测试统一使用 `E:\CJW-Archive\IDOS-Build` 作为唯一构建目录。
- 禁止在源码目录 `D:\CMakeCoder\IDOS` 及其子目录中执行源码内构建或创建构建目录；不再使用源码目录中已有的 `build` 或 `out` 子目录进行编译。
