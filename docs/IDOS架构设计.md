# IDOS 架构设计

> 本文为已确认的新目标方案，不表示当前代码已经完成。后续直接改造现有框架，删除被替代的类与接口，不保留两套调用链。方案不受旧实现类划分约束。

## 1. 目标与实施边界

新增数据、工况、父子节点、菜单和渲染，通过具体实现与注册完成。通用框架只调用接口，不判断井、网格、断层或具体工况；具体实现可以识别自己的数据。

树与渲染独立但职责一致：通用展示对象管理身份与所有权，Provider 提供展示，GuiProvider 提供交互，Server 同步工程。

- 允许增加本文方案类及具体业务数据、Provider、GuiProvider、Metadata、命令实现；禁止增加方案外类及包装层。
- 类内接口、成员可按需要增删调整；签名是设计基线，不是冻结 API。消除重复职责、同义接口和过多参数，稳定依赖在构造或绑定时关联。
- 禁止新增结构体、命名空间、匿名命名空间、自由函数及 lambda；辅助方法放在所属类中，非公开方法为 private。
- 不引入 domain、统一交互结果协议、多层 Context、额外适配类来弥补职责不清。
- 命名借鉴 QGIS 的职责清晰与可读性，不照搬其接口。类名提供上下文，方法使用 add/remove、create、populate/update 等直接名称。
- 改动前梳理调用路径，接通替代实现后删除旧类、接口、构建项与测试依赖。既有日志、应用、插件、领域值类等未被替代职责保留。

## 2. 模块边界

| 模块 | 职责 |
|---|---|
| core | 数据对象、工程、数据类型工厂与注册、树结构/模型/同步、命令与历史 |
| providers | 文件和外部数据源解析，格式工厂与注册 |
| gui | 通用树 View、GuiProvider、平台公共界面接口 |
| render | 通用渲染对象、Provider、GuiProvider、Metadata、Registry、Server、Scene、View、VTK |
| app | 服务装配、内置具体树/渲染实现、导入命令和 UI 协调 |
| plugins | 专业数据、展示、交互和命令的具体扩展 |
| assistant | 模型进程、请求服务、命令调用及助手界面 |
| analysis / python | 算法和脚本入口，通过命令提交业务修改 |

providers、gui、render、assistant 面向 core。core 不依赖界面、渲染、app 或插件。gui 与 render 不互相依赖，通过工程事件和应用层协作。具体实现放在 app 或插件，通用模块不反向依赖具体实现。VTK 构造限于 render 和具体 RenderProvider。

源码目录以 [IDOS源码目录.md](IDOS源码目录.md) 的固定文件布局为准。公共接口位于所属功能目录根部，同名头文件与源文件成对放置。IDOSProject 位于 core 根目录；通用树结构、模型和同步位于 core/tree，树视图与 GUI 扩展接口位于 gui/tree；插件加载、描述、列表模型和管理界面位于 app 根目录。不设置 app/plugins 或 app/render，不保留 IDOSRenderController。目录调整须经用户确认。

## 3. 数据与工程

| 类 | 职责与主要接口 |
|---|---|
| IDOSDataObject | 数据 ID、名称、类型及通知；id()、name()、setName()、typeId() |
| IDOSProject | 数据与关系管理；add()、take()、remove()、object()、objects()、beginUpdate()、endUpdate() |
| IDOSDataMetadata | 类型描述及工厂；typeId()、displayName()、create() |
| IDOSDataRegistry | 类型注册；add()、remove()、metadata() |

工况也是具体业务数据，保存配置和数据引用。共享原始数据与工况专属配置分别表达；引用使用稳定数据 ID。数据拥有关系、引用关系和展示父子关系不能混用。

工程发布新增、删除前、删除、业务数据变化和批量通知。删除前通知在批量操作内仍立即发送，以解除展示关联。beginUpdate/endUpdate 只合并通知，不提供业务回滚。

