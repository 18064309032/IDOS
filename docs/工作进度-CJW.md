# IDOS 工作进度（CJW）

## 修复 EGRID 与配套 INIT 维度不一致导致网格导入失败（2026-10-19）

- 排查 `D:\QtCoder\IDOS\data\UPSCALE_GASWATER_ECL\UPSCALE_GASWATER_ECL.EGRID` 时确认，EGRID 网格维度为 125×21×25，而同名前缀 INIT 的 INTEHEAD 记录为 125×21×50，活跃单元数也不一致。此前导入逻辑把配套 INIT 不匹配视为整个网格导入失败，导致有效 EGRID 几何也被丢弃。
- 网格 Provider 现在保留可读取的网格；不匹配或无法读取的 INIT、UNRST、SMSPEC 配套内容单独跳过并写入应用日志。避免将不兼容的动态属性或摘要结果错误关联到网格。
- `D:\CMakeCoder\IDOS-Build` Debug `idos` 构建通过。尚未在 GUI 中重新导入该 EGRID 验收网格显示和日志提示。

## GRDECL 属性关键字与大网格导入排查（2026-10-19）

- 用户反馈 Petrel GRDECL 导入时 OPM 报 `Unknown keyword: POISSONSRATIO`。此问题是 OPM Parser 缺少关键字定义，和后续出现的 `bad allocation` 内存分配失败是两个独立问题；用户明确当前关注关键字识别。
- 已检查 OPM 源码仓库 `D:\QtCoder\Builder\opm-cmake`。IDOS 当前网格解析器设置了忽略未知关键字，因此未知属性不会保存在 Deck 中；项目已有 `resources/opm/extra-keywords.json` 的 OPM 自定义关键字扩展机制，但 GRDECL 网格 Provider 尚未加载该扩展文件。
- OPM 源码及关键字定义生成流程仍待继续检查；尚未改动 OPM 源码、关键字定义或重编 OPM。后续应确认 `POISSONSRATIO` 的数组类型/段信息，并让网格解析器识别后保留属性值。
- 同日完成 IDOS 网格导入中的 PRESSURE 属性提取和大型 QVector 移动交接优化；Debug `idos` 构建通过。尚未使用用户的 Petrel GRDECL 在 GUI 中回归导入，不能据此认定关键字问题或内存问题已经验收解决。

## 修复 OPM fmt 运行库部署（2026-10-19）

- Release 启动时报缺少 `fmt.dll`。此前部署命令在 `idos_providers` 构建阶段执行，且 `idos` 的复制步骤排在 Qt 部署之前，最终产物目录没有该文件。移除 Provider 层重复复制，并将配置对应的 `fmt.dll`/`fmtd.dll` 复制放在 `idos` 所有部署步骤之后。
- 在唯一构建目录 `D:\CMakeCoder\IDOS-Build` 完成 Release `idos` 构建，退出码为 0。核对 `bin/Release`：`fmt.dll` 存在；AI 配置、4.68 GB 模型及 52 个运行时文件均已部署。`windeployqt` 有 `VCINSTALLDIR` 未设置警告，未影响构建成功。

## 支持从 GRDECL 导入工况网格（2026-10-19）

- 工况“导入网格”文件筛选器新增 `.GRDECL`/`.grdecl`。
- ECLIPSE 网格 Provider 通过 OPM Parser 读取 GRDECL 的网格关键字，并将 ACTNUM 传给 EclipseGrid；网格导入解析上下文忽略未知关键字，使 Petrel 导出的 `POISSONSRATIO` 等非几何属性块被跳过而不阻断几何导入。
- 属性导入仍使用网格属性 Provider，避免两个 Provider 都声明 `.GRDECL` 后导入入口选错。
- `IDOSImportGridCommand` 明确调用网格 Provider。Debug 下 Provider 与 App 的 C++ 编译通过，Release `idos` 完整构建通过；尚未使用实际 GRDECL 文件做运行验收。
- 依据 Petrel `eclipse.GRDECL` 的 `POISSONSRATIO` 报错，在网格解析上下文忽略未知关键字块；Release Provider 源文件编译通过。Release 应用仍在运行，完整重新链接/实际文件验收待应用关闭后完成。
- GRDECL 导入现在复用同一份 OPM Deck，将识别到且单元数匹配的 `PRESSURE` 数组创建为网格静态属性，并关联到导入网格；压力数值暂按原始值保存，单位标记为未知，避免错误换算。Debug `idos` 构建通过；尚未在 GUI 中用 `Zi224_Pp.GRDECL` 完成导入验收。
- 针对约 353 万单元网格导入出现 `bad allocation`，调整 GRDECL 导入次序：属性对象在 OPM Deck 释放前提取，随后释放 Deck，再转换几何数组；大型 `COORD`/`ZCORN`/`ACTNUM` 和属性数组通过按值接收并移动到数据对象，避免 setter 再复制整份 QVector。导入完成后立即释放 OPM EclipseGrid。Debug `idos` 构建通过；实际文件重试验收待进行。

## 工程存储性能第一轮优化（2026-10-19）

- 根据 40.6 MB 工程的实测日志，优化前保存 3.36 秒、打开 2.24 秒；首次版本 3 实测保存 3.67 秒、打开 2.24 秒，没有变快。直接构造表格数值/字符串列的尝试增加了重复 `QVariantMap` 转换，现已撤回，不再作为性能优化保留。
- 对象级日志定位到 `PRESSURE`、`SWAT`、`BG` 报告步属性及多口井。网格属性改用固定值、报告步编号和二维报告步数组布局，减少每个报告步重复创建 HDF5 组/数据集；新保存格式升为版本 3，版本 1/2 工程继续走原读取实现。
- 保存/打开按对象记录超过 100 ms 的耗时，日志包含对象类型和名称。`D:\CMakeCoder\IDOS-Build` Release `idos` 全量构建通过；Debug 版本 3 已在 40.6 MB 工程上运行，实测结果未显示速度提升。未运行测试；尚未在大工程上测量。
- 当前实现仍将各对象数据整体恢复到内存，`QVariant`/`QVector` 序列化不适合单工程承载数十至上百 GB；真正支持此规模需要后续分块数据访问/延迟加载设计，本轮不宣称解决该容量上限。

## 校正唯一构建目录（2026-10-19）

- 将 `AGENTS.md` 中过时的 `E:\CJW-Archive\IDOS-Build` 更新为 `D:\CMakeCoder\IDOS-Build`，与 `CMakePresets.json` 的 `${sourceDir}/../IDOS-Build` 配置一致。该目录当前存在，后续 CMake 配置、编译和构建测试均使用此目录。

## 单页模拟运行与自动生成临时 DATA（2026-10-19）

- 将“输入、运行、结果”三个二级页面合并为一个模拟工作区页面，统一显示运行状态、历史记录和结果文件；移除旧输入工作流页及 DATA 浏览页，避免主操作被多个页面拆分。
- Ribbon“运行”现在先检查原始工况，再调用现有 Eclipse DATA Provider 按当前工程中的网格/静态属性生成临时 DATA，复制到新的独立运行目录并启动 Flow。生成、检查、复制及 Flow 错误均写入应用输出日志；临时 DATA 在准备完成后自动清理。
- Flow 成功仍通过现有结果导入命令将动态属性加载到工程树；Ribbon 播放控件继续控制结果步。运行历史保留在唯一工作区，可双击历史记录切换结果目录。
- 工况树上的显式“生成模拟输入”保留为按需对话框，不属于运行必需步骤。运行设置继续集中保存运行目录上级路径、WSL 发行版、Flow 可执行文件及线程数。
- Ribbon 中新增“工况查看”分组收纳表格、属性曲线和井控制；“工作区”入口改名为“模拟运行”并补充用途提示；删除重复的 Ribbon“结果文件”入口，结果文件只从模拟运行工作区查看。
- 更新中文翻译并整理 Ribbon：表格、属性曲线、井控制归入“工况查看”；“工作区”入口及停靠面板统一命名为“模拟运行”，按钮可显隐面板且状态与标题栏关闭操作同步，无工况时仍可用；结果文件入口保留在模拟运行工作区。Debug 构建在 `D:\CMakeCoder\IDOS-Build` 完成翻译生成和 C++ 编译，链接阶段因运行中的 `idosd` 占用 `idos_appd.dll` 失败；未运行测试。

## 简化模拟运行流程并加入历史记录（2026-10-19）

- 将运行设置入口放到 Ribbon，统一配置运行目录上级文件夹、WSL 发行版、Flow 可执行路径和 CPU 线程数。未指定运行目录时，按工况输入文件位置创建默认目录。
- 移除运行阶段可见的预检和初始化区块。选中工况后点击 Ribbon“运行”，系统自动执行输入检查、创建唯一运行目录、复制并验证 DATA 及其引用文件，然后启动 Flow；失败信息写入日志/输出面板。
- 删除运行工作区内重复的运行/停止按钮，由 Ribbon 运行和停止 Action 控制。运行页保留状态、日志和该工况的历史记录；每次运行记录时间、状态、运行/输出目录及执行参数，双击历史记录可打开对应结果目录。
- Flow 成功后自动导入动态属性并切换到结果页。结果文件页可查看当前或历史运行目录中的输出文件。
- 更新中文翻译。唯一构建目录 `E:\CJW-Archive\IDOS-Build` 当前不存在，环境没有 `E:` 盘，故未在其他目录构建；尚待在指定构建机编译和 GUI 验收。

## 整理模拟工作区的运行与结果页面（2026-10-19）

- 重新设计右侧模拟工作区外壳：顶部固定显示工况身份，下面用带图标的三阶段导航切换“输入、运行、结果”专属视图；每个阶段独立滚动，不再把所有功能堆成一个长页面。
- 输入阶段集中显示工况与 DATA 生成；运行阶段集中显示预检、初始化和 Flow；结果阶段显示结果文件及导入操作。Ribbon 操作直接切换到对应阶段并定位到具体控件。
- 结果文件清单、状态、文件打开、运行目录访问和动态属性导入统一放入结果卡片；结果控件改为普通嵌入页面，表格和按钮样式与工作区统一。Ribbon 的预检、初始化、运行、停止和结果入口直接滚动定位到对应流程区域。
- 按 `AGENTS.md` 指定的唯一构建目录尝试配置 Debug 构建，但当前环境没有 `E:` 盘，CMake 无法创建 `E:\CJW-Archive\IDOS-Build`，因此本轮模拟工作区重设计尚未编译。未改用其他构建目录。

## 合并模拟输入与 DATA 生成页面（2026-10-19）

- 将输入摘要与 DATA 生成器放入同一个白色面板，使用共同边框和统一内边距；两部分之间仅保留轻分隔线，不再显示成上下两个独立控件。
- 移除摘要区重复的“生成 DATA”按钮，仅保留生成器中的主操作按钮；“浏览原始 DATA”继续作为辅助入口。
- 生成器内布局改为零外边距并统一编辑框与按钮样式，使其嵌入输入面板时与周边控件对齐。保留输入页滚动及 Ribbon/工况树“生成模拟输入”入口。
- `D:\CMakeCoder\IDOS-Build` Debug `idos` 构建通过，程序已启动供查看；部署阶段出现 `VCINSTALLDIR` 未设置警告，不影响目标生成。尚未手动加载工况验收面板视觉效果。
## 模拟工作区右侧面板调整（2026-10-19）

- 用户选择简化右侧工作区：保留“输入、运行、结果”三个页签，左侧工况树作为工况选择入口，不再重复显示独立工况页。
- 输入页改为工况名称、当前运行 DATA、原始来源、输入检查状态和两个带图标的操作入口。解析摘要不再作为单独页面；DATA 浏览器按需打开独立窗口。
- 运行页继续提供运行预检、初始化和 Flow 入口；结果页继续显示当前工况结果文件。Ribbon 与左侧树、中央三维视图、底部输出区保持原有布局。
- 删除仅用于右侧独立摘要页的 `IDOSSimulationParseSummaryWidget`，并同步 CMake、公共头检查清单及源码目录说明。DATA 解析数据仍用于输入状态判断和其他现有业务。
- 使用 `D:\CMakeCoder\IDOS-Build` 构建 `idos` Debug 目标通过；期间因旧应用进程占用 `idos_appd.dll`，关闭该进程后重试成功。部署阶段有 `VCINSTALLDIR` 未设置警告，不影响构建结果。
- 已启动构建产物 `D:\CMakeCoder\IDOS-Build\bin\Debug\idosd.exe`供查看。尚未加载工程并手动验收 DATA 路径显示、浏览窗口、生成窗口及三页签布局。

## 将 Flow 运行控制嵌入工作区（2026-10-19）

- 按用户反馈，将“运行 Flow”页中的“打开运行控制”按钮替换为现有运行控制内容本身，直接在右侧工作区显示运行状态、目录、输入文件、WSL/Flow 配置、启动/停止和输出日志；保留运行预检及初始化入口。
- 运行控件改为普通嵌入式页面。工作区因工程数据更新重建时先将运行控件暂时移出页面，再复用到新页，避免运行过程中状态写回触发重建而销毁正在执行的进程；空闲控件随当前工况切换重建。
- `D:\CMakeCoder\IDOS-Build` Debug `idos` 构建通过；程序已重新启动供查看。部署提示 `VCINSTALLDIR` 未设置，不影响构建。

## 移除模拟工作区二级操作页签（2026-10-19）

- 同意输入/运行页中的二级操作页签会把一个连续工作流程拆散。右侧只保留“输入、运行、结果”三个主页签。
- 输入页改为纵向滚动布局：上方为工况与 DATA 状态，下方直接嵌入 DATA 生成器；点击摘要卡片的“生成 DATA”按钮会滚动到生成区。DATA 浏览器仍按需单独打开。
- 运行页改为纵向滚动布局，按顺序显示运行预检、初始化、Flow 控制三个区块。Ribbon 的预检、初始化、运行和停止操作会切换到运行页并定位到对应区块。
- 刷新工作区时继续复用正在运行的 Flow 控件和 DATA 生成器，避免更新工况状态时丢失进程或生成预览；切换工况/工程时为新上下文重建相关控件。
- 本版待编译并启动查看。

## AI 工程查询与连续命令调用（2026-10-17）

- 根据首次手动验收中助手把“查询当前工程有哪些数据”回答成命令使用说明的反馈，强化系统提示及 `project.objects` 命令描述：工程内容/对象查询必须实际调用 `project.objects`，特定对象或工况详情必须调用 `project.inspect`，禁止让用户自行运行命令。未编译或启动模型；需按该输入复测工具调用。
- 用户再次反馈模型把 `project.objects` 当文本输出。对明确的工程数据清单请求，AssistantService 直接排入现有 `project.objects` 结构化命令执行流程，将真实结果放入对话后再让模型组织回答；一般请求仍由模型自动选择工具。这样该只读查询不依赖模型是否服从工具调用提示。仍未编译或联调。
本轮沿用现有 AssistantService、LocalModelService、CommandRegistry 和 CommandManager，补齐源码调用链；没有重构 Project、两棵树或通用渲染框架，没有启动编译、C++ 测试或真实模型联调。

- 新增具体只读命令 `IDOSListObjectsCommand`（`project.objects`）与 `IDOSInspectObjectCommand`（`project.inspect`）及各自 Metadata，在 MainWindow 的命令注册表注册。查询返回工程元信息、当前对象/工况标识、数据名称/类型/归属，支持筛选、稳定排序及分页；详情查询提供工况引用分页以及井、网格、属性的有限摘要，不序列化完整网格数组或测井采样。查询使用现有 Query 类型，不进入撤销历史。
- LocalModelService 的 `chat()` 改为接收消息列表。AssistantService 保存用户、助手、工具调用和工具结果；工具结果带原始 `tool_call_id` 返回模型，成功或失败均进入后续请求，直到模型给出普通回复或停止。下一轮用户请求带上会话历史，历史超限时按完整轮次移除旧消息，避免拆散工具调用与结果。
- 删除从普通回复文本解析并执行命令的旧路径，只执行结构化工具调用。校验工具调用标识、参数格式和本轮提供的命令集合；拒绝截断响应和同批重复调用标识。业务修改仍由现有 CommandManager 执行并进入撤销历史，未引入另一套业务分发。
- 每轮最多执行 16 个工具调用；命令分步骤通过事件循环执行。界面发送按钮在运行中变为“停止”，显示执行进度与结果。停止不回滚已完成命令，正在同步执行的命令完成后停止后续步骤；尚未执行的调用补齐 `not_executed` 结果，后续会话不会携带悬空工具调用。
- 工程切换/销毁清理历史及待发送输入，取消网络请求并使旧流程失效；命令执行期间发生切换时不将旧结果写入新会话。模型进程退出时停止请求，网络请求设置超时；健康检查忽略已取消的旧回复。
- 更新助手回归用例，使用现有测试类内的本地 HTTP 模型响应模拟，覆盖查询→执行→结果返回、错误参数修正、查询分页和工况引用、查询不进入撤销历史、普通文本不执行、调用上限、取消、历史保留与工程切换。补充新头文件独立检查清单、CMake 源码条目和中文翻译。
- 已执行：模块依赖检查通过（505 个源文件）；本轮 15 个助手/查询/测试文件的声明、括号、禁用语法、翻译及 CMake 条目静态核对通过；`git diff --check` 通过。上述 C++ 回归用例尚未编译或运行，真实模型的工具调用效果和界面交互仍待用户编译后验收。
- 当前范围仍为工程查询与已注册业务命令；没有新增相机旋转、渲染显示控制或模拟运行的 AI 命令。

## 编译反馈修正（2026-10-17）

- 修正工程恢复后已勾选对象要拖动才显示的问题：Server 先恢复相机再装入场景，原先裁剪范围在空场景上计算；RenderView 现于重建可见对象后、绘制前按对象边界重算裁剪范围，不重置相机位置和缩放。新增 `onRenderRestoreClipping` 回归用例，覆盖清空场景后恢复远距离相机、无需交互即得到有效裁剪范围，并检查保存的相机状态不变。本次未编译、未执行该用例或 GUI 验证。
- 根据用户编译反馈，为 `idosrendertoolprovider.cpp` 补充 `tools/idostool.h`：连接 `RenderView::toolEnabled/toolDisabled` 信号时，Qt 元类型模板需要参数 `IDOSTool` 的完整定义，前置声明不足。未启动编译。
- 修复 `IDOSRenderToolProvider::onGridXrayToggled` 将通用 RenderProvider 当作网格 Provider 使用的问题：开启和关闭透视两条路径均检查转换为 `IDOSGridRenderProvider` 后调用网格显示接口，并直接包含对应头文件；不向通用渲染接口添加网格业务函数。
- 本次仅修改源码并静态核对，由用户继续编译，未启动构建。
- 同类调用检查同时修正工况测试对通用 Provider 的 `setVisible` 调用，改为由 RenderObject 设置显示状态。

## 接口与完整调用链核对、源码收尾（2026-10-17）

本轮依据已确认的树框架修订及后续接口简化要求继续改造。以下为已落实的源码工作；按用户要求没有配置、编译、运行 C++ 测试或启动 GUI。此前目录与架构文档的工作区修改保留，本轮文档只更新此记录。

### 接口及所有权

- 业务数据类统一为 Object 后缀，包括 Well、Grid、GridProperty、GridFilter、GridSurface、GridBoundary、FluidRockTable、FractureSchedule、FractureMaterialCatalog、NaturalFractureSet、ProductionHistory、MicroseismicSet、SeismicVolume、SimulationSummaryResults 及示例插件 PointSet；文件、包含、CMake、测试、翻译上下文同步迁移，类型持久化标识保持稳定，无旧类别名。
- RenderProviderMetadata/Registry、ToolProviderMetadata/Registry 使用确认名称；Data、Tree、Render、Tool、Command 注册接口统一 registerMetadata/unregisterMetadata/metadata，并按注册成功转移所有权、失败保留调用方所有权处理。文件解析注册器按单独确认的约定继续保持原单例及非 QObject 设计。
- CommandRegistry 由 MainWindow 创建和持有，AssistantWidget/Service 借用；工程切换解绑旧 Manager、绑定新 Manager，助手不再拥有或决定命令注册表。Command 与 Manager 对工程/注册表使用非拥有的失效保护。
- 数据对象删除全局可见性字段、访问接口及工程转发信号；可见性、选择、透明度由每个视图的 Provider 保存。RenderObject 删除与 Provider 重复的三份回退状态，通用渲染接口没有 IDOSDataObject/Project 依赖。

### 菜单、两棵树、命令和工程

- 完整路径为 View 命中节点/空白 → 对应 Provider 创建 QAction 并连接具名槽 → 专用 Model 明确业务接口及信号 → AppTreeAccess → 应用业务入口 → 具体 Command → Project。节点归属、菜单 session、多选固定目标及命令目标检查保留；删除 setTreeModel 转发接口，View 使用 setModel。
- Model 匹配 Metadata 时先限定所属树，避免另一棵树的 Provider 抢先匹配；更换 Metadata 时不复用旧 Provider。注销中的 Metadata 不再参与创建，树注册器析构按注销流程清理存活节点。
- 鼠标右键按命中位置、键盘右键按当前索引选择菜单；无有效键盘索引时使用视口中心定位。拒绝外部模型索引，并保护菜单嵌套事件循环期间 View/Menu 被销毁的情形。
- 七类导入命令保留各自实现；网格过滤器进一步拆为 Create/Update/DeleteGridFilterCommand 三个命令及各自 Metadata，删除旧 GridFilterCommand 操作字符串分支。复制/移动及工况引用的既有事务算法继续服务剪贴板和引用业务，不作为菜单的通用动作路由。
- Project::add 拒绝同 ID 不同对象及其他容器拥有的对象，不静默替换；新建、导入、删除恢复及插件输出命令检查插入结果或完整预校验。导入重做/撤销检查缓存仍存在、归属和 ID 冲突；删除工况/网格撤销先校验全部待恢复子对象，避免先恢复一部分再报错。缓存对象已丢失的新建命令不在重做时另造新身份。
- 命令执行/撤销在工程批量更新中发布结果，内部已有批次可嵌套；失败仍由各命令的预检和回滚负责，不把批量通知当作回滚。工程先清空已提取的通知集合再发送，避免重入清掉下一批通知；Simulation 补接批量数据变化。失败时 CommandManager 不移动历史位置或销毁 redo。

### 渲染、保存恢复及撤销

- 源数据变化由 RenderServer 经 Metadata 更新具体 Provider 的依赖，再通知 Scene/View；AppTreeAccess 根据实际目标视图回写两棵树的勾选状态。首次勾选才创建显示对象，隐藏不存在的显示对象不创建；井/网格显示设置及属性显示菜单首次使用也可建立实际目标视图中的显示对象。
- 删除数据前按视图保存 Provider 状态和选择；删除撤销、导入重做或批量回滚重新加入数据时，在依赖可用后恢复对应显示对象，保留各视图独立的显隐、透明度和选择。切换工程或移除视图清理缓存，缓存不持有源数据。
- RenderServer::restoreState 改为返回成功状态：先准备并校验 Provider，再恢复视图状态及替换场景；无效对象/类型/Provider 状态失败时销毁临时对象，不污染原场景；视图状态失败时回退视图状态和相机链接。成功恢复清除旧场景多余对象，并恢复选择、相机链接和当前视图；主窗口记录恢复失败。
- 工程 HDF5 保存/读取继续使用已注册数据 Metadata，无需重新读取源文件；加载校验实际工厂类型及插入结果，源数据不再写可见性，显示状态通过工程的展示快照保存。保存的原子替换及失败保护保留。

### 插件和工具

- 示例插件、裂缝建模、多尺度裂缝插件迁移注册接口；命令 Metadata 注册失败时由本地 unique_ptr 释放，成功才转交 Registry。插件卸载仍检查历史和活实例，先注销/销毁展示及菜单，再卸载动态库。
- DfnGenerationWidget、FractureCheckWidget、CurveFitEvaluationWidget、FieldDifferenceWidget 构造接收 IDOSInterface，绑定打开时的工程；检查窗口同时绑定原目标视图。工程切换、服务销毁或目标失效关闭旧窗口。
- PointZoom、PointProbe、DistanceMeasurement、Selection、ClippingPlane、SliceByPoints 各自具有 ToolProvider/Metadata，动作直接调用本工具实例；删除导航 Provider 按 QAction 下标切换六种交互工具的分发。导航视图控制保留所属 Provider，工具条仍由视图拥有。

### 静态验证与待运行验证

- 已补充 Project 插入所有权/同 ID 冲突/失败历史保持和渲染多视图删除撤销/状态替换/无效恢复保护用例；已有渲染用例改为显式请求显示，不再假设添加数据即创建所有视图的 RenderObject。测试源码已迁移，尚未执行。
- 依赖边界检查覆盖 501 个源文件并通过，50 个边界检查器用例通过；项目包含路径、CMake 实际文件、类成员引用及方法声明名称检查通过；翻译 XML 可解析且无 unfinished。旧总导入类、旧过滤器分发类、旧 Registry 类名、setTreeModel、triggerAction、submitAction、bindMenu、bindItem 等替代接口在源码/测试中无残留。git diff --check 通过。
- 本轮未执行 CMake、编译、公共头文件独立编译、C++ 测试和 GUI 验收。源码静态核对不能替代这些验证；后续构建和运行验收只能使用 E:/CJW-Archive/IDOS-Build，不把此前旧版本测试结果当作本次通过结果。

## 导入命令拆分：补齐遗漏（2026-10-17）

- 上一轮的目录迁移、接口命名和渲染分离不等于整套架构交付完成。本轮检查发现：架构方案要求的七种导入命令仍由 `IDOSImportObjectsCommand::Mode` 统一分派，上一轮漏做了这部分。
- 已删除 `idosimportcommands.h/.cpp` 和 `IDOSImportObjectsCommand`，按方案拆为 `IDOSImportCaseCommand`、`IDOSImportGridCommand`、`IDOSImportGridPropertyCommand`、`IDOSImportWellDataCommand`、`IDOSImportFluidRockTablesCommand`、`IDOSImportDataCommand`、`IDOSImportSummaryResultsCommand`。各命令及其 Metadata 放在同名文件对中，直接继承既有命令基类，各自保留所需参数、校验、导入、撤销及缓存；没有新增 Mode 基类、转发头或旧接口别名。独立的 RestartResults 命令继续保留。
- 原有井数据合并及前后快照、工况导入时的流体岩石表拆分、INIT 活动网格值展开、网格替换缓存、工况引用恢复、重做复用已读数据等实现迁入对应命令。流体岩石表导入直接使用自己的解析器，删除不再需要的公共对象筛选及加载转发函数。
- 主窗口的七类导入入口直接创建各自命令，注册器的 Metadata 也各自创建明确的命令类型。助手和脚本继续使用既有命令注册协议。流体岩石表关联 Metadata 移至既有 CaseReference 命令文件，不再混在导入总文件中。CMake 源文件列表、独立头文件检查清单和现有测试调用同步迁移。
- 静态检查：依赖边界检查覆盖 485 个源文件并通过；七对新文件的声明、括号、成员、头文件路径及 CMake 引用检查通过；源码和测试中无旧总文件、旧命令类及被替换 Metadata 名称残留。未配置 CMake、未编译、未运行 C++ 测试；撤销/重做及实际导入效果尚未运行验收，不能据此宣称整套架构已经完成。文档仅更新本工作记录。

## 目录与接口改造：源码阶段（2026-10-17）

