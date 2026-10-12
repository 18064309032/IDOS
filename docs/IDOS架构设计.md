# IDOS 架构设计

更新日期：2026-10-13。

> 本文记录已确认架构与当前主要接口，后续目标不等于全部完成。签名以源码头文件为准，实施及验收状态见工作进度；直接改造现有框架，不保留两套调用链。工具框架已实现，具体内置工具待接入。

阅读入口：[项目概览](../README.md)、[开发指南](开发指南.md)、[源码目录](IDOS源码目录.md)、[工作进度](工作进度-CJW.md)。

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
| render | 通用渲染对象、Provider、Metadata、Registry、Server、Scene、View、VTK 与视图工具框架；不保留 RenderGuiProvider |
| app | 服务装配、内置具体树/渲染实现、导入命令和 UI 协调 |
| plugins | 专业数据、展示、交互和命令的具体扩展 |
| assistant | 模型进程、请求服务、命令调用及助手界面 |
| analysis / python | 保留的接口目标；算法及 Python 运行时待实现，未来业务修改走命令 |

providers、gui、render、assistant 面向 core。core 不依赖界面、渲染、app 或插件。gui 与 render 不互相依赖，通过工程事件和应用层协作。具体实现放在 app 或插件，通用模块不反向依赖具体实现。VTK 构造限于 render 和具体 RenderProvider。

源码目录以 [IDOS源码目录.md](IDOS源码目录.md) 的固定文件布局为准。公共接口位于所属功能目录根部，同名头文件与源文件成对放置。IDOSProject 位于 core 根目录；通用树结构、模型和同步位于 core/tree，树视图与 GUI 扩展接口位于 gui/tree；插件加载、描述、列表模型和管理界面位于 app 根目录。不设置 app/plugins 或 app/render，不保留 IDOSRenderController。目录调整须经用户确认。

## 3. 数据与工程

| 类 | 职责与主要接口 |
|---|---|
| IDOSDataObject | 数据 ID、名称、类型及通知；继承 objectId()、name()、setName()，提供 typeId() |
| IDOSProject | 数据与关系管理；add()、take()、remove()、object()、objects()、beginUpdate()、endUpdate() |
| IDOSDataMetadata | 类型描述及工厂；typeId()、displayName()、create() |
| IDOSDataRegistry | 类型注册；add()、remove()、metadata() |

工况也是具体业务数据，保存配置和数据引用。共享原始数据与工况专属配置分别表达；引用使用稳定数据 ID。数据拥有关系、引用关系和展示父子关系不能混用。

工程发布新增、删除前、删除、业务数据变化和批量通知。删除前通知在批量操作内仍立即发送，以解除展示关联。beginUpdate/endUpdate 只合并通知，不提供业务回滚。

数据、树节点、渲染对象分别管理 ID。同一数据可有多个引用节点和各视图独立的渲染对象。

## 4. 数据读写

| 类 | 职责与主要接口 |
|---|---|
| IDOSDataProvider | 一次解析；read(filePath)、lastError() |
| IDOSDataProviderMetadata | 格式描述、匹配和工厂；id()、displayName()、fileExtensions()、canHandle(filePath)、create() |
| IDOSDataProviderRegistry | 格式注册和匹配；instance()、add()、remove()、metadata()、create(id)、providers(filePath) |

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
| IDOSTreeView | 展示、选择与输入；currentIndex()、selectedItems()、menuItem() |
| IDOSTreeServer | 工程同步与引用刷新；setProject()、addTree()、removeTree() |

数据树与工况树使用同一框架，通过不同树实例、注册内容和根节点组织形成，不分别写带业务判断的 Model。

默认分组使用通用 TreeItem 表达，由 Registry.addGroup(treeName, group) 注册，不增加分组 Metadata/Context 类。对象分支结构由具体 Provider 构建。TreeItem 表达分组、对象、内部子项和引用，必要关联与所属工况在类内明确保存，操作不靠固定父节点形状猜测目标。

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
| IDOSRenderProvider | 几何、图例、数据依赖与显示状态；dataSource()、update()、dependsOn(dataIds)、removeDataReference(dataId)、actors()、legends()、visible()/setVisible()、highlighted()/setHighlighted()、opacity()/setOpacity() |
| IDOSRenderMetadata | 数据类型与 Provider 工厂；typeId()、create(data) |
| IDOSRenderRegistry | 工厂注册；add()、remove()、metadata() |
| IDOSRenderScene | 拥有展示对象；add()、remove()、clear()、object()、objects() |
| IDOSRenderView | 展示、相机、选择和拾取；scene()、selection()、setSelection(ids) |
| IDOSRenderServer | 工程同步；setProject()、addView()、removeView() |

