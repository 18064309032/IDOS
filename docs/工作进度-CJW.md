# IDOS 工作进度（CJW）

更新日期：2026-10-13

本文记录当前工程已落地的主要能力、尚未完成的工作和后续顺序。状态按当前源码核对；“已实现”表示代码中已有对应功能，不代表所有功能都已完成完整的端到端验收。

## 最新状态摘要

- E 盘 CMake 工程是本文对象，与 D 盘 Visual Studio 工程分开维护；用户已确认构建根目录为 E:/CJW-Archive/IDOS-Build，后续统一使用其 win-x64-debug-local 子目录。下文旧测试目录仅为当时实际执行记录，不作为后续构建约定。
- 工程、树、渲染、命令和插件框架已落地。新建和导入可用，工程打开/保存/另存为/设置仍禁用。
- Flow 已接入 WSL2，支持实时输出、进程组停止、独立结果目录、UNRST 自动导入和报告步播放；真实算例验收目前以 SPE1 为主，未提供离线环境包。
- 本地助手已实现模型进程与命令调用，普通文本对话使用“你：”“助手：”同行格式。Python 控制台仅有界面，运行时未接入；analysis 仍是接口目标。
- 通用工具框架已实现，不使用友元、静态回调或 SLOT；Qt 过滤器处理输入，VTK 成员函数指针处理状态。具体工具尚未注册，悬浮工具条当前隐藏。
- 最近完整相关范围回归为工具框架阶段 5/5（64.45 秒）；随后友元和 VTK 调整运行各自专项，最后选择比较调整运行树回归。没有将这些局部测试合并宣称为本日全量重新验收。
- 历史 case_workflow 卸载阶段发生过超时；本日工具框架范围内该用例通过（38.37 秒）。没有完成历史偶发超时根因证明，保留观察，不将其描述为当前每次必现阻塞。

操作见 [使用指南](使用指南.md)，开发与验证见 [开发指南](开发指南.md)。下文按时间保留变更和实验记录，发生方案调整时以本摘要、架构文档及后续记录为准。

## 当前进度

### 平台与框架

- 工程、数据对象、工况、树视图、渲染视图、命令撤销重做和插件管理等平台基础功能已形成。
- 平台提供工程数据导入和新建工况流程。数值模拟插件不再重复实现 DATA 导入和新建工况入口。
- 网格动态属性可存储多个报告步，并可通过 UNRST 导入命令进入工程。
- 插件可扩展树节点和对象交互；插件存在仍被工程或命令历史引用的数据时，会拒绝卸载。

### 数值模拟插件

| 功能 | 当前状态 | 说明 |
|---|---|---|
| Deck 浏览 | 已实现 | 浏览已导入 DATA 文件中的关键字 |
| 解析摘要、工况表格 | 已实现 | 查看导入时保存的解析信息 |
| DATA 生成 | 已实现 | 编辑来源 DATA 并导出输入文件 |
| 属性曲线、井控表 | 已实现 | 查看工况中已有的曲线表和井控数据 |
| 运行预检 | 已实现 | 检查运行准备条件并报告阻断项 |
| 初始化 | 已实现 | 准备隔离的运行目录并复制 DATA 及关联输入文件；不会启动模拟器 |
| 运行状态 | 已实现 | 查看初始化状态和输入文件是否完整 |
| 结果文件浏览 | 已实现 | 查看运行目录中不同于初始化输入的文件 |
| UNRST 动态属性导入 | 已实现 | 可从结果文件窗口或网格树的“动态属性”组选择 UNRST，导入命令共用 |
| 报告步播放 | 已实现 | 选择动态属性并浏览、播放其报告步 |
| 运行归档、工作流、清单报告 | 已实现 | 查看准备阶段、归档运行目录和导出清单 |
| 模拟器启动与停止 | 已实现 | 运行状态窗口通过 WSL2 启动 Flow，支持发行版、程序路径和 CPU 线程配置、实时日志及进程组停止 |
| 模拟结束后自动导入结果 | 已实现 | 正常退出后从本次独立输出目录定位 UNRST，经现有命令导入启动时绑定的工况网格；失败不记为成功 |
| 拟合评价 | 未实现 | Ribbon 中仍是禁用的占位操作 |