- 本轮在已有工作区修改上继续实施，直接移动、重写现有文件并迁移调用方，不增加旧接口别名、转发头或兼容调用链。按本次要求未配置 CMake、未编译、未运行 C++ 测试或界面；文档仅更新本工作记录，架构设计、源码目录说明及其他文档未在本轮修改。
- 目录整理：命令基础设施移入 `src/core/command`；数据 Metadata/Registry 移入 `src/core/data`；树 Provider/Metadata/Registry 与 Model/Item 统一位于 `src/core/tree`；文件解析接口保留在 `src/core/providers`。通用渲染框架位于 `src/render`，工具位于 `src/render/tools`；工程渲染装配的 Server/Registry/Metadata 移入 `src/app/render`。具体 Provider 和业务窗口按 well、grid、case、fluidrock、seismic、data 归入 app 对应目录，模拟运行及插件管理分别归入 app/simulation、app/plugin。同名头文件和实现一同迁移，CMake、源码 include、插件与现有测试引用同步调整。
- 接口命名：工厂统一使用 `create`，展示状态统一使用 `saveState`/`restoreState`；Model 查询使用 `item`、`index`、`object`、`objects`；Access 装配接口使用 `setDataModel`/`setCaseModel`，两个专用 View 使用 Qt 的 `setModel` 并分别提供 `dataModel`/`caseModel`。批量删除、重命名使用 `removeObjects`/`rename`，对应 `removeRequested`/`renameRequested`。文件读取沿用返回 `QList<IDOSDataObject*>` 的 `read(file)`；迁移所有受影响的具体实现和调用端，删除被替换名称。
- 树链路对齐：数据树和工况树继续使用各自的 Model/View；节点及空白 Provider 自己创建 QAction、连接具名槽，经专用 Model 的明确业务信号进入 AppTreeAccess 与命令。源码中不保留 `triggerAction`、`submitAction`、`bindMenu` 或基于 QAction operation 属性的通用分发。工程会话、节点归属、多选目标和命令校验继续保留。
- 渲染接口重写：`IDOSRenderProvider` 仅提供 `update`、`props`、`saveState`、`restoreState`，删除通用接口中的数据对象访问和数据依赖操作；`idos_render` 不再链接 `idos_core`。RenderView 使用通用 VTK prop 接口添加和拾取展示内容，处理三维包围盒与缩放，不判断井、网格等业务类型。外部数据可由其具体 Provider 自行持有，直接装入 RenderObject/Scene；通用渲染模块无需 IDOSProject 或 IDOSDataObject。
- 工程渲染同步：源数据与渲染对象的对应关系改由应用层 RenderServer 保存；应用层 Metadata 声明依赖并装配、更新具体 Provider 输入。网格与过滤器使用明确的属性输入，删除数据前解除依赖，输入缺失时清理无法继续使用的展示；数据变化和撤销重绑输入时保留 Provider 实例。井、网格、地震、裂缝、边界、曲面及示例点集插件均迁移到新接口，显示状态仍按视图独立管理。
- 静态检查：项目头文件实际路径与 CMake 源文件路径无缺失；依赖边界检查覆盖 474 个源文件并通过，50 个边界检查用例通过；项目类型的头文件声明覆盖检查、实现与声明名称检查、源码括号配对、旧接口残留、禁用 C++ 语法、翻译 XML 解析及 `git diff --check` 检查通过。检查脚本同步禁止 render 依赖 core，允许插件使用应用层公开的渲染注册接口，仍禁止插件依赖主窗口或具体业务窗口。临时迁移脚本已移除。
- 待验证：本轮结果是未经编译的源码改造，不表示构建或功能验收通过。后续获准编译后仅使用 `E:\CJW-Archive\IDOS-Build`，重点验证头文件独立编译、工程状态恢复、网格过滤依赖删除与撤销、多视图选择/显隐、插件装卸，以及节点/空白/多选菜单。此前完整 case_workflow 中已记录的即时创建渲染对象假设与现有延迟创建行为差异，仍需运行验证，未在本轮宣称解决。

## 删除菜单 submitAction 转发函数（2026-10-16）

- 按用户要求删除 ItemProvider 和 TreeMenuProvider 的 submitAction 声明、实现及所有调用；不新增替代包装函数。各具名槽直接调用现有 Model::triggerAction，并明确列出对应业务所需参数，不再遍历 QAction 动态属性统一打包。
- Model::triggerAction 在原有操作/节点检查中校验参数 treeSession；网格着色子菜单保存同一菜单 session，节点 Provider 的 QObject 生命周期继续隔离已删除节点的旧动作。补充子菜单 session、操作及 gridId 路由回归。
- 同步当前架构声明及约定。E:\CJW-Archive\IDOS-Build 的 Debug 全量构建通过（含公共头文件检查和插件）；check_includes、check_includes_regressions、project_dialog、tree_incremental 四项通过，井菜单/工况导入/流体岩石关联/插件菜单专项为 6 通过、0 失败（含初始化与清理）。源码、测试及当前架构声明无旧函数引用；本轮文件 diff 空白检查通过。完整 case_workflow 既有渲染问题未复测或改动。

## 菜单去除过度封装（2026-10-16）

- 按用户要求，各具体 Provider 使用 new QMenu、menu->addAction 和 connect 直接创建菜单与连接具名槽，删除通用 createMenu/addMenuAction/selectedData/addDeleteAction/canDeleteData 接口，以及工况空白菜单和单表菜单的单动作包装函数。
- 多选列表直接传给具体业务成员，不再通过菜单属性反查选择；节点身份直接取所属 Provider，多选动作只保存业务需要的参数。删除能力判定回到具体导入数据/流体岩石 Provider，不在 core 增加能力接口。井/表关联参数仅保留命令所需的 objectId/role。
- 保留执行阶段的 submitAction 作为必要校验：读取菜单创建时的 session，确认上下文后调用既有 Model/Access/命令链；节点 Provider 销毁时由 Qt 断开动作连接。保持复制粘贴、多选删除和旧菜单失效行为。
- 同步当前架构声明及约定；本轮 C++ 源码/头文件净减少 113 行。E:\CJW-Archive\IDOS-Build 的 Debug 全量构建通过（含公共头文件检查和插件）；check_includes、check_includes_regressions、project_dialog、tree_incremental 四项全部通过，井菜单/工况导入/流体岩石关联/插件菜单专项为 6 通过、0 失败（含初始化与清理）。本轮文件 diff 空白检查通过；完整 case_workflow 既有渲染问题未复测或改动。

## Provider 菜单职责与具名槽整理（2026-10-16）

- Grid/Well/SimulationCase/ImportedData/FluidRock/PointSet 及工况空白菜单的 QAction 直接连接所属 Provider 的具名 on 槽；删除 QAction operation 属性和 Model::onMenuActionTriggered 通用解析槽。Provider 槽以明确操作调用原有 Model/Access 业务链，不直接访问工程或渲染服务。
- 流体岩石与导入数据的 createContextMenu 按分类、单数据、多选明确分流，分类导入和对象操作由 private 成员分别组装。单表只保留一个添加到工况入口；多选保留兼容数据关联与批量删除。
- 删除 DataTreeRootProvider 的 addDataActions/createMimeData/supportsDelete 工具职责；根 Provider 仅初始化固定分类。MIME 创建回到 DataTreeMenuProvider；各业务 Provider 自己组装重命名和关联菜单。通用 ItemProvider 只复用上下文快照、选中数据查询、删除能力查询和安全提交，不解析业务类型或操作属性。
- 更新 9 条中文翻译上下文及当前接口文档；翻译编译为 1718 条完成、0 条未完成。新增分类导入、单表入口去重、跨 Provider 混合选择删除目标和具名槽分发回归；保留旧菜单 session 与插件生命周期保护。
- 验证：E:\CJW-Archive\IDOS-Build 的 Debug 全量构建通过，包含公共头文件检查和插件。check_includes、check_includes_regressions、project_dialog、tree_incremental 四项全部通过；onWellInputMenus/onCreateAndImport/onFluidRockReferences/onExamplePluginMenu 专项为 6 通过、0 失败（含初始化与清理）。本轮文件 diff 空白检查和翻译 XML 解析通过。完整 case_workflow 的既有渲染问题未在本轮复测或改动。

## ItemProvider 节点设置接口更名（2026-10-16）

- 按用户要求将节点绑定接口统一命名为 setItem，同步声明、实现、Item 构造调用和架构文档；原有一次性绑定及所属关系校验保持。
- 旧接口名称扫描无残留，git diff --check 通过；E:\CJW-Archive\IDOS-Build 的 Debug idos_core 编译通过。接口更名不新增或重复运行功能测试。

## 树菜单按节点与空白分派、Provider 自行连接动作（2026-10-16）

- 按用户确认，右键节点仅调用该 Item Provider 的菜单工厂，右键空白仅调用树 MenuProvider；节点无菜单时不退回空白菜单。View 保留选择更新、扩展信号、弹出及释放，删除 bindMenu/onActionTriggered 和 populateTransferMenu。
- 网格、井及其子节点、工况及其业务子节点的菜单移入所属 Provider；内部节点使用相应业务 Provider，refresh 区分对象根和业务子项，避免子项重复展开整棵业务树。数据分类中的井目录绑定 Well Provider。既有导入数据、流体岩石表及 PointSet 节点在创建动作时直接完成连接。
- Provider 的 createMenu 保存创建时的选中节点 ID 和 session；addMenuAction 直接连接 Model 的具名槽，由 Model 校验后走原有 triggerAction/Access/命令链，不保存工程或渲染服务。树 MenuProvider 通过菜单扩展信号补充复制、剪切、粘贴，保留 MIME 快照和目标索引；数据根 Provider 共用既有数据传输描述及通用节点操作。多选不启用单节点业务动作，支持既有批量删除和复制。
- 迁移 44 条已有中文翻译至新上下文；翻译编译为 1709 条完成、0 条未完成。同步当前架构声明及 AGENTS 菜单职责约定。修复既有工程对话框测试仍使用已删除 Model::project 接口的问题，改用 MainWindow 工程和 Model session。
- 验证：E:\CJW-Archive\IDOS-Build 的 Debug 全量构建通过，包含主程序、插件、公共头文件检查与现有测试目标。新增直接由 Provider 创建并触发动作、旧 session 拒绝、无树 MenuProvider 时节点菜单可用、弹出窗口标志、节点/空白菜单隔离、工况树网格菜单及插件删除/撤销/工程切换的回归；最终相关目标重编通过。tree_incremental 与 project_dialog 通过；菜单专项 onWellInputMenus/onCreateAndImport/onFluidRockReferences/onExamplePluginMenu 为 6 通过、0 失败（含初始化与清理）。check_includes 与 check_includes_regressions 通过。更新既有批量导入测试中要求重建分类的过时断言，改为验证分类保留且无删除通知。git diff --check 和翻译 XML 检查通过。
- 验证限制：完整 case_workflow 未通过，onDfnGeneration/onFractureDisplay/onDeleteImportedData 断言渲染对象已存在失败，随后 onDataGroupChecks 解引用未创建的渲染对象发生访问异常；该批用例仍依赖此前即时创建渲染对象的行为。本次未改渲染初始化或扩展这批验收，不将菜单专项通过视为完整工况流程通过。

## 树右键菜单弹出标志修复（2026-10-16）

- 修复 TreeView 接管无父对象菜单时调用 QWidget::setParent 丢失原有窗口标志的问题；视图级菜单和节点菜单两条路径均保留 windowFlags，避免菜单作为普通子控件嵌入树面板。
- 本轮仅修改两处父对象设置；完成 diff 空白检查，未编译或运行界面验收。

## 清理节点菜单、TreeView 和 Access 剩余跨模块依赖（2026-10-16）

- ImportedData/FluidRock/示例插件 PointSet 节点菜单只创建操作描述，移除直接读取工程、执行命令和操作渲染的槽。导入数据、选择地震 Inline、导入流体岩石表和挂接工况迁入应用层接口；PointSet 删除通过已注册命令 ID 进入相同 Model/Access 链，应用层不依赖插件数据类型。
- TreeView 删除 Project/RenderView 指针及四个关联接口，不再向 QAction 写工程指针；拖放使用 Model sessionId 判断上下文。TreeView 统一将 QAction 界面触发转成 Model 操作，普通菜单、节点菜单及插件扩展共用同一条 Model 信号链；删除 MenuProvider 重复绑定助手和触发槽。菜单扩展信号返回后再绑定动作。
- 网格属性选择、过滤器预览、实际状态查询迁入现有 IDOSGridRenderProvider 业务成员接口。AppTreeAccess 仅调用业务接口、请求 RenderServer 装配并回写 Model，移除具体数据转换、过滤算法和直接渲染对象修改。业务接口接收 View 和数据列表，不保存 Project/Server，不新增类或文件。
- 应用层从自身工程成员获取业务上下文，清理依赖 TreeView::project 的所有旧调用。对话框返回后检查工程/目标存活；地震 Inline 操作重新查找渲染对象，避免使用弹窗期间失效的 Provider。
- 迁移既有测试中的 TreeView 目标视图 API 调用至 RenderServer，移除旧的视图覆盖方式；无当前视图的勾选预期改为拒绝。未新增或运行测试。
- 静态检查通过：10 个修改类的实现名称与头文件声明对应、31 类菜单操作均有应用层处理、源码括号配对、翻译 XML 解析及 git diff --check。TreeView 无工程/渲染引用，三个节点菜单无直接命令/渲染调用，Access 无具体过滤算法或渲染对象修改。上述检查不代替编译和运行验证。
- 节点对话框迁入 MainWindow 后补齐 40 条已有中文翻译；架构声明与编码约定同步更新。本轮未编译；指定 E:\CJW-Archive\IDOS-Build 当前不存在，编译及运行验证尚未完成。

## 菜单参数迁移编译错误修正（2026-10-16）

- 修正 onDataViewWellData 中把 const QVariantMap 参数与 nullptr 比较的迁移遗漏，改为校验树视图及其工程有效性；扫描同文件其他 action 指针表达式，剩余使用属于实际 QAction 指针。未编译或运行测试。

## 菜单经 Model 信号提交操作（2026-10-16）

- Data/Case TreeMenuProvider 仅绑定 TreeView，通过 Model 查询节点和源数据；Data 构造移除 MainWindow 参数。删除菜单中的工程、渲染视图、设置窗口指针及 QAction 关联表，不再直接调用工程命令或渲染接口。
- MenuProvider 调用 Model::triggerAction，Model 发 actionTriggered，AppTreeAccess 接收并校验关联工程/Model，再调用 MainWindow::executeTreeAction。保留 26 个既有菜单动作；导入、删除、重命名、工况、井数据窗口和显示设置逻辑迁入现有应用类，业务修改仍复用命令，设置窗口在工程切换时统一清理。Case 菜单直接连接 Simulation 的信号链已移除。
- 动作捕获 Model sessionId/节点 ID，旧菜单提交前校验有效性。Access 按动作触发时的当前 RenderView 调用显示接口；RenderAvailableRole 由 Access 同步，菜单只查询 Model。修正井轨迹子项的显示设置能力查询，保留网格属性/过滤器关联校验。
- 剪贴板与拖放经 Model 专用 transferRequested 信号进入 Access 和应用命令接口；GUI 线程直连同步返回 bool，保留预校验、实际失败返回 false 及失败不清空剪切剪贴板的语义。剪贴板工程标识由 Access 写入 Model，菜单不读取工程。
- 迁移界面文本补入 IDOSMainWindow 中文翻译上下文，共补齐 124 条已有翻译；翻译 XML 可解析。架构完整声明同步更新。静态检查：所有菜单动作均有应用层分派；修改类的实现名称与声明对应；Data/Case 菜单无工程/渲染/主窗口依赖；git diff --check 通过。
- 本次未新增或运行测试，未执行编译。指定构建目录 E:\CJW-Archive\IDOS-Build 当前不存在，本次改动尚未经过编译及运行验证。

## 网格菜单工程校验修正（2026-10-16）

- 修正 gridDisplayRenderObject 中将 RenderView 当作 TreeView 调用 project() 的错误；恢复通过保存的 TreeView 校验工程上下文，防止旧菜单作用于切换后的工程。未编译或运行测试。

## Access 统一协调模块（2026-10-16）

- IDOSTreeAccess 改为 QObject 基类；IDOSAppTreeAccess(project, parent) 通过 setDataTreeModel/setCaseTreeModel/setRenderServer 对接工程、两个模型和渲染服务，一个工程共用一个 Access。MainWindow 在工程切换及销毁时释放旧 Access，清理模型并重新装配。
- Model 删除 Project/Access 指针、访问器和工程监听槽；Provider 删除 model/project/access/changed 及刷新事务辅助接口。Provider 只保存节点数据、勾选状态及业务节点描述，Model 管理刷新复用、节点增删和 Qt 通知。
- Model 勾选写入发 checkStateChanged；Access 接收并执行当前视图渲染，组操作共用一次批量更新和相机处理。RenderServer 变化经 Access 回写 Model::setCheckState，不重发操作信号。节点勾选能力与当前可操作条件分别保存，避免普通业务子项意外出现勾选框。
- 网格属性选择、过滤器预览及状态查询从 TreeProvider 迁到应用层 Access；菜单和示例插件移除从 Model 获取 Project 的调用，View 的菜单工程上下文由 MainWindow 显式设置。更新现有测试调用方，未新增或运行测试。
- 旧接口引用扫描无残留，修改后的核心类实现名称与头文件声明对应，git diff --check 通过；这些静态检查不代替编译和运行验证。同步架构完整声明、源码目录及 AGENTS 当前架构约定。当前指定构建目录 E:\CJW-Archive\IDOS-Build 不存在，本轮未编译；不得将此前 D 目录构建结果作为本次改动验收。

## 勾选直接写入 Provider（2026-10-16）

- Provider::setData(CheckStateRole) 直接调用 setChecked，并读取实际 checkState 返回是否达到目标状态；删除 Model::itemChecked 与 AppTreeAccess::onItemChecked 的请求转发链。
- AppTreeAccess::setChecked 直接执行 RenderServer 操作，保留目标视图固定、Scene 更新与相机处理；渲染变化继续刷新 Model。Model 对勾选写入刷新实际状态，覆盖组项及具体 Provider 的状态变化。
- 已同步当前架构接口与说明；未编译或运行测试。

## 根 Provider 独立设置（2026-10-16）

- IDOSTreeModel 构造仅接收 Registry 和 QObject parent；使用 setRootProvider 设置业务根 Provider，接管其所有权并在模型重置通知内替换旧根及子树，已有工程时重建节点。初始空根保持 Model 可直接使用。
- MainWindow 的 Data/Case 根 Provider 改在 initProviders 中设置；同步现有测试调用方与架构声明。拒绝空指针、已绑定 Provider 和结构修改期间的替换，不转移无效请求的所有权。
- 本次未执行编译或测试。

## Tree 合并至 TreeModel（2026-10-16）

- 按用户确认删除 IDOSTree 类及源码/CMake/公共头文件条目。Model 构造接收 ItemProviderRegistry，直接拥有根 Item、关联 Project/Registry/Access 并同步工程；MainWindow 移除独立 Data/Case Tree 字段，仅保存两个 Model。
- Model 的 data/setData 直接委派 ItemProvider，setChecked 是 CheckStateRole 写入的便捷入口。普通写入和外部属性变化使用 Qt 原生 dataChanged；删除 Tree 专属的数据/结构/勾选状态转发信号。Item/Provider 改为 model()，View 使用 treeModel()，Access 绑定 Model，路径工具同步迁入 Model。
- Model 直接配对执行 Qt 行通知；Item 的结构提交由 Model 校验和授权，不使用友元、不开放裸字段。工程切换及完整 refresh 使用模型重置，重置内不嵌套行通知。Provider 继续按 key 复用节点。
- 勾选由 Provider::setData 校验并发 Model::itemChecked，AppTreeAccess 固定目标视图后执行 Provider::setChecked。修复既有 Access 槽和组递归中遗漏的实际调用。渲染状态/视图切换及行增删刷新实际勾选状态，不读取工程可见性覆盖视图。
- 更新所有菜单、插件和现有测试调用方，修正无 Access 时勾选写入应拒绝的已有断言；未新增或运行测试。架构记录 13 个完整声明，固定源码目录与编码约定同步。最终 D:\CMakeCoder\IDOS-Build 的 Debug 全目标构建返回 0，主程序、全部插件、公共头文件检查和现有测试目标编译通过；check_includes 检查 465 个源文件通过。旧 Tree 类型/文件及友元扫描无残留，include 与 git diff --check 检查通过。日志：D:\CMakeCoder\IDOS-Build\tree-model-merge-build-verified.log。未运行功能测试。


## 禁用类友元并收口访问接口（2026-10-16）

- 编码规范增加：禁止类友元和普通友元函数，仅必要的运算符友元函数例外。src/tests 已清理全部 friend class 声明，不修改第三方代码。
- ItemProvider 使用校验所属关系的一次性 setItem、挂接前一次性 setMetadataId 和只读 metadataId；Tree/Registry 不再直接访问其私有字段。setData 只写入、Tree 成功后统一通知，删除跨类访问的 m_settingData 标志。refresh 的属性通知保持。
- Tree 通过 Item 的完整插入/移除接口管理结构；Item 封装私有父子关系和根节点重入保护，并保证已挂接节点的 Tree 前后通知成对。Provider 清理离线子项也调用 removeChild，不再访问 Item 子项容器。根节点首次绑定使用受校验 setTree。
- MenuProvider 的 view 查询公开，删除 View 友元。Object 增加仅离线对象可用的 restoreObjectId，工程加载不再访问 protected setter；井几何结果改为构造时确定值；导入 Metadata 的通用 schema 工具移入 CommandMetadata protected 接口，不再借用命令私有函数。
- 架构完整声明同步。最终 Debug 全目标构建返回 0，主程序、全部插件、公共头文件检查及现有测试目标编译通过。src/tests 扫描无 friend 声明残留，git diff --check 通过。日志：D:\CMakeCoder\IDOS-Build\no-friend-build-verified.log。未新增或运行功能测试。


## Tree 写入与数据通知统一（2026-10-16）

- 按用户确认，删除 Tree::changed，增加真正的 Tree::setData(item, value, role)，将 Tree::itemChanged / Model::onItemChanged 更名为 dataChanged / onDataChanged。Model 的 setData 统一委派 Tree。
- Tree 校验节点归属后处理写入：CheckStateRole 校验有效二态并进入已有 setCheckItem 请求链，不提前通知实际状态；其他 role 委派 ItemProvider::setData，失败不发成功通知，成功后发 dataChanged。名称写入同时通知 DisplayRole/EditRole/ToolTipRole。
- 通过 Provider 内部写入标志和作用域恢复抑制这次 setData 引起的重复通知；refresh 等直接属性变更仍由 Provider 的 protected changed 发 Tree::dataChanged，保留既有自动刷新路径，不新增公共 notifyDataChanged。完整声明和调用链同步更新。
- D:\CMakeCoder\IDOS-Build 的 Debug 全目标构建返回 0，主程序、全部插件、公共头文件检查和现有测试目标编译通过；check_includes 检查 467 个源文件通过，旧接口残留检查及 git diff --check 通过。日志：D:\CMakeCoder\IDOS-Build\tree-setdata-build.log。未新增或运行功能测试。


## Tree 冗余友元清理（2026-10-16）

- 核对 Item 和 ItemProvider 对 Tree 的全部访问：insertItem/removeItem/project/access/changed 均为公共接口，不访问 Tree 私有状态。移除 IDOSTree 内的两条冗余 friend 声明，架构声明同步。其他类中用于结构提交、Item 绑定及 Metadata 来源记录的友元未改动。
- Debug 的 idos_core 目标编译通过，退出码 0；未新增或运行测试。日志：D:\CMakeCoder\IDOS-Build\tree-friend-cleanup-build.log。


## 菜单 Provider 绑定视图接口收口（2026-10-16）

- 基类及 Data/Case 的 createMimeData/canCut/canPaste/paste 移除重复 view 参数，内部统一使用构造时绑定的 view()；TreeView 调用同步。
- 删除 Case 公共 createContextMenu(QMenu*, view) 重载及 Data 的 m_caseMenus、构造参数和借用调用。Data 自行创建 MIME、校验和执行传输，并拥有添加到工况/重命名动作；Case 只服务自身视图。沿用原 MIME 协议、动作 ID、工程绑定及共用业务命令，保持跨树复制/粘贴和拖放路径。私有菜单追加不再暴露为工厂。
- 新迁入 Data 上下文的 9 条中文翻译和完整类声明同步。D:\CMakeCoder\IDOS-Build 的 Debug 全目标构建返回 0，主程序、插件、公共头文件检查及现有测试目标编译通过；随后 translations 目标编译通过（1501 条完整翻译）。翻译 XML、旧接口残留与 git diff --check 检查通过。日志：D:\CMakeCoder\IDOS-Build\tree-menu-bound-view-build.log。未新增或运行功能测试。


## 数据解析 Provider 接口命名统一（2026-10-16）

- IDOSDataProviderRegistry 的 add/remove/create/providers(filePath) 分别统一为 registerMetadata/unregisterMetadata/createProvider/metadataForFile(filePath)。Metadata 及具体解析 Metadata 工厂统一 createProvider，应用注册、导入命令、菜单与现有测试调用方同步，不保留旧名称别名。
- 本轮仅统一命名；单例、参数、返回类型、所有权和文件匹配逻辑保持原实现。接口示例和架构说明同步更新。Debug 的 idos 目标及其依赖构建通过，退出码 0；git diff --check 通过。日志：D:\CMakeCoder\IDOS-Build\data-provider-naming-build.log。未新增或运行测试。


## 菜单 Provider 统一 QObject 基类（2026-10-16）

- IDOSTreeMenuProvider 继承 QObject 并声明 Q_OBJECT，构造时以所属 TreeView 为 parent。Data/Case 菜单 Provider 删除重复 QObject 继承与初始化；Data 菜单构造移除冗余 parent 参数，主窗口调用同步。
- TreeView 使用 QPointer 观察菜单 Provider，替换时删除旧实例，析构时由 QObject 父子机制释放，外部提前销毁也不会留下悬空成员指针。架构中的完整类声明和所有权说明同步更新。
- 在 D:\CMakeCoder\IDOS-Build 完成 Debug 全目标构建，退出码 0；主程序、插件、公共头文件检查和现有测试目标均编译通过。check_includes 检查 467 个源文件通过，git diff --check 通过。日志：D:\CMakeCoder\IDOS-Build\tree-menu-qobject-build.log。未新增或运行功能测试。

## 树接口收口与 Access 落地（2026-10-16）

- 按用户本轮确认，节点扩展统一为 IDOSTreeItemProvider / IDOSTreeItemProviderMetadata / IDOSTreeItemProviderRegistry，同目录头文件/实现、主窗口、Interface、插件、CMake 和现有测试调用方同步，不保留旧类型别名。
- 工厂统一 createProvider(data)，Registry 按注册顺序匹配 supports；Item(provider) 直接接管所有权并私有绑定反向指针，删除临时 Provider 替换及 setProvider/setRootProvider 路径。Tree 构造可接收最终根 Provider。源数据非拥有 QPointer，Provider 记录实际 Metadata 来源，注销按该身份清理。
- 新增 core/tree/IDOSTreeAccess 和 app/IDOSAppTreeAccess；后者由所属 Tree 管理 QObject 生命周期，连接 itemChecked、渲染实际状态及当前视图变化。一次请求固定目标视图并批量更新。Model 不访问 Server，RenderServer 不再持有或遍历 Tree/Item。
- checkState 查询当前实际状态，setChecked(Qt::CheckState) 执行请求；删除勾选状态缓存、setCheckState 和工程 isVisible 回写树的路径。组节点默认遍历和三态汇总由 ItemProvider 提供，具体网格节点保留属性/过滤器语义。Access 提供 QObject* targetView 供 app 领域 Provider 使用，core/gui 不新增 render 依赖。
- MenuProvider 构造绑定唯一 TreeView，主菜单工厂仅接收 selectedItems/contextItem；View 接管菜单 Provider 并在替换/销毁时释放。View 接口统一 contextItem/createContextMenu，并提供 setTreeModel/treeModel；保留拖放与剪贴板功能。
- Qt roles 读写/flags 委派 ItemProvider，changed 携带 roles；Model 的 CheckStateRole 只接受有效 Checked/Unchecked。井、网格、工况和 PointSet 以 key 复用节点，仅插入缺失项、移除失效项；全树显式 refresh/工程切换仍重新装配。
- 工程绑定与数据更新不提前创建隐藏 RenderObject；首次显示才创建，隐藏保留复用，保存状态恢复按条目补建。
- 架构文档记录本轮 14 个完整类声明和调用链，固定源码目录、AGENTS.md 与公共头文件独立编译清单同步。静态核对无旧通用类型/工厂/树遍历接口残留，改名后 include 存在，git diff --check 通过。最终在 D:\CMakeCoder\IDOS-Build 执行 Debug 全目标构建返回 0，主程序、全部插件、公共头文件检查和现有测试目标编译通过。日志：D:\CMakeCoder\IDOS-Build\tree-access-build-verified.log。未新增或运行功能测试，不将编译通过视为交互验收完成。