Metadata 创建具体 RenderProvider，Server 装入通用 RenderObject 后交给 Scene。不增加仅装配 Provider 的具体 RenderObject 子类。每个 View 的 Scene、Object、Provider 和显示状态独立。

Provider 不访问工程或 Scene，数据关联非拥有。工程同步由 Server 处理，不引入 Provider Context、synchronize() 或数据 Provider 包装层。具体 Provider 用 dependsOn 描述额外数据依赖，用 removeDataReference 清理非拥有引用；Server 统一触发更新和 Scene 通知，不识别业务类型。显示模式、属性着色等专属能力只放具体 Provider，由对应功能或树 GuiProvider 调用。

一个对象可产生多个 actor；View 将拾取 actor 映射回 RenderObject，再通过 Provider 获取源数据。当前 actors() 覆盖普通几何；体渲染等需求在方案内现有类中扩充接口。

```text
工程新增数据 → RenderServer → Metadata 创建 Provider
  → 通用 RenderObject → Scene 接管 → View 展示
工程变化 → RenderServer → Provider 更新 → Scene / View 刷新
```

## 7. 菜单与交互

不增加 MenuContributor、ActionRegistry、InteractionProvider 或菜单组织器类。TreeView 在类内创建菜单、调用适用 GuiProvider、弹出和销毁菜单。按用户当前需求，RenderView 不启用右键菜单；渲染菜单、RenderGuiProvider 及其注册契约已删除，属性刷新与引用清理由 RenderServer 和具体 Provider 处理。

- 右键目标与选择分别查询；数据树菜单仅针对实际节点，空白处不进入菜单流程，也不回退到已选节点。通用 TreeView 默认关闭空白菜单，由应用仅为工况树开启必要的新建和导入入口。多个 GUI 实现共同贡献节点菜单，排序明确；通用视图动作由 View 或注册的通用 GUI 实现提供。
- 稳定服务依赖在构造或绑定时设置；populateMenu 只接收菜单和目标 View，通过明确关联获取目标及工程，不使用隐式全局“当前对象”。
- 局部显示操作以实际目标 View 和对象稳定 ID 为目标，不依赖事后变化的活动视图。
- QAction 与子菜单归本次 QMenu。动作保存稳定 ID，执行时重新校验；不长期保存节点指针，不共用 pending 目标成员。
- 菜单调用业务命令，局部显示操作调用目标 Provider；不接管 Server 同步和容器所有权。
- 动作连接具名成员或 on 开头的槽，不使用 lambda。

## 8. 命令体系

| 类 | 职责与主要接口 |
|---|---|
| IDOSCommand | 操作或查询类型、执行及撤销状态；validate(error)、execute()、undo()、preview()、result()、errorCode()、errorString() |
| IDOSCommandMetadata | 描述、JSON 参数规范和工厂；name()、title()、description()、schema()、create(arguments, project) |
| IDOSCommandRegistry | add(unique_ptr)、remove()、find(name)、metadata()、toToolJson() |
| IDOSCommandManager | 每工程独立历史；execute(command)、execute(name, arguments)、undo()、redo()、canUndo()、canRedo()、isClean()、setClean()、clear()、result()、error() |

Manager 构造时绑定工程，应用通过 setRegistry 绑定共用命令注册表。内部 C++ 可直接构造命令，助手通过名称和 JSON 参数创建；Python 运行时仍未接入。Manager 保存执行结果与错误，命令销毁后仍有效，不新增结果包装类。

创建、编辑、删除、导入合并及工况引用修改走业务命令。查询不进入历史；相机、截图、临时高亮直接操作 View，局部显示状态操作目标 Provider，不混入业务撤销历史。

