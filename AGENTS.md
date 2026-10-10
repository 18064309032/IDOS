# 编码规范

- 每个模块中同名的 `.h` 和 `.cpp` 必须放在同一目录；公共类的头文件和实现文件一同放在模块根目录，内部类在所属子目录内成对放置。
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

# 渲染架构约定

- 渲染模块按接口协作，职责限定为 Registry、Metadata、RenderProvider、RenderObject、Scene、View 和 RenderServer。
- Registry 注册并按数据类型查找 Metadata；Metadata 负责根据数据对象创建对应的 RenderObject，不负责监听工程变化或同步场景。
- RenderObject 持有自己的 RenderProvider，并通过公开的 `renderProvider()` 接口返回它。Provider 与对应的数据对象关联，数据对象指针在创建 Provider 时传入；不额外引入 `IDOSDataRenderProvider` 包装层。
- RenderProvider 提供渲染对象所需的通用渲染操作。具体数据类型的渲染行为由对应 Provider 实现；不得把数据类型专属操作放入通用平台接口。
- Scene 只管理 RenderObject；View 从 Scene 获取 RenderObject，并通过其 Provider 获取 actor、图例及其他渲染能力。
- RenderServer 连接工程信号，负责根据 Registry 中的 Metadata 创建、更新和移除 RenderObject，并协调 Scene 与 View。工程事件同步逻辑属于 RenderServer，不属于 Metadata 或 RenderProvider。
- 不引入 `IDOSRenderProviderContext`、`IDOSRenderObjectChange`、Provider `synchronize()` 等用于把工程同步委托给 Provider 的机制；确需工程同步时由 RenderServer 直接处理。
- 数据对象 ID 与 RenderObject ID 分别管理。一个 RenderObject 可以对应多个 actor；View 将 actor 操作映射回 RenderObject，再由其 Provider 关联到源数据对象。
- 实现既定架构时，删除确认不需要的旧接口和适配层，不为复用旧调用链保留重复抽象；改动前梳理完整调用路径，避免叠加补丁。

# 编译目录

- 后续所有 CMake 配置、编译和构建测试统一使用 `E:\CJW-Archive\build\win-x64-debug-local` 作为唯一构建目录。
- 禁止在源码目录 `E:\CJW-Archive\IDOS` 及其子目录中执行源码内构建或创建构建目录；不再使用源码目录中已有的 `build` 子目录进行编译。