## 构建目录更正与编译收尾（2026-10-16）

- 按用户最新确认，唯一构建目录改为 D:\CMakeCoder\IDOS-Build，AGENTS.md 同步更新。现有缓存对应当前源码与 Visual Studio 2022，已找到其内置 CMake，之前关于环境无法编译的判断已失效。
- 使用现有配置执行 Debug 全量目标构建；修复 GridTreeProvider 和示例 PointSetTreeProvider 使用 IDOSTree 完整类型却缺少直接 include 的错误，两处均补齐 tree/idostree.h。
- 最终 cmake --build D:\CMakeCoder\IDOS-Build --config Debug --parallel 6 返回 0。主程序 idosd.exe、核心/GUI/渲染/业务库、各插件、公共头文件独立编译目标及全部现有测试可执行文件均构建成功；check_includes 检查 463 个源文件通过。构建日志位于 D:\CMakeCoder\IDOS-Build\tree-framework-build.log。未运行功能测试，编译成功不代表交互及生命周期已完成运行验收。

## 树 Registry 接口统一（2026-10-16，续作）

- 通用 TreeMetadata/TreeRegistry 更名为 IDOSTreeProviderMetadata/IDOSTreeProviderRegistry，同目录头文件和实现文件、CMake、公共头文件清单、Interface、MainWindow 与插件引用同步替换，不保留旧类型别名。
- Metadata 使用 id/createProvider；Registry 使用 registerMetadata/unregisterMetadata/createProvider 及 metadataRegistered/metadataUnregistering/metadataUnregistered。Tree 与工况引用节点均经 Registry 工厂创建 Provider。
- 保留注销前同步清理节点、再销毁 Metadata 的顺序，并禁止注销中的同一 Metadata 创建新 Provider。工厂校验目标 Item 和 supports，创建结果仍由节点持有。既有测试调用方同步新接口，未新增或运行测试。
- 静态检查：源码及现有测试无旧通用类型/文件名引用，改名后的树 include 和 CMake 路径均存在，git diff --check 通过。本轮不表示全部目标框架完成；编译仍受当前环境缺少 CMake 和 E: 构建盘限制。

## 节点属性与根 Provider 迁移（2026-10-16，续作）

- TreeItem 仅保留父子结构和 Provider 所有权；身份、名称、图标、数据关联、勾选及自定义 role 移入 Provider。每个节点拥有独立 Provider，不再借用父节点实例。Model 的 data/setData/flags 委派 Provider，勾选请求仍通过 Tree 发给 RenderServer。
- 新增 Data/Case 根 Provider 并由 MainWindow 装配，分类生成从窗口移入 Data 根 Provider；移除 Registry 分类模板和 clone 链。统一节点 refresh、insertItem/removeItem、itemFromIndex/indexFromItem 及结构通知命名，调用方同步迁移。
- 根分类中文翻译、CMake、头文件清单和固定源码目录同步更新。以下旧记录中的分类模板复制和属性归属待迁移描述已由本节替代。Registry 统一命名等后续迁移尚未完成。
- 未运行测试；当前环境无可用 CMake 且 E: 构建盘不可见，未执行编译，仍需在规定构建目录完成编译验收。

## 树菜单旧链清理（2026-10-16，续作）

- 已将 Grid、井数据和井显示菜单及其操作迁入具体 IDOSDataTreeMenuProvider，保留原有动作 ID、命令、窗口及目标工程/视图绑定。Data MenuProvider 负责 Data 树菜单组合；节点已有菜单仍调用节点 Provider::createContextMenu。Case MenuProvider 直接继承 QObject 和 IDOSTreeMenuProvider，负责工况菜单，并通过同名重载提供跨树引用操作。两棵树分别绑定自己的 MenuProvider。
- 删除 IDOSGridTreeGuiProvider、IDOSWellTreeGuiProvider、IDOSWellDisplayTreeGuiProvider 及通用 IDOSTreeGuiProvider；TreeRegistry 已删除 addGuiProvider/removeGuiProvider/guiProviders 和 GUI Provider 成员。Case MenuProvider 改由 MainWindow 以 QObject 所有权持有，Data MenuProvider 弱引用它以共用既有引用 MIME 与命令实现。core 不再持有 GUI 菜单实例。
- 修复此前回退遗留的 WellTreeProvider 构造签名、失效勾选函数及中文翻译 XML；按当前四个实际菜单类重新生成中文上下文，全部菜单源文本有翻译。CMake、固定目录清单和现有测试调用方同步移除已删除接口。
- 补齐分类模板复制时的 Provider 独立实例：Tree 复制分类时调用 Provider::clone，导入数据及流体/岩石分类覆盖该工厂，避免实际树节点丢失导入菜单。分类勾选使用 Scene 批量更新，固定发起时的目标视图，保留一次场景变更通知。
- 静态核对：Data MenuProvider 的 48 个完整成员定义与头文件声明匹配，无重复定义或游离代码块；项目内部 include 均可解析；CMake 显式源文件/公共头文件引用无缺失；中文翻译 XML 可解析；源码及现有测试调用方无旧 TreeGuiProvider/Registry 菜单接口引用。
- 本阶段完成菜单旧注册链清理；Item/Provider 属性归属、每个内部节点独立 Provider、根 Provider 与目标 Registry 命名仍待后续迁移，不能将本记录解释为全部树目标接口已经完成。当前环境缺少可用 CMake 和 E: 构建盘，未执行编译或运行回归。

## 树框架讨论补充（2026-10-16）