## 当前数值模拟流程

```text
平台导入 DATA / 新建工况
          ↓
运行预检
          ↓
初始化运行目录并复制输入文件
          ↓
运行状态窗口配置并启动 WSL2 中的 Flow
          ↓
成功结束后自动导入 UNRST 为动态属性，选择属性并播放报告步
```

OPM 解析库负责输入和结果读取；Flow 是通过 WSL2 调用的独立求解器。本机 Ubuntu-24.04 的 /usr/bin/flow（2026.04）已通过独立计算及 IDOS 初始化、计算、自动导入与异常路径的 SPE1 验证，尚未打包离线运行环境。更多真实工况及报告步播放仍需专项验收。

## Flow 接入（2026-10-12）

- RunStatusWidget 在现有类内管理 WSL2 和 Flow 进程，默认 Ubuntu-24.04、/usr/bin/flow、2 线程，配置保存在用户 QSettings。
- 每次运行创建独立 results-UUID 目录；实时输出同时保存到 flow-console.log。使用 setsid --wait 等待实际计算，停止时向本次 Linux 进程组发送 TERM，超时升级为 KILL。
- 窗口关闭仅隐藏；工程切换/销毁及目标数据移除取消任务。任务存活时拒绝卸载插件，完成时重新校验启动时工程、网格和工况引用。
- 现有结果导入命令补充 Metadata，由应用注册 simulation.results.import；手动结果窗口与自动导入均经公共命令入口调用，不依赖 app 内部头文件。
- 实际 SPE1 专项测试已通过初始化、123 个时间步计算、UNRST 自动导入、撤销重做、无 UNRST、启动失败、取消和 Linux 进程组退出检查，带空格路径验证通过。
- 固定构建目录完整 Debug 构建通过，独立头文件编译、291 个源码文件的架构边界检查及中文翻译检查通过。9 项回归已全部通过：7 项首次通过，工况和树测试修正旧断言后复验通过；复验启用本机 Flow 专项用例。
- 修复原工况测试仍调用已删除静态接口的问题，改用当前菜单动作和文件对话框完成验证；渲染测试明确设置各视图状态和着色属性，树测试验证没有目标视图时拒绝勾选，不再假设全局可见性联动。
- 用户实际导入 SPE1 发现 GAS_STEAM_RATIO 没有单位维度却被强制转换 SI 的错误。导入器改为无维度字段读取原值、带维度字段仍转换 SI，并用 RAII 清理构建失败的工况。
- 同时修复关键词源文件索引重复拼接 DATA 目录的问题：OPM 的文件位置相对于进程工作目录，直接归一化为绝对路径。新增注入井字段、压力 SI 转换和文件索引路径回归。Flow 专项改为从真实 DATA 解析及真实网格开始，初始化、计算、自动导入和撤销重做验证通过；专项记录为 flow-import-pipeline.log，4 项通过、无失败或跳过。
- 本次修复完整 Debug 构建通过，常规 CTest 8/9 通过；case_workflow 在 240 秒后超时，最后日志停在示例插件测试的主窗口插件卸载阶段，尚未定位。真实 DATA/Flow 专项已单独通过，不将此常规回归超时记录为全部验收通过。

## 运行信息统一输出（2026-10-12）

- 移除播放工具栏中的过程提示，每个实际报告步变化通过 IDOSLogger::message 进入主界面的输出信息面板；手动切换、自动播放和外部步变化均逐步记录，同一变化的同步信号不重复输出。
- Flow 状态、启动校验错误、停止、自动导入与原始标准输出/错误输出统一转发。输出按完整行组装，支持分批数据、CR/LF、无换行尾部和 WSL 启动提示的 UTF-16；不将这些用户消息仅写入调试面板。
- 初始化及文件复制、预检各项、手动结果导入、DATA 输入包生成、运行归档、清单报告保存的信息和错误同步输出，中文翻译完成。
- 插件、专项测试及独立头文件编译通过，291 个源码文件的架构检查通过，中文翻译无未完成项。onSimulationOutput 验证真实输出信息面板、手动/自动/外部步变化和每步单条消息；onFlowOutputFraming 验证实际 WSL 的分批、CR/LF 及无换行尾部；onFlowExecution 验证真实 SPE1 计算、结果导入、撤销重做及取消，三个专项均通过。此前工况测试的插件卸载超时仍保留为未解决事项，本次未宣称整套回归通过。