数据、树节点、渲染对象分别管理 ID。同一数据可有多个引用节点和各视图独立的渲染对象。

## 4. 数据读写

| 类 | 职责与主要接口 |
|---|---|
| IDOSDataProvider | 一次解析；read(source)、error() |
| IDOSDataProviderMetadata | 格式描述、匹配和工厂；key()、displayName()、supports(source)、create() |
| IDOSDataProviderRegistry | 格式注册和匹配；add()、remove()、providers(source) |

类型工厂回答“创建什么数据”，格式工厂回答“如何读取数据”。一个类型可有多个格式，一个格式可产生多种数据。手工或算法产生的数据不必经过 DataProvider。

每次解析创建独立实例，结果由调用方接管；解析器不修改工程、不创建树或渲染对象。导入命令负责校验、合并、替换、引用建立和提交。候选格式按明确规则选择，不依赖哈希遍历顺序。导出能力按实际需求在这些类内补充。

```text
文件 / 外部数据源
  → DataProvider 解析
  → 导入命令校验、合并并提交
  → Project 保存数据
```

## 5. 树框架

| 类 | 职责与主要接口 |
|---|---|
| IDOSTreeItem | 通用节点、关联及子节点所有权；id()、name()、parent()、children()、dataId()、itemKey()、provider() |
| IDOSTreeProvider | 非拥有地关联数据，构建和更新分支；dataSource()、populate(item)、update(item) |
| IDOSTreeGuiProvider | 菜单和节点行为；name()、supports(item)、populateMenu(menu, view)、isCheckable(item)、isChecked(item)、setChecked(item, checked) |
| IDOSTreeMetadata | 数据类型与 Provider 工厂；typeId()、create(data) |
| IDOSTreeRegistry | 工厂、GUI 扩展和默认分组；add()、remove()、metadata()、addGuiProvider()、removeGuiProvider() |
| IDOSTree | 拥有根节点、受控修改和通知；root()、insert()、remove()、clear() |
| IDOSTreeModel | Qt 模型适配及节点查询；nodeFromIndex()、indexFromNode() |
| IDOSTreeView | 展示、选择与输入；currentItem()、selectedItems()、menuItem() |
| IDOSTreeServer | 工程同步与引用刷新；setProject()、addTree()、removeTree() |

数据树与工况树使用同一框架，通过不同树实例、注册内容和根节点组织形成，不分别写带业务判断的 Model。

默认分组使用 Registry 类内描述注册键、父分组、标题、图标和顺序，不增加分组 Metadata/Context 类。对象分支结构由具体 Provider 构建。TreeItem 表达分组、对象、内部子项和引用，必要关联与所属工况在类内明确保存，操作不靠固定父节点形状猜测目标。

对象分支根拥有 TreeProvider，父节点拥有子节点。独立数据引用分支可拥有独立 Provider；内部子项通过所属分支访问 Provider，不重复拥有它。Provider 经 Tree 的受控接口修改已展示节点，Model 据此发送正确的 Qt 插入、删除和变化通知。

Model 不解释具体业务角色，不处理网格属性互斥等规则。GuiProvider 提供具体能力、状态和行为；未提供专属实现时使用明确默认行为。菜单可组合多个实现，勾选等修改操作必须有明确负责者，冲突不静默执行。

```text
工程新增数据 → TreeServer → Metadata 创建 Provider
  → 通用 TreeItem → Tree 接管 → Model / View 展示
工程变化 → TreeServer → Provider 更新受影响分支
```

## 6. 渲染框架