- 命令绑定工程，参数使用数据 ID；工况相关操作明确工况目标。
- 验证、执行成功后才提交历史；失败不进入历史、不破坏重做记录；撤销失败不移动历史位置。
- 失败需恢复数据与关系，通知批处理不等于事务回滚。
- 删除命令拥有脱离工程的数据，撤销时恢复，清理历史时释放未恢复数据。
- 重做使用保存的解析/计算结果，不重新读取外部文件。
- 当前 CommandManager 直接拥有命令历史，不使用 QUndoStack 或 QUndoCommand 包装层。

## 9. 助手、插件与应用

IDOSAssistantService 负责模型请求、命令调用、结果处理与目标工程绑定；IDOSAssistantWidget 只负责输入和展示。现有模型进程和网络服务职责保留或迁入服务，不额外叠加包装类。

每次请求绑定发起时工程，工程切换后取消或使旧请求失效。助手通过命令 Metadata 获取描述及参数，经目标 CommandManager 执行，不直接修改节点、actor 或业务数据。显示操作明确目标 View 并使用应用公开能力。

应用负责服务装配、活动视图、树定位和工具栏协调。IDOSInterface 提供 project()、dataRegistry()、treeRegistry()、renderRegistry()、commandRegistry()、toolRegistry() 和 projectChanged()，插件通过公共入口调用，不访问主窗口内部实现。gui 仅前置声明渲染及工具 Registry，不增加对 render 的编译或链接依赖。

插件实现 canUnload()，检查存活数据、命令历史和自身持有的任务，包含脱离工程但尚未销毁的实例。PluginRegistry 拥有插件实例，拒绝不安全的卸载，不把卸载失败保存为禁用。具体 GuiProvider 注销时同步清理自己创建的 QAction，避免存活菜单保留已卸载 DLL 的代码与数据。

```text
树菜单 / 工具栏 / 脚本 / 助手
  → CommandManager → Project
  → TreeServer / RenderServer → 各自 Provider 更新展示
```

## 10. 所有权与注册生命周期

```text
Project → DataObject
Tree → 根 TreeItem → 子 TreeItem
对象分支根 TreeItem → TreeProvider
RenderView → Scene → RenderObject → RenderProvider
RenderView → ToolBar → ToolProvider → QAction / IDOSTool
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
IDOSFaultRenderMetadata / IDOSFaultRenderProvider
创建、编辑、删除断层命令及 Metadata
需要导入时增加对应格式 DataProvider / Metadata
```

无需类型专属 TreeItem 和 RenderObject；无需 GUI、渲染或导入时省略对应实现。

验收覆盖新数据与工况注册、分类及父子节点、内部子项与引用、菜单组合、空白和多选、勾选联动、多视图状态隔离、更新删除、撤销重做、注销与助手调用。新增功能只增加方案内实现与注册，通用 Model、View、Server 不增加具体业务判断。

## 12. 迁移与工程规范

当前代码的旧类型注册名称、专属树模型、集中菜单、具体 RenderObject 子类和 Controller 业务规则是迁移对象，不作为新方案约束。保留可复用解析与算法，删除被替代类和适配层，同步 CMake、测试、翻译和文档。未替代的日志、应用、插件等基础设施不机械删除。

编码、头文件自包含及翻译规则遵循 AGENTS.md。配置、编译及构建测试统一使用 E:\CJW-Archive\IDOS-Build\win-x64-debug-local，禁止源码内构建。

## 13. 视图工具框架接口设计（2026-10-13，通用框架已实现）

本节记录已确定并实现的通用工具框架；五个类 IDOSToolMetadata、IDOSToolRegistry、IDOSToolProvider、IDOSToolBar、IDOSTool 位于 src/render 根部。具体工具放在 app 所属功能目录或插件，本轮不注册内置业务工具。不增加 ToolManager、ToolServer、Context、包装层或命名空间，不恢复已删除的 RenderGuiProvider 和渲染右键菜单。验收状态以工作进度文档为准。

### 13.1 职责与所有权

| 类型 | 职责与所有权 |
|---|---|
| ToolMetadata | 功能定义、展示信息、适用性与工厂；Registry 拥有 |
| ToolRegistry | 全应用共用定义的注册注销；应用拥有，工具条借用 |
| ToolProvider | 一个视图中的功能集合；ToolBar 拥有；拥有 Action、菜单及交互工具 |
| ToolBar | 绑定视图后创建实例、排列入口；View 拥有，借用 Registry |
| IDOSTool | 具体交互及操作状态；Provider 拥有，非拥有地绑定 View |
| RenderView | 画布、工具条位置、多工具调度；非拥有地引用运行工具 |