## 树节点菜单（2026-10-12）

- 根据用户确认，树右键菜单只针对实际节点；空白位置直接结束事件，无效节点不调用 GuiProvider 或菜单扩展信号，也不沿用当前选择。
- 保留节点自身菜单及工具栏创建入口，架构文档同步更新；示例插件测试改为断言空白菜单为空。
- 当时在 E:/CJW-Archive/build/win-x64-debug-local 重新配置并编译主程序、树测试和工况测试通过。onMenuTargets 专项通过，验证有选择时空白菜单为空、选择不变及节点菜单目标正确；记录为 tree-node-menu-test.log。本次未运行整套回归。此目录不再作为后续构建路径。

## 渲染菜单与接口梳理（2026-10-12）

- 渲染菜单原本由 IDOSRenderView 添加 Fit to Object、Opacity、Hide、Fit All，并调用具体 RenderGuiProvider 添加专属操作；按用户当前需求，鼠标和键盘上下文菜单事件均直接接受，不创建或弹出菜单。右键的 VTK 相机交互仍走原有鼠标事件。
- 本次为接口分析，没有批量删除公共契约。渲染菜单的 createMenu、menuObject、actionObject、onFitObject、onOpacity、onHideObject、onResetCamera 及注册扩展链不再有交互入口，仍被示例插件的程序化菜单测试调用；列为整条调用链精简候选，不将其描述为已经删除。
- 检查范围为 src 的 291 个及 tests 的 13 个自有 C++ 头文件/源文件；结合声明、定义、具名信号连接、继承重写、测试及目标架构判断。名称出现次数仅用于筛选，未作为可删除的充分依据；本次不构成外部二进制插件兼容性审计。

| 候选接口或成员 | 当前证据 | 判断与建议 |
|---|---|---|
| IDOSInterface / IDOSAppInterface::treeDataMetadata() | 只有接口声明和实现，无插件或业务调用；实现转发到 TreeRegistry::dataMetadata | 高优先级精简候选；当前插件已有 dataRegistry 和 treeRegistry 公共入口 |
| IDOSInterface / IDOSAppInterface::ribbonCategory() | 只有声明和查询实现，无调用；创建和插入分类各自直接检查 Ribbon 名称 | 高优先级精简候选；不影响仍在使用的 addRibbonCategory、insertCategoryPage、removeRibbonCategory |
| IDOSTreeView / IDOSRenderView::contextMenuAboutToShow | 没有业务 connect；渲染信号仅被本次 QSignalSpy 观察 | 可去掉未使用的第二种菜单扩展入口；当前菜单组合已由 GuiProvider 负责 |
| IDOSAssistantWidget::statusChanged | 只有声明和 emit，没有订阅；Widget 自身已更新状态文字 | 若不保留外部状态观察能力，可精简 |
| IDOSTreeView::currentItem、IDOSRenderView::selectedObjects、isActive、IDOSRenderServer::registry | 当前没有业务调用；currentItem 和 selectedObjects 明确列在设计方案中，实际选择链使用 QModelIndex 或 selection() 的稳定 ID | 属于公共便利/预留接口，需按最终契约精简，不按零调用自动删除 |
| IDOSGridRenderGuiProvider | 除菜单外，onObjectsChanged 刷新当前属性着色，onDataRemoving 在属性销毁前取消着色引用 | 不能整类直接删除；若移除渲染 GUI 扩展链，先将数据同步职责归回方案内的 Server 与具体 Provider，保留属性更新及删除回归 |
| IDOSWellRenderGuiProvider、IDOSPointSetRenderGuiProvider | 都是渲染菜单相关实现；后者含动作销毁和插件注销配套逻辑 | 若整条渲染菜单链删除，应一并处理注册、注销、构建项、头文件检查和示例菜单测试，不只删除 QAction 创建 |
| IDOSCommandManager::setClean | 无当前调用；isClean 已有测试；主窗口工程保存入口仍禁用 | 属于保存流程未接通，保留并在保存成功后接入，不当作死接口 |
| IDOSWell 完井/井段/井控接口及 IDOSGridCell::volume、IDOSGridProperty::numX/numY/numZ/valueAt、IDOSWellPath::firstMd | 多项仅有声明和实现，尚无实际业务使用；底层值类与领域模型其余功能仍在使用 | 属于尚未消费的领域能力，后续按业务范围裁剪；不能由此删除整个井、网格或日志基础设施 |
| private 成员与 on 开头辅助方法 | 扫描并复查低引用状态成员，未发现可仅凭本次证据直接删除的状态成员；低引用槽仍有具名连接或直接调用 | 不为减少数量删除实际状态或 Qt 框架回调；渲染菜单辅助方法按整条菜单链处理 |