| 类 | 职责与主要接口 |
|---|---|
| IDOSRenderObject | 通用具体类，独立身份及 Provider 所有权；id()、name()、provider() |
| IDOSRenderProvider | 几何、图例与显示状态；dataSource()、update()、actors()、legends()、visible()/setVisible()、highlighted()/setHighlighted()、opacity()/setOpacity() |
| IDOSRenderGuiProvider | 对象菜单及专属交互；name()、supports(object)、populateMenu(menu, view) |
| IDOSRenderMetadata | 数据类型与 Provider 工厂；typeId()、create(data) |
| IDOSRenderRegistry | 工厂与 GUI 注册；add()、remove()、metadata()、addGuiProvider()、removeGuiProvider() |
| IDOSRenderScene | 拥有展示对象；add()、remove()、clear()、object()、objects() |
| IDOSRenderView | 展示、相机、选择和拾取；scene()、menuObject()、selectedObjects() |
| IDOSRenderServer | 工程同步；setProject()、addView()、removeView() |

Metadata 创建具体 RenderProvider，Server 装入通用 RenderObject 后交给 Scene。不增加仅装配 Provider 的具体 RenderObject 子类。每个 View 的 Scene、Object、Provider 和显示状态独立。

Provider 不访问工程或 Scene，数据关联非拥有。工程同步由 Server 处理，不引入 Provider Context、synchronize() 或数据 Provider 包装层。显示模式、属性着色等专属能力只放具体 Provider，由对应功能和 GuiProvider 调用。

一个对象可产生多个 actor；View 将拾取 actor 映射回 RenderObject，再通过 Provider 获取源数据。当前 actors() 覆盖普通几何；体渲染等需求在方案内现有类中扩充接口。

```text
工程新增数据 → RenderServer → Metadata 创建 Provider
  → 通用 RenderObject → Scene 接管 → View 展示
工程变化 → RenderServer → Provider 更新 → Scene / View 刷新
```

## 7. 菜单与交互

不增加 MenuContributor、ActionRegistry、InteractionProvider 或菜单组织器类。TreeView 和 RenderView 在类内创建菜单、调用适用 GuiProvider、弹出和销毁菜单。

- 右键目标与选择分别查询；空白处也进入菜单流程。多个 GUI 实现共同贡献菜单，排序明确；通用视图动作由 View 或注册的通用 GUI 实现提供。
- 稳定服务依赖在构造或绑定时设置；populateMenu 只接收菜单和目标 View，通过明确关联获取目标及工程，不使用隐式全局“当前对象”。
- 视图菜单以实际点击的 View 与拾取对象为目标，不依赖事后变化的活动视图。
- QAction 与子菜单归本次 QMenu。动作保存稳定 ID，执行时重新校验；不长期保存节点指针，不共用 pending 目标成员。
- 菜单调用业务命令，局部显示操作调用目标 Provider；不接管 Server 同步和容器所有权。
- 动作连接具名成员或 on 开头的槽，不使用 lambda。

## 8. 命令体系

| 类 | 职责与主要接口 |
|---|---|
| IDOSCommand | 参数、执行及撤销状态；validate(error)、execute(error)、undo(error)、isUndoable()、result()、error() |
| IDOSCommandMetadata | 描述、参数和工厂；name()、description()、parameters()、create(project, arguments) |
| IDOSCommandRegistry | add()、remove()、metadata()、commands() |
| IDOSCommandManager | 每工程独立历史；execute(command)、execute(name, arguments)、undo()、redo()、canUndo()、canRedo()、isClean()、setClean()、clear()、result()、error() |

Manager 构造时绑定工程和 Registry。内部 C++ 可直接构造命令，助手和脚本通过名称和 JSON 参数创建。Manager 保存执行结果与错误，命令销毁后仍有效，不新增结果包装类。

创建、编辑、删除、导入合并及工况引用修改走业务命令。查询不进入历史；相机、截图、临时高亮直接操作 View，局部显示状态操作目标 Provider，不混入业务撤销历史。

- 命令绑定工程，参数使用数据 ID；工况相关操作明确工况目标。
- 验证、执行成功后才提交历史；失败不进入历史、不破坏重做记录；撤销失败不移动历史位置。
- 失败需恢复数据与关系，通知批处理不等于事务回滚。
- 删除命令拥有脱离工程的数据，撤销时恢复，清理历史时释放未恢复数据。
- 重做使用保存的解析/计算结果，不重新读取外部文件。
- 使用 QUndoStack 时必须适配上述契约，不可 push 后才判断成功。