```text
应用 → ToolRegistry → ToolMetadata
                          ↓ create(view)
View → ToolBar → ToolProvider → QAction / QMenu
                         └───→ IDOSTool
View ──非拥有引用─────────────→ 启用工具 / 捕获工具
```

功能定义共享；Provider、Action、IDOSTool 按窗口独立创建。Metadata 可以描述一个功能集合，不强制一个工具只有一个操作。View 不识别具体业务工具，Server 仍仅负责工程同步。

### 13.2 IDOSToolMetadata

以下声明省略 include、导出宏和防卫宏；正式代码遵守头文件自包含、函数在 cpp 实现等项目规范。

```cpp
class IDOSToolMetadata
{
public:
    virtual ~IDOSToolMetadata();
    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QString iconPath() const = 0;
    virtual QString groupId() const = 0;
    virtual int order() const = 0;
    virtual bool supports(const IDOSRenderView* view) const = 0;
    virtual IDOSToolProvider* create(IDOSRenderView* view) const = 0;
};
```

- id 非空且全局唯一，如 idos.view.navigation；groupId 为稳定分组标识，不翻译。注册期间 ID、分组、顺序不变；更新定义先注销再注册。
- name、description 由具体实现使用 tr；iconPath 为资源路径。一个入口时可采用默认展示信息；多个 Action 的文案和图标由具体 Provider 设置，工具条不覆盖各入口及菜单项的含义。
- supports 只读判断视图模式等适用范围；选择等瞬时条件由 Provider.updateState 管理，不随每次选择反复重建实例。
- create 返回绑定目标 View 的独立 Provider，失败返回 nullptr。ToolBar 校验目标后接管；非法返回实例清理并报告。Metadata 不保存 View、Action、运行状态；Provider 不长期借用 Metadata 内部数据。

### 13.3 IDOSToolRegistry

```cpp
class IDOSToolRegistry : public QObject
{
    Q_OBJECT
public:
    explicit IDOSToolRegistry(QObject* parent = nullptr);
    ~IDOSToolRegistry() override;
    bool add(IDOSToolMetadata* metadata);
    bool remove(const QString& id);
    void clear();
    bool isBusy() const;
    void attach(IDOSToolBar* toolBar);
    void detach(IDOSToolBar* toolBar);
    IDOSToolMetadata* metadata(const QString& id) const;
    QList<IDOSToolMetadata*> metadata() const;
signals:
    void metadataAdded(const QString& id);
    void metadataAboutToBeRemoved(const QString& id);
private:
    QMap<QString, IDOSToolMetadata*> m_metadataById;
    bool m_changing;
};
```

add 拒绝空指针、空 ID、重复 ID，成功接管所有权，失败不接管。remove 对未知 ID 返回 false，先同步通知各 ToolBar 清理实例，再删除定义；clear 使用同一路径。查询返回借用指针，不跨注销保存。m_changing 防止通知期间重入变更。注册、注销和实例装配均在 GUI 线程执行，注销前通知不得排队到 DLL 卸载之后。

isBusy 查询注册变更或已装配工具的 Qt/VTK 回调及生命周期切换是否正在执行；忙碌时 add/remove 拒绝变更，setRegistry 和 setToolBar 同样拒绝不安全换绑。Registry 内部非拥有地跟踪 ToolBar 用于注销保护，不成为第二个工具实例容器。插件加载设施在工具回调未退出时拒绝卸载 DLL。

remove 成功不等于整个插件可卸载；其他数据、历史、任务等仍由既有 canUnload 检查。异步工具在接入前另行明确取消、结束及卸载保护，不以同步注销冒充任务已结束。

### 13.4 IDOSToolProvider

```cpp
class IDOSToolProvider : public QObject
{
    Q_OBJECT
public:
    explicit IDOSToolProvider(IDOSRenderView* view);
    ~IDOSToolProvider() override;
    IDOSRenderView* view() const;
    virtual QList<QAction*> actions() const = 0;
    virtual QList<IDOSTool*> tools() const = 0;
    virtual void updateState() = 0;
private:
    QPointer<IDOSRenderView> m_view;
};
```