- 树新框架及调用逻辑已补入 [IDOS架构设计.md](IDOS架构设计.md#2026-10-16-树框架与调用逻辑讨论定稿)。本次讨论结论优先于下方较早记录：新框架不设 `IDOSTreeServer`；`IDOSTree` 自己监听 Project 并维护节点，Tree 与 RenderServer 通过树勾选请求及实际状态变化信号协同，Model 不访问 Server。
- `IDOSTreeView` 持有视图级 `IDOSTreeMenuProvider`；其菜单工厂函数命名为 `createContextMenu(view, selectedItems, contextItem)`。节点 Provider 可有同名无参 `createContextMenu()`。节点 Provider 负责自身节点语义和刷新，组节点由具体 Provider 展开/汇总；Data Tree 与 Case Tree 使用各自的 `IDOSDataTreeRootProvider` / `IDOSCaseTreeRootProvider` 和 `IDOSDataTreeMenuProvider` / `IDOSCaseTreeMenuProvider`。
- 实施进度：已将原 `IDOSTreeServer` 的 Project/Registry 监听、树填充与更新逻辑移入各自的 `IDOSTree`，MainWindow 改为分别绑定两棵树；树路径保存/恢复工具已转到 `IDOSTree`，移除 TreeServer 源文件及 CMake 引用。勾选请求由 Model 调用 `IDOSTree::setCheckItem()` 并通过 `itemChecked(item, state)` 发出；RenderServer 已注册两棵 Tree，针对当前视图处理通用对象显隐及组节点递归，并监听 Scene 对象增删改回写节点勾选状态。网格树 Provider 已承接网格、属性和过滤器的显隐及状态查询；TreeView 不再执行勾选，也不再从共享 GUI Provider 查询勾选状态。旧 `IDOSTreeGuiProvider` 的勾选接口及对应业务实现已移除；删除了只负责组节点勾选的 `IDOSDataGroupTreeGuiProvider`，组状态现在由 RenderServer 聚合。新增 `IDOSTreeMenuProvider` 接口，并让 `IDOSTreeView` 接受视图级菜单 Provider、以选择集合和右键项创建菜单，同时提供 `contextMenuAboutToShow(QMenu*)` 插件扩展信号。Tree Metadata 的 Provider 工厂现接收目标 Item 和数据对象，数据节点及工况引用节点各自绑定 Provider；节点 `createContextMenu()` 已接入 View。示例 PointSet 及 FluidRock 表节点菜单已迁至节点 Provider。Case 树使用 `IDOSCaseTreeMenuProvider` 创建空白、节点和多选菜单，并处理该树的 MIME、剪切、粘贴和拖放；Data 树使用 `IDOSDataTreeMenuProvider` 创建领域和多选菜单，并借用 Case MenuProvider 的既有跨树传输实现。View 调用各树 MenuProvider，TreeRegistry 不再保存或分发 GUI Provider。井、网格及井显示菜单已迁入 Data MenuProvider，旧注册接口和 GUI Provider 已删除。
- 验证限制：`git diff --check` 通过；未运行测试。当前执行环境找不到 `cmake` 命令，且规定的构建盘符 `E:` 不可见，暂时无法从此环境编译。新接口尚未全部实现，文档不代表当前源码已具备全部目标签名。
- 本轮继续迁移：流体表及分类菜单、流体目录导入进入 `IDOSFluidRockTableTreeProvider`；导入数据分类菜单及地震体 Inline 操作进入 `IDOSImportedDataTreeProvider`，对应分类组显式绑定 Provider；多选数据删除进入 `IDOSDataTreeMenuProvider`。已移除 FluidRock 与 Imported Data GUI Provider 实现、MainWindow 注册及 CMake 项。该阶段剩余的井、网格、井显示和 Case 菜单旧注册链已在上方续作中移除。

## 框架设计讨论归档（2026-10-16，待实施）

- 已将本次确认的树、渲染、工具、命令命名及接口写入 [架构设计定稿记录](IDOS架构设计.md#2026-10-16-框架设计定稿记录待实施)，并在文首注明优先关系。旧章节和本文件此前审查记录保留为迁移基线；与新设计冲突的提议（每场景一个 RenderServer、Metadata 追加菜单、静态函数工厂替代命令 Metadata 等）不再作为目标。
- 已确认：每节点独立 Provider；基本属性由 Provider 提供，Item 管结构；根 Provider 只绑定所属 Item；Tree 的 insertItem/removeItem、itemRemoving 等命名；刷新统一 refresh；移除独立 TreeGuiProvider，节点 Provider 创建菜单、View 发 contextMenuAboutToShow 供插件追加；一个 RenderServer(registry) 绑定工程并管理多个视图；勾选当前视图，首次显示创建，隐藏后复用；工具每视图每工具独立 Provider。
- 三组能力统一 Provider/ProviderMetadata/ProviderRegistry；方法 registerMetadata、unregisterMetadata、metadata、createProvider；信号 metadataRegistered、metadataUnregistering、metadataUnregistered。命令保留 Metadata，工厂 createCommand，一个命令一个业务操作，拆分多模式导入大类。Object 后缀用于业务数据实体，通用渲染对象保留 RenderObject。
- 插件算法窗口统一 QWidget 非模态和完整功能名，通过 IDOSInterface 取得平台能力，避免重复构造参数；算法不依赖 Interface/界面，命令管理校验、提交和历史。保留工程/视图目标绑定、窗口失效及 DLL 卸载保护。
- [源码目录](IDOS源码目录.md#2026-10-16-目标目录草案待迁移) 增加实际/目标布局区分；树体系集中 app/tree 仍须与 TreeServer 协调渲染的最终连接方式一起确认，不执行未经最后确认的搬迁。工具构造签名、多选菜单、批量回滚、请求/实际状态通知和属性存储次序继续收口。数据工厂合并与 HDF5 职责迁移仍未决定。
- 后续顺序：先收口上述接口与所有权，再统一 Registry/Interface 装配和命令归属；按批次迁移树、渲染、工具、插件窗口和独立导入命令，接通新链后删除旧链，逐批验证工程切换、多视图、插件注销、菜单、保存恢复及撤销重做。
- 本轮仅更新文档，未改 C++、CMake 或目录，未执行构建和功能测试。完成文档内容及 diff 空白检查，不作为架构实施验收。


## 当前框架对照审查与改造清单（2026-10-16）

### 审查边界与结论

本轮按用户要求对照 `IDOS架构设计.md` 核对当前源码，只整理框架，不修改 C++、CMake、目录或运行行为。以下是静态审查结果，不代表全部运行场景均已复测。2026-10-15“架构讨论暂存”仍是待决方案；取消单例、根 Provider 装配、每节点 Provider、存储职责迁移和工厂合并不得当作已经实施或已经定案。

已符合并应保留的骨架：Project 持有业务数据；Tree/TreeItem 管节点及分支 Provider；RenderView/Scene/RenderObject 管每视图独立 Provider；树与渲染分别由 Server 接收工程事件；通用 Model/View/Scene/Server 本轮抽查未发现井、网格或裂缝子类型分支；菜单修改数据主要走命令；动作、查询和显示状态已有区分。`idos_gui` 实际链接 core 与 Qt Widgets，业务窗口由 app 目标显式编译，不能因文件放在 gui 就判定反向依赖。现有日志、应用、插件及领域基础设施继续保留。

### A. 可在现有目录和类内推进的改造

| 编号／顺序 | 当前代码及证据 | 需要改的地方 | 验收要点 |
|---|---|---|---|
| A1／先做 | `assistant/idosassistantservice.cpp:15` 构造公共 CommandRegistry；`app/idosmainwindow.cpp:522,1919` 经 AssistantWidget/Service 注册并取得命令表 | 公共命令注册表改由应用装配层明确持有，助手只借用。给现有 Service 绑定接口，不新增服务包装类。所有菜单、插件、工程历史及助手使用同一实例；明确它晚于历史命令和插件注册销毁 | 不启动本地模型时菜单和插件命令照常工作；工程切换保留注册；助手调用相同命令；销毁时无悬空 Registry |
| A2／先做 | `app/providers/case/idossimulationcasetreeprovider.cpp:32,46` 的 dependsOn 总为真并 clearChildren；`app/providers/grid/idosgridtreeprovider.cpp:40,89` 对任意非空 ID 变化重建子树；`core/idostreeserver.cpp:194` 遍历分支并更新 | 收窄业务 Provider 的依赖判断；先复用未变化的节点和 Provider，再用 Tree 受控接口增删改真实变化项。依赖删除前后的关系都要覆盖，不能只看删除后的 Project。Server 继续统一同步，不由根 Provider 再监听一套工程增删改 | 改一口井不重建无关工况网格；改名保持展开/选中及节点 ID；删除共享属性后所有引用正确清理；撤销恢复正确。大工程测量改前/改后耗时，当前不宣称已有性能回归实测 |
| A3／随后 | `render/idosrenderserver.cpp:187` 的新对象创建路径设为不可见后，仍立即 bindDataReferences、update；具体 Provider 在 update 中准备几何 | 在现有 Provider/View/Server 内明确轻量实例与昂贵几何的初始化时机；隐藏对象保留依赖及脏状态，首次实际显示再生成几何。不要增加包装 Provider 或让 Server 判断业务类型 | 打开多数据、多视图工程时测量耗时/内存；首次勾选正确显示；隐藏期间更新后显示的是新数据；显示状态恢复、拾取及图例正确 |
| A4／随后 | `core/idoscommandmanager.cpp:60,150,174` 手工置 m_running，随后执行可扩展的命令；`core/idoscommand.cpp:94` 无异常转换边界 | 明确插件/业务命令异常契约；至少保证运行标志和未入历史的命令在异常路径得到清理。异常转换不能伪称通用回滚，已发生的业务修改仍由具体命令恢复 | 注入失败/异常后能继续执行命令；原重做记录不丢；撤销失败不移动历史；部分提交不残留。此项是异常路径风险，未复现当前正常操作失败 |
| A5／随后 | `core/providers/idosdataproviderregistry.h:39` 是无通知的全局格式表；Tree/Render/DataRegistry 已有注册变更信号；`core/idoscommandregistry.cpp:10` 使用 unique_ptr 按值接收，而其他表使用成功才接管的裸指针 | 明确各注册接口失败时的所有权，并使文档与签名一致，不机械统一成裸指针。补格式注销与存活解析器的生命周期约束；审查平台注册调用失败后的释放/回滚，拒绝重复能力时给出可诊断结果 | 重复注册无泄漏；插件部分注册失败能撤回自己已成功注册的项；注销不销毁仍被解析操作依赖的 DLL；现有类型恢复不依赖格式是否启用 |
| A6／随后 | `app/idospluginregistry.cpp:352-426` 常规卸载检查 canUnload；关闭程序时跳过该检查并有意保留库映射。裂缝/多尺度插件已有命令存活计数和窗口清理 | 明确常规卸载与进程退出两套顺序，以及退出时保留 DLL 的归属和理由；平台类型统一从所属 Metadata 工厂创建，排查其他插件生成对象的 MSVC 局部虚表、静态字符串和存活菜单回调。保留现有保护，不能直接删除保留映射逻辑 | 历史、外部持有命令、模态框、任务存活时拒绝常规卸载；清理后可卸载；平台结果在卸载后仍能保存、展示及销毁；退出顺序覆盖插件命令尚存的工程 |
| A7／低优先级 | `app/idosmainwindow.cpp:1616,1644` 解释测井节点 key、识别 Well 并管理专属窗口；`:2102,2221` 汇集通用和测井展示状态 | 将节点激活的业务判断逐步归还现有井 GuiProvider/业务窗口；MainWindow 保留 Dock 装配、工程/视图协调及通用状态收集。先定现有接口的调用边界，不新增菜单管理器、Context 或通用业务窗口包装层 | 测井激活、浮动窗口、重复打开、工程切换和 UI 保存重开行为不变。该代码在 app 层是合法依赖，此项是职责细化，不是模块越界 |
| A8／贯穿实施 | 架构第 10 节概括“失败不转移所有权”，与 unique_ptr 命令注册接口不完全一致；接口表未列出当前 RenderProvider 的 dependencyIds/setDataReference；文档混有已实现与昨晚待决方向 | 按项更新“目标／当前实现／待决”及真实接口，不把暂存讨论直接改成已完成；更新头文件内过时用法，例如格式 Provider 仍示例 read(path, project)，实际解析不接管工程 | 文档与头文件相符；固定目录不变；移除旧接口时一并清理引用、构建和测试，不保留两套调用链 |

### B. 昨晚讨论过、必须先定案的架构调整

| 待决事项 | 当前真实实现 | 建议定案方式 |
|---|---|---|
| Registry 统一归属、取消单例 | DataProviderRegistry 和 PluginRegistry 是单例；DataRegistry 通常由 IDOSApplication 拥有，QApplication 测试环境由 MainWindow 新建；Tree/Render/ToolRegistry 由 MainWindow 拥有；CommandRegistry 由 AssistantService 拥有 | 建议公共能力统一由 MainWindow 创建持有、IDOSInterface 借用；明确多窗口是否隔离注册。实施时改完所有调用方及测试，再删除 instance 路径。A1 可先解决助手持有公共命令的问题 |
| 树目录和根 Provider | Tree/Model 在 core，TreeView/GuiProvider 在 gui，具体 Provider 在 app；MainWindow::initProviders 从 216 行开始创建固定分类，再经 Registry.addGroup 装配 | 保持当前固定目录，直到确认整套树迁移方案。若采用根 Provider，应先确定根节点所有权、setRootProvider 装配和 Server 同步边界；当前没有设计确认的具体根 Provider 类，不擅自新建 |
| Provider 是否每节点独占、是否合并 GUI | 当前对象分支根和独立引用分支拥有 Provider；内部属性叶子可以没有 Provider；多个 GuiProvider 共同贡献菜单，TreeView 检查勾选负责人冲突 | 先决定内部节点是否确实需要独立实例。即使合并也保留插件追加菜单能力、明确勾选唯一负责人；不能为了对称新增尚未批准的 TreeGuiMetadata |
| 数据工厂和格式工厂是否合并 | DataMetadata/DataRegistry 用于类型创建和无源文件恢复；DataProviderMetadata/Registry 用于格式匹配和解析，多格式可以产生同类型 | 建议当前保留两者：类型与格式分开。若合并必须先解决同类型唯一工厂、插件禁用格式后仍能恢复工程的问题，不能直接删除 DataRegistry |
| HDF5 编码职责 | `core/idosproject.cpp:475,641` 组织对象读写并调用 DataMetadata；`core/providers/idosdatametadata.cpp` 承载通用压缩编码和 HDF5 读写 | 若确认瘦身 Metadata，把通用编解码迁入现有 Project，领域字段仍由对象 projectData/restoreProjectData 提供；不新增 ProjectStorage。须保留 v1/v2 兼容、紧凑数组、中文路径及原子提交能力 |

### 建议实施批次与保留项

1. 先明确 B 中 Registry 归属，再做 A1；同时补齐注册失败语义和销毁顺序的文档，验证独立助手、插件加载和工程切换。
2. 做 A2 的依赖与增量更新，再做 A3 的延迟几何；分别建立大工程基线，不把性能修改与目录搬迁混在一起。
3. 做 A4/A5/A6 的异常、注册注销和插件生命周期验证；保留普通卸载保护及退出保留映射，直到替代方案通过销毁测试。
4. 最后按确认方案收拢 MainWindow 的具体业务；树目录、根 Provider、工厂和存储迁移只有定案后再实施，不新增方案外类。

保留已经完成的 DataObject/TreeItem/RenderObject 独立 ID、每视图 Scene 和显示状态、Server 工程同步、GuiProvider 组合菜单、Action/Query 命令区分、工程存储与显示状态持久化。普通模型的角色透传、Qt 对象转换及 app 中的具体业务转换不作为违规类型判断。物理 gui 目录中由 app 编译的业务窗口不误判为通用 gui 依赖 app。

本轮验证：只读核对上述调用链；依赖检查 `tools/check_includes.ps1` 通过（473 个源文件）。未修改运行代码，不运行构建或功能测试；先前测试记录不能作为本轮重构验收。下一步建议先定 Registry 统一归属，再开始 A1。


## 裂缝导入入口统一（2026-10-16）

- 按用户确认，移除裂缝建模插件中禁用的“裂缝导入”占位按钮及对应翻译。天然裂缝导入统一使用数据树右键入口；数据 Provider、导入命令和格式支持保持原有实现。
- 唯一构建目录 `D:\CMakeCoder\IDOS-Build` 的 Debug 裂缝建模插件编译通过。本轮仅清理界面入口，不新增测试。


## DFN 生成入口去重（2026-10-16）

- 按用户确认，移除多尺度裂缝插件中禁用的“DFN 生成”占位按钮及其空的“网络生成”面板；DFN 生成功能入口保留在裂缝建模插件。生成的天然裂缝集仍为工程数据，可供后续分析使用。
- 本轮仅调整菜单入口并清理对应的无用翻译；唯一构建目录 `D:\CMakeCoder\IDOS-Build` 的 Debug 多尺度裂缝插件编译通过。未新增测试。


## 多尺度裂缝插件：三场差异与生产曲线拟合评价（2026-10-16）

- 本轮按用户确认先完成这两项功能，各自使用独立 Widget；界面、算法及命令归属 `src/plugins/idosmultiscalefractureplugin`，不增加插件。三场差异是属性场的前后比较，不包含耦合求解；曲线拟合评价计算误差指标，不包含参数自动历史拟合。
- `analysis.field.difference` 对同一网格的两个属性或动态报告步计算“后 - 前”，仅计算活动单元，非活动单元存 NaN；结果作为普通网格属性加入工程，支持保存、撤销及缓存重做。动态报告步读取不改变当前显示步。输入量纲/单位必须一致，未知元数据需要手工确认，不执行单位换算。
- `analysis.curve.evaluate` 为只读查询，不进入历史。支持工程汇总曲线和 UTF-8 CSV；时间统一为 UTC 毫秒，在重叠范围内插值到观测时刻，不外推或按序号匹配。提供 RMSE、MAE、MAPE、R²、配对数及范围外观测点数；MAPE 排除零观测，常数观测的 R² 未定义。界面最多显示 1000 点，CSV 导出包含全部评价样本、指标和输入配置。
- 两个窗口随工程切换关闭；命令仍在历史或外部持有时阻止 DLL 卸载，文件对话框打开时同样阻止卸载。中文翻译已完成（109 条有效文本，无未完成翻译）。差异属性通过平台 Metadata 工厂构造，修正 MSVC 局部虚函数表导致卸载插件后关闭工程崩溃的问题。
- 验证：唯一构建目录 `D:\CMakeCoder\IDOS-Build` 的 Debug `idos_case_tests`（含主程序及插件依赖）与 `idos_render_header_checks` 编译通过；`case_workflow` 为 28 通过、0 失败、2 跳过（WSL Flow 实样环境未配置）。覆盖差异动态步/活动单元/保存重载/缓存重做/卸载后销毁，以及曲线指标/插值/时间边界/重复时间/无重叠/单位/CSV 引号与无表头/UTC 汇总时间/界面失效/导出/卸载保护。依赖边界检查通过（473 个源文件），差异和曲线窗口中文截图已检查。未运行 Release 构建或真实生产数据验收。


## OPM 未知及自定义关键字支持（2026-10-15）

- OPM Parser 新增从外置 JSON 批量注册关键字定义的接口；定义名或别名与现有关键字冲突时会明确报错，不覆盖 OPM 内置定义。IDOS 工况 DATA 解析会加载程序目录 `opm/extra-keywords.json` 中的定义。
- IDOS 工况解析启用未知关键字原文 token 保留：JSON 未配置的合法关键字也会进入 Deck，后续可查看、索引或保留内容；JSON 已配置的关键字则按 schema 解析为字段。默认 JSON 的 `keywords` 为空，并附有不启用的 `MYKEY` 示例供复制修改。
- 保留内容以记录及 token 表示，保留引号与星号语法；不会保留原文件的空白、注释或逐字节排版。未配置未知关键字的记录边界仍依赖 `/` 和下一条 OPM 已识别关键字。
- Debug/Release `opmcommon` 已在 `D:\QtCoder\Builder\opm-cmake\build` 编译；对应 OPM 库及两个公共头文件已同步到 IDOS `thirdparty/opm`，原库备份在 `D:\CMakeCoder\IDOS-Build\opm-original-libs-20261015`。唯一构建目录 `D:\CMakeCoder\IDOS-Build` 的 Debug `idos_providers` 和 `idos` 编译通过；外置 JSON 已部署到 `bin/Debug/opm/extra-keywords.json`。未运行测试或实样导入验收。

## 删除工况时清理随工况导入的流体表（2026-10-15）

- 工况导入 `.DATA` 时会把相渗表等解析为工程级流体/岩石表对象并添加到数据树；工况保存其引用。删除工况此前只清理私有网格和网格属性，导致这些自动生成的表留在数据树。
- 导入创建的表现在在表数据中记录来源工况 ID。删除工况时清理由工况导入且没有被其他工况引用的表；共享表、独立导入后关联的表以及井继续保留。兼容旧工程数据时，若表未带来源标记，则使用其源文件路径与工况 DATA 文件匹配识别。撤销删除会恢复表对象；复制工况时会把复制表的来源标记重映射到新工况。
- 删除确认提示及命令说明已更新。尚未编译或运行验收。

## ECLIPSE DATA 缺少 NUMRES 时的导入兼容（2026-10-15）

- `ACTIONX_BOX.DATA` 含 `COORDSYS`，但缺少独立 RUNSPEC `NUMRES`；其 `SPECGRID` 第四项已提供值 `1`。OPM 解析器据此报错，阻断工况导入。
- 工况 DATA Provider 现仅在错误确认为 `COORDSYS` 缺少 `NUMRES` 时，从 `SPECGRID` 记录推断数量，并在源文件同目录创建临时副本，将声明插入 `RUNSPEC` 后交给 OPM 解析；原 DATA 文件不修改。临时副本解析后删除，关键字索引中的主文件路径仍映射回原文件。若必要信息无法可靠推断或重试解析仍失败，则返回错误，不静默忽略其他关键字或错误。相同解析兼容也用于工况 DATA 生成及生成文件校验。
- 尚未编译或使用 `ACTIONX_BOX.DATA` 运行验收。

## 生成 DATA 属性关联修正（2026-10-15）

- 用户确认生成前工况已包含 PORO、PERM、MULTPV、区域属性等数据；核查实际生成文件发现仅有 DIMENS、COORD、ZCORN、ACTNUM。原因是树按 gridId 显示属性，而新建工况导入时没有把属性对象 ID 写入工况引用；DATA 生成器只从工况引用收集属性，且语法校验未检查属性是否写出。
- 工况 DATA Provider 现将解析出的网格属性关联到工况；生成时对无属性引用的旧工况回退收集同一网格的静态属性，并在解析生成文件后检查每个已引用属性关键字均存在。替换源关键字时识别 OPM 对 MULTX/MULTY/MULTZ 使用的内部族名 MULT_XYZ；三个属性使用标准数据关键字构造后，再恢复实际别名，避免重复保留源数组或手工构造导致多余 `/` 行。用户截图对比发现，静态网格属性虽写入文件并通过语法校验，但误放在 PROPS 段导致重新导入时不显示；现改为写入 GRID 段。另修正工况树把用于生成的 `case.gridProperty` 内部引用误显示在“井与开发计划”下的问题；引用保留，但网格属性只由网格节点展示。唯一构建目录 Debug `idos_providers` 和 `idos_app` 构建通过；需重启程序并重新打开工程验收树显示。
- 工况 DATA 生成新增井引用检查：已有源 DATA 中定义的 WELSPECS 井沿用原始计划；若工况额外关联了源文件中不存在的井，生成会指出具体井名并停止，避免成功提示但静默漏井。当前 IDOSWell 虽存有 I/J/K 字段，新建井默认值为 0，实测射孔导入也未映射到网格单元；井头 X/Y 与轨迹不能在缺少水平坐标参考系时安全转换成网格 I/J。后续需提供网格映射、COMPDAT 层位及开发控制录入，再完成新井的计划关键字导出。

## 打开工程菜单精简（2026-10-15）

- 移除 Ribbon 和 File 应用菜单中“打开工程” QAction 附加的最近工程二级下拉；保留 File 应用菜单、启动页最近工程列表和新建/打开/保存入口。
- 通过唯一构建目录 `D:\CMakeCoder\IDOS-Build` 的 Debug `idos_app` 构建；未运行测试或桌面交互验收。

## 渲染交互与包围框（2026-10-15）

- 裁剪平面和按点切片不再要求预先使用选择工具；在三维视图启动工具后，可直接点击可见对象确定操作目标。按点切片的首次点击同时作为路径起点。
- 增加视图级“显示包围框”开关，按旧版外框坐标轴样式显示边界、XYZ 轴标题、刻度数值及远侧网格线；边界按当前视图可见对象计算并留出间距，随场景变化更新，状态保存到视图展示状态。
- 增加视图级 Z 方向比例按钮，支持直接输入 0.01 至 100 倍垂向比例；比例按视图保存，坐标框刻度仍显示未夸张的模型 Z 值。
- 新增中文翻译与工具图标；通过唯一构建目录 `D:\CMakeCoder\IDOS-Build` 的 Debug `idos_app` 构建（包含 `idos_render`）。未运行测试或桌面交互验收。

## 架构讨论暂存（2026-10-15，明天继续）

用户要求先保存本次讨论，明天继续。本节为拟调整方向及待决事项，尚未实施；不代表源码、架构文档或目录已经切换。本轮只记录讨论，不移动源码、不删除类、不改接口。此前 UI 持久化、性能优化和验收记录继续有效。

已讨论并认可的方向：

- 同种业务数据的不同格式各有一套 DataProviderMetadata 与 DataProvider，共享数据类型 ID，例如两个井入口都生成 IDOSWell，类型为 idos.well；各格式的 Provider ID 不同。类型 ID、格式 Provider ID、具体对象 ID 分开管理。
- 同种数据共用 GUI/树和 Render 实现类，不按导入格式重复实现。每个树节点拥有自己的 Provider 实例；每个视图中的 RenderObject 拥有自己的 RenderProvider，非拥有地关联源数据，各视图显示状态独立。
- Registry 不设静态单例。用户最终倾向 MainWindow 统一创建和持有 DataProviderRegistry、TreeRegistry、RenderRegistry、PluginRegistry；IDOSInterface 仅提供必要访问入口，不并存 Application、MainWindow、单例三种归属路径。工程切换保留能力注册，重建工程相关实例。
- PluginMetadata/PluginRegistry 管理插件信息与生命周期；插件加载时注册具体能力，卸载 DLL 前清理相关实例、菜单、动作、回调和注册。
- 用户倾向将树体系及其 Provider 放在 app 层，以便协调 Project、RenderServer。这是对原 core/gui 树分层的拟调整，需整体梳理依赖，不能只移动 TreeItem；具体目录及迁移清单尚未确认，不执行目录变更。
- MainWindow 只装配数据树、工况树的根 Provider 并设置给 Model。根 Provider 初始化根和固定分类，业务节点 Provider 初始化内部子项；Model 保持通用 Qt 模型接口。TreeServer 统一协调工程增删改同步，避免根 Provider 重复维护同步逻辑。
- Provider 创建时绑定所属节点，按需设置 Project、RenderServer 等非拥有引用；能从所属节点取得的关系不重复保存。菜单操作仍记录发起时的实际目标，执行前检查有效性，不能临时改用新的活动视图。
- 装配工程时一次创建已有节点及各视图展示对象对应的轻量 Provider，后续持续复用；数据或视图增删时增量创建和销毁。菜单、对话框、几何、GPU 等昂贵资源按需生成，避免重新拖慢打开速度。

明天必须先定案的问题：

1. 数据对象工厂：可评估合并 IDOSDataMetadata、IDOSDataRegistry 与格式注册体系，但需明确同类型的唯一工厂、冲突检测及注册/注销规则。工程恢复不能依赖原始格式 Provider 是否启用。此前直接建议删除两类过早，目前尚未决定删除。
2. 存储职责：通用 HDF5 编码拟从 DataMetadata 移到现有 IDOSProject；具体字段继续由数据对象 projectData/restoreProjectData 提供。不新增用户已否定的 ProjectStorage 类。
3. 所有权：建议 Model 关联 Tree，Tree 拥有根节点，父节点拥有子节点，每个节点拥有自己的 Provider；Provider 回指节点和服务均非拥有。setRootProvider 是装配入口，不能造成 Model 与根节点重复拥有 Provider，也不能由根 Provider 和根节点互相拥有。
4. 扩展能力：当前多个 TreeGuiProvider 可以为同一节点贡献菜单，例如井的基础操作与显示设置。若改成每节点一个 Provider，或合并 TreeProvider/TreeGuiProvider，仍须保留插件追加交互的能力。独立 TreeGuiMetadata 当前尚不存在，是否增加尚未决定，不为形式对称机械增加类。
5. 收益与成本：当前 populateMenu 已只有 menu、view 两个参数；重构应着眼职责、所有权和生命周期，而非单纯减少类或参数。Provider 预创建成本随数据量和视图数增长，须控制资源初始化时机。

下次从“类型工厂的唯一归属”和“节点/Provider 所有权”开始讨论，再确认接口和完整迁移方案。验收场景包括：同种数据两种格式、无源文件保存重开、两个视图状态隔离、根节点与工程增删改、插件追加菜单及卸载、工程切换清理旧实例。讨论确认后再更新架构设计和源码目录并实施。

## 已完成工作记录

- 通用渲染交互工具补齐（2026-10-15）：在现有每视图工具中加入裁剪平面和按点切片。裁剪平面作用于当前选中的可见对象，退出或取消时恢复；按路径点切出展平剖面并在独立剖面窗口查看。剖面是临时视图结果，不作为工程数据保存。`idos_render` 与 `idos_app` Debug 编译通过；尚未进行真实网格交互验收。
- ECLIPSE 汇总结果接入首版（2026-10-15）：在模拟工况的“汇总结果”节点导入 `.SMSPEC`，由 OPM 读取配套 `.UNSMRY`，数据保存为工程对象并通过 `case.summary.import` 命令支持撤销/重做；结果节点提供汇总曲线查看窗口，汇总曲线随 `.idos` 工程保存和恢复。实现已按当前目录约定放入 `core/data/case`、顶层 `providers/case`、`gui/simulation` 及现有 `app` 装配路径。指定构建目录中 `idos_app`、`idos_providers` Debug 编译通过；尚未完成真实工程运行验收，也不代表支持 `.h5` 汇总文件。
- UI 状态持久化完成（2026-10-15）：.idos 增加可选 presentation（UI 子版本 1），沿用现有 Project、TreeServer/TreeView、RenderServer/View/Provider、MainWindow、ToolBar 和测井窗口，不新增存储类或目录。保存并恢复 2D/3D 视图集合、活动视图与上下文、独立显隐/不透明度、井显示设置、网格模式/边线/着色/属性关联及筛选预览、地震切片、活动/主页相机与联动、背景/坐标轴/图例/光标跟踪、工具栏位置与显隐、测井通道/深度范围、树展开/多选/当前项/滚动。按数据 ID 和稳定树路径关联，不保存临时渲染或树节点 ID；缺失的 UI 条目回退，旧工程无 UI 状态时使用默认视图。切换工程清理旧渲染和测井窗口，恢复阶段不标记修改；显示交互需要保存但不进入业务撤销历史。
- 本机 UI 设置通过 AppConfigLocation/ui.ini 保存窗口几何、主窗口工具条、按视图集合区分的 ADS 停靠布局及最近 10 个工程；最近工程列表显示在启动页。测试使用独立设置路径或禁用本机设置，不改动用户偏好。业务撤销历史、临时对话框、计算进程、后台读写与崩溃恢复仍不在本轮范围。README、使用指南、架构及调用链同步；平台新增 2 项中文翻译，无 unfinished。
- 验证：唯一构建目录 D:/CMakeCoder/IDOS-Build 完成 Debug 全目标及全部公共头文件独立编译；完整 CTest 10/10 通过（81.03 秒）。命令用例新增多视图/浮动测井窗口保存重开、每视图参数、相机联动后解除、显示后仍保持干净、工程切换清理、非法 UI 数据回退、地震切片、本机窗口恢复。可见窗口测试初始缺少正式程序既有的 AA_ShareOpenGLContexts，补齐测试 initMain 后再次通过，OpenGL 错误输出为零，最终 command_registry 复测通过（ui-state-command-final.txt）。
- 警告处理：不屏蔽 C4996/STL4043，不改第三方库。已将本轮完整重编译暴露的业务 QStringList、示例点集及测试 QVector 整体比较改为标准迭代或逐项比较；最后相关目标编译日志无 C4996/STL4043（ui-state-confirm-build.txt、ui-state-context-build.txt）。第三方预编译库缺少 PDB 的既有 LNK4099 调试信息提示仍保留，不宣称全部构建警告清零。

- 工程读写性能优化（2026-10-15）：写入格式升级为版本 2，保留版本 1 读取。复合字段紧凑编码、同构记录按列写原生数组，正常保存以同目录原子替换提交，去掉整文件复制；加载批量恢复，增加读写与显示分段计时。实际桌面 123.idos 的 31 个对象逐字段一致，原文件未改动：763259965 字节降至 7011967 字节，旧版读取 121.335 秒；首次新版保存 2.198 秒、读取 2.406 秒，复测保存 1.229 秒、读取 1.457 秒，显示装配 68 毫秒。结果受缓存及机器负载影响。优化副本位于 D:/CMakeCoder/IDOS-Build/123-optimized.idos。合成 2000 轨迹点及 2000 记录测试：40.77 MB 降至 81.3 KB，保存 2471→47 毫秒、读取 4086→63 毫秒。旧格式读取、全部数据类型往返、覆盖保存、中文路径、非法字段、截断及未来版本、提交失败保留目标均通过；命令测试 22/22，Debug 构建完成，CTest 10/10 通过（60.61 秒）。当前仍同步读写，后台进度与取消尚未实施；旧文件首次打开仍需读取旧布局，另存为即可升级。

更新日期：2026-10-15

本文记录当前工程已落地的主要能力、尚未完成的工作和后续顺序。状态按当前源码核对；“已实现”表示代码中已有对应功能，不代表所有功能都已完成完整的端到端验收。

- 共享业务数据全量接入（2026-10-14，待续）：按用户要求忽略缓存/工程中间文件 DeckCache.ptd、coord.bin、actnum.bin、zcorn.bin。当前已接入范围包括井头、LAS、井轨迹/井斜、井顶、射孔/完井、微地震、压裂泵注工作簿、天然裂缝、流体/岩石表、VOL 生产历史、ECLIPSE DATA/EGRID/INIT/GRDECL/UNRST、ZIMS 地层面/边界及 SEG-Y。已对共享业务数据目录完成一轮扩展名和样例检查。未完成：有效 HDF5 汇总文件；18 张工作表的录井/完井日报 Excel（含岩性、井漏、钻井液、套管、岩屑伽马、元素录井、整米/迟到参数、地质记录与专报等）；对全部非 GRDECL 业务格式逐类确认并完成实际导入/查看验收。ECLIPSE SMSPEC/UNSMRY 首版接入已记录于上方，待编译及真实工程验收。共享 SEG-Y 样例约 650 MB，需评估并完成真实文件导入及显示验收。用户于 2026-10-14 要求本轮先暂停，未完成项下次继续。构建验收当时未执行：用户指定构建目录 `E:\CJW-Archive\IDOS-Build\win-x64-debug-local`，当前环境没有 E: 盘符。

## 最新状态摘要

- 单文件二进制工程首版已接入（2026-10-15）：直接扩展 IDOSProject、DataMetadata 与现有数据和值类型，没有新增 ProjectStorage。单个 HDF5 .idos 文件保存工程信息、全部 16 种平台注册类型的业务数据、ID 及关系；接通打开、保存、另存为、未保存提示和失败保护。加载不依赖原始导入文件，树与渲染由现有 Server 重建。Debug 全量构建及独立头文件检查通过；保存恢复专项通过，完整 CTest 的其余 9 项通过，旧工程对话框标题断言更新后专项复测通过。显示状态、标准 VTKHDF 导出和共享大文件性能验收继续后置，详细范围见文末。

- 对象交互需求纠正（2026-10-14）：撤掉“复制引用／剪切引用／从工况移除”通用菜单及引用拖放，改成数据、工况、网格和属性对象本身的复制粘贴、拖动复制和重命名。工况深复制关联对象及网格、属性、筛选并重映射 ID；网格跨工况移动，属性跨同尺寸网格移动；数据树整井及流体/岩石表新增独立“添加到工况…”菜单，支持多选。此前引用交互记录仅为历史，不代表用户需求完成。本次相关 Debug 目标编译通过，命令 16 项及树交互 22 项全部通过，0 skipped；未重跑全量 CTest 或扩展原生鼠标自动化。详细限制见文末。
- 历史记录：工况引用交互首批（已被本次需求纠正替换，2026-10-14）：整井及流体/岩石表支持同工程工况引用的关联、复制、剪切转移和移除，菜单、Ctrl+C/X/V、普通/Ctrl 拖动复制与 Shift 拖动转移共用可撤销命令；多选、源/目标失效、重复与跨工程拒绝已接入。源数据不复制、不移出工程。修正树/渲染同步覆盖多选，以及工况根节点实际隐藏父节点的识别。Debug 全量构建、214 个头文件独立编译和 CTest 10/10 通过（61.05 秒）；真实 Windows 鼠标复制/Shift 转移及 Ctrl+C/V 已验收。完整数据副本、工况副本、网格/属性/筛选转移和井部件复制仍待实施；工程持久化继续后置。详细范围与日志见文末。

- 树菜单、剪贴板与拖放核对（2026-10-14，设计阶段）：按用户同意的第一步检查当前实现并形成架构第 18 节操作规则表。当前 TreeView 仍是单选，没有复制剪切粘贴和拖放入口；现有表关联命令可复用。纠正此前“复制工况共享网格”的口头建议：现有删除/导入按工况私有网格处理，应复制网格及附属数据，井和表共享引用。井部件沿用整井 dataId，不能仅按对象 ID 复制。发现删除网格会收集筛选而删除工况尚未收集筛选的生命周期差异，实施工况复制/转移前需统一。此轮只完成现状核对、规则清单和实施/验收顺序，没有接入新交互或修改功能代码，不宣称功能已实现。下一步先完成工况引用关联/复制/转移/移除的统一菜单、快捷键和拖放路径，再完成数据与工况副本，之后推进工程持久化。


- 同步用户手工目录调整（2026-10-14）：按当前仓库位置同步 12 个头源文件的路径变化，不重新移动用户文件。工况对话框、新建工程、调试信息、运行信息及 Python 控制台回到 gui 根部；gui 保留 tree、well、grid、simulation 业务目录。IDOSInterface 按用户最终调整位于 gui 根部，使用 GUI_EXPORT，由 idos_gui 编译；消费者引用、CMake、独立头文件清单、依赖检查、翻译位置和目录/架构文档同步。此前“gui 根部仅保留 CMake 和导出头”等记录为历史，以本条和当前源码目录为准。验证结果见文末。


- 业务目录细分（2026-10-14）：用户确认将 app/providers 与 gui 继续按职责组织，并补充要求整理 gui/providers 和 gui 根部。app/providers 分为 well、grid、case、fluidrock、seismic、data、render；gui 分为 tree、well、grid、case、simulation、output、python、plugin、project，TreeGuiProvider 从 gui/providers 并入 gui/tree，公共插件接口归 gui/plugin，新建工程对话框归 gui/project。gui 根部仅保留 CMake 和导出头，app/providers 根部仅有业务子目录。本轮迁位 110 个头源文件，429 个文件全部保留；业务窗口仍编入 idos_app，未改变功能及链接模块归属。本文此前“gui 根部集中界面”“app/providers 平铺”等描述保留为历史，以本条和当前源码目录文档为准。构建与回归结果见文末记录。
- 源码目录迁移（2026-10-14）：按用户确认的桌面 src 布局迁移仓库文件，未用桌面副本覆盖源码；177 个文件迁位，429 个 C++ 头源文件全部保留且头源配对同目录。通用 Provider/Metadata/Registry 集中于 core/providers，日志归 core/logger，具体展示实现归 app/providers，具体命令归 app/command，界面归 gui，插件管理界面归 app/plugin，渲染和工具 Provider/Metadata 归 render/providers。DataProvider 通用接口改由 idos_core 导出，RenderInteractionTool 改由 idos_render 导出。gui 中 20 组业务界面仍由 app/CMakeLists.txt 明确编入 idos_app，不改变既有具体 Provider 绑定；idos_gui 不反向依赖 app 或 render。源码目录全文文件树、架构、开发指南、调用链路、AGENTS、CMake、include 边界检查和翻译位置均已同步。此条替代下方历史记录中的 app/case、app/well、app/grid、gui/tree、core/log 等旧目录描述；功能未完成事项维持原记录。验证结果见文末本轮记录。

- 数值模拟集成平台（2026-10-14）：按用户确认移除数值模拟插件边界。IDOSSimulation 及模拟业务窗口迁入 app/case，井显示迁入 app/well，内置视图工具实现迁入 app 根部并由主窗口直接注册；模拟不再参与插件发现与卸载。原 DLL 导出、构建目标、独立 TS/QRC 及跨插件中转信号删除，翻译与图标合并到平台。Debug 构建、全部迁移头文件独立编译及 CTest 10/10 通过（52.86 秒）。下方“数值模拟必需插件”“工具由插件注册”等为历史方案，以本条和文末记录为准；其他专业插件及通用安全卸载机制保留。

- 收尾验收（2026-10-14）：当前源码 `E:/CJW-Archive/IDOS` 已在唯一目录 `D:/CMakeCoder/IDOS-Build` 完成 Debug 全项目构建及独立头文件检查，最终 CTest 10/10 通过（51.32 秒）。补齐 Petrel 井头固定样例、泵注/射孔 Excel、多井轨迹扩展字段和流体表导入/引用/去重/撤销重做回归；修复依赖检查目录清单、插件对 app 菜单类的直接依赖、SEG-Y CP037 解码及工况测试的文件选择阻塞。此前“E 盘不可用”“尚未构建”的记录保留为历史，以本次验收范围为准。实际 WSL Flow 两个用例因未设置 `IDOS_FLOW_SMOKE_DATA` 跳过；共享大文件及全部业务格式的手动实样验收仍未完成。

- 旧工程导入能力核对及迁移（2026-10-14）：只读检查 `D:/VSCoder/IDOS` 的格式清单、Provider、对象和 `data/data/PVT.txt`、`SCAL.txt` 样例。新工程已覆盖井、ECLIPSE DATA/EGRID/UNRST 的主要路径，仍缺地震体、解释层、断层、面/等值厚度/边界、速度网格和生产历史。新增 ECLIPSE GRDECL/INC 属性 Provider，网格属性菜单支持选择这些文件，导入前校验目标网格单元数并按网格尺寸保存，支持 PORO、PERMX/Y/Z、MULTPV、SIGMAV、MULTX/Y/Z、SATNUM、ROCKNUM、PVTNUM、EQLNUM、FIPNUM。DATA 内的相渗表可从工况树打开；另新增 TXT/DAT/PRN ECLIPSE 表文件导入，支持 DENSITY、PVTO、PVDO、PVDG、PVTG、PVZG、PVTW、SWOF、SGOF、SWFN、SGFN、SOF2、SOF3、SLGOF、SGWFN、ROCK。独立表文件导入创建工程级表数据对象；数据树表节点可通过可撤销命令关联到工况。旧工程调用链未直接搬入。当前实现尚未运行时导入验收。

- ECLIPSE INIT 静态属性导入（2026-10-14）：网格属性入口新增 `.INIT`，使用 OPM `EInit` 读取数值网格数组，并由导入命令依据目标网格 ACTNUM 将活动单元值还原到全网格单元位置；未活动单元填 0，按目标网格尺寸保存。GRDECL/INC 支持逻辑保持不变。导入器跳过非数值及长度不匹配活动单元数的数组；中文筛选已更新。运行验收定位到 Provider 先设 INIT 全网格尺寸会清空仅含活动单元的值；现已移除该重置，并在命令完成 ACTNUM 展开后设置目标尺寸。`idos_app` Debug 构建通过；待重新打开程序导入 INIT 样例验收。

- Petrel 井头导入（2026-10-14）：在现有 Wellheader Provider 内增加 VERSION 1 区段表头与带引号记录解析，保留原七列表格式；从 Petrel 记录提取井名、地面坐标、符号、井 datum 和 TD (MD)，KB/MSL/TVDSS 垂向基准映射到现有井头模型。共享样例含 Zi224Vertical、Zi224H 两口井；新增对应导入测试，待构建和运行验收。水平坐标系名称暂不存入当前仅含垂向 datum 的空间参考模型。

- 非 GRDECL 数据导入第一批：新增微地震事件集、压裂泵注程序、ASCII 天然裂缝集、生产历史及压裂液/支撑剂目录对象和对应 Provider；按领域分入 `core/data/seismic`、`core/data/fracture`、`core/data/production` 及对应 `providers` 目录。数据树“微地震”“水力压裂”“天然裂缝”“生产汇总”“流体”节点增加导入入口，使用工程命令导入并支持撤销/重做。针对共享数据目录样例，支持 Zi224 的事件 TSV（EventDate/EventTime/East/North/Down/LMag）、带列名及单位行的压裂泵注 TXT、ASCII `BEGIN FORMAT` 天然裂缝格式（节点坐标、法向量及头部属性）、`.VOL` 的 `*FIELD/*DAILY` 分井时间序列，以及按中文表头识别的流体/支撑剂 TSV 目录。压裂液归入“流体”，支撑剂归入“水力压裂”。CMake 和源码目录清单已同步，`D:\CMakeCoder\IDOS-Build` 的 `idos_app` Debug 构建通过。尚未在运行应用中逐个导入样例验收。压裂施工/射孔 Excel、SEG-Y、其他微地震变体尚未接入；天然裂缝只支持 ASCII 变体，`.VOL` 只支持共享目录中的字段布局。

- 流体/岩石函数接入（2026-10-14）：将 DATA 中已有 SWOF、SGOF、SWFN、SGFN 表显示在工况树，并从表节点打开指定工况的属性曲线窗口。旧工程 PVT/SCAL 文本样例以 ECLIPSE 关键字、数字记录和斜杠结束表，现按此格式新增 TXT/DAT/PRN 导入。独立文本表进入数据树的流体或岩石函数组；数据树负责导入，工况仅保存表对象引用，并可从数据表节点发起关联；工况树显示已关联表。导入与关联均沿可撤销命令执行。解析关键字范围及固定源码目录已同步，当前调整尚未构建或运行验收。

- 当前视图改由 `IDOSRenderServer` 唯一管理；`IDOSProject` 记录明确的当前工况 ID。树节点右键会先同步当前选择，模拟结果播放按当前工况定位共享网格。网格显示、筛选和井显示面板改为空构造加分步 setter，菜单动作继续固定目标视图与对象 ID。本轮构建和回归结果见文末记录。

- 网格筛选已按用户确认采用独立 `IDOSGridFilterWidget` 和旧版两页操作方式，筛选结果以独立 `IDOSGridFilter` 保存在源网格的“筛选”分组；具体能力与本轮验证见下文。显示设置仍为外观／着色两页，工程文件持久化仍待实施。

- 2026-10-14 晚间讨论确定先整理并实施两棵树的节点菜单，再推进模拟输入生成与接口精简。已完成菜单首批节点收紧，并接入工况重命名、指定工况生成输入、单井数据摘要、井轨迹表格查看及 `.dev` 定向导入／替换；井显示设置由模拟插件现有树 Provider 提供。井曲线分类/叶节点查看已分别支持全部通道和单通道，标志层分类/叶节点查看已分别支持全部层位和单层位；LAS、tops、完井及 checkshot 导入按对应 Provider 文件过滤。静态／区域属性入口明确命名但复用现有 DATA 导入器，未增加格式或关键字过滤。D:/CMakeCoder/IDOS-Build 配置成功，`idos`、`idos_app` 和 `idossimulationplugin` Debug 构建通过；全量构建出现示例点集的 STL4043 警告及部署脚本的 VCINSTALLDIR 警告。井摘要、轨迹、曲线和输入数据菜单专项通过；本轮完整 `case_workflow` 回归停在 Windows 文件选择对话框，已终止，整套回归仍需重跑。模拟主线定位为导入数据、按工况组织、生成 DATA 输入、运行 Flow、关联结果；完整交互式建模可后续增量加入，不作为现阶段模拟闭环的前提。

- 下一阶段已确认转向上方数据树：先补齐井分层、射孔及 Checkshot 的导入和查看，再推进表面、断层、边界等几何数据；SEG-Y 完整解析单独规划。旧工程的分类节点不等同于已完成解析，其 SEG-Y Provider 当前仅登记文件占位。井分层、射孔及 Checkshot 的数据表达、providers/well 解析入口和查看表已实现，验证记录见本轮详细状态。射孔应区分实测深度区间与模拟 COMPDAT 网格层号，不能把文本深度当作网格层号。导入继续沿用命令和快照回滚机制。

- 当前源码位于 D:/CMakeCoder/IDOS；依据最新 AGENTS.md，唯一构建目录为 E:/CJW-Archive/IDOS-Build/win-x64-debug-local。当前环境无法访问 E: 盘时不得改用其他目录构建。
- 工程、树、渲染、命令和插件框架已落地。新建和导入可用，工程打开/保存/另存为/设置仍禁用。
- Flow 已接入 WSL2，支持实时输出、进程组停止、独立结果目录、UNRST 自动导入和报告步播放；真实算例验收目前以 SPE1 为主，未提供离线环境包。
- 本地助手已实现模型进程与命令调用，普通文本对话使用“你：”“助手：”同行格式。Python 控制台仅有界面，运行时未接入；analysis 仍是接口目标。
- 通用工具框架已实现，不使用友元、静态回调或 SLOT；Qt 过滤器处理输入，VTK 成员函数指针处理状态。视图工具由数值模拟插件注册，空工具条自动隐藏。
- 视图工具由数值模拟插件注册：悬浮工具条提供适配全部对象、三维标准视角、坐标轴、视图操作和网格透视等功能，每个窗口操作自己的视图；程序退出时先注销视图工具，用户不能禁用或卸载该必需模块。工具条拖动及双窗口行为仍待运行验收。目前不新增主页“编辑”面板，待网格和井的编辑流程明确后再评估。
- 视图工具销毁排查：移除及重排 Action 后同步销毁普通按钮，避免 DLL 卸载后仍保留 SVG 图标及插件字符串引用，保留 QWidgetAction 框架所有权处理。用户随后确认此前析构崩溃已消除；后续主页迁移又修改了清理路径，需要重新验收退出。
- 最近完整相关范围回归为工具框架阶段 5/5（64.45 秒）；随后友元和 VTK 调整运行各自专项，最后选择比较调整运行树回归。没有将这些局部测试合并宣称为本日全量重新验收。
- 历史 case_workflow 卸载阶段发生过超时；本日工具框架范围内该用例通过（38.37 秒）。没有完成历史偶发超时根因证明，保留观察，不将其描述为当前每次必现阻塞。

操作见 [使用指南](使用指南.md)，开发与验证见 [开发指南](开发指南.md)。下文按时间保留变更和实验记录，发生方案调整时以本摘要、架构文档及后续记录为准。

## 两棵树的目标右键菜单（2026-10-14，目标与实现对照）

以下是右键菜单目标与当前实现的对照。树节点与右键菜单分开表述；节点勾选负责目标视图的显示／隐藏，不另设同义菜单。数据树空白处保持无菜单，工况树空白处保留“新建工况…”和“导入工况…”。没有对应业务能力的地震、断层等数据分类，以及工况下运行设置、流体与岩石属性、初始状态等分类，暂不添加占位菜单。

| 树节点 | 目标右键菜单 | 与当前实现的差异 |
| --- | --- | --- |
| 数据树／井分类 | 新建井…；导入井数据… | 已实现 |
| 单口井 | 显示设置…；导入井数据…；查看井数据…；删除井… | 已实现，摘要表显示轨迹、曲线、完井、标志层和校深概况 |
| 井轨迹 | 显示设置…；查看轨迹数据…；导入／替换轨迹… | 已实现；导入按 `.dev` 过滤并替换轨迹 |
| 井曲线分类及曲线叶节点 | 分类：导入井曲线…、查看曲线数据…；叶节点：查看曲线数据… | 已实现；分类查看全部曲线，叶节点查看对应曲线 |
| 完井、标志层、校深分类及叶节点 | 分类：按类型导入与查看数据；叶节点：查看对应数据 | 已实现；专属导入按 LAS、tops、完井和 checkshot 数据类型过滤 |
| 工况节点 | 生成模拟输入…；重命名工况…；删除工况… | 已实现；生成及重命名均绑定右键目标工况 |
| 工况／网格分类 | 导入网格… | 已实现 |
| 数据树／流体分类 | 导入流体表… | 已实现；只导入流体关键字表 |
| 数据树／岩石物理函数分类 | 导入岩石物理函数… | 已实现；只导入岩石物理关键字表 |
| 数据树／流体或岩石表节点 | 添加到模拟工况… | 通过命令保存工况 ID 引用 |
| 网格节点 | 按属性着色（单色、可用属性）；显示设置…；删除网格… | 数据树与工况树均可从网格菜单删除；删除会清理网格属性、筛选，并从引用该网格的工况移除 |
| 工况／静态属性、区域属性、动态属性分类 | 分别导入对应类型的属性 | 入口已明确；静态与区域属性继续复用通用 DATA 导入器，不作格式或关键字过滤 |
| 属性叶节点 | 在目标视图显示；显示设置…；删除属性… | 已实现 |
| 网格／筛选分类 | 新建筛选… | 已实现；设置条件并保存后在本分类下生成结果节点 |
| 已保存的筛选结果节点 | 编辑筛选…；显示设置…；重命名筛选…；删除筛选… | 已实现；分别作用于所右键结果，勾选启用、取消勾选停用 |

例如“网格 A → 筛选 → 高孔隙度区域／第一层网格”中，后两个名称只是用户保存的独立筛选结果，不是固定菜单项；两者各自具有上述结果菜单。通用复制／粘贴、复制工况和筛选结果“创建副本”尚未确定数据及引用语义，本阶段不列入目标菜单。静态和区域属性入口目前仍调用通用 DATA 导入器。截图复核后按用户对导入归属的澄清，工况树网格和属性菜单保留删除入口：删除属性会从所有引用工况中移除；删除网格会一并删除其属性与筛选，并从引用工况移除。确认提示已明确影响范围。`onGridSettings` 覆盖工况网格和属性菜单，专项通过；`onWellInputMenus` 井摘要、轨迹、曲线及标志层专项通过。完整 `case_workflow` 本次在 Windows 文件选择对话框处停滞，已终止该测试进程；未将整套工况回归记为通过。

## 晚间讨论：模拟输入与接口边界（2026-10-14，设计记录）

- 平台当前可以导入网格、井和多类属性，并已有工况引用、Flow 运行和结果查看基础。目标是先完成“已导入数据 → 工况组织与校验 → 生成 DATA 输入包 → Flow 运行 → 结果关联工况”的闭环；不以从零创建网格或完整井轨迹建模为前置条件。建模能力后期可以沿用相同的 Project 数据、命令、树及渲染链逐项加入。
- 工况节点新增“生成模拟输入…”右键项，主页现有“生成 DATA”入口后续应复用同一生成流程。打开 Widget 时固定发起操作的工况 ID，展示本次引用的数据和必要生成设置；用户选择临时输入或保存到指定目录，点击“生成”后先校验，再生成 DATA 及所需配套输入文件。临时输入也需实际落盘供 Flow 读取；运行目录中的文件保留或清理须与路径有效性一致。正式导出的文件长期保留。运行操作可自动使用临时生成流程，避免要求用户每次先手动导出。
- 只有生成成功后才在对应工况记录实际生成的输入路径，运行前检查该文件及配套文件仍存在。原始导入路径 `sourceFile` 与最近生成／运行的输入路径应分开，不能互相覆盖；临时路径失效后不得作为永久工程引用。工程保存重开的持久化能力已确认必要，但目前暂不实施，后续需连同正式生成路径及数据引用一起保存。
- 当前 `IDOSSimulationDataGenerationWidget` 读取原始 DATA 文本供编辑，并打包已索引的输入文件；`IDOSSimulationInitializeWidget` 仍以工况 `sourceFile` 为输入。它们尚未从 Project 中的网格、属性和井自动组装 DATA，也尚未将新生成路径接入模拟初始化。本节描述的是目标流程，不把现有“生成 DATA”按钮误记为已完成该能力。
- 参数引用的基础已有：`IDOSCaseItemRef` 以对象 ID 记录工况引用，网格属性以 `gridId` 关联网格。生成器需沿引用解析对象并校验缺失、维度、单位和关键字映射；井控、时间及其他模拟参数是否已有完整数据表达仍需逐项盘点。可先以原始 DATA 为模板，替换／写入已能可靠表达的部分，再扩展到完全由 Project 数据生成；不能仅因对象可导入就宣称任意组合都可运行。
- 接口讨论暂列为待梳理问题，不在本轮改造：`IDOSProject` 是打开工程期间的数据统一持有者；Metadata 的 `create()` 只接收创建 Provider 所必需的固定依赖，Provider 方法再接收本次操作所需参数。TreeGuiProvider 负责菜单与调用入口，工程数据修改走命令，局部显示走目标视图／RenderProvider。Project 当前选择、当前工况、全局可见性、命令历史和单对象／批量信号的最终归属尚未决定；现有 `currentCaseChanged` 有模拟插件订阅，不能孤立删除。先完成菜单，再按完整调用链审查冗余接口，避免同时保留两套路径。
- 以上仅为讨论结论和后续工作边界；本次没有修改模拟、菜单或架构代码，没有新增编译与运行验收。

## DATA 内存生成范围盘点（2026-10-15）

- 用户确认目标是将工况关联的工程内存数据转换成对应 DATA，而非继续仅编辑原始 DATA 并复制其引用文件。
- 当前工况对象以稳定 ID 引用网格、网格属性、井和流体／岩石表。网格内存保存 COORD、ZCORN、ACTNUM 与维度；网格属性保存关键字、值数组及维度；井保存位置、完井段及部分井控值；流体／岩石表保存表格数据。ECLIPSE 导入器另提取部分相渗表及 WELSPECS、WCONPROD、WCONINJE 记录。
- 当前导入器未保存完整 Deck 或全部关键字记录，只保存摘要、关键字来源位置索引和部分结构化数据；工况缺少可重建完整 RUNSPEC、初始状态及输出设置的结构。因此现在直接输出完整 DATA 会丢失未建模的模拟设置。
- 生成器实施前需定义关键字覆盖规则与预检：按工况引用解析对象；验证网格唯一性、引用完整性、属性网格/尺寸/关键字映射，以及井、表格和运行参数所需字段。无法从内存确定的必要设置必须阻止生成或明确要求保留的模板输入，不能静默生成不完整输入。
- 现有 DATA 导出窗口和初始化窗口仍基于原始 sourceFile；生成路径尚未独立保存或接入初始化。本盘点未修改代码，尚未实现生成与运行衔接，也未进行编译或运行验收。

## 本轮详细状态（2026-10-13）

- 修复取消筛选未恢复：临时应用改变主网格 Provider，保存成独立结果后此前未记录关联；取消结果勾选只隐藏结果，主网格仍保留临时筛选。现在保存记录目标视图的预览结果 ID，取消对应结果时清除主网格预览并重建完整活跃单元，保留显隐、着色和外观。新的临时条件清除旧关联，取消其他结果不误清；结果删除／撤销创建通过 Server 的现有 removeDataReference 路径清理对应预览。指定 E 盘目录主程序／插件、工况测试及独立头文件构建通过。onGridFilters、onGridSettings、onRenderLifecycle 专项通过（含初始化／清理 5 passed，0 failed，11119ms，grid-filter-cancel-tests.txt），覆盖取消后恢复完整活跃网格、着色保留、各视图隔离、后来新预览不误清，以及删除结果清理对应预览；树和两项依赖边界 CTest 3/3 通过（grid-filter-cancel-ctest.log，19.00 秒），git diff --check 通过。保留既有 Qt 尺寸／postEvent 警告，本轮未运行完整 case_workflow 或真实 Flow。


- 网格筛选按用户确认改为独立 `IDOSGridFilterWidget`，沿用旧版 `IDOSGridDisplaySettingsPage` 的筛选交互；原先嵌入显示设置的筛选页及其实现、声明和翻译已移除。网格筛选页为 I/J/K 方向列表、范围、间隔选择、并集／交集及显示／隐藏／反选；属性筛选页为属性搜索与勾选、独立条件编辑、七种比较、边界／无效值／容差、可拖动范围条和直方图。顶部切换临时或已保存筛选，保存／另存为通过命令；网格右键和筛选分组进入独立窗口，结果节点具有独立编辑、显示设置、重命名、删除和勾选。数据对象／Metadata 位于 core/data/grid，窗口、命令及结果渲染 Provider／Metadata 位于现有 app/grid。工程文件持久化尚未实现，保存仅进入当前运行工程。


- 网格筛选验证：指定 E 盘目录主程序、插件、工况测试及独立头文件编译通过。`onGridFilterCriteria`、`onGridFilters`、`onGridSettings`、`onRenderLifecycle` 专项通过（含初始化／清理 6 passed，0 failed，10425ms，grid-filter-widget-final-tests.txt）。覆盖旧版方向列表与间隔选择、范围与勾选项并集、三轴并集／交集、显示／隐藏／反选、七种属性比较、边界包含、无效值、容差、禁用条件、鼠标范围拖动、原单元 ID/IJK、目标视图固定、动态报告步、独立结果及外观、缺失属性保留并失效／恢复、创建／更新／重命名／删除及撤销／重做、网格删除级联与恢复。修正新命令清理未执行更新历史时误释放另一删除命令持有对象的所有权问题；更新命令不拥有脱离对象。修正范围手柄释放时补取最终位置，保留实际鼠标输入回归。
- CTest 命令、主窗口、树及两项边界检查 5/5 通过（grid-filter-widget-ctest.log，33.70 秒）；331 个源码依赖边界检查通过，中文翻译 XML 无重复／未完成项，git diff --check 通过。网格／属性两页中文截图已复查；Debug 目录无 Release VTK DLL。保留既有 Qt 尺寸／postEvent 及第三方 OPM 调试信息警告。本次未运行完整 case_workflow、真实 Flow 或百万级网格性能验收；早前被用户中断的组合测试不作为本轮验收依据。

- 网格显示设置统一：按用户确认，以 `IDOSGridSettingWidget` 直接替换原着色面板并放在现有 `app/grid`；网格及属性节点共用“外观/着色”两页，网格入口默认外观，属性入口预选属性并打开着色页。外观接入表面、线框、表面加边线、点显示、显隐、不透明度、边线颜色和像素线宽，着色保留 22 种色带及范围、无效值、图例。边线开关对应已有显示模式；高亮临时覆盖边线外观，取消后恢复，数据/属性更新保留用户设置。菜单职责合并到已有 `IDOSGridTreeGuiProvider`，删除数值模拟插件中的两个旧着色类、四个文件及其注册/注销链，并撤销相应 app 依赖例外。菜单固定保存工程、树、数据 ID 和目标视图弱引用，属性删除回退单色，网格/视图失效及工程切换关闭面板。同步架构、固定目录、使用说明及平台翻译；插件中的旧着色翻译上下文已移除。
- 网格设置验证：E 盘指定目录主程序、配套插件、工况/树/主窗口测试及独立头文件构建通过。`onGridSettings`、`onRenderLifecycle`、井显示与符号两个专项、`onRequiredSimulationPlugin` 通过（含初始化/清理 7 passed，0 failed，10618ms，grid-settings-final-test.log）。覆盖两种节点共享面板、旧菜单固定目标、全部色带、四种显示模式、边线开关联动、透明度、独立边色/线宽、高亮恢复、非法参数拒绝、各视图隔离和删除清理。修正既有必需插件用例从主窗口子对象查找插件自有动作的错误假设，改为逐一核对实际主页分组动作，禁用/卸载/退出验证保留。主窗口、树及两项边界检查 CTest 4/4 通过（grid-settings-ctest.log，27.52 秒）；319 个源码文件边界检查、翻译 XML/完成项及 git diff --check 通过。外观、着色中文截图已复查，Debug 目录无 Release VTK DLL。本次未执行完整 case_workflow 或真实 Flow；既有 Qt 尺寸/postEvent 及第三方 OPM 调试信息警告仍有出现。

- 井绘制第一阶段对齐旧工程：将旧 render_core 的 0～27 井符号几何迁入现有 IDOSWellRenderProvider 的 private 类方法，默认井口使用专用符号，保留原有点/球体/圆锥选项；符号列表使用同一几何生成预览。自动符号优先读取井头文件，无井头时按井类型和开关状态回退，显示覆盖不修改数据。按旧编号接入生产、注入、关闭、停井的井色及射孔色，提供沿用井色开关和部分单色模式；实测关闭射孔使用关闭配色。井口、轨迹与射孔线默认开启管状线，沿用旧版 ambient/diffuse/specular 参数，粗细仍为像素线宽。当前没有独立停井字段，停井通过对应符号表达；二维井组样式和旧版裁剪/平滑差异未在本阶段迁移。
- 井符号验证：E 盘指定目录主程序、插件、工况测试及独立头文件编译通过。五项功能专项通过（onWellSymbolsAndStatusColors、onWellDisplaySettings、onWellInputMenus、onGridColoring、onRenderLifecycle，含初始化/清理 7 passed，0 failed，10320ms，well-symbol-final-test.log），覆盖全部 28 个编号的几何线段数量、符号预览、四类状态及独立射孔色、关闭射孔、自动符号回退、非法编号不提交、源符号不变、管状线启停和多视图隔离。主窗口、树及两项边界检查 CTest 4/4 通过（well-symbol-ctest.log，27.32 秒）；321 个源码边界检查及 git diff --check 通过。符号列表和状态配色中文截图已复查，翻译完成并部署。本次未运行完整 case_workflow 或真实 Flow，既有 Qt 尺寸/postEvent 警告仍有出现。

- 井显示设置已接入：数值模拟插件新增 IDOSWellDisplayWidget 和 IDOSWellDisplayTreeGuiProvider，具体井及井轨迹节点提供“显示设置…”，井轨迹入口预选轨迹页，其他数据分类不增加该入口。每视图 IDOSWellRenderProvider 保存井口点/球体/圆锥、颜色和大小，轨迹颜色、像素线宽、平滑及 MD 裁剪，实测射孔颜色与线宽，井名标签、整体显隐和不透明度。MD 裁剪对边界插值，不修改源数据；实测射孔沿已有轨迹绘制，COMPDAT 网格层号不解释为实测深度。面板及菜单固定绑定发起时的工程、井和视图；井/渲染对象删除、工程切换、视图关闭及插件退出清理绑定。更新架构、固定目录、使用指南及中文翻译，边界检查仅对两个井显示实现文件开放具体 app Provider 调用。
- 井显示验证：指定 E 盘目录的主程序、配套插件、工况/树/主窗口测试及独立头文件构建通过。onWellDisplaySettings、onWellInputMenus、onGridColoring、onRenderLifecycle 专项通过（含初始化/清理 6 passed，0 failed，10117ms，well-display-final-test.log），覆盖旧菜单目标、多视图隔离、颜色提交、裁剪边界/非法范围、井口几何、平滑、实测射孔、标签、显隐、透明度、源数据不变、渲染对象移除及工程切换。最终 CTest 主窗口、树和两项边界检查 4/4 通过（well-display-final-ctest.log，27.02 秒），321 个源码文件边界检查及中文翻译完成检查通过；中文面板截图 well-display-settings.png 已复查。两处旧测试的“最后一项是删除井”假设改为按动作名称验证。专项保留 Qt 窗口尺寸及 postEvent 警告，无功能断言失败；本次未运行完整 case_workflow 或真实 Flow。Debug 产物目录复查无 Release VTK DLL。

- 显示配置入口统一命名：按用户确认，网格及网格属性的“着色设置…”改为“显示设置…”，面板标题同步改为“显示设置”，英文源文为 Display Settings。平台及插件中文翻译、使用指南同步更新；“着色依据”和“在目标视图中显示”保持现有操作含义。后续井显示面板入口沿用该名称，当前尚未接入井显示面板。翻译 XML 和 git diff --check 通过，本次仅调整文案，未构建或运行测试。

- 构建后部署配置修正：VTK 不再将同一 bin 目录中的 Debug/Release DLL 全部复制到目标目录，按当前配置选择配对版本；windeployqt 显式使用 --debug/--release，VC DebugCRT 仅在 Debug 部署，并处理未找到运行库目录的空列表。已在 E:/CJW-Archive/IDOS-Build/win-x64-debug-local 重新配置及生成，通过四种配置各 164 条 VTK 复制命令检查，并核对 Qt 参数及 CRT 脚本；清理 Debug 产物目录中此前误复制的 164 个 Release VTK DLL。git diff --check 通过，本次未重新编译或运行程序。

- 井导入文件筛选修正：井分组及具体井的导入窗口仅汇总 `idos.well.` Provider 的格式，移除 ECLIPSE 网格与模拟工况选项；“所有支持的文件”同步限定为井格式，并去掉重复扩展名。Registry 接口提供通用 ID 前缀筛选，不判断具体业务类型。本次未新增或运行测试。D 盘构建先因页面文件不足失败，改为串行后 Registry 源码编译通过，但链接因 `idos_providersd.dll` 被占用（LNK1168）受阻，尚未生成完整新程序。

- 井输入数据第一阶段已接入：`providers/well` 新增井分层、射孔及 Checkshot 的 Provider/Metadata，应用注册后复用 `wellData.import`，按井名合并并支持单井目标、撤销/重做。井分层使用 Measured Depth，射孔 FIELD/FT 换算到米并保留日期文本，未映射层号为 0。Checkshot 新增 core 值类型，保留原始符号和声明的单位/深度/时间类型，按深度排序、成对保存时间，重复相同点合并，冲突及非单调曲线拒绝导入。现有 Well 的合并和导入快照已覆盖射孔及校深；井树分类提供只读数据表。
- 井输入验证：指定 D 盘目录的全项目构建、独立头文件及 include 检查通过。旧工程真实样例验证 14 口井/98 条层位（简表及 Petrel 格式）、5 口井射孔、15 口井校深；新增用例覆盖单位换算、带空格的名称、无效记录整文件失败、多井合并、与既有 COMPDAT 共存及移除源文件后的重做。相关 CTest 七项首次执行六项通过，工具条回归旧断言因新增拖动把手失败；已仅调整测试以区分框架 Action 和业务 Action，tree_incremental 复跑通过（5.06 秒）。井菜单、着色及渲染生命周期专项通过（含初始化/清理 5 passed，0 failed，2697ms）；没有宣称完整 case_workflow/Flow 回归完成。
- 本阶段界限：上方树“井”分组及具体井的导入入口已支持这三类文件；全局观测等分组仍是后续工作。分层当前保存井名/层位/MD，其他 Petrel 字段及可选类型列暂未进入模型；射孔尚不处理其他 EV 事件、枪参数及网格映射；Checkshot 仅提供数据查看，不自动推断单位、基准或执行时深转换。

- 着色目标名称修正：主窗口将主视图及新建视图的 Dock 标题同步到对应 RenderView 的 windowTitle；面板显示该实际标题并监听标题变化，不再显示 main3d 等内部 ID。此前顶部两列信息布局也已完成重建。主程序、插件和测试编译通过，`onGridColoring` 专项通过（含初始化/清理 3 passed，0 failed，2278ms），覆盖信息列对齐、实际标题显示及更名同步。

- 顶部信息对齐修正：按用户澄清将网格及目标视图信息拆为 FormLayout 两行，标签与值分列，两个值使用相同左边界；撤回此前误改的下拉框固定高度。中文翻译及对应标签几何断言已更新。源码编译到链接阶段，运行中的程序仍占用数值模拟 DLL（LNK1168），此次布局修改尚未完成链接和运行验证。

- 着色面板对齐调整：属性及色带下拉框统一为 26px 高度，保留 FormLayout 共用的左右边界，并补充几何对齐断言。构建在链接数值模拟 DLL 时遇到 LNK1168（运行中的程序占用文件），本次改动尚未完成链接和运行验证，需关闭程序后重建。

- 色带扩展：从旧工程 `D:/VSCoder/IDOS/src/app/widgets/idospropertyeditorwidget.cpp` 迁入全部 19 种色带的 RGB 控制点，保留初版蓝到红、蓝到黄、灰度三个标识，共 22 项。颜色定义及 256 点插值统一位于现有 `IDOSGridRenderProvider` 类中，插件 Widget 通过同一接口生成下拉列表预览；默认采用旧工程 Default 色带，反转同时更新预览和实际着色。新增名称均已完成中文翻译；本阶段不改变 Widget 构造与目标绑定方式。
- 色带验证：主程序、插件、测试及独立头文件编译通过，include 边界检查通过。旧工程 18 种命名色带的 RGB 控制点逐项对照一致，Default 另按旧工程八个控制点迁入。`onGridColoring` 和 `onRenderLifecycle` 专项通过（含初始化/清理 4 passed，0 failed，2586ms），覆盖全部 22 项实际 LookupTable 采样与预览端点一致、反转后的颜色及预览、多视图隔离与原有生命周期检查。下拉列表截图为 `D:/CMakeCoder/IDOS-Build/win-x64-debug-local/tests/coloring-ramps.png`。

- 网格着色已实现并通过专项验证：数值模拟插件新增 `IDOSColoringWidget`、`IDOSColoringTreeGuiProvider`，网格右键提供“着色依据”和“着色设置…”，属性右键提供“在目标视图中显示”和“着色设置…”。面板共用，绑定打开入口时的目标视图，每视图的 `IDOSGridRenderProvider` 保存单色、三种色带、反转、自动/手动范围、无效值颜色和图例开关。自动范围仅统计当前报告步的有限活动单元值，缺失/非有限值改为 NaN 着色；手动非法范围不提交。属性删除解除属性绑定，网格或视图失效关闭面板，工程切换及插件清理销毁面板。
- 着色验证：主程序、数值模拟插件、工况测试及独立头文件编译通过；`check_includes` 和包含四个新增边界用例的检查器回归通过（2/2，10.44 秒），`project_dialog` 通过（7.80 秒），工况专项 `onGridColoring`、`onRenderLifecycle` 通过（QtTest 含初始化/清理 4 passed，0 failed，2297ms）。专项覆盖右键入口、绑定原目标视图、不同视图状态隔离、手动范围及非法输入、图例开关、NaN、属性/网格删除和退出。截图 `D:/CMakeCoder/IDOS-Build/win-x64-debug-local/tests/coloring-settings.png` 已查看，中文面板布局正常；未宣称完整 case_workflow 或全量回归通过。
- 着色接入采用已确认的具体 app Provider 调用路径；include 检查器仅允许两个插件着色 `.cpp` 引用 `app/grid/idosgridrenderprovider.h`，插件公共头及其他插件继续禁止该依赖。暂未实现离散分类色带、单位补全、跨报告步统一自动范围或更多格式解析。

- 着色方案确认阶段记录：`IDOSColoringWidget` 与 `IDOSColoringTreeGuiProvider` 位于数值模拟插件根目录，通过现有 TreeRegistry 扩展网格/属性右键菜单，两种入口共用面板。原网格 GuiProvider 保留勾选职责，每视图网格 RenderProvider 保存着色状态；需扩充色带、范围及图例设置接口，并处理绑定失效和插件清理。已同步架构及固定源码目录，该阶段仅记录设计；后续实现及验证见本节上方最新记录。

- 退出崩溃修复确认：用户确认保留退出期间的插件 DLL 映射后正常退出。最终退出路径为停止任务、清理插件 UI 和业务实例，Registry 持有的动态库保持映射至进程结束；普通插件卸载继续显式卸载 DLL。该反馈验证了本次退出问题已解决，并支持提前卸载 DLL 导致后续 Qt 清理访问失效资源的判断；尚未定位具体残留资源，不宣称已完成全部退出场景回归。

- 退出崩溃再排查：16:34:04 最新日志显示定向清理 Ribbon 延迟删除后仍在主窗口析构函数体完成后崩溃，先前两项局部清理不能证明根因已解决。退出路径现改为销毁插件业务实例并保留 Registry 所有的 QLibrary 映射至进程结束，避免仍存活的 Qt 控件、图标缓存及共享字符串在主窗口基类/QApplication 清理时访问已卸载 DLL。普通插件卸载仍显式卸载 DLL。源码格式检查通过，未编译或运行验证，需复测；不将推断记录为已确认根因。

- 退出崩溃后续：用户确认补充播放控件清理后仍崩溃，16:30:48 日志仍结束于主窗口析构函数体完成。此前将播放控件遗漏判作根因证据不足。核对 SARibbon 上游实现后发现 `ActionRemoved` 已从布局移除 item，而普通按钮由 `takeAt()` 调用 `deleteLater()`；事件循环退出后这些按钮仍可能在插件 DLL 卸载后持有图标及字符串。主页 Action 清理现通过 Panel/Layout 公共接口取得对应 widget，在移除 Action 后定向处理该 widget 的 `DeferredDelete`，使按钮资源在 DLL 卸载前释放；不运行普通事件循环。修改格式检查通过，尚未编译及退出复测。

- 退出崩溃排查：用户日志显示全部插件卸载及 `IDOSMainWindow` 析构函数体完成，之后在 `SARibbonMainWindow` 基类析构期间崩溃。数值模拟插件把 `IDOSSimulationResultPlaybackWidget` 作为 Ribbon 播放控件加入 Results Panel，但卸载时仅清除工况上下文，没有确保该插件自有 QWidget 在 DLL 卸载前销毁；现已在主页 Action 清理后显式销毁仍存活的播放控件，避免 Ribbon Panel 在基类析构阶段调用已卸载插件中的析构代码。此前手动删除 Ribbon 工具按钮的代码也已移除。日志中的 `disconnect(nullptr, ...)` 警告通过空指针检查消除。修改尚未编译或运行验证，需复测退出。

以下保留主页和工具迁移阶段记录，当时未完成编译及界面验收；后续着色功能的编译和专项验证以上方最新记录为准，专项通过不等同于全部迁移功能验收。

### 1. 已确定的业务与界面分工

- 数值模拟是默认业务模块，继续由插件实现，不把工况、求解器与结果业务写入主页类。
- 专业插件计算模拟所需的参数、属性和模型数据；部分诊断、拟合能力服务于结果分析。插件输出到模拟输入的完整转换、单位校验和数据追溯还没有全部实现。
- 工况新建、导入和工况相关对象管理保留在工况树，主页不新增重复的新建/导入工况按钮。
- 主页固定顺序为：工程、窗口、模拟、结果、输出、扩展。
- 数值模拟不再创建独立 Ribbon 标签页；主页不再提供重复的视图分组，也不新增编辑页。其他专业插件保留各自业务页面。
- 视图操作使用所在视图的悬浮工具条；主页模拟动作沿用当前工况解析逻辑。运行任务、结果播放与当前目标的完整多窗口行为仍需验收。

### 2. 主页功能迁移明细

| 主页分组 | 当前注册内容 | 实现与限制 |
|---|---|---|
| 工程 | 新建工程、打开、保存、另存为、工程设置 | 沿用原平台实现；打开/保存/另存为/设置仍禁用，迁移没有实现工程持久化 |
| 窗口 | 新建三维窗口、二维窗口、窗口布局 | 保留原平台实现，并排列在模拟分组之前 |
| 模拟 | Deck、解析摘要、运行预检、初始化、运行状态、停止任务、工作流 | 来自数值模拟插件；运行状态打开配置/运行窗口，不能描述为新增了独立的一键启动按钮 |
| 结果 | 工况表格、属性曲线、井控表、结果文件、报告步播放、曲线对比、拟合评价 | 查看与分析合并；属性曲线当前主要查看已有相渗曲线；拟合评价仍禁用 |
| 输出 | 平台截图/图片导出、DATA 导出、运行归档、工况清单报告 | 归档与报告迁入输出，不新增报告种类；DATA 生成继续编辑来源输入文本 |
| 扩展 | AI 助手、插件管理、Python 控制台 | 保留原平台入口；Python 运行时仍未接入 |

- 新增 `IDOSInterface::homeRibbonPanel(groupId)`，应用实现按稳定 ID 查询平台已有的 `simulation`、`results`、`output` Panel。
- 平台拥有主页及共享 Panel。插件只添加自己的动作和控件，不创建或删除这些共享分组。
- 数值模拟插件通过较少的大按钮及多行小按钮排列原功能，播放控件继续作为大控件嵌入；窄窗口下的实际宽度、溢出和排布尚未检查。

### 3. 必需插件机制

| 项目 | 当前代码行为 | 待验收重点 |
|---|---|---|
| 元数据 | `IDOSPluginMetadata` 增加 `required`，独立于原 `UI` 功能类型 | 加载和目录信息中的字段一致 |
| 声明 | 可选 `required()` 导出；缺少导出默认 false；数值模拟返回 true | 新版模块被识别为必需模块 |
| 启动 | 先加载发现的必需插件，再恢复可选插件；忽略必需插件的历史禁用设置 | 曾禁用数值模拟后仍自动加载 |
| 管理器 | 必需插件显示身份及说明，取消可编辑勾选 | 鼠标、键盘操作均不能禁用 |
| 注册器 | 拒绝用户禁用、单独卸载、普通批量卸载 | 绕过界面直接调用也不能卸载 |
| 加载失败 | 主窗口提示数值模拟模块不可用 | DLL 缺失、依赖缺失及初始化失败的提示 |
| 退出 | 明确的 shutdown 路径允许资源清理和 DLL 卸载 | 退出清理不会被用户卸载限制挡住 |

该机制限制用户在运行中的应用里卸载模块；它没有新增安装器、模块修复或自动更新功能。

### 4. 所有权、退出顺序与接口边界

- 插件创建的 QAction 改由插件 QObject 持有。结果播放对应的 QWidgetAction 同样转为插件所有。
- 退出时先清理业务窗口和任务，再从共享 Panel 移除插件动作，同步释放对应 Ribbon 按钮及播放控件；最后卸载 DLL。平台的主页及分组保留到平台自身析构。
- 视图工具注销时同步销毁按钮、Provider、动作、菜单和交互工具；普通按钮不依赖 `deleteLater()` 才释放图标。拖动把手继续遵循 QWidgetAction 所有权规则。
- 数值模拟菜单由工具栏 QWidget 持有，Provider 析构先解除 QAction 菜单关联，再释放菜单。
- `IDOSRenderProvider` 增加显示模式虚接口，网格 Provider 覆盖现有实现，工具插件通过公共接口读取和恢复模式，清理了对 app 内部网格 Provider 头文件的直接引用。
- 用户已经确认此前 IDOSToolBar 析构崩溃修复有效。新的主页动作/播放控件清理，以及带运行任务退出，尚未重新运行确认。

### 5. 悬浮工具条与交互最终状态

| 功能 | 最终代码状态 | 已知边界/验证状态 |
|---|---|---|
| 标准视角 | 顶、底、前、后、左、右、等轴测 | 已移除重复的透视菜单项；投影按钮单独控制正交/透视 |
| 主视图菜单 | 已移除保存/返回自定义主视图按钮及菜单 | 通用相机接口未在本轮批量删除 |
| 视图复位、图例 | 从主页迁入工具条 | 新入口的状态同步及二维视图行为待运行检查 |
| 隐藏/显示全部 | 有可见对象时隐藏全部，全部隐藏后显示全部 | 树勾选按当前视图刷新，属性着色保留；显示全部会显示该视图已有的全部对象，并非仅恢复先前可见子集 |
| 树选择与高亮 | 树选中只更新工况/对象上下文 | 已移除普通画布默认拾取路径 |
| 选择工具 | 仅启用后拾取对象，停用时清除高亮；空白点击清除 | 选择会同步当前树对象；取消后不应继续选择 |
| 鼠标跟踪 | 无按键移动时拾取表面坐标，坐标气泡跟随鼠标 | 约束在视图范围内；离开模型或视图隐藏，空白处不显示虚构坐标 |
| 点探针 | 显示对象、渲染单元/点编号、坐标、已有单元/点属性，标记拾取单元 | 单元失败时补充点拾取；编号来自渲染数据，不承诺等同 I/J/K 或原始模拟编号 |
| 探针轮廓 | 按对象变换转世界坐标后作前景叠加轮廓 | 已移除单元内部额外黄色点；纯点数据自身的标记保留 |
| 探针信息 | 点击附近持续显示，失败显示未拾取提示 | 每类数值属性最多展示 12 项，只显示当前渲染数据已有数组，不自动查询所有油藏属性 |
| 距离测量 | 三维坐标计算距离，2D 叠加细线、小十字端点、两位小数，无中间刻度 | 单次两点，第三次有效拾取开始下一次测量；取消/停用清除，未迁入老工程 Delete 撤销点功能 |
| 点缩放 | 表面拾取后居中放大，一次完成后退出 | 尚未重新运行验收 |
| 网格透视、视图联动 | 沿用目标视图操作及恢复逻辑 | 显示模式接口变更后需回归多窗口和卸载/退出 |
| 背景、坐标轴、截图、缩放 | 保留工具条现有功能 | 控件可用性、二维支持、重复入口与窄窗口排布待验收 |

### 6. 本轮涉及的主要文件

| 文件/目录 | 主要职责与修改 |
|---|---|
| `src/gui/idosinterface.h`、`src/app/idosappinterface.*` | 主页既有分组的插件访问接口 |
| `src/app/idosmainwindow.*` | 六组主页布局，删除原视图入口，必需模块失败提示及退出入口 |
| `src/app/idospluginmetadata.*`、`src/plugins/idosplugin.h` | required 字段与导出契约 |
| `src/app/idospluginregistry.*` | 必需模块优先加载、禁用/卸载保护、退出清理、元数据字符串生命周期 |
| `src/app/idospluginlistmodel.cpp`、`src/app/idospluginmanagerwidget.cpp` | 必需身份展示、锁定勾选与说明 |
| `src/plugins/idossimulationplugin/idossimulationplugin.*` | 主页注册、动作所有权、播放控件及退出清理 |
| `src/plugins/idossimulationplugin/idosrendertoolprovider.*` | 工具条动作及状态，补齐复位/图例、移除冗余菜单 |
| `src/plugins/idossimulationplugin/idosrenderinteractiontool.*` | 测距、探针、选择、取消与标注 |
| `src/render/idosrenderview.*`、`src/render/idostoolbar.*` | 拾取坐标、跟踪气泡、去除默认选择、同步销毁工具按钮 |
| `src/render/idosrenderprovider.*`、`src/app/grid/idosgridrenderprovider.h` | 公共显示模式接口及具体覆盖 |
| `src/app/grid/idosgridtreeguiprovider.cpp` | 不可见网格的属性勾选状态修正 |
| `i18n/idos_zh_CN.ts`、插件 CMakeLists | 中文翻译及 VTK 依赖 |
| `tests/case_workflow_test.*` | 原视图装饰用例迁入工具条，新必需插件回归用例 |

### 7. 验证结果与当前阻碍

| 验证项 | 当前结论 |
|---|---|
| `git diff --check` | 本轮检查通过，行尾转换提示不是格式失败 |
| 翻译 TS XML 解析 | 通过；不等同于 lrelease 构建和中文界面运行验收 |
| include 边界检查 | 299 个源码文件通过 |
| 此前退出崩溃 | 用户在同步销毁按钮改动后确认恢复；后续迁移需再测 |
| 新增必需模块回归 | 已编写，未执行 |
| 原视图装饰回归 | 已更新为工具条动作，未执行 |
| 主程序/插件/独立头文件编译 | 本轮未执行 |
| 实际新主页、探针、测距和 Flow | 本轮未完成运行验收 |

- 已找到 Visual Studio 内置 CMake 3.31.6，早先记录的找不到 CMake 只适用于 PATH 查找。
- 当前机器只有 C:、D: 盘，没有指定构建路径所需的 E: 盘。继续遵守唯一构建目录规则，没有改用源码内 build 或其他目录。
- 公共接口及 Provider 虚接口有变动，恢复构建环境后应重新编译主程序与全部配套插件，不能仅替换数值模拟 DLL。

### 8. 接下来应执行的验收

1. 恢复指定构建目录，配置并构建主程序、所有插件、独立头文件检查及工况测试。
2. 运行 `onRequiredSimulationPlugin`：历史禁用仍自动加载，主页功能归属正确，禁用/单独卸载被拒绝，退出完成清理。
3. 运行 `onViewDecorations`，检查方向标和图例的工具条状态与实际视图一致。
4. 人工核对主页六组顺序、没有重复数值模拟页/视图组、结果播放和窄窗口排布。
5. 用网格、井和点数据验收探针拾取、失败提示、轮廓、编号/属性；验证取消后不残留标注。
6. 验收鼠标气泡无按键跟随、双窗口隔离，以及选择工具关闭后不能再拾取对象。
7. 验收测距两次拾取、旋转/缩放后端点和标注、第三次拾取重置、Esc 与停用清理。
8. 检查隐藏/显示全部与两棵树勾选，另一视图不被改变；回归视图联动和网格透视恢复。
9. 分别在无任务、正在运行、播放结果、显示标注时退出，核对任务停止、按钮及 VTK/Qt 资源释放，无 SVG 析构崩溃。
10. 重跑相关历史回归及真实 Flow SPE1 流程，形成新的日志记录后再更新为运行验收通过。

功能推进顺序：先完成迁移验收，再做工程持久化；随后完善专业插件输出到模拟输入的衔接、拟合评价、Python 与更多算例验证。

## 数值模拟插件接管视图工具（2026-10-13）

本节保留早期迁移记录；最终必需模块策略与主页布局以上方“本轮详细状态”为准。

- 将视图工具 Metadata、Provider 和交互工具从 `app` 迁入 `idossimulationplugin`。平台继续创建每视图 ToolBar 和共用 Registry，但不预注册具体工具；无插件注册时工具条自动隐藏。
- 数值模拟插件加载时注册导航工具，为已有和新建视图生成独立实例；卸载时先移除 Metadata，Registry 同步销毁视图内 Provider、Action 和交互工具。
- 曾在主页 RibbonPage 增加网格与井的“编辑”面板，后按用户决定移除。网格和井操作仍从原有入口使用；待编辑流程明确后再考虑主页入口。
- 清理插件操作、主页面板接口、翻译和使用说明，保留工具条插件注册路径。尝试构建 `idossimulationplugin`，但当前环境没有 CMake 命令，指定构建目录也尚未配置；因此本次修改未完成编译验证。

## 阶段记录与既有能力

### 树上下文与视图高亮分离（2026-10-13，阶段记录）

- 树选中和活动视图切换仅更新上下文，不再按树数据 ID 设置渲染选择；各视图保留各自的鼠标选择。
- 初版保留普通画布点击选择，后按用户要求删除。最终行为为仅选择工具开启时拾取和高亮，停用时清除；树选中不触发渲染高亮。
- 交互工具消费左键释放，避免点探针及测距等操作松开鼠标后触发普通选择；选择变化通知工具栏，刷新“缩放至选中对象”等按钮状态。
- 本轮源码改动尚未完成编译和运行验收；其余工具条功能问题等待具体行为描述。

### 平台与视图交互调整记录

以下按修改阶段保留记录，包含后来被替代的三维测量表示、固定位置提示等方案。最终实现及验收状态见“本轮详细状态”。

- 按用户确认移除点探针单元内部额外添加的黄色拾取点，保留单元叠加轮廓和信息气泡。纯点数据自身的探针标记仍保留。本次未编译运行验证。

- 根据点探针截图，信息显示证明已成功拾取，但三维局部线框可能与网格表面重合而不可见。探针标记改为前景二维叠加轮廓及拾取点，轮廓按对象变换转换到世界坐标后投影，避免模型遮挡；新拾取先隐藏旧结果，失败显示未拾取提示。补齐 VTK FiltersCore、FiltersGeneral、CommonTransforms 依赖。格式检查通过，尚未编译运行确认。

- 鼠标跟踪坐标气泡改为跟随光标并约束在视图内，离开模型或视图时隐藏。点探针允许有效点或单元，单元拾取失败时增加可见模型点拾取，不再因缺少单元编号拒绝纯点数据；点拾取显示编号、坐标、已有点属性及黄色点标记。格式和翻译 XML 检查通过，尚未编译运行确认。

- 鼠标跟踪开启无按键 MouseMove，在模型表面移动即显示实际拾取坐标，空白处隐藏。移除平台的默认画布点击选择及延迟点击路径，只有插件选择工具开启时才拾取对象，停用时清除高亮；Esc 退出当前交互工具。
- 参照老工程点探针，新增单元拾取及局部线框标记，在点击附近持续显示对象名称、渲染单元/点编号、世界坐标与渲染数据中现有的单元/点属性；下次拾取、取消或停用清理标记。未接入老工程的全局选择集或额外属性库，编号为当前渲染数据编号。补齐 VTK FiltersExtraction 与 CommonDataModel 依赖。格式、翻译 XML 检查通过，尚未编译运行。

- 测距样式按确认调整为 1.5px 细线、小十字端点和两位小数，中间刻度关闭，标注使用屏幕叠加显示；底部距离结果同样保留两位小数。显式配置端点形状并补充 VTK FiltersSources 链接依赖。本次未编译运行验收。

- 根据测距截图修正模型遮挡测量线与数字的问题：改用 VTK 屏幕叠加距离表示，端点仍以三维世界坐标设置，距离按三维坐标计算；黄色连线和带阴影数字不再参与模型深度遮挡，关闭中间刻度，保留端点。格式检查通过，尚未编译运行验收。

- 测距补充 VTK 三维端点与测量线：拾取第一点后显示起点，第二点后保留黄色连线和距离标注，底部结果持续显示；下一次测量、取消、停用及工具销毁时清除。测量表示由插件交互工具持有，关闭原生交互以复用当前拾取流程；补齐插件的 VTK 链接依赖。尚未编译和界面运行验收。

- 测距交互修正：模型坐标拾取按 VTK 渲染像素尺寸换算鼠标位置，拾取范围限制为当前 Scene 的可见 Actor；测距步骤和结果改为视图底部持续显示，关闭工具或取消时隐藏。距离为两个模型表面拾取点的三维直线距离，单位沿用模型坐标。本次未编译及运行验收。

- 按用户确认移除悬浮工具条的主视图按钮，以及“返回主视图 / 将当前视图设为主视图”菜单；清理对应 Provider 成员、槽、翻译与使用说明。本次格式及翻译 XML 检查通过，未编译运行。

- 标准视角菜单移除重复的“透视”项，仅保留顶、底、前、后、左、右与等轴测；透视/正交投影由独立投影按钮切换。本次修改未进行编译及界面运行验收。

- 视图工具的“清空显示”改为按当前可见状态切换“隐藏全部 / 显示全部”，可通过同一按钮或树勾选恢复显示。当前视图 Scene 更新时刷新两棵树的勾选状态；网格属性节点仅在网格可见且属性被选用时勾选。保留隐藏前的着色属性，其他视图显示状态独立。本次尚未编译和运行验收。

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
| 运行状态 | 已实现 | 查看初始化状态，配置并启动 Flow，显示实时日志和任务状态；新主页入口待验收 |
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

## 内置视图工具接入（2026-10-13）

- 应用注册导航工具 Metadata，并在每个 View 的悬浮工具条装配独立 Provider，提供适配全部对象、三维标准视角菜单和坐标轴开关。
- Provider 直接调用目标 IDOSRenderView 的既有操作；Ribbon 仍调用相同视图接口。视角与坐标轴状态从各自目标 View 更新，标准视角入口在二维视图禁用。工具条先显示坐标轴和标准视角，适配全部对象排在最后。
- 工具条最左侧增加 10px 拖动把手，以双列点阵和移动光标提示；用户拖动后保留自定义位置，画布缩放时将工具条约束在画布范围内。Qt 程序的运行时拖动行为尚未验收。
- 后续根据界面截图移除工具栏自定义“更多工具”按钮和菜单；根据最新截图将图标与边框内边距恢复为舒展尺寸。更新使用指南、架构及中文翻译。本次未运行构建或测试；双窗口交互、窄窗口排布和 Ribbon 同步仍需运行验收。

## 内置视图工具参考 ParaView 扩展（2026-10-13）

- 参照 D:/VSCoder/IDOS/src 旧版工具条，增加主视图保存/恢复、正交投影切换、比例缩放、背景颜色、截图到剪贴板和清空当前视图显示；保留适配全部对象、三维标准视角、坐标轴、缩放至选中对象及顺/逆时针旋转。
- RenderView 根据选中渲染对象的可见 Actor bounds 适配相机；无可见选中几何时不改变相机。选择变化时同步更新按钮可用状态。光标坐标显示在视图左下角；网格 X-ray 保存并恢复各网格原透明度及显示模式。新增图标和中文翻译。
- 视图联动通过共享 VTK 相机并监听相机修改实现，同窗口其他三维视图加入或解除联动。点缩放、点探针和两点距离测量由 Provider 持有的 IDOSTool 实现；点缩放一次拾取后自动退出，测距和点探针显示拾取坐标/距离提示。
- 旧版工具条中的垂向夸张、解释对象显隐及剖切、裁剪仍需适配当前 View / Provider / 命令体系；不能将旧版全局管理器或数据树操作直接接入。当前增项尚未构建或运行验收。

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

唯一构建目录为 D:/CMakeCoder/IDOS-Build；本轮 Debug 配置、主程序/插件及相关测试目标构建通过。井节点菜单按井分组直属对象识别，修复井根节点“删除井”入口；按父节点类别键判断井数据分类与叶节点，恢复分类菜单；网格节点恢复“新建筛选”入口，筛选组菜单仍保留该操作。case_workflow 多次创建主窗口时自动启动本地助手，导致无关子进程启动并让整套回归超时；为该 CTest 设置 IDOS_DISABLE_ASSISTANT_AUTOSTART，并在主窗口启动处尊重此测试环境变量。针对性 CTest（case_workflow、tree_incremental、well_import、well_path）全部通过，4/4，25.23 秒；case_workflow 17.24 秒。构建与部署仍显示现存 VCINSTALLDIR 未设置警告。菜单其余节点/多选及工程持久化按后续计划继续验收。

1. 编译并运行必需插件及主页迁移回归，验收退出清理；随后按“本轮详细状态”的清单验证工具交互、双窗口和结果播放。
2. 先按架构第 18 节补齐节点菜单、多选、复制剪切粘贴和拖放，并验证撤销/重做；随后实施工程持久化与保存重开，当前保存相关入口仍禁用。
3. 扩大真实工况、网格/动态属性和报告步播放验收范围。
4. 按部署需求准备 WSL2、Flow 离线环境，补充安装及运行验证流程。
5. 根据业务需要实现拟合评价、通用分析、Python 运行时及更多任务配置。
6. 持续观察历史插件卸载超时；新增功能后运行必要全量回归并准确更新验收范围。

## 数值模拟迁入主页与必需模块（2026-10-13）

- required 与 UI 功能类型独立：插件元数据保存 required，新增可选 required() 导出；数值模拟导出 true。必需插件启动优先加载，历史禁用设置不生效；注册器拦截禁用、单独卸载及普通批量卸载，插件管理器锁定勾选并显示必需身份。退出与宿主更换走显式 shutdown 清理。应用在数值模拟加载失败时给出提示。
- 主页固定为工程、窗口、模拟、结果、输出、扩展，移除视图分组及独立数值模拟标签页。开放现有主页 Panel 查询接口；业务仍由插件实现。查看/分析进入结果，DATA 导出、归档与报告进入输出，新建/导入工况仍在工况树。
- 插件动作由插件 QObject 持有，播放 QWidgetAction 也转为插件所有；卸载清理先移除共享 Panel 的插件动作并同步销毁对应按钮和控件，保留平台 Panel。工具条补齐视图复位、图例，原主页视图动作与槽已删除。
- 同步架构、使用说明与中文翻译。格式和翻译 XML 检查通过。发现 Visual Studio 内置 CMake 3.31.6，当前机器仅有 C:、D: 盘，指定的 E:/CJW-Archive/IDOS-Build/win-x64-debug-local 无法使用，因此未执行配置、编译或运行测试。早先“无 CMake”的记录仅指 PATH 查找结果，不能代表未安装工具。

- 收尾检查：修正插件对 app 内部网格 Provider 的依赖，通用 RenderProvider 增加显示模式虚接口，具体网格 Provider 覆盖，插件通过该接口恢复透视前显示状态。299 个源码文件 include 边界检查通过。更新原视图装饰测试到工具条入口，并新增必需模块启动/禁用/卸载及主页分组回归用例；受指定构建盘不可用影响，这些 C++ 回归未执行。

## 当前目标绑定与面板接口（2026-10-14）

- `IDOSRenderServer` 统一保存和切换当前视图，主窗口删除重复的活动视图 ID，并通过 `IDOSInterface` 向插件提供渲染服务。工程保存当前对象和明确的当前工况 ID；切换视图时恢复该视图的对象与工况上下文。工况树右键先同步当前节点，菜单动作继续保存目标视图和对象 ID；多选状态保留。
- 模拟结果播放按当前工况 ID 核对所选网格引用，避免同一网格属于多个工况时遍历工程取第一个。模拟插件主页动作也使用明确的当前工况。网格显示、网格筛选和井显示面板改为空构造与 `setProject`、`setView`、目标 setter 分步绑定。
- 固定 Debug 构建及独立头文件检查通过。首轮全量测试 7/9，通过测试发现并修复筛选面板初始化顺序和树右键清除多选两个回归；随后 `case_workflow`（37.38 秒）与 `tree_incremental` 单独重跑通过。最后补充无效工况上下文清理后，`idos_app` 再构建通过。

## 已保存筛选结果取消勾选的恢复（2026-10-14）

- 补齐“先应用预览、再修改筛选窗口条件而不重新应用、直接另存为”的路径。此前保存条件与仍在主网格上的预览条件不相等，保存过程因此跳过预览结果 ID 绑定；之后取消树上已保存结果的勾选只隐藏结果，主网格仍显示旧预览。
- 保存成功后，主网格预览同步为实际保存的条件和显示属性，再绑定新结果 ID；取消该结果勾选时，原有树 GuiProvider 清除预览并重建完整主网格。针对这一操作顺序新增回归：修复前在预览 ID 断言处失败，修复后 `onGridFilters` 专项通过。固定 Debug 目录中的 `idos_app` 与 `idos_case_tests` 构建通过；完整 `case_workflow` 通过（41.42 秒，`filter-cancel-case-ctest.log`）。

## 已保存筛选结果重新勾选（2026-10-14）

- 发现取消勾选会清除主网格预览，但重新勾选仅显示结果对象，没有把保存的筛选条件重新应用到主网格。树节点重新勾选时，主网格 Provider 重新加载结果条件及所需属性引用，并关联结果 ID、更新视图；取消勾选仍只清理与该结果关联的预览。
- `onGridFilters` 增加“取消后重新勾选”对主网格条件、关联 ID 和单元数量的断言，并调整动态属性步切换后的原有单元数量断言。程序关闭后，固定 Debug 目录的 `idos_case_tests` 构建通过；完整 `case_workflow` 通过（35.30 秒，`filter-recheck-case-ctest.log`）。

## 工具条隐藏全部的显示状态恢复（2026-10-14）

- 工具条“隐藏全部”原来把场景对象全部置为不可见，再次点击“显示全部”则全部置为可见，因此会错误勾选原本未勾选的已保存筛选结果。渲染视图在隐藏时记录各渲染对象的可见状态，显示时恢复这组状态；没有隐藏记录时，“显示全部”仍可显示全部对象。
- `onGridFilters` 增加通过工具条动作执行隐藏/显示后检查主网格恢复、筛选结果保持未勾选的回归。固定 Debug 目录构建通过；完整 `case_workflow` 通过（50.06 秒，`hide-restore-case-ctest.log`）。

## 当前接口调用链路整理（2026-10-14）

- 新增[接口调用链路](接口调用链路.md)，按当前源码记录启动装配、命令修改工程、文件导入、工程信号到树/渲染、树勾选、网格/井局部显示、模拟 Flow 与结果导入、工具/插件/助手入口，并标明工程持久化及 Python 运行时尚未接通。开发指南加入阅读入口。本文为文档梳理，未修改运行代码，也未执行构建测试。

## 流体与岩石表导入与工况引用分离（2026-10-14）

- 数据树“流体”和“岩石物理函数”分类的右键菜单已区分：各自使用对应标题、文件类型提示，并将类别传给导入命令；即使选择混合文件，也只创建所选分类的表对象。若文件不含该类别的受支持关键字，会报告对应类别的错误。
- 数据树“Fluids”和“Rock Physics Functions”节点提供独立表格导入，导入命令只创建工程级数据对象，不再要求或自动修改目标工况。表格节点可通过“添加到模拟工况”选择目标，使用命令记录稳定 ID 引用；撤销和重做分别作用于导入对象或工况引用。
- 工况树“流体与岩石属性”只显示已关联表格引用，并保留表节点的属性曲线入口；该分组不再提供导入菜单。工况表格查看接口从工程引用对象读取数据。
- 经用户确认新增 `core/data/fluidrock` 存放可共享的数据类与 DataMetadata、`app/fluidrock` 存放树 Metadata/Provider、`providers/fluidrock` 存放 ECLIPSE 表文本解析 Provider；工况 GUI 和跨类型导入命令继续留在各自已有目录。
- 按用户要求，原 `providers/eclipse` 目录已拆为 `providers/grid` 与 `providers/case`，其中 Provider 类名保留 ECLIPSE 标识；空 `eclipse` 目录已移除。
- 更新固定源码目录清单和使用指南。尚未构建或运行验收；E 盘当前在环境中不可用，需使用既定构建目录前先恢复该盘路径。

## 工况树流体与岩石表去重（2026-10-14）

- 修复工况树把工况内嵌曲线表和工程级表引用各显示一遍的问题。新导入 `.DATA` 中解析出的表现在转换为工程级流体/岩石表对象，工况只保存对象 ID 引用；旧数据已有表引用时，曲线读取优先使用引用数据，不再把内嵌副本叠加进曲线列表。工况树在存在表引用时只显示引用节点，无引用的旧对象继续使用内嵌表作为兼容显示。
- 尚未构建或运行验证。

## Petrel 井头表导入（2026-10-14）

- 井头 Provider 新增 Petrel `VERSION 1` 格式解析，支持引号包裹的井名/空字段及空格分隔字段；旧七列井头格式保持兼容。
- 从 `Name`、`Surface X/Y`、`Well symbol`、`Well datum name/value` 和 `TD (MD)` 读取井名、平面坐标、符号、已识别垂向基准及测深。KB 同时填充 KB 与地面高程；MSL、TVDSS 保存为对应垂向基准。当前井数据模型不保存水平坐标系文本，因此样例中的 CRS 注释暂不持久化。
- Metadata 可按 `Wellheader` 文件名或文件内容识别该格式；中文翻译和导入样例说明已更新，并为 `testdata/井数据/井头/Wellheader.txt` 增加导入回归用例。
- 尚未构建或运行验证：按当前 `AGENTS.md` 唯一构建目录 `E:\CJW-Archive\IDOS-Build\win-x64-debug-local` 在本环境中不存在；遵守目录约定，未在其他目录构建。

## QXlsx 与 Shapelib 依赖迁入（2026-10-14）

- 将旧工程 `thirdparty/QXlsx` 和 `thirdparty/shapelib` 的头文件、Debug/Release 静态库及 Shapelib 许可证文件复制到新版 `thirdparty`。
- 两库的头文件分别位于 `include`，Debug/Release 库平铺放在各自 `lib` 根目录；根 CMake 导入目标已按该布局更新，`idos_providers` 链接两个目标，供 Excel、Shapefile Provider 使用。
- 尚未构建验证：当前环境无法访问 `AGENTS.md` 指定的唯一构建目录 `E:\CJW-Archive\IDOS-Build\win-x64-debug-local`；未使用其他目录配置或构建。

## 泵注与射孔 Excel 数据导入（2026-10-14）

- 压裂泵注程序 Provider 新增 `.xlsx` 读取，逐个扫描工作表并按泵注速率、流体名称和流体体积列识别施工步骤；每个有效工作表导入为独立阶段数据，保留原列名和单元格值。原 TSV 导入保持兼容。
- 井完井 Provider 新增射孔 Excel 读取，按 Perf top/bottom MD 建立米制测深区间，并将阶段号、射孔名、孔数、间距、孔径、隧道长度、相位、温度、质量指数、应力梯度等原列完整保存在完井记录扩展属性。可从具体井的“完井”节点导入以合并到该井；从井分类导入时以文件名中的井名创建/匹配井。
- 两类 Provider 的 Excel 扩展名、文件选择器和中文翻译已更新；架构与进度文档同步。
- 尚未构建或实样验收。按当前 `AGENTS.md` 只能使用 `E:\CJW-Archive\IDOS-Build\win-x64-debug-local`，当前环境无该盘路径，因此未改用其他目录构建。

## MSVC QString 迭代器弃用警告（2026-10-14）

- VS 2022 14.44 在编译 `idos_providers` 时通过 `QString` 模板实例化触发 STL4043，提示 MSVC 专有的 `stdext::checked_array_iterator` 将来会移除。项目使用 C++17，截图所示为 warning，不是 `std::span` 编译错误。
- 仅对 MSVC 的 `idos_providers` 目标定义 `_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING`，保留其他目标和其他弃用诊断。尚未重新构建确认警告消失。

## Petrel 与 ECLIPSE 井轨迹格式拓展（2026-10-14）

- 井轨迹 Provider 现在按文件内容识别 Petrel 井轨迹（支持 9/11 列、DLS 和 AZIM_GN）及 ECLIPSE 多井 `.TRJ`，不依赖文件扩展名；允许多井单文件导入。轨迹点保留 DLS、网格方位角等额外列，井轨迹表格展示这些来源字段。
- 轨迹导入选择器扩展到 `.TRJ`、`.DEV` 和无扩展名文件；Metadata 通过内容检查避免仅按扩展名误识别。
- 尚未构建或用共享目录实样验收。按当前 `AGENTS.md` 唯一构建目录 `E:\CJW-Archive\IDOS-Build\win-x64-debug-local` 在本环境中不可用，未改用其他目录构建。

## 导入功能与构建收尾验收（2026-10-14）

- 使用 CMake 4.4.2、Visual Studio 2022 MSVC 19.44、Qt 5.15.2，从当前 E 盘源码配置到 `D:/CMakeCoder/IDOS-Build`，打开渲染与测试；主程序、全部插件及独立头文件检查构建通过。README 与 CMake 预设统一使用该目录，未使用旧 E 盘构建产物。
- include 检查同步已确认的 fluidrock、fracture、production、seismic 和拆分后的 grid/case 公共目录；QXlsx 改为第三方尖括号包含。429 个源码文件检查通过，检查器 40 个边界用例通过，继续拒绝插件直接包含 app 内部工况菜单类。
- 工况生成输入及属性曲线请求由主窗口接入公共 IDOSInterface 信号，再由数值模拟插件响应；删除插件遍历并转换具体 app GuiProvider 的依赖。架构文档同步，未增加类或目录。
- Petrel 井头回归原先指向仓库缺失的共享文件，现加入测试内固定样例。新增 Excel 泵注数据保留、射孔区间/扩展字段及非法深度检查，多井轨迹与 DLS/AZIM_GN 保留检查；流体表覆盖按类别独立导入、不自动关联工况、拒绝重复引用、内嵌副本与引用并存时树节点去重，以及撤销/重做和移除源文件后的重做。
- 全量回归发现 Windows Qt 不提供 IBM 037 编码器，SEG-Y 现直接按 CP037 映射解码文本头；测试使用固定 EBCDIC 字节验证自定义道头位置和 IBM 浮点样本。工况测试此前在文件选择框重复 selectFile 后未能确认，表现为 240 秒超时；改为填写非原生文件对话框的文件名输入再确认。专项定位排除了此前仅凭尾部日志推断插件析构阻塞的结论，不宣称解决所有历史卸载问题。
- 最终完整 CTest 10/10 通过，51.32 秒，日志 `D:/CMakeCoder/IDOS-Build/closeout-accepted-ctest.log`；case_workflow 11.66 秒，QtTest 19 passed / 0 failed / 2 skipped。两个跳过项为实际 WSL Flow 执行和输出分帧，需设置 SPE1 输入包后单独验收。构建日志为 `closeout-repair-build.log`，最终工况测试重编译日志为 `closeout-case-final-build.log`。翻译 XML 解析及 git diff --check 通过。
- 构建仍存在第三方静态库缺少 PDB 的 LNK4099，以及 core/测试中 Qt 容器触发的 STL4043；未扩大 providers 原有屏蔽范围，不能宣称全工程零警告。尚未执行 Release、共享目录全部实样、约 650 MB SEG-Y 显示及真实 Flow 验收。工程持久化、汇总结果和日报工作簿等未实现事项继续保留；本轮未扩展这些功能。

## 数值模拟取消插件边界并集成平台（2026-10-14）

- 用户确认迁移后，复用已有模拟业务实现。原 IDOSSimulationPlugin 改为 IDOSSimulation，放在既有 app/case，由主窗口创建、初始化及提前销毁；主页模拟/结果/输出分组保持原有功能。工程切换继续隔离上下文并取消旧任务，平台退出负责停止任务及清理窗口和动作。
- 模拟业务窗口全部迁入 app/case；井显示面板及菜单迁入 app/well；IDOSRenderToolMetadata、IDOSRenderToolProvider、IDOSRenderInteractionTool 迁入 app 根部。主窗口直接注册内置视图工具及井显示菜单，不新增功能目录、包装层或第二条调用链。
- 删除数值模拟插件 DLL 导出、独立 CMake 目标、QRC 和 TS；专属图标归平台 images/images.qrc，翻译归 idos_zh_CN.ts。补齐迁移类的 app DLL 导出和全部迁移头文件独立编译检查。平台直接连接工况菜单至模拟功能，去掉之前为跨插件连接添加的两个 IDOSInterface 信号。其他专业插件的公共接口与安全卸载机制保留。
- 构建后清理当前配置下遗留的 idossimulationplugin.dll 和独立 QM，防止旧产物被重新发现；验证 Debug 插件目录仅包含其余四个扩展 DLL。撤销井显示跨模块依赖例外，检查器继续拒绝插件引用 app 内部 Provider。
- 原必需插件回归改为平台回归：历史禁用设置不影响功能，插件目录和列表中没有数值模拟，普通插件全部卸载后模拟动作及内置工具仍保留。保留工况、工具、井显示、动态结果播放和退出回归；测试程序直接链接平台实现，不再重复编译模拟窗口源码。
- 唯一构建目录 D:/CMakeCoder/IDOS-Build 的 Debug 全项目及独立头文件检查通过（simulation-platform-verified-build.log）；429 个源码边界检查及 40 个检查器用例通过。翻译 XML 有效且无 unfinished，git diff --check 通过。完整 CTest 10/10 通过，52.86 秒（simulation-platform-ctest.log），case_workflow 12.69 秒。两个实际 WSL Flow 用例仍因未设置 IDOS_FLOW_SMOKE_DATA 跳过，未宣称完成真实求解器或 Release 验收。第三方缺 PDB 等既有构建警告保留。
- 架构、固定源码目录、使用指南、开发指南、接口调用链路及 README 已同步。工程持久化与共享实样等后续事项不在此次迁移范围内。
## 源码目录重新归类实施与验收（2026-10-14）

- 以 C:/Users/Administrator/Desktop/src 为用户确认的布局参考，仅迁移仓库实际文件，不覆盖现有实现。迁位 177 个文件，迁移前后均为 429 个 C++ 头源文件；全部映射目标存在，所有非 main.cpp 的实现均有同目录同名头文件。桌面布局未列出的导出头、DataMetadata/Registry、基础界面及 CMake 文件保留并按职责归类；清理 7 个迁空目录。
- core/providers 承接通用 DataProvider、TreeProvider、Metadata、Registry；具体数据及对应 Metadata 仍在 core/data 的业务目录，DataObject 和 TreeServer 位于 core 根部。DataProvider 接口由 core 导出，具体格式实现仍由 providers 库导出，并显式包含自身导出头。core/log 更名 core/logger。
- app/providers 集中具体树、菜单及渲染实现，app/command 集中具体命令，app/plugin 保存插件列表与管理界面；内置工具 Provider/Metadata 位于 app/providers/render，通用 RenderInteractionTool 归 render。render/providers 集中渲染和工具 Provider/Metadata。
- gui 集中界面源码；其中 20 组业务界面保留 APP_EXPORT，由 app/CMakeLists.txt 显式编入 idos_app。通用 idos_gui 不依赖 app 或 render，公共头不得泄露这些业务界面；依赖检查按精确目标文件清单识别归属，并新增允许业务界面绑定及拒绝通用界面反向依赖的回归用例。未新增包装层或通用业务协议。
- 同步所有项目 include、CMake 源清单及 include 路径，保留插件 SDK 公开入口；同步 AGENTS、架构、完整源码文件树、README、开发指南及调用链路。lupdate 识别 1259 个已有文本，新增 0，翻译 XML 有效且 unfinished 为 0。
- 唯一构建目录 D:/CMakeCoder/IDOS-Build 中 Debug 全项目构建及 213 个头文件独立编译通过（layout-build-verified.log）。429 个源码边界检查、45 个边界检查回归用例及 git diff --check 通过。完整 CTest 10/10 通过，66.51 秒（layout-ctest.log）；case_workflow 13.22 秒。两个真实 WSL Flow 用例因未设置 IDOS_FLOW_SMOKE_DATA 跳过，未宣称真实求解器、共享大文件或 Release 验收完成。第三方 OPM/QXlsx 缺 PDB 等既有警告保留。
## app/providers 与 gui 业务目录细分及验收（2026-10-14）

- 按用户确认的分类，将 app/providers 下 56 个文件迁入 well、grid、case、fluidrock、seismic、data；保留 render 作为跨业务的内置视图工具目录。同一业务的 Provider、Metadata、GuiProvider 保持集中，不再按类型拆散。
- gui 下 54 个文件归入 tree、well、grid、case、simulation、output、python、plugin、project。TreeView 和 TreeGuiProvider 同放 tree，原 gui/providers 空目录移除；公共插件接口归 plugin，新建工程对话框归 project，根部仅保留 CMakeLists.txt 与 idos_gui.h。
- 同步项目与测试 include、CMake 源清单、翻译位置、固定目录完整文件树、架构及编码约定。保留原链接目标归属：20 组业务窗口继续由 idos_app 编译，通用 gui 不依赖 app 或 render。依赖检查允许 tree、output、python、plugin、project 的通用公开接口，按 CMake 的精确清单拒绝反向引用业务窗口；新增 3 个公开接口回归，并将业务窗口边界用例更新到分组路径。
- 本轮总计迁位 110 个文件；429 个头源文件全部存在，非 main.cpp 的实现都有同目录同名头文件，app/gui 的 CMake 源引用无缺失。lupdate 识别 1259 个已有文本，新增 0，中文翻译 unfinished 为 0。git diff --check 通过。
- D:/CMakeCoder/IDOS-Build 的 Debug 全量构建及 213 个头文件独立编译通过（business-layout-build.log）；再次构建主程序通过，旧目录的 autogen 依赖警告已消失（business-layout-incremental.log）。429 个源码边界检查及 48 个检查器用例通过。完整 CTest 10/10 通过，56.00 秒，case_workflow 12.97 秒（business-layout-ctest.log）。两个实际 WSL Flow 用例仍因未设置 IDOS_FLOW_SMOKE_DATA 跳过；未增加真实求解器、共享大文件或 Release 验收范围。第三方既有构建警告保留。
## 同步用户最终源码位置及验收（2026-10-14）

- 根据用户手工调整后的实际目录，同步 12 个头源文件的位置变化；保留当前文件位置。工况对话框、新建工程、调试信息、运行信息和 Python 控制台位于 gui 根部，tree、well、grid、simulation 仍按业务分目录。
- 用户进一步确认 IDOSInterface 的头文件和实现均位于 src/gui 根部。最终使用 GUI_EXPORT、包含 idos_gui.h，并由 idos_gui 编译；core 的构建清单不含该接口。同步所有消费者引用、app/gui/core CMake、213 个独立头文件检查路径、边界用例、翻译位置、AGENTS、架构及完整源码文件树。此前临时移入 core 的配置已替换，不保留两套实现或导出入口。
- 429 个 C++ 头源文件全部保留，头源配对同目录；CMake 源引用无缺失。lupdate 识别 1259 个已有文本，新增 0，翻译无 unfinished。git diff --check 通过。
- 唯一构建目录 D:/CMakeCoder/IDOS-Build 中 Debug 全量构建及 213 个头文件独立编译通过（current-layout-build.log）；主程序再次增量构建通过，旧路径的 autogen 依赖警告已消失（current-layout-incremental.log）。最终 CTest 10/10 通过，54.08 秒，case_workflow 11.94 秒（current-layout-ctest.log），包括 429 个源码边界检查及 48 个检查器用例。两个实际 WSL Flow 用例仍因未设置 IDOS_FLOW_SMOKE_DATA 跳过；共享大文件、真实求解器和 Release 的验收范围未扩大。第三方既有缺 PDB 等警告保留。
## 工况引用菜单、剪贴板与拖放首批实现及验收（2026-10-14）

- 在既有 app/command 中增加 IDOSCaseReferenceCommand 及 Metadata 成对文件，注册 case.references.copy、case.references.move、case.references.remove。支持井和流体/岩石表的批量引用，按对象/源工况/引用 ID/role/partKey/itemKey 校验；复制生成独立引用 ID，转移保留已有引用 ID，旧引用无 ID 时补齐；撤销和重做恢复精确前后列表。项目对象数不因关联、转移或移除引用变化。
- 原“添加流体/岩石表到工况”的菜单与 case.fluidRock.attach 参数入口由现有 Metadata 创建同一引用命令；删除 IDOSImportObjectsCommand 中旧的表关联模式、校验与修改分支。保留外部入口，不保留第二套修改逻辑。
- 扩充现有 TreeGuiProvider 的 Qt MIME、复制/剪切、粘贴与移除能力，默认不支持；Case GuiProvider 提供具体业务判断。TreeView 装配菜单和 Ctrl+C/X/V，使用 ExtendedSelection，维护源快照及目标持久索引；多选时不提供只能处理单个对象的旧专属动作。命令执行时再次校验源/目标和工程会话，不持有长期裸节点指针，不把 C++ 指针放入可传递载荷。
- 拖动使用 Qt QDrag/QMimeData，默认及 Ctrl 拖动增加引用，Shift 拖动转移已有工况引用；合法目标显示边框，非法目标拒绝。原生拖动对象按 Qt 生命周期由父视图拥有；不调用默认 Model 的行移动来伪造数据修改。跨工程、外部文件、固定分类、网格、完整工况和井部件不开放这批操作。
- 主窗口当前节点与渲染上下文同步增加源树和重入保护，避免间接上下文通知把源树的多选压回单选。工况根节点按实际 tree()->root() 隐藏父节点识别，恢复该节点的既有菜单并使粘贴目标正确。
- 整批预检和前后列表准备均在写入前完成，更新双方完整状态后再发布通知。重复、失效或混合不兼容批次整体失败，不清空重做历史；剪切本身不移除源，成功粘贴后才清理当前剪切载荷。撤销/重做遇到外部引用变化或源数据消失时拒绝执行，历史位置保持不变。
- 新增命令原子性及引用部件身份测试，覆盖无效第二项、重复、自粘贴、通知时双方状态完整、失败保留 redo、外部状态漂移及数据失效后的 redo。命令测试完整 14 passed、0 failed（tree-references-command-all.txt）。树交互覆盖菜单源快照、多选、快捷入口、移除保留数据、非法目标/动作/MIME、外部拖入、工程切换及源删除。
- Windows 原生测试展示两个实际 TreeView，通过真实 Ctrl+C/V 和 Win32 鼠标输入执行跨树复制、Shift 拖动转移及撤销；输入由独立线程发送，避免依赖 OLE 拖放期间的 GUI 定时器，不修改应用业务线程模型。最终交互专项 6 passed、0 failed、0 skipped，5474 ms（tree-references-ui-final.txt）；测试结束释放鼠标/修饰键并恢复光标。树回归禁用无关的助手自动启动。
- 唯一构建目录 D:/CMakeCoder/IDOS-Build 的 Debug 全项目与 214 个头文件独立编译通过（tree-references-complete-build.log），431 个源码边界检查及 48 个检查器用例通过。完整 CTest 10/10，通过耗时 61.05 秒，tree_incremental 9.70 秒、case_workflow 12.84 秒（tree-references-ctest.log）。两个实际 WSL Flow 用例仍因未设置 IDOS_FLOW_SMOKE_DATA 跳过；未扩大真实求解器、共享大文件或 Release 验收范围。第三方原有 PDB/Qt 容器警告保留。
- 平台翻译 active 源文本 1276 条，新增文本已完成中文翻译，无 unfinished；架构第 18.6 节、使用指南、固定源码文件树及 CMake 已同步，git diff --check 通过。后续继续完整数据/工况副本及私有网格、部件规则；删除工况未统一筛选生命周期的既有差异仍待对应阶段处理，不宣称这些功能已完成。

## 对象操作需求纠正与验收（2026-10-14）

- 用户明确要求操作数据、工况、网格和属性本身，原“复制引用／剪切引用”的实现方向错误。TreeView 删除引用移除协议，菜单统一 Copy/Cut/Paste，具体 GuiProvider 负责多选关联入口和对象重命名入口。
- 在既有 app/command 增加 IDOSTransferObjectsCommand 及 Metadata 成对文件，注册 objects.copy / objects.move。具体 DataObject 提供 clone 能力，默认不支持；现有平台具体数据复制全部值字段，构造分配新 ID。工况复制递归复制关联对象与网格子对象，重映射对象与筛选属性 ID；清空运行状态及输出目录，撤销/重做保留新对象 ID。
- 同工程 application/x-idos-objects 仅包含运行期工程身份和稳定标识，不包含裸指针；粘贴从当前工程内存复制数据，不重新读取源文件。普通/Ctrl 拖动复制，Shift 拖动移动。剪切仅记录源，完成粘贴才改变归属。
- 工况直接属于工程，数据树分类固定，故工况及工程级数据无同级移动目标，不支持剪切重排；可复制和拖动复制。网格在不同工况间移动；属性在尺寸一致的网格间移动，拒绝名称冲突和有筛选依赖的属性移动。筛选单独跨网格复制、井内曲线等部件、外部文件和跨工程传输未开放。
- 数据树整井及流体/岩石表新增“添加到工况…”入口，选择具体目标工况，支持多选；此入口共用已有可撤销关联命令，不创建数据副本。数据、网格及属性补充重命名入口，已有工况和筛选入口保留；动态属性树显示 name，模拟器 keyword 不随显示重命名修改。
- 删除工况时将网格的筛选与属性一并接管，撤销恢复，避免复制工况后删除遗留筛选。
- 相关 Debug 目标构建通过（object-transfer-build-final.log）；命令测试 16 passed/0 failed/0 skipped（object-transfer-commands.txt），包括独立数值修改、工况深复制、筛选 ID 重映射、删除及撤销、属性移动及尺寸不兼容失败保留 redo；树测试 22 passed/0 failed/0 skipped（object-transfer-tree.txt），包括多选添加到工况、对象数增长、新 ID、菜单/快捷入口、目标兼容性及跨工程/失效源拒绝。源码依赖边界检查 433 个文件通过。中文翻译 1311 项全部完成。日志位于唯一构建目录 D:/CMakeCoder/IDOS-Build。
- 本次按用户要求控制验证范围，未重跑全部 CTest，未扩展原生鼠标测试；先前原生引用拖放验收不用于宣称本次对象拖放已做原生鼠标验收。

## 单文件 HDF5 工程存储首版及验收（2026-10-15）

- 用户确认采用单个二进制 .idos 工程，工程信息由 IDOS 定义，全部已解析数据保存到文件内部，不嵌入原始导入文件；明确不增加 IDOSProjectStorage。实施在 IDOSProject 提供 load/save/filePath/isModified，元信息和对象变更追踪未保存状态。
- 扩展 DataMetadata 的工程读写分发和 HDF5 值编解码；既有具体数据及组合值类型逐字段实现 projectData/restoreProjectData，不转储 C++ 内存。Object 只授予 Project 原 ID 恢复权限。新增 IDOSGridPropertyMetadata 成对文件，补齐属性的类型创建和注册；应用与独立窗口测试共用平台数据注册方法。
- 文件 /IDOS 保存格式标识、版本 1、工程信息、当前选择和对象索引，/Objects/<序号>/Data 保存业务字段。实际数值使用原生小端 HDF5 数组；大于等于 1024 项时分块、Deflate 1 压缩及 Fletcher32 校验。当前文件为 IDOS 自定义 HDF5 结构，未实现标准 VTKHDF 导出，不能声称可被 ParaView 直接打开。
- 覆盖 16 种平台注册类型：井、网格、属性、筛选、地层面、边界、模拟工况、流体/岩石表、微地震、地震、泵注计划、天然裂缝、生产历史、压裂液/支撑剂目录。井内全部已有字段及存在标记、扩展属性、曲线空值，动态属性全部报告步及当前步均保留。示例插件点集也实现相同存储接口；未来插件未实现存储时明确失败，禁止静默丢失对象。
- 加载先在临时 Project 恢复对象和原 ID，再校验容器、工况关系、数组尺寸、枚举及报告步，成功后切换当前工程。版本不支持、未注册类型或文件损坏不改变当前工程。筛选保留缺失属性条件，兼容既有失效结果仍可编辑的语义。拒绝 HDF5 外部链接、外部数组和虚拟数据集，确保恢复不读取源文件。
- 保存先完成临时 HDF5 文件，再通过 QSaveFile 原子替换，发生序列化/写入/提交错误保留旧文件。当前 VTK 随附 HDF5 窄字符接口不能直接读取部分中文路径，增加短路径及 Qt 文件读写/HDF5 内存映像回退。回退受 QByteArray 2 GB 上限约束，并会额外占用内存；共享约 650 MB SEG-Y 工程与更大工程尚未做性能验收。
- MainWindow 接通打开、保存、另存为，支持 Ctrl+O/S/Shift+S，标题显示工程名/文件名和修改标记；新建、切换及关闭前提供保存/放弃/取消。用户已明确允许关闭当时运行的 IDOS（PID 8316，唯一 Debug 产物路径），解决 DLL 被占用导致的 LNK1168 后完成构建。
- 新增保存恢复专项覆盖全部平台类型、原始源文件路径不存在、中文工程文件名、工程元信息和选择、网格几何、筛选属性 ID、静态与动态属性、井曲线 NaN、地震采样 NaN、压缩数组、嵌套字典/列表、日期和二进制值；逐对象比较字段及原 ID。失败保护覆盖未支持的值类型中途保存失败后原文件逐字节一致、文件截断、未来版本及类型注册缺失。主窗口用例验证保存标记、打开后的树节点和加载失败前的源工程隔离。
- 唯一构建目录 D:/CMakeCoder/IDOS-Build：Debug 全目标构建通过（project-storage-complete-build.log），新增属性 Metadata 的独立包含检查通过（project-storage-header-build.log），435 个源码边界检查通过，平台 1359 项和示例插件 27 项中文翻译全部完成。专项 5 passed/0 failed/0 skipped（project-storage-first-tests.txt）。完整 CTest 其余 9 项通过；project_dialog 的旧标题断言与新工程名/未保存标记不一致，按功能更新后仅重建并复测该目标，1/1 通过、3.88 秒（project-storage-dialog-ctest.log）。首轮全套日志保留为 project-storage-ctest.log，不伪称该首轮 10/10。未重复运行已经通过的其他 9 项。
- 当前持久化范围为业务数据、全局对象显隐和当前选择；不保存撤销历史、相机、窗口布局或各视图独立显隐/着色。恢复只重建展示，不自动恢复或启动求解器进程。工程设置编辑入口仍待实现。后续优先实际工程大文件保存/重开验收，再接视图状态与格式升级策略。

- 编译警告修复（2026-10-15）：IDOSDataMetadata 同构记录字段比较改为显式检查数量和键，避免 Qt 5 QStringList 比较实例化 MSVC 已弃用的 checked_array_iterator。未屏蔽警告或修改第三方代码；idos_core Debug 重编译无该警告，工程往返与性能测试 4/4 通过。

## 渲染选择高亮行为调整（2026-10-15）

- 用户要求只有选择工具显示对象选中边线，并且再次点击选择工具关闭后边线消失。选择工具退出时清除 RenderView 选择状态。
- 裁剪与按点切片改为在工具内部保存目标对象 ID，不再通过通用 RenderView 选择状态标记目标对象；裁剪和切片仍可直接点选可见模型开始操作，避免非选择工具留下选中边线。更新对应英文提示和中文翻译。
- 裁剪、切片和 Z 轴缩放图标统一为现有渲染工具的 24×24 蓝色线框样式，去掉渐变色及圆角底板；边界框图标沿用同一线框配色，并将新增图标登记到 images/manifest.json。
- 在唯一构建目录 D:/CMakeCoder/IDOS-Build 编译 idos_render 通过；未运行测试。

## 保存后标题修改标记修复（2026-10-15）

- 主窗口标题根据工程 `isModified()` 显式添加星号，避免 Qt 的 `[*]` 标题占位符在保存后仍显示星号；Qt 窗口修改状态仍同步设置。保存成功会发出 modified=false 并刷新标题，后续修改再恢复星号。
- 删除不再使用的标题占位符翻译；待编译 idos_app 验证，未运行测试。

## 中央启动页与最近工程列表（2026-10-15）

- 无活动工程时，中央渲染停靠区显示启动页，提供新建、打开和最近工程入口；最近工程双击打开，路径失效的条目显示为禁用。没有最近工程时提供空状态提示。
- 按用户要求将启动页抽为 `gui/idosprojectstartwidget` 独立类，由 `idos_gui` 编译；该类展示最近工程并发出操作信号，工程操作由 app 主窗口处理，保持 `idos_gui` 不依赖 app。
- 工程打开或新建后中央区域切换至现有渲染视图；工程销毁/关闭后恢复启动页。现有 RenderView 的外框继续区分渲染视图区域，并保留活动视图强调色。
- `IDOSProjectStartWidget` 已置于 gui 根部并由 `idos_gui` 导出编译；源码目录、架构设计和翻译上下文已同步。
- 唯一构建目录 D:/CMakeCoder/IDOS-Build 自动重新配置后，`idos_app`（含依赖 `idos_gui`）Debug 构建通过，0 个警告、0 个错误；中文翻译编译完成，1407 项已翻译、0 项未完成。未运行测试或 UI 交互验收。
- 启动页类及配对文件由 `IDOSProjectStartPage` / `idosprojectstartpage` 统一更名为 `IDOSProjectStartWidget` / `idosprojectstartwidget`，并同步主窗口、CMake、源码目录、架构文档和翻译上下文；重命名后重新构建 `idos_app` 和中文翻译均通过，旧名称检索为空。
- 工程 Ribbon 面板加入“关闭工程”。关闭前复用未保存工程确认；确认关闭后解除工程与树、渲染和助手的关联、释放工程对象，并回到中央启动页。该操作也保留在 File 应用菜单；无活动工程时 Ribbon 入口禁用。
- 启动页调整为浅色工作区背景和居中欢迎卡片，新建/打开操作增加说明和主次按钮样式。中文翻译同步。
- 欢迎卡片与最近工程卡片统一铺满启动页可用宽度；新建、打开工程操作在欢迎卡片内靠左排列。
- 最近工程列表移除重复的路径悬停提示；保留整行悬停铺色和选中状态反馈。
- 启动页增加品牌蓝短强调线和最近工程标题图标，强化层次但不改变卡片宽度、按钮排列及列表交互。
- 启动页内容恢复靠上排列，最近工程卡片填充剩余工作区高度；撤销垂直居中和最近列表限高调整。
- 状态栏重新整理为工作提示与右侧信息区：工程名旁用绿/橙/灰状态点表示已保存、未保存或无工程，视图维度和对象数量以同一字重、留白和细分隔线呈现，不使用系统默认图标。
- 网格属性与动态属性导入成功后不再弹模态提示，改为状态栏显示 5 秒并写入信息日志；导入失败仍弹警告。
- 插件管理器对插件名、类别、描述按主程序翻译表显示中文，避免插件在初始化翻译器之前返回的英文元数据直接显示；搜索同时按本地化文本匹配。
- 工况树与输出面板统一使用停靠控件直接承载子控件，避免工况树被额外滚动容器包裹造成边框内缩。
- 主页 Ribbon 工程区改为“新建/打开/保存”常用动作大图标，“另存为/关闭/工程设置”收进单独的工程管理面板；新增蓝色线框 `app-project-close.svg` 并注册到资源和图标清单。中文面板名已补译。
- 按用户澄清仅移除 Ribbon 与 File 应用菜单中“打开工程”动作附加的二级下拉菜单；File 应用菜单和启动页最近工程列表均保留，点击打开工程仍直接打开文件选择框。唯一构建目录中的 `idos_app` Debug 构建通过，0 个警告、0 个错误；中文翻译生成完成，0 个 unfinished。未进行 UI 交互验收。

## 工况内存数据生成 DATA 首版（2026-10-15）

- 原生成窗口只复制源 DATA 文本及索引到的 INCLUDE 文件，没有导出工况内存对象。现改为从工况引用读取当前角点网格、ACTNUM 和静态网格属性，以源 DATA 为模板，通过 OPM FileDeck 展开 INCLUDE 并替换 DIMENS、COORD、ZCORN、ACTNUM 及受支持的属性关键字。
- 网格坐标和连续属性通过 OPM DeckKeyword 按源文件单位系统写回；输出文件生成后再次由 OPM Parser 解析校验。源文件保留其他模拟器设置。生成结果写入独立时间戳目录，不覆盖原始文件，并将路径记录到工况运行信息。
- “初始化”窗口优先使用已生成的 DATA 文件；此文件已展开 INCLUDE，因此运行目录只需复制该 DATA 文件。未生成或路径失效时，仍按原行为从源 DATA 与索引文件初始化。
- 首版仅支持一个角点网格，以及 PORO、PERMX/PERMY/PERMZ、MULTPV、SIGMAV、MULTX/MULTY/MULTZ、SATNUM、ROCKNUM、PVTNUM、EQLNUM、FIPNUM 和 ACTNUM。动态属性、块中心网格及其他关键字属性会明确报错，不会静默忽略。保留模板外部求解器设置；源 DATA 文件仍须可访问。
- 唯一构建目录 D:/CMakeCoder/IDOS-Build 中 `idos_app` Debug 编译通过。并行编译一次遇到 MSVC 编译器堆内存不足，改为单线程构建后通过。未运行测试或实际生成界面验收。
- 单位安全：属性对象持久化值单位状态；旧工程缺少该字段时按既有 SI 内存约定兼容。通过 OPM DATA/文本导入的值按 SI 解释；单独 `.INIT` 二进制导入标记为单位未知，生成器拒绝将其错误转换。
- 修复 DIMENS 运行时构造错误：DIMENS 属于结构关键字，改用 NX/NY/NZ 三个 DeckItem 组装记录，不再调用仅适用于 data keyword 的数值数组构造函数。`idos_app` Debug 重编译通过；本次未运行实际生成验收。

## OPM Debug 库部署（2026-10-15）

- 按用户指定，将 E:/CJW-Archive/Builder/opm-cmake/build/lib/Debug 的 opmcommond.lib 和配套 PDB 部署至 thirdparty/opm/lib；本轮仅更新 Debug 二进制。
- 从源静态库提取 Tables.cpp.obj 检查，仍含 fmt v12 的 dllimport 未解析引用（assert_fail、report_error、vformat），不能宣称该库不依赖 fmt。未额外部署 fmt，尚未进行 IDOS 链接或 GRDECL 运行验收。

## OPM fmt 运行依赖部署（2026-10-15）

- 将 vcpkg 的 Debug/Release fmt 导入库、DLL 及许可证归档到 thirdparty/opm。OPM::opmcommon 通过 OPM::fmt 传递链接依赖，idos_providers 构建后按配置部署对应 DLL，工程不依赖本机 vcpkg 绝对路径。仅接入 fmt 依赖，不改变 OPM 解析逻辑。
- 验证：D:/CMakeCoder/IDOS-Build 配置生成及 idos_providers Debug 构建通过，fmtd.dll 已自动部署，原 fmt 未解析符号错误消失。保留第三方 QXlsx 缺失 PDB 警告；未执行 Release 编译及 GRDECL 运行回归。日志为 opm-fmt-configure.log、opm-fmt-build.log。

## VS 构建目录统一（2026-10-15）

- 用户重新明确唯一构建目录为 E:/CJW-Archive/IDOS-Build，替代此前 D:/CMakeCoder/IDOS-Build 约定；历史构建记录保留原实际路径。公共预设显式设置 binaryDir 为源码旁的 IDOS-Build，所有继承预设共用该目录，防止 VS 回退到源码内 out/build。AGENTS、README、开发及使用指南同步。未删除旧构建目录，未重建工程。

## OPM 关键词部署脚本命名（2026-10-15）

- 将 cmake/CopyFileIfAbsent.cmake 更名为 DeployOpmKeywords.cmake，同步主程序 POST_BUILD 引用；保留目标不存在才复制、已有扩展定义不被覆盖的行为。本轮仅更名及说明注释，未重新构建。
- 用户最终指定脚本名称为 cmake/CopyOpmKeywords.cmake，已同步 POST_BUILD 调用，替代上述临时命名。

## 网格同名配套文件导入（2026-10-15）

- 按用户确认，以同目录、完整主文件名相同且扩展名不同匹配配套文件。IDOSGridEclipseProvider 一次解析网格、INIT 静态属性、UNRST 动态属性和 SMSPEC/UNSMRY 汇总结果；属性绑定本次网格，汇总结果通过既有 grid.import 命令关联目标工况。日志和会话文件不作为业务数据导入，不使用名称前缀匹配。
- UNRST 解析移至现有 IDOSGridPropertyEclipseProvider，手动 simulation.results.import 复用同一实现；命令保留工程重复校验、提交和历史职责。配套文件解析或维度校验失败时释放全部临时对象，不部分写入工程；整组撤销、重做复用已解析数据，不重读源文件。已存在工况汇总结果时拒绝重复导入。
- 新增 onGridCompanionFiles 合成回归，覆盖静态活跃单元展开、多个动态报告步、汇总关联、精确文件名筛选、日志忽略、整组撤销/重做、损坏或维度不匹配失败保护、失败保留 redo，以及手动动态结果导入复用。使用指南、架构及中文翻译同步。
- 回归同时修复手动 UNRST 导入重做时仍检查原文件存在的问题。新部署 OPM 的缺失 INCLUDE 默认策略为直接退出进程，现于 DATA/属性解析入口明确设为 THROW_EXCEPTION，并清理已处理的 ErrorGuard 错误，失败仍正常报告且不提交。
- 验证：E:/CJW-Archive/IDOS-Build 的 Debug 主程序/应用/Provider/工况测试构建及全部公共头文件独立编译通过；新增专项 3 passed/0 failed/0 skipped（grid-companions-focused.txt）。工况 CTest 最终通过，17.94 秒；QtTest 20 passed/0 failed/2 skipped，两个实际 WSL Flow 用例因未设置 IDOS_FLOW_SMOKE_DATA 跳过（grid-companions-case-verified.log）。447 个源码边界检查、48 个检查器回归通过；平台 1446 条中文翻译全部完成，git diff --check 通过。首轮工况缺失 INCLUDE 导致退出的失败日志保留，不宣称首轮通过；既有 QXlsx 缺失 PDB 和 Qt 窗口标题提示保留。本轮未运行其余完整 CTest、Release 或真实大文件性能验收。

## 裂缝三维显示与几何检查（2026-10-15）

- 自然裂缝集接入平台 RenderMetadata/RenderProvider，使用双精度坐标绘制有效多边形，支持树勾选、显示模式、透明度、选中高亮及各视图独立显示状态；工程保存和重开恢复显示。渲染由平台持有，卸载裂缝插件仍可显示已有数据。
- 核心数据提供只读几何报告，检查顶点数量、缺失或非有限坐标、重复顶点、退化面积、共面性、自交、法向量及其方向、属性范围和裂缝 ID。无效几何不进入绘制；属性问题保留有效几何，不自动修改源数据。
- 裂缝建模插件启用“裂缝检查”，优先检查当前裂缝集，否则检查工程中的全部裂缝集。非模态报告列出问题，双击选中源对象并在打开报告时的目标视图定位；数据变化、删除、切换工程、关闭目标视图或卸载插件时关闭相关报告。
- 本轮不改裂缝导入，不启用方位角计算、裂缝重构和储量算法；不包含裂缝间连通性分析或自动修复。源码目录、架构、使用指南及中文翻译已同步。
- 验证：用户保存并关闭占用 DLL 的程序后，E:/CJW-Archive/IDOS-Build 的 Debug 构建及公共头文件独立编译通过。裂缝专项 4 passed/0 failed/0 skipped；工况 CTest 通过（19.84 秒），QtTest 22 passed/0 failed/2 skipped，两个实际 WSL Flow 用例因缺少测试配置跳过。451 个源码边界检查和 48 个检查器回归通过；中文显示与报告截图已检查，git diff --check 通过。日志为 fracture-display-focused.txt、fracture-display-ctest.log、fracture-display-verified-build.log 和 fracture-display-boundaries.log。未运行其他完整 CTest、Release 或真实现场裂缝数据验收。

## 裂缝检查报告窗口拆分（2026-10-15）

- 按用户要求，将插件入口中的报告窗口布局、表格填充和行激活处理抽到现有插件目录内的独立 IDOSFractureGeometryReportDialog，头源文件配对。插件保留检查入口、工程切换与卸载清理、源对象选中和视图定位。
- 对话框用 QPointer 显式保存工程及目标视图，删除表格 project/view 动态属性；通过 issueActivated 信号传递源数据 ID 与定位中心。沿用数据变化、删除及视图关闭时关闭报告的行为。CMake、目录、架构说明及中文翻译上下文同步。
- 验证：E:/CJW-Archive/IDOS-Build 的 Debug 工况测试目标（含插件）构建通过；case_workflow CTest 通过（18.60 秒），22 passed/0 failed/2 skipped。453 个源码边界检查及 git diff --check 通过。构建和测试日志为 fracture-dialog-build.log、fracture-dialog-ctest.log。

## 裂缝共面检查容差修复（2026-10-15）

- 用户导入 Utah FORGE Filtered_800m_19 后，4814 条裂缝中 4787 条因非共面被排除。核查原始 ASCII 文件可重现该数量：毫米级文本舍入造成最大约 0.000834 米离面偏差，原跨度乘 1e-9 的数值容差不适合作为工程共面容差。
- 共面检查独立采用最大坐标跨度乘 1e-4（0.01%）；重复顶点、退化和自交仍使用原严格容差。报告附带 planeTolerance 和 maximumPlaneDistance，原始坐标不作投影、修复或改写，已有工程直接使用新检查。该标准是工程容差，不表示任意精度要求下严格共面。使用指南同步。
- 回归覆盖大坐标微扰、实际六边形的舍入坐标及明显翘曲反例；可通过 IDOS_FRACTURE_TEST_DIR 指定现场 FAB 目录，经注册读取器逐文件检查，不把本机路径写入测试或 CMake。
- 用户关闭运行程序后，E:/CJW-Archive/IDOS-Build Debug 构建通过；本次设置上述环境变量，对下载的 30 个 FAB 文件共 146238 条裂缝全部通过几何有效性检查。case_workflow CTest 通过（38.46 秒），22 passed/0 failed/2 skipped。453 个源码边界检查及 git diff --check 通过。日志为 fracture-planarity-build.log、fracture-planarity-ctest.log，现场计数见 tests/case_workflow.txt。本轮未逐面人工验收或执行 Release 构建。

## 数据导入文件多选（2026-10-15）

- 按用户要求，将 IDOSImportedDataTreeGuiProvider 的共享数据导入入口改为文件多选，天然裂缝等使用该入口的格式均可通过 Ctrl/Shift 选择多个文件。各文件依次调用既有 data.import 命令，单独提交、单独撤销重做；失败继续后续文件，最后按路径汇总失败原因，成功项保留。本轮不改其他专用导入窗口。
- 同步使用指南及中文错误提示。E:/CJW-Archive/IDOS-Build Debug 构建通过；case_workflow CTest 通过（18.20 秒），453 个源码边界检查及 git diff --check 通过。日志为 data-multiselect-build.log、data-multiselect-ctest.log。未进行原生文件选择窗口的手动多选验收。

## 共享导入数据删除（2026-10-15）

- 修复天然裂缝等数据只有导入和显示、缺少删除入口的问题。IDOSImportedDataTreeGuiProvider 提供“删除数据”确认菜单，支持单个及多个受支持的数据集；捕获目标工程和数据 ID，组目录和包含专用删除类型的混合选择不提供该入口。井、网格和属性沿用已有专用删除命令。
- 新增 app/command 中配对 IDOSDeleteDataCommand/Metadata，注册 data.delete。适用于天然裂缝、微地震、地震体、压裂施工、材料目录、生产历史、层面、边界及流体/岩石表。预校验全部目标，保存并移除受影响工况引用，整批分离数据；失败回滚，失败命令不进历史。命令持有分离对象，支持整批撤销/重做，清理历史时释放仍分离的数据；树和多视图根据工程信号清理和重建。
- 新增回归覆盖单选和多选菜单、取消删除、批量删除、工况引用及 refId 恢复、多视图清理和重建、重复 ID 和不支持类型失败时保留 redo、清空历史释放对象。CMake、目录、架构、使用指南及中文翻译同步。
- E:/CJW-Archive/IDOS-Build Debug 工况测试目标构建通过；case_workflow CTest 通过（19.67 秒），24 passed/0 failed/2 skipped。455 个源码边界检查及 git diff --check 通过。日志为 data-delete-build.log、data-delete-ctest.log；两个实际 WSL Flow 测试仍因缺少测试配置跳过。
- 公共头文件独立编译目标 idos_render_header_checks 完成通过，包含新增删除命令头文件。

## 数据树分类总节点勾选（2026-10-15）

- 按用户澄清只处理数据树，不向工况树分类增加勾选。新增 app/providers/data 中配对 IDOSDataGroupTreeGuiProvider，在包含可显示后代的数据分类上提供总勾选；支持嵌套井组、按数据 ID 去重、全部显示/全部隐藏及半选状态。空分类或没有可显示数据的分类不显示勾选框。
- 分类 Provider 通过现有具体 GuiProvider 的可勾选节点确定作用范围，并仅修改目标视图中现有 RenderObject 的显示状态；遇到可显示父对象便不继续激活其内部属性节点。复用 Scene 批量通知，显示后仅适配一次相机，不修改源数据可见性或其他视图。
- TreeItem 改为存储 Qt::CheckState，保留 bool 接口；Model 直接返回三态，View 读取 GuiProvider::checkState，默认实现委托原 isChecked。View 清理没有负责 Provider 的陈旧勾选状态，井 GuiProvider 明确不领取分类组的勾选职责。通用框架无数据类型、树名称或分类规则判断；数据树分类规则只在新具体 Provider。源码目录、架构及使用指南同步，无新增界面文本。
- 新增 onDataGroupChecks 回归覆盖天然裂缝、井和边界三类、嵌套井组、半选及全选/取消、批量场景通知、视图切换独立状态、直接显隐后的状态刷新、空分类、无目标视图，以及同名组位于工况树时拒绝处理。E:/CJW-Archive/IDOS-Build Debug 主程序与工况测试构建通过；case_workflow 通过（21.33 秒），25 passed/0 failed/2 skipped；457 个源码边界检查通过。
- 补充构建首次与运行中的测试并行，部署 Debug CRT 时复制冲突失败；保留 data-group-check-extra-build.log，随后在测试退出后顺序重跑，不宣称首轮补充构建通过。主构建和工况回归日志为 data-group-check-build.log、data-group-check-ctest.log。
- 顺序补充构建通过，包含新增分类 GuiProvider 和全部公共头文件独立编译（data-group-check-extra-verified-build.log）；tree_incremental 通过（7.96 秒）。额外 project_dialog 首轮失败为此前标题格式修改遗留的旧断言（仍期待 IDOS - Project One[*]，实际为 Project One*），仅同步三处标题预期后重建重跑通过（4.12 秒，data-group-check-project-ctest.log），未改标题功能。git diff --check 通过。未执行其他完整 CTest 或 Release 构建。

## 裂缝建模插件 DFN 生成首版（2026-10-15）

- 按用户确认将功能代码留在现有裂缝建模插件目录：新增配对 IDOSDfnGenerationWidget（参数与结果提示）、IDOSDfnGenerator（纯几何计算）、IDOSDfnGenerateCommand/Metadata（fracture.dfn.generate）。插件入口仅注册动作/命令及管理窗口，未新增插件、平台算法类或裂缝数据类型。
- 独立窗口选择角点网格、可选静态密度权重、数量、种子、首末层、均匀长度范围、长宽比、走向/倾角均值及正态标准差。生成 1 至 10000 条矩形裂缝；仅从所选层的活跃、有限正权重单元抽样，按双精度角点插值随机生成中心，保持旧矩形生成的 +X 朝 +Y 走向和朝 +Z 倾斜约定；长度沿用网格坐标单位。
- 期望尺寸可能按比例缩短以适应所选单元整体包围盒，实际 Length/Width、Strike/Dip 和生成参数随裂缝记录保存。不是反演重构，不实现物理 P32/P21 标定，不对不规则网格边界、断层和非活跃孔洞裁剪；不计算面间连通性。生成同步执行，无外部 Workflow_2 依赖；方位角拟合、反演重构和缝控储量仍禁用。
- 命令校验参数和源数据，生成后复用平台几何检查，成功才提交独立 IDOSNaturalFractureSet；失败不进历史。整组撤销，重做复用缓存，不依赖源属性继续保持原值。结果初始隐藏，可在数据树勾选显示，复用既有渲染、检查、删除和工程持久化。
- 注册命令实例计数，覆盖撤销/重做和外部持有的未执行命令；实例存在时禁止常规卸载，窗口在切换工程和卸载时同步清理。发布到平台前经 QJsonArray 转换复制嵌套字符串，避免平台数据引用插件 DLL 的静态文字存储；清空相关命令历史后可卸载插件，已有裂缝仍可检查和保存。
- 新增 onDfnGeneration 集成回归：按钮状态、Widget 参数、活跃单元/密度过滤、大坐标有效几何和方向、同种子逐值复现、不同种子实际顶点变化、输入错误/零权重失败保留 redo、缓存重做、树和渲染、工程保存重开、工程切换关闭窗口、历史和外部命令卸载保护、卸载后几何检查及保存。新增头文件均独立包含编译；源码目录、架构和使用指南同步。
- 验证：E:/CJW-Archive/IDOS-Build Debug 主程序/插件/工况测试与公共头文件检查构建通过。最终 case_workflow 通过（20.76 秒），26 passed/0 failed/2 skipped，两个实际 WSL Flow 用例因缺少测试配置跳过。463 个源码边界检查、68 条插件中文翻译无 unfinished、git diff --check 通过；中文参数窗口截图 dfn-generation.png 已复查。日志为 dfn-final-build.log、dfn-final-ctest.log，QtTest 明细见 tests/case_workflow.txt。未运行 Release、全部 CTest 或真实大网格性能/物理标定验收。

## 树菜单 Provider 迁移续作（2026-10-16）

- 恢复 Data Tree MenuProvider 的通用菜单委派、数据多选删除和剪切/粘贴 Provider 路由。Grid 菜单迁移本轮未合并，保留现有 IDOSGridTreeGuiProvider 注册与实现；井树和井显示菜单仍使用现有 GUI Provider。
- 后续继续时从当前完整的 Data Tree MenuProvider 与 GridTreeGuiProvider 两条现有路径开始，先按具体迁移边界拆分，再逐项迁移并核对资源和翻译。

## 节点 Provider 与菜单简化（2026-10-16）

- IDOSTreeItemProvider 直接继承 QObject 并启用 Q_OBJECT；具体节点 Provider 移除重复 QObject 继承，节点继续拥有 Provider。
- 井、网格、工况、导入数据及流体/岩石物理 Provider 在 createContextMenu 内直接创建 QAction、设置业务参数并连接具名槽；移除 addObjectActions、addAssociationActions、addImportAction 等菜单装配辅助接口，槽直接调用 Model 现有操作入口。
- 保留菜单工程会话校验及单选、多选分派；节点删除后 Provider 的 QObject 生命周期和连接随之结束，树所有权回归增加 QPointer 失效验证。
- 验证：唯一构建目录 IDOS-Build 的 Debug 全量构建通过；check_includes、check_includes_regressions、project_dialog、tree_incremental 全部通过，最终整理后再次构建及树回归通过；工况菜单相关四个定向用例通过（含初始化/清理共 6 passed）。

## 数据树与工况树 Model、View 拆分（2026-10-16）

- 新增 core/tree 下 IDOSDataTreeItemModel、IDOSCaseTreeItemModel，分别继承通用 IDOSTreeModel；新增 gui/tree 下 IDOSDataTreeView、IDOSCaseTreeView，分别继承通用 View，并拒绝挂接另一棵树的 Model。主窗口和 AppTreeAccess 使用专用类型装配及公开访问。
- 具体 Provider 继续自己创建并连接菜单，调用专用 Model 的明确参数函数及业务信号；移除通用 triggerAction/actionTriggered、Access 通用动作槽和 MainWindow 操作字符串分发，业务参数不再经 QVariantMap 菜单协议传递。井头、轨迹、曲线、分层、完井与校深导入分别连接明确接口。
- AppTreeAccess 分别连接业务信号并校验当前工程/Model，继续在触发时获取实际目标渲染视图；MainWindow 业务处理仍使用既有命令。剪贴板传输保留独立 transferItems/transferRequested 机制，不属于菜单业务操作分发。
- 示例插件点集专属删除由 Metadata 连接 Provider 的具名信号到插件槽，执行前校验工程会话，继续使用插件 pointset.delete 命令；平台 Model 不加入插件类型或通用命令执行入口。
- 同步架构完整类声明、固定源码目录与规则；新类加入独立头文件编译检查。回归改用明确业务信号，并覆盖专用 View 类型约束、节点/session 校验、单选/多选和空白菜单。

- D:\CMakeCoder\IDOS-Build Debug idos 构建通过；已重新启动程序供查看。部署仍提示 VCINSTALLDIR 未设置，不影响构建。

- D:\CMakeCoder\IDOS-Build Debug idos 构建通过；程序已重新启动供查看。部署仍提示 VCINSTALLDIR 未设置，不影响构建。

- D:\CMakeCoder\IDOS-Build Debug idos 构建通过；程序已重新启动供查看。部署仍提示 VCINSTALLDIR 未设置，不影响构建。