- 固定构建目录中主程序、工况测试、独立头文件编译和 include 边界检查通过。onRenderArchitecture 验证鼠标和键盘事件都不创建菜单、选择不变；onRenderLifecycle 验证属性更新和删除清理；onViewDecorations 验证视图装饰，三项专项通过。测试记录为 render-context-test.log，本次未运行整套回归。

## 接口精简实施（2026-10-12）

- 用户确认清理后，删除 IDOSRenderGuiProvider、IDOSGridRenderGuiProvider、IDOSWellRenderGuiProvider、IDOSPointSetRenderGuiProvider 共 4 个类和 8 个文件，同时删除注册/注销接口与持有容器、构建项、独立头文件检查旧项及插件注册状态。渲染右键菜单实现已删除，不再只关闭交互入口。
- 删除 RenderView 的 createMenu、menuObject、selectedObjects、fit、isActive、菜单辅助函数与槽、project/setProject/setRegistry、m_project/m_registry/m_menuId；保留实际使用的相机、选择、高亮、可见性、透明度、截图与视图装饰能力。
- 删除 IDOSInterface/IDOSAppInterface 的 treeDataMetadata 与 ribbonCategory、TreeRegistry 的 dataMetadata、RenderServer 的 registry、TreeView 的 currentItem，以及树/渲染的 contextMenuAboutToShow 和助手的 statusChanged 无业务订阅信号；清理相应实现、无用局部变量与旧测试。
- 删除点集树的空白创建分支和 onCreatePointSet，创建仍由示例插件工具栏和 pointset.create 命令提供；移除仅供旧菜单使用的点尺寸便利接口。井节点的新建/导入、点集节点删除及业务命令保留。
- 原 GridRenderGuiProvider 的非菜单职责迁入既有类：具体 Provider 的 dependsOn 判断属性依赖、removeDataReference 清理非拥有引用；通用 RenderServer 统一刷新受影响对象并通知 Scene，不识别网格或属性类型。不增加类、包装层或 synchronize 调用链。
- 渲染生命周期测试扩展为两个视图的真实属性值刷新、批量延迟通知、局部显示状态保留，以及属性 take 后在销毁前立即取消引用和图例；无需手动调用 update 掩盖同步问题。示例插件测试保留状态隔离、树菜单目标和卸载保护，移除渲染菜单测试。
- 整套回归发现旧项目测试仍要求数据树空白菜单，以及工况树创建测试依赖必要的空白入口。修正数据树断言；TreeView 默认关闭空白菜单，应用只为工况树开启新建/导入入口，保留没有节点时创建工况的能力，不在通用 View 内判断具体树名称。
- 保留 CommandManager::setClean 及未被替代的井、网格和日志基础能力，避免把尚未接通的保存或领域功能当作废弃功能删除。源码目录、架构、示例说明与删除功能的中文翻译同步清理。
- 固定构建目录完整 Debug 构建通过；包含插件接口、应用接口、助手 Widget 和渲染/树头文件的独立编译通过；283 个项目源码文件的架构边界检查通过；平台及示例中文翻译无未完成项，git diff --check 通过。
- 最终 CTest 9/9 通过，包括 project_dialog、case_workflow 和 tree_incremental，记录为 interface-cleanup-final-ctest.log（70.72 秒）。case_workflow 的可选真实 Flow/WSL 输出测试本次未配置，按原有条件跳过；此前已通过的实际计算验证不替代本次结果。此前记录的插件卸载超时在本轮未复现，不据此宣称已定位其原因。
- 渲染架构、属性生命周期、视图装饰与示例插件四项也已单独通过，记录为 interface-cleanup-test.log。此前旧空白菜单断言导致的两项失败已修正并由最终整套回归覆盖。