Provider 初始无 QObject 父对象，ToolBar 接管时设置父对象。Action、IDOSTool 以 Provider 为父对象。QMenu 是 QWidget，不能强制设普通 QObject Provider 为父对象；可以使用目标 View 作为 QWidget 父对象，同时由 Provider 在析构中显式销毁菜单，注销先关闭顶层及子菜单。不得因此留下菜单跨插件卸载存活。

actions 返回固定的顶层入口及顺序，返回指针不转移所有权；QAction 通过 QMenu 提供子 Action 和子菜单，菜单项不重复返回到顶层列表。tools 列出本实例拥有的全部交互工具；一次性操作返回空列表。装配后这两个集合稳定，菜单内容可动态更新。

updateState 只同步 enabled、visible、checked，不修改业务或启停工具；具体 Provider 使用具名 on 槽实现操作，监听需要的 View / Scene 状态及 toolEnabled/toolDisabled。勾选同步用 QSignalBlocker 避免重新触发。运行互斥由 View 决定，QActionGroup 仅反映对应入口的选择关系。

不保留此前建议的虚 deactivateTools：ToolBar 销毁 Provider 前遍历 tools，统一调用 View 停用，不依赖插件各自正确实现清理。工程依赖由应用或插件明确注入，不增加万能上下文。局部操作调用实际 View / RenderProvider，业务修改调用命令，Provider 不同步工程。

### 13.5 IDOSTool

```cpp
class IDOSTool : public QObject
{
    Q_OBJECT
public:
    explicit IDOSTool(IDOSRenderView* view);
    ~IDOSTool() override;
    IDOSRenderView* view() const;
    bool isActive() const;
    bool isBusy() const;
    bool setActive(bool active);
    virtual QString exclusiveGroup() const;
    virtual void cancel();
signals:
    void callbackFinished(bool consumed);
protected:
    bool eventFilter(QObject* watched, QEvent* event) override final;
    virtual bool onEventFilter(QObject* watched, QEvent* event);
    virtual void onActivated();
    virtual void onDeactivated();
    bool observeVtkEvent(vtkObject* source, unsigned long eventId, float priority = 0.0f);
    virtual void onVtkEvent(vtkObject* source, unsigned long eventId, void* callData);
private:
    void activate();
    void deactivate();
    void removeVtkObservers();
    void onVtkEventReceived(vtkObject* source, unsigned long eventId, void* callData);
    QPointer<IDOSRenderView> m_view;
};
```

方案确定采用 Qt 事件过滤器处理输入，VTK observer 监听必要渲染状态，不设置 EventMode、工具 priority 或自定义 handleEvent 分发链。exclusiveGroup 默认空，可并行；相同非空分组中的工具互斥，如 pointer-edit，启用期间分组不变。

不使用友元。View 更新启用列表并进入生命周期切换阶段后调用 setActive；该接口校验 View 正在切换、启用列表与请求一致且工具不忙碌，外部直接调用拒绝。内部私有 activate/deactivate 安装移除过滤器并调用生命周期钩子，统一清理 observer。eventFilter 为 final，具体工具覆盖 onEventFilter；isActive 查询启用状态，isBusy 查询回调是否未返回。callbackFinished 通知 View 输入消费及回调退出，由 View 自己处理待执行请求和点击状态，不直接读写其成员。cancel 清理未完成输入和临时预览，不提交未完成业务修改。

具体工具在 eventFilter 返回 true 消费 Qt 输入，false 继续 Qt 原有传递；QVTK 自行将未截断输入转换给 VTK，不手动 InvokeEvent 重复转发。多个过滤器后安装者先执行，采用启用安装顺序，不维护第二套优先级调度。View 自身激活、尺寸及取消相关框架监听须在工具之前得到必要通知；实施验证实际安装顺序，不能让工具消费输入导致焦点边框失效。

observeVtkEvent 使用 VTK 的 AddObserver 成员函数指针重载，直接监听到基类普通成员 onVtkEventReceived，再在忙碌保护范围内调用具体工具 onVtkEvent。不是 Qt 信号槽转换，不使用 SLOT、vtkEventQtSlotConnect、手写静态回调或 clientData。基类保存源对象弱引用、事件 ID 和 tag，停用及析构只移除自己添加的监听；源对象提前销毁时弱引用为空，跳过移除，不影响其他工具。相同源对象及事件重复注册拒绝。