## 9. 助手、插件与应用

IDOSAssistantService 负责模型请求、命令调用、结果处理与目标工程绑定；IDOSAssistantWidget 只负责输入和展示。现有模型进程和网络服务职责保留或迁入服务，不额外叠加包装类。

每次请求绑定发起时工程，工程切换后取消或使旧请求失效。助手通过命令 Metadata 获取描述及参数，经目标 CommandManager 执行，不直接修改节点、actor 或业务数据。显示操作明确目标 View 并使用应用公开能力。

应用负责服务装配、活动视图、树定位和工具栏协调。IDOSInterface 提供 project()、dataRegistry()、treeRegistry()、renderRegistry()、commandRegistry() 和 projectChanged()，插件通过公共入口注册并调用相同能力，不访问主窗口内部实现。gui 仅前置声明 RenderRegistry，不增加对 render 的编译或链接依赖。

插件实现 canUnload()，检查存活数据、命令历史和自身持有的任务，包含脱离工程但尚未销毁的实例。PluginRegistry 拥有插件实例，拒绝不安全的卸载，不把卸载失败保存为禁用。具体 GuiProvider 注销时同步清理自己创建的 QAction，避免存活菜单保留已卸载 DLL 的代码与数据。

```text
树菜单 / 视图菜单 / 工具栏 / 脚本 / 助手
  → CommandManager → Project
  → TreeServer / RenderServer → 各自 Provider 更新展示
```

## 10. 所有权与注册生命周期

```text
Project → DataObject
Tree → 根 TreeItem → 子 TreeItem
对象分支根 TreeItem → TreeProvider
RenderView → Scene → RenderObject → RenderProvider
Registry → Metadata / GuiProvider
CommandManager → 历史命令 → 尚未恢复的脱离工程数据
```

Server 借用工程、Registry 和容器；Provider 非拥有地关联数据；GuiProvider 不拥有操作目标。注册成功转移所有权，失败不转移；重复注册拒绝并报告。

注册变化自动补建已有数据展示，注销前清理依赖展示再销毁工厂。多项注册可合并通知。插件存在存活数据、命令或任务时先清理或拒绝卸载，不能直接卸载 DLL。切换/销毁工程断开旧连接并清空展示。

## 11. 扩展示例与验收

断层功能按需要提供：

```text
IDOSFault / IDOSFaultMetadata
IDOSFaultTreeMetadata / IDOSFaultTreeProvider / IDOSFaultTreeGuiProvider
IDOSFaultRenderMetadata / IDOSFaultRenderProvider / IDOSFaultRenderGuiProvider
创建、编辑、删除断层命令及 Metadata
需要导入时增加对应格式 DataProvider / Metadata
```

无需类型专属 TreeItem 和 RenderObject；无需 GUI、渲染或导入时省略对应实现。

验收覆盖新数据与工况注册、分类及父子节点、内部子项与引用、菜单组合、空白和多选、勾选联动、多视图状态隔离、更新删除、撤销重做、注销与助手调用。新增功能只增加方案内实现与注册，通用 Model、View、Server 不增加具体业务判断。

## 12. 迁移与工程规范

当前代码的旧类型注册名称、专属树模型、集中菜单、具体 RenderObject 子类和 Controller 业务规则是迁移对象，不作为新方案约束。保留可复用解析与算法，删除被替代类和适配层，同步 CMake、测试、翻译和文档。未替代的日志、应用、插件等基础设施不机械删除。

编码、头文件自包含及翻译规则遵循 AGENTS.md。配置、编译及构建测试统一使用 E:\CJW-Archive\build\win-x64-debug-local，禁止源码内构建。

## 13. 实施进度（2026-10-12）