## 助手对话样式（2026-10-12）

- 在既有 IDOSAssistantWidget 中调整展示，不新增类。对话区移除外层边框，用户消息使用浅蓝底色，助手回复按自然段展示，系统提示弱化颜色，不重复显示角色标题。
- 移除 HTML 表格消息拼接，使用 QTextDocument 的 Markdown 解析及原生 QTextFrame；支持段落、列表、加粗和代码块，原始 HTML 不作富文本解释。用户和系统消息按纯文本插入，保留原始输入。
- 输入框、键盘提示和发送按钮合并在同一圆角区域，焦点边框与按钮状态统一；空输入禁用发送，Enter 发送及 Shift+Enter 换行保留。Windows 界面使用 Segoe UI，代码使用 Consolas，新增界面文案完成中文翻译。
- 调整行距、消息间距和代码块背景；回复到达时，底部阅读跟随最新消息，翻阅历史时保持滚动位置。发送、工程绑定、模型调用及命令执行链保持原有职责。
- 助手测试保留命令调用、参数错误和跨工程失效检查，补充 Markdown 列表/加粗、原始输入保留、输入按钮状态、消息跟随和历史位置验证；实际 Widget 在 480 与 320 像素宽度下截图检查，记录于构建目录 tests/assistant-conversation.png 和 assistant-conversation-narrow.png。
- 固定目录中主程序、助手测试及独立头文件编译通过；283 个项目源码文件的架构检查、中文翻译与 git diff --check 通过。助手与两项 include 检查的 CTest 3/3 通过（assistant-presentation-ctest.log，12.80 秒），本次未重复整套回归。480 和 320 像素宽度的最终实际截图已复查。

## 用户消息大圆角气泡（2026-10-12）

- 按用户提供的气泡参考，在既有 IDOSAssistantWidget 内绘制大圆角背景，选用柔和蓝灰色 #e8edf5；短消息按文字宽度形成右对齐胶囊，长消息限宽换行并使用圆角矩形。助手回复继续使用自然段及 Markdown 排版。
- 不新增类。原生 QTextFrame 保留消息内容及选择，气泡宽度在消息插入和视口尺寸变化时重新计算，区分 Qt 文本框架的外边距与实际背景范围；视口绘制沿用 QTextDocument 布局和选择高亮，背景使用抗锯齿圆角。
- 主程序、助手测试、独立头文件编译和 283 个源码文件的边界检查通过；助手与 include 检查 CTest 3/3 通过（assistant-bubble-ctest.log，12.92 秒）。既有消息格式、原始输入、底部跟随和历史滚动测试通过；480 和 320 像素宽度的最终实际截图已检查，git diff --check 通过。本次未重复整套回归。

## 助手恢复默认外观（2026-10-12）

- 根据用户最新要求，撤掉助手对话区、输入区、状态提示的自定义字体、颜色、边框与圆角，删除气泡绘制、动态宽度计算、视口绘制/焦点样式事件及相关辅助接口，恢复 Qt 默认控件外观。
- 消息恢复为带角色标识的纯文本，不作 Markdown 或 HTML 样式渲染；保留换行、文本选择、原始输入、底部跟随及翻阅历史时的位置保持。
- 只在发送按钮本身保留蓝色圆角、悬停、按下和禁用样式；键盘发送、输入空白/忙碌时禁用及模型/命令调用逻辑保留。测试改为检查对话和输入控件无自定义样式、按钮样式仍在以及消息原文不变。
- 主程序、助手测试、独立头文件编译与 283 个源码文件边界检查通过；助手及 include 检查 CTest 3/3 通过（assistant-plain-ctest.log，13.90 秒）。本次未重复整套回归。