VTK 回调同步执行，源事件约定在 GUI 线程触发，基类对其他线程事件不调用工具，不排队保存 callData 指针。callData 仅当前回调期间有效。Qt 信号继续使用成员函数指针，输入仍通过 Qt eventFilter，不重复转换或转发。工具不能在自身 Qt/VTK 回调内销毁自己或卸载 DLL，回调结束信号发出期间仍保持忙碌保护。

### 13.6 IDOSToolBar

```cpp
class IDOSToolBar : public QToolBar
{
    Q_OBJECT
public:
    explicit IDOSToolBar(QWidget* parent = nullptr);
    ~IDOSToolBar() override;
    bool setRegistry(IDOSToolRegistry* registry);
    IDOSToolRegistry* registry() const;
    IDOSRenderView* view() const;
    bool setView(IDOSRenderView* view);
    bool isBusy() const;
signals:
    void layoutChanged();
private slots:
    void onMetadataAdded(const QString& id);
    void onMetadataAboutToBeRemoved(const QString& id);
    void onRegistryDestroyed();
    void onViewStateChanged();
    void onViewModeChanged();
private:
    void rebuildTools();
    void addTool(const QString& id);
    void removeTool(const QString& id);
    void clearTools();
    void arrangeActions();
    void updateToolStates();
    void updateVisibility();
    QPointer<IDOSRenderView> m_view;
    QPointer<IDOSToolRegistry> m_registry;
    QMap<QString, IDOSToolProvider*> m_providers;
};
```

setRegistry 在未绑定 View 时只关联注册表，不创建实例；换绑前清理旧实例和连接。setView 校验目标 View 已通过 setToolBar 接管本实例，并拒绝跨窗口换绑，不能从外部独立建立第二条所有权路径。工具条内部管理父控件与可见状态，通过 layoutChanged 通知 View 重新定位，不读写 View 私有成员。Registry.attach/detach 只接受确实绑定本注册表的工具条，用于非拥有生命周期跟踪，不使用友元。

预设组顺序为 navigation、display、interaction、output；未知组按 groupId 排在后面。同组按 Metadata order、id 排列；功能内部按 actions 顺序排列。ToolBar 自己拥有分隔 Action，不识别具体工具子类。插件不直接 addAction 绕过注册跟踪。

工具条叠放在画布顶部居中，不占布局高度，不是独立 Dock。View 按画布宽度和工具条 sizeHint 定位；没有适用且可见入口时隐藏。用户主动隐藏不停止工具；自动显示不能覆盖用户隐藏偏好。窄窗口需要验证嵌入式 QToolBar 溢出行为，不能假定与 QMainWindow 完全相同；必要溢出菜单在本类实现，复用原 Action。工具条外不设阻挡整块画布输入的覆盖层。

### 13.7 IDOSRenderView 扩展

```cpp
public:
    bool setToolBar(IDOSToolBar* toolBar);
    IDOSToolBar* toolBar() const;
    QVTKOpenGLNativeWidget* vtkWidget() const;
    bool enableTool(IDOSTool* tool);
    bool disableTool(IDOSTool* tool);
    bool isToolEnabled(const IDOSTool* tool) const;
    QList<IDOSTool*> enabledTools() const;
    bool captureTool(IDOSTool* tool);
    void releaseTool(IDOSTool* tool);
    IDOSTool* capturedTool() const;
    bool isToolTransitioning() const;
signals:
    void toolEnabled(IDOSTool* tool);
    void toolDisabled(IDOSTool* tool);
    void viewStateChanged();
    void viewModeChanged();
private slots:
    void onToolDestroyed(QObject* object);
    void onToolBarDestroyed(QObject* object);
private:
    void positionToolBar();
    void cancelToolInteraction();
    void clearEnabledTools();
```