- 数据对象及井、网格、工况数据已迁入 core/data；新增独立 DataMetadata、DataRegistry 与内置类型 Metadata，由应用注册。删除旧 TypeRegistry、ObjectTypeMetadata 及其模板工厂。
- Project 的旧 addObject/takeObject/removeObject/objectById 接口已替换为 add/take/remove/object，全部源码和测试调用同步修改，不保留别名。
- CommandManager 直接拥有命令历史，失败不提交历史或清除重做记录，撤销失败不移动历史位置，结果在命令销毁后保留。删除 QUndoStack 和 QUndoCommand 依赖；命令使用 execute/undo，注册表通过 setRegistry 绑定。具体业务命令迁入 app 对应目录。
- RenderObject 已成为通用具体类，RenderMetadata 通过 create 创建 Provider，RenderServer 负责装配对象。删除井和网格的 RenderObject 子类，具体渲染 Provider/Metadata 迁入 app/well 和 app/grid。
- 平台插件接口、加载与管理设施已迁入固定目录；工况对话框迁入 app/case。源码目录边界检查已同步功能目录规则。
- 通用 Tree、TreeItem、TreeProvider、TreeMetadata、TreeRegistry、TreeModel 与 TreeServer 已实现；井、网格及工况分支由对应 Provider 构建，引用节点使用源数据注册的 Provider。旧专属模型、节点类、构建器、注册表及集中菜单接口已删除。
- 数据更新调用已有 TreeProvider 的 update，保留分支根节点的 ID、Provider 和持久索引；新分支调用 populate，分类或数据实例改变时重新装配。默认 update 调用 populate，具体 Provider 可覆盖以实现更细粒度更新。
- 工况统一使用 DataObject 的 typeId；模拟工况标识为 idos.case.simulation，删除重复 caseTypeId。删除无调用的 pendingImportPaths、IDOSItemState、IDOSColorMap 及旧 GUI 分层清单，目录回归检查采用当前模块边界。
- TreeGuiProvider 和 RenderGuiProvider 已实现，菜单保留实际目标视图和稳定对象标识；井、网格、工况菜单迁入对应业务目录。RenderController 和 DataObjectHandling 已删除，渲染业务联动迁入具体 GuiProvider。
- AssistantService 负责模型请求、工具协议和命令执行，Widget 负责界面。异步回复校验请求身份与绑定工程，切换工程取消旧请求。
- 数据解析 Registry 已删除 Impl 包装类，按注册顺序查找，拒绝重复类型并支持注销；内置格式由应用注册。旧 DataLoadService 已删除，LAS、ECLIPSE 与测井视图迁入固定目录。
- 井导入通过命令合并已有数据，保存修改前后快照，撤销恢复原通道，重做不重新读取文件。导入失败读取 CommandManager 保存的错误，避免访问已销毁的命令。
- 框架主体改造已落地，固定构建目录的 Debug 构建及 9 项回归测试全部通过。回归包含动态树注册、节点所有权、菜单目标、工程切换、渲染生命周期、井合并撤销重做及助手旧工程响应拒绝；独立头文件编译和目录边界检查通过，中文翻译无未完成项。这些检查覆盖现有内置实现，第三方新数据类型及插件卸载仍应按第 11 节逐项验收。
- idosexampleplugin 已完成点集扩展验收，只依赖公共 core/gui/render 接口，不链接 app。专项测试覆盖 DLL 加载、工厂和命令注册、失败命令不入历史、八个子节点、工况引用、树更新、实际三维截图、视图菜单与状态隔离、删除和撤销重做、旧菜单目标、跨工程及延迟删除的卸载保护、未关闭菜单时注销、工厂创建的游离实例与重新加载。完整 Debug 构建和 9 项回归测试通过，插件头文件独立编译、模块边界检查和中文翻译完成。此示例验证点集链路，其他新工况和业务扩展仍按第 11 节验收。