## 助手全部使用默认样式（2026-10-12）

- 按用户最新要求，去掉发送按钮剩余的颜色、圆角、悬停、按下和禁用样式，以及固定尺寸和自定义光标。整个助手面板不再设置自定义样式表、字体或配色，所有控件使用 Qt 默认外观。
- 固定目录主程序与助手测试编译通过；助手 CTest 通过（assistant-default-test.log，3.43 秒），测试检查发送按钮与其他助手控件的样式表均为空，git diff --check 通过。

## 恢复今晚调整前的助手界面（2026-10-12）

- 用户进一步确认要保持今晚修改前的样子，按今晚工作开始前最近保存的 e2c5885（2026-10-12 18:18:33）恢复助手界面：原有左右消息显示、状态提示、76 像素多行输入框、输入框右侧发送按钮及原始键盘提示。撤回本轮新增的对话排版、圆角绘制、一体式输入区与默认外观实验。
- 助手源码与该版本仅保留此前已完成的无订阅 statusChanged 信号清理差异；头文件保留必要 QString 自包含。撤回本轮展示测试与新增样式文案，助手命令调用测试恢复原有内容，其他功能改动保持其当前状态。
- 主程序、助手测试与独立头文件构建通过；助手 CTest 通过（assistant-evening-restore-test.log，2.74 秒），翻译无未完成项，git diff --check 通过。本次未重复整套回归。

## 助手原布局删除样式表（2026-10-13）

- 按用户最新要求，在已恢复的原布局上删除助手面板和状态提示的样式表，以及消息 HTML 的内联颜色和背景色；保留消息排列、输入框与发送按钮位置。src/assistant 中不再包含 setStyleSheet、style= 或 bgcolor 设置。
- 固定构建目录主程序和助手测试构建通过；助手 CTest 通过（assistant-remove-stylesheet-test.log，2.85 秒），git diff --check 通过。

## 助手默认外观布局协调（2026-10-13）

- 根据用户对宽面板截图的反馈，保留默认外观，删除消息 HTML 表格排列，角色和正文按普通段落统一左对齐，消息之间只保留一行空白；输入框与对话文档采用面板继承的界面字体，不设置颜色、气泡或样式表。
- 多行输入框从 76 像素降低到 44 像素，发送按钮同高对齐，保持原有右侧位置、Enter 发送和 Shift+Enter 换行逻辑。
- 当时主程序与助手测试构建通过，助手 CTest 通过（assistant-layout-test.log，2.64 秒），git diff --check 通过。此前曾建议运行另一构建目录的程序，该建议已按用户确认纠正；后续编译和启动均以 E:/CJW-Archive/IDOS-Build/win-x64-debug-local 为准。

## 助手角色与正文同行（2026-10-13）

- 按用户确认的格式，将角色和正文改为同一行：“你：内容”“助手：内容”，消息之间留一行空白；保持左对齐与默认外观，中文冒号格式完成翻译。
- 固定目录主程序与助手测试编译通过，助手 CTest 通过（assistant-inline-role-test.log，2.71 秒），git diff --check 通过。

## 底部面板标签图标（2026-10-13）

- 新增输出列表和调试虫形 SVG 图标，助手复用现有对话图标，Python 控制台保留终端图标；四个标签统一为 16×16 像素，沿用项目现有线条风格并保留文字。
- 固定构建目录主程序与工程测试编译通过，283 个源码文件 include 边界检查通过；project_dialog CTest 通过（panel-icons-test.log，11.81 秒）。

## 视图工具框架接口方案（2026-10-13，待实施）