- setToolBar 拒绝绑定其他窗口的 ToolBar；失败不接管且保留旧实例。同指针重复调用成功但不重建。成功接管、内部绑定 View；nullptr 移除。替换先停用旧工具、释放捕获，再销毁旧工具条。View 不持有 Registry。
- enableTool 只接受绑定本 View 的工具，重复启用不重复 activate；先取消和退出同互斥组旧工具，再激活新工具。disableTool 对未启用实例返回 false；取消、释放捕获、deactivate、移出列表、发通知。View 保存 QPointer 引用，销毁通知仅防悬空，正常析构前必须停用。
- vtkWidget 返回借用画布指针供 IDOSTool 安装过滤器，不转移所有权；不允许工具替换渲染窗口或 interactor style。View 不逐个转发鼠标、滚轮和键盘事件。
- captureTool 只接受已启用工具，不能抢占其他工具捕获；View 管理鼠标获取及取消。鼠标获取保证跨边界输入回到画布，但不自动跳过其他 Qt 过滤器。捕获工具处理自己的连续指针输入并消费；其他工具发现捕获者不是自身时跳过相关处理。因此增加 IDOSTool* capturedTool() const 查询，明确检查规则，不引入自定义分发。释放、取消、失焦、画布隐藏、停用必须结束捕获。
- 普通输入经过 Qt 已安装过滤器，返回 true 后停止；并行启用不表示每个工具都会收到每个输入。需要持续更新的功能监听对应相机、选择或数据状态，不增加第二套广播。
- 工具仅接收约定的鼠标、滚轮、键盘等输入，不接收所有 QEvent。尺寸、绘制、边框、生命周期仍由 View 自己处理。工具条点击激活所在 View，但不能转成画布点击。
- Qt/VTK 回调中可以提出启停请求，但销毁工具、注销定义、替换工具条、关闭画布或卸载 DLL 必须等回调栈退出。异步请求通过生效信号更新 UI；不能让 deleteLater 的析构发生在 DLL 卸载之后。不自行实现事件快照迭代和分发队列。
- viewStateChanged 通知公开视角、显示及选择状态；viewModeChanged 用于适用工具重评估。实施时梳理现有 decorationsChanged、selectionChanged，不让同一工具重复订阅相同变化。Action 从 View 实际状态读回，Ribbon 或脚本修改同样反映到工具条。
- View 析构先停用工具、销毁 ToolBar，再释放画布和 Scene。ToolBar 自身析构也执行工具清理，处理直接销毁入口。

### 13.8 应用入口、调用及注销

IDOSInterface 新增 virtual IDOSToolRegistry* toolRegistry() const = 0；gui 仅前置声明类型，不链接 render。应用创建 Registry；主窗口 createView 装配 ToolBar，不识别具体工具。必须确保 Registry 最终销毁前完成视图和插件实例清理。

```cpp
IDOSToolBar* toolBar = new IDOSToolBar();
toolBar->setRegistry(toolRegistry);
if (!view->setToolBar(toolBar))
{
    delete toolBar;
}
```

注册链：插件 initGui → Registry.add → 各 ToolBar 判定 supports → Metadata.create(view) → 创建并排列 Action。新窗口绑定后装配已有定义。

一次性操作：Action → Provider 具名槽 → 实际 View / RenderProvider / 业务命令。持续交互：Action → Provider → View.enableTool(IDOSTool) → 安装 Qt 过滤器 / 注册必要 VTK observer。Ribbon 明确解析活动 View 后调用相同操作接口，不另写业务，也不改变局部工具绑定目标。

注销链：Registry.remove → 同步注销前信号 → 关闭相关菜单 → 从工具条移除 Action → 停用 Provider.tools 中全部工具并释放捕获 → 销毁 Provider / Action / 菜单 / IDOSTool → 销毁 Metadata → 完成其他插件清理 → 卸载 DLL。不依赖 deleteLater 才完成卸载前析构。

### 13.9 实施与验收

通用框架已实现，应用持有 Registry，创建 View 时装配空 ToolBar，插件入口已开放。测试具体类仅用于验证，不注册到产品，不增加产品业务按钮。验收计划包括空画布、无适用入口隐藏、分组菜单、窄窗口溢出、跨窗口绑定拒绝、隐藏与移除的区别、动态注册注销、Registry / View 销毁顺序、双窗口状态隔离、Ribbon 同步。具体已执行覆盖见工作进度，不把所有验收计划视为已经测试。

交互验收包括多个返回 false 的工具与相机操作并行、消费后停止分发、互斥切换、拖动捕获和跨边界释放、失焦取消、回调内启停及注销保护、打开菜单时卸载。真实工具若需要几何拾取等能力，按需求扩充现有 View 的明确接口，不暴露全部 VTK 成员。