- 按用户要求详细编写 ToolMetadata、ToolRegistry、ToolProvider、ToolBar、IDOSTool 及现有 RenderView 扩展接口，记录在架构设计第 13 节，源码目录同步列出五组成对目标文件，均位于现有 src/render 根目录。
- 方案覆盖每窗口悬浮工具条、多 Action 与下拉菜单、多工具并行、输入传递与消费、互斥和捕获、动态注册注销及插件卸载。Provider.tools 用于统一清理，避免安全退出依赖具体插件实现。
- 按用户反馈取消 EventMode、eventMode() 和 observeEvent()；统一使用 handleEvent() 的 bool 返回值控制继续传递或消费，不再设置工具事件模式分类。
- 后续讨论已确定最终输入方案：IDOSTool 使用 Qt eventFilter，渲染状态使用 VTK observer；取代上一条临时 handleEvent 方案，删除自定义事件分发与工具 priority。基类统一安装移除过滤器、记录清理 observer，View 保留启停、互斥、捕获和生命周期。设计已确定，代码待实施。
- 本轮仅编写设计文档，没有修改功能代码；后续先实施注册装配和多工具事件调度最小闭环，再按第 13.9 节验收。执行文档 git diff --check，不运行编译和功能测试。

## 通用视图工具框架实现（2026-10-13）

- src/render 新增五组成对类：IDOSTool、IDOSToolMetadata、IDOSToolRegistry、IDOSToolProvider、IDOSToolBar。Registry 拥有工具定义，每窗口 ToolBar 拥有独立 Provider、Action 与交互工具，支持多操作和下拉菜单、稳定排序、动态注册注销及窄窗口溢出入口。
- View 接入 setToolBar、工具启停、互斥和捕获；Qt 输入通过工具过滤器，VTK 状态通过带生命周期保护的 observer，停用统一移除监听。框架过滤器保留窗口激活、失焦取消和原有未消费点击拾取；回调中启停排队，支持嵌套事件处理后继续执行。eventFilter 为基类 final，具体工具覆盖 onEventFilter，避免跳过捕获和回调安全检查。
- 应用创建共用 ToolRegistry，所有新建视图绑定工具条；IDOSInterface / AppInterface 开放 toolRegistry。注册表和插件卸载增加 Qt/VTK 回调忙碌保护，拒绝回调栈内注销或卸载。尚未注册内置业务工具，空工具条保持隐藏，下一步在 app 或插件中实现具体工具。
- 新增三项工具行为测试并纳入现有 tree_incremental：动态注册、重复拒绝、双窗口实例隔离、菜单调用、坐标轴状态同步、模式适用性、手动隐藏保持、跨窗口绑定拒绝、事件消费、互斥、捕获、回调内停用与嵌套事件循环、Qt/VTK 回调注销拒绝、observer 停用清理和注销捕获释放。
- 固定目录主程序、工况/树测试与独立头文件编译通过，293 个源码文件 include 边界检查通过。CTest 5/5 通过：check_includes、check_includes_regressions、project_dialog、case_workflow、tree_incremental（tool-framework-ctest.log，64.45 秒）。git diff --check 与翻译 XML 检查通过；本轮没有重跑其余测试或真实 Flow 专项。

## 工具框架移除友元访问（2026-10-13）

- 按用户要求移除本轮工具框架全部 friend 声明，Tool、ToolBar、Registry 与 View 不再直接访问彼此私有成员。
- Tool.setActive 校验 View 生命周期阶段及启用列表，保留防绕过保护；ToolBar.setView 校验 View 已接管实例，父控件及隐藏状态由工具条自身管理。忙碌检查使用查询接口，回调结束和布局更新通过信号通知 View。
- 设计接口同步更新；新增测试检查外部直接激活工具被拒绝，沿用多窗口、事件、互斥、捕获和 Qt/VTK 注销保护回归验证。
- 固定目录主程序及独立头文件编译通过，include 边界检查通过。CTest 主窗口、树和边界检查 3/3 通过（tool-no-friend-ctest.log，18.64 秒）；随后加强回调结束信号期间的忙碌保护，新增对应注销测试，最终工具专项 3/3 通过（tool-no-friend-final-test.log，247 毫秒），git diff --check 通过。

## VTK 状态事件改用 Qt 槽连接（2026-10-13）