## 14. 实施状态（截至 2026-10-13）

通用工具框架、成员函数 VTK 监听、回调注销保护及空工具条隐藏已实现；内置工具未注册。工程持久化、Python 运行时和拟合评价仍待实施。测试结果按工作进度中的具体日期和范围解释，不将历史全量通过记录套用于后续全部改动。

- 数据对象及井、网格、工况数据已迁入 core/data；新增独立 DataMetadata、DataRegistry 与内置类型 Metadata，由应用注册。删除旧 TypeRegistry、ObjectTypeMetadata 及其模板工厂。
- Project 的旧 addObject/takeObject/removeObject/objectById 接口已替换为 add/take/remove/object，全部源码和测试调用同步修改，不保留别名。
- CommandManager 直接拥有命令历史，失败不提交历史或清除重做记录，撤销失败不移动历史位置，结果在命令销毁后保留。删除 QUndoStack 和 QUndoCommand 依赖；命令使用 execute/undo，注册表通过 setRegistry 绑定。具体业务命令迁入 app 对应目录。
- RenderObject 已成为通用具体类，RenderMetadata 通过 create 创建 Provider，RenderServer 负责装配对象。删除井和网格的 RenderObject 子类，具体渲染 Provider/Metadata 迁入 app/well 和 app/grid。
- 平台插件接口、加载与管理设施已迁入固定目录；工况对话框迁入 app/case。源码目录边界检查已同步功能目录规则。
- 通用 Tree、TreeItem、TreeProvider、TreeMetadata、TreeRegistry、TreeModel 与 TreeServer 已实现；井、网格及工况分支由对应 Provider 构建，引用节点使用源数据注册的 Provider。旧专属模型、节点类、构建器、注册表及集中菜单接口已删除。
- 数据更新调用已有 TreeProvider 的 update，保留分支根节点的 ID、Provider 和持久索引；新分支调用 populate，分类或数据实例改变时重新装配。默认 update 调用 populate，具体 Provider 可覆盖以实现更细粒度更新。
- 工况统一使用 DataObject 的 typeId；模拟工况标识为 idos.case.simulation，删除重复 caseTypeId。删除无调用的 pendingImportPaths、IDOSItemState、IDOSColorMap 及旧 GUI 分层清单，目录回归检查采用当前模块边界。
- TreeGuiProvider 已实现，节点菜单保留实际目标视图和稳定对象标识；井、网格、工况菜单迁入对应业务目录。RenderController 和 DataObjectHandling 已删除。按当前需求删除渲染菜单及 RenderGuiProvider，属性依赖更新和引用清理由 RenderServer 与具体 Provider 处理。
- AssistantService 负责模型请求、工具协议和命令执行，Widget 负责界面。异步回复校验请求身份与绑定工程，切换工程取消旧请求。
- 数据解析 Registry 已删除 Impl 包装类，按注册顺序查找，拒绝重复类型并支持注销；内置格式由应用注册。旧 DataLoadService 已删除，LAS、ECLIPSE 与测井视图迁入固定目录。
- 井导入通过命令合并已有数据，保存修改前后快照，撤销恢复原通道，重做不重新读取文件。导入失败读取 CommandManager 保存的错误，避免访问已销毁的命令。
- 框架主体改造已落地，固定构建目录的 Debug 构建及 9 项回归测试全部通过。回归包含动态树注册、节点所有权、菜单目标、工程切换、渲染生命周期、井合并撤销重做及助手旧工程响应拒绝；独立头文件编译和目录边界检查通过，中文翻译无未完成项。这些检查覆盖现有内置实现，第三方新数据类型及插件卸载仍应按第 11 节逐项验收。
- idosexampleplugin 已完成点集扩展验收，只依赖公共 core/gui/render 接口，不链接 app。专项测试覆盖 DLL 加载、工厂和命令注册、失败命令不入历史、八个子节点、工况引用、树更新、实际三维截图、各视图状态隔离、删除和撤销重做、旧树菜单目标、跨工程及延迟删除的卸载保护、未关闭树菜单时注销、工厂创建的游离实例与重新加载。渲染菜单及相应测试已按当前需求移除；本次验证结果和未完成事项以工作进度文档为准。此示例验证点集链路，其他新工况和业务扩展仍按第 11 节验收。