- 按用户确认改用 vtkEventQtSlotConnect：connectVtkEvent 注册状态监听，VTK 事件进入基类 Qt 槽，再调用具体工具 onVtkEvent。Qt 输入过滤器方案保持不变，不创建 QEvent 转发 VTK 输入。
- 删除手写静态回调、clientData 数据分配/删除、relay 和 observer tag 列表；使用连接类 Disconnect 统一清理。保留 Qt 槽执行及回调结束信号期间的注销/卸载忙碌保护，具体工具无需了解 VTK 回调函数签名。
- VTK 固定 Connect 接口内部使用字符串槽签名；按 GUI 线程直接连接处理，不跨事件生命周期排队传递 callData。源码和架构接口同步更新。
- 主程序、工具测试和独立头文件编译通过，293 个源码文件依赖检查通过；CTest 树与工具回归、边界检查 2/2 通过（tool-vtk-qt-ctest.log，7.93 秒）。新增验证事件参数传递、事件源先销毁、Qt 槽及结束信号期间拒绝注销、停用后连接移除。git diff --check 通过。

## VTK 监听采用成员函数指针（2026-10-13）

- 按用户最终确认，将 VTK 状态监听改为 AddObserver 成员函数指针重载；observeVtkEvent 接入普通成员回调，再调用具体工具 onVtkEvent。不再使用 vtkEventQtSlotConnect 或 SLOT，取代上一节方案；Qt 输入过滤器和其他新式 Qt 连接保持不变。
- 删除静态回调及 clientData 管理；基类保存事件源弱引用、事件 ID 和 tag，拒绝重复监听，停用及析构只清理自身监听，保留源对象提前销毁和回调执行时的注销保护。
- 主程序、工具测试及独立头文件编译通过，293 个源码文件依赖检查通过；CTest 工具/树回归及边界检查 2/2 通过（tool-vtk-member-ctest.log，7.95 秒），事件参数、源销毁、停用和回调注销保护测试保留通过，git diff --check 通过。

## 渲染选择列表 STL4043 警告清理（2026-10-13）

- 将 IDOSRenderView.setSelection 中 QList<QString> 的整体相等比较改为长度及按顺序逐项比较，保持原有去重、顺序和重复选择不通知的行为，避免实例化 Qt 5.15.2 使用 checked_array_iterator 的比较实现。
- 没有修改 Qt/STL 头文件、语言版本或添加屏蔽宏。固定目录主程序与树测试构建通过，本次重编译日志 stl4043-build.log 无 STL4043 / checked_array_iterator 警告；这是本次构建范围的验证，不表示所有其他模块全量重编译都无同类警告。
- tree_incremental CTest 通过（stl4043-ctest.log，4.92 秒），git diff --check 通过。

## 文档更新（2026-10-13）

- README 重写为项目概览、能力边界、构建、使用、架构、测试和文档导航，删除占位助手、旧类型注册表、右侧属性面板及无渲染构建等过时描述。
- 新增使用指南与开发指南，覆盖数据和视图、Flow 输入/初始化/计算/结果、助手资源、构建目录、测试、翻译和扩展。插件、点集示例和图标说明同步更新。
- 架构文档接口按头文件核对，源码目录补充已存在的结果导入命令；保留编码规范及既有目录，不修改 AGENTS.md 的规则。
- 本轮为文档变更，检查相对链接、代码/文档状态一致性和 git diff --check，不配置或编译，不新增本轮功能验收记录。后续具体工具接入和工程持久化实施后继续补充操作说明。
- 已核对 9 份项目说明文档的本地链接与代码围栏，验证通过；CMake 预设版本需求按本机官方手册确认，未宣称根脚本的旧最低版本等同于实际配置兼容性。

## 后续顺序

构建路径已按用户纠正同步至 README、开发指南、架构设计及 AGENTS.md。本次只修正文档，不搬迁旧构建缓存，也不把旧目录测试结果记录成新目录验收。

1. 在 app 接入适应窗口、标准视角、坐标轴等具体工具，完成可见悬浮工具条和窄窗口交互验收。
2. 实施工程持久化并验证保存重开，当前相关入口禁用。
3. 扩大真实工况、网格/动态属性和报告步播放验收范围。
4. 按部署需求准备 WSL2、Flow 离线环境，补充安装及运行验证流程。
5. 根据业务需要实现拟合评价、通用分析、Python 运行时及更多任务配置。
6. 持续观察历史插件卸载超时；新增功能后运行必要全量回归并准确更新验收范围。
