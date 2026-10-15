# IDOS 架构设计

更新日期：2026-10-16。

> 本文同时保存目标设计和现有实现说明。以本次新增的“2026-10-16 框架设计定稿记录”作为最新目标；旧章节中与其冲突的 GuiProvider、构造参数、Registry 命名及多视图装配描述属于待替换方案，不继续作为新实现依据。当前可调用签名仍以源码头文件为准，不能把文档接口当作已经实现。直接改造现有框架，不保留两套调用链。

阅读入口：[项目概览](../README.md)、[开发指南](开发指南.md)、[源码目录](IDOS源码目录.md)、[工作进度](工作进度-CJW.md)。

## 2026-10-16 框架设计定稿记录（待实施）

本节整理用户逐项确认的设计。此次只更新文档，不改源码、构建或文件位置。当前目录、现有 API 和已通过测试的行为继续作为迁移起点；完成改造后同步删除旧接口、旧类、构建项和测试调用，不并存两套实现。接口省略导出宏和普通成员，签名为目标约定。

### 1. 名称与职责

业务数据实体统一使用 `Object` 后缀，例如 `IDOSGridObject`、`IDOSWellObject`、`IDOSGridPropertyObject`、`IDOSNaturalFractureSetObject`。领域值类型不因这一规则机械追加 Object。通用基类保留 `IDOSObject`、`IDOSDataObject`；通用渲染对象保留 `IDOSRenderObject`，不增加只为装配 Provider 的 GridRenderObject/WellRenderObject 子类。

三组扩展能力统一如下：

| 范围 | 实例 | 描述及工厂 | 注册表 |
|---|---|---|---|
| 树 | IDOSTreeProvider | IDOSTreeProviderMetadata | IDOSTreeProviderRegistry |
| 渲染 | IDOSRenderProvider | IDOSRenderProviderMetadata | IDOSRenderProviderRegistry |
| 工具 | IDOSToolProvider | IDOSToolProviderMetadata | IDOSToolProviderRegistry |
| 命令 | IDOSCommand | IDOSCommandMetadata | IDOSCommandRegistry |

注册接口统一为 `registerMetadata(metadata)`、`unregisterMetadata(metadataId)`、`metadata(metadataId)`；创建接口为 `createProvider(...)` 或 `createCommand(...)`。不使用 C++ 关键字 register 作为方法名。

所有 Registry 的 QObject 信号统一为：

```cpp
void metadataRegistered(const QString& metadataId);
void metadataUnregistering(const QString& metadataId);
void metadataUnregistered(const QString& metadataId);
```

注册成功转移 Metadata 所有权并发 registered；注册失败不转移、不发成功信号。注销先发 unregistering，让相关实例与回调清理完成，再销毁 Metadata，最后发 unregistered。异步任务未完成时须拒绝/延后注销，不能仅凭信号已发出认定安全。Metadata 的 ID 是能力定义 ID，不等同于源数据 ID 或展示实例 ID。

Metadata 负责能力描述、匹配和具体实例创建；Registry 负责注册、查找及转调工厂，不持有创建出的 Provider/Command。注册项匹配规则与排序必须明确，不靠哈希顺序决定。

### 2. 树：每个节点都有自己的 Provider

根节点、固定分类、对象节点、内部节点、引用节点均拥有自己的 Provider 实例。节点属于同一业务也不共用有状态 Provider。

```text
Tree → 根 Item → 子 Item → 各自的 Provider
                         ↑ Provider 非拥有地回指所属 Item
```

TreeItem 管父子结构与 Provider 所有权。Provider 提供 id、dataId、name、displayName、icon、勾选状态等基本属性；Item 对外查询可以委托 Provider，不再保存另一套可独立修改的同义属性。name 为内部名称，displayName 为用户可见名称。固定分类没有源数据时 dataId 为空。节点/展示 ID 与源数据 ID 分开。

```cpp
explicit IDOSTreeProvider(IDOSTreeItem* item);
IDOSTreeItem* item() const;
QString id() const;
QString dataId() const;
QString name() const;
QString displayName() const;
QIcon icon() const;
virtual Qt::CheckState checkState() const;
virtual void setChecked(Qt::CheckState state);
virtual void refresh();
virtual QMenu* createContextMenu();
```

基础属性的存储、委托入口和构造次序在实现前统一，不能出现 Item::id 调 Provider::id 又反向调用造成递归。Provider 关联 Item 的指针此前示例称 m_node，正式命名统一为 m_item。Provider 不删除 Item。

取消并存的 populate()/update()，统一为 refresh()：首次准备缺少内容，后续更新已有内容、复用未变化节点，不默认 clearChildren 重建整支。TreeServer 负责结构装配及工程同步；Provider 说明具体节点内容/需要的子项，已挂接结构的增删改必须经过 Tree 的受控接口。具体子项工厂及 refresh 的结构修改边界仍需在实现前细化。

根节点的具体实现为：

```cpp
explicit IDOSDataTreeRootProvider(IDOSTreeItem* item);
explicit IDOSCaseTreeRootProvider(IDOSTreeItem* item);
```

不再给根 Provider 重复传 Project、Registry、TreeView 等；通过所属 Item、Tree 取得已有关系。根节点和固定分类可注册扩展，不能把插件可扩展分类写死。

Tree 构造绑定 Registry，保存非拥有 Project 关联；setProject 断开旧工程、连接新工程，清理旧展示并触发初始化。Server 每棵 Tree 一个实例，构造只绑定 Tree；从 Tree 取 Project/Registry，不重复保存关联，不保留多树 addTree/removeTree/setProject 调用链。

```cpp
explicit IDOSTree(IDOSTreeProviderRegistry* registry,
                  QObject* parent = nullptr);
void setProject(IDOSProject* project);
IDOSProject* project() const;
IDOSTreeProviderRegistry* registry() const;

bool insertItem(IDOSTreeItem* parent, IDOSTreeItem* item,
                int row = -1);
bool removeItem(IDOSTreeItem* item);
void setChecked(const QString& itemId, Qt::CheckState state);

explicit IDOSTreeServer(IDOSTree* tree,
                        QObject* parent = nullptr);
```

Tree 统一使用节点事件名称：

```cpp
void projectChanged();
void itemAdded(const QString& itemId);
void itemChanged(const QString& itemId);
void itemRemoving(const QString& itemId);
void itemRemoved(const QString& itemId);
void itemChecked(const QString& itemId, Qt::CheckState state);
```

itemRemoving 发出时节点仍有效。这里 itemId 是节点 ID，不写成 dataId；Project 的源数据事件保持两者概念区分。项目事件到 TreeServer 的具体转接信号、批量形式另外确定，不能把源数据新增直接冒充节点已经新增。

Qt Model 的 beginInsertRows/endInsertRows、beginRemoveRows/endRemoveRows 仍须在实际结构变更前后成对执行；完成事件不能取代前置通知。insertItem 必须先校验并完成可能失败的准备，再进入不可失败的结构提交和成对通知；row=-1 表示追加。

树数据新增流程：命令加入 Project → 工程事件经 Tree 交给 TreeServer → 查询匹配 Metadata 及父节点 → 创建 Item → Registry::createProvider(metadataId, item) → Item 接管 Provider → refresh/完成装配 → Tree::insertItem → Model 通知 View。Metadata 注册变化时为已有数据补建缺少展示；未注册的类型暂不生成节点。分类注销只移除展示，不删除源数据。

### 3. 勾选与实际显隐

选择、当前项与勾选分开；勾选针对 RenderServer 的当前视图。节点 Provider 的 setChecked 只传 Qt::CheckState，不反复传 item、TreeView、RenderView，也不保存 RenderServer。

```text
TreeModel::setData(index, value, CheckStateRole)
→ Item 对应 Provider::setChecked(state)
→ 所属 Tree::setChecked(itemId, state)
→ Tree::itemChecked(itemId, state)
→ TreeServer 接收并解释具体节点关联/请求
→ RenderServer::setVisible(dataId, visible)
→ 当前视图的 Scene 查找或创建展示，设置显隐
→ 实际结果通知树刷新勾选状态
```

TreeServer 若直接调用 RenderServer，需要位于 app；不允许在 core 中引用 render。该连接方案及相应目录迁移须在实施前最终确认，不暗中给 Provider 重新增加 Server 指针。分类批量、属性切换等专属语义由具体节点 Provider 明确提供，通用 Server 不判断井/网格等子类型。

itemChecked 在这条请求链中的含义必须与状态刷新区分：程序恢复状态/聚合三态不再次发出业务请求；PartialChecked 不能简单转换成 true。失败以后按实际显隐恢复，不能把发出请求或 void 返回视为操作成功。一次请求进入 RenderServer 时确定当前视图，操作中途不改用另一个视图。

### 4. 菜单：Provider 创建，View 扩展和显示

不再保留独立 IDOSTreeGuiProvider。先迁移其菜单、勾选、拖放等尚在使用的能力，再删除原类和调用链；不能仅删菜单接口而丢失其他行为。

```cpp
// 每个节点 Provider 已绑定自己的 Item。
virtual QMenu* createContextMenu();

// TreeView
IDOSTreeItem* contextItem() const;
QList<IDOSTreeItem*> selectedItems() const;
void contextMenuAboutToShow(QMenu* menu);
```

右键点击节点 → 该节点 Provider 创建基础菜单 → TreeView 发 contextMenuAboutToShow(menu) → 已连接插件检查 contextItem/selectedItems 并追加动作 → View 弹出并在关闭后销毁菜单。没有基础菜单时 View 可创建空菜单；空白处是否允许菜单由树的配置决定，不复用上次节点。

信号由 TreeView 发出，每次菜单创建后、弹出前发一次；插件连接建立一次，不逐节点连接 Provider。追加必须同步完成；动作、子菜单归本次 QMenu。Metadata 不增加 supportsContextMenu/populateContextMenu 一类菜单扩展职责。

动作捕获发起时工程、稳定数据/节点 ID 和必要目标，执行时重新校验，不长期保存 Item 裸指针。卸载前断开插件连接并清理尚存的插件 QAction，避免菜单继续调用 DLL。多选基础菜单组合规则尚需明确，不能直接套用单节点动作。

这是借鉴 QGIS 图层树菜单扩展的设计，不宣称 QGIS 浏览器采用每节点 Provider；QGIS 浏览器另有独立 DataItemGuiProvider。参考：[图层树菜单工厂](https://api.qgis.org/api/classQgsLayerTreeViewMenuProvider.html)、[图层树 View](https://api.qgis.org/api/classQgsLayerTreeView.html)。

### 5. 渲染：一个 Server 管理多个视图

```cpp
explicit IDOSRenderServer(IDOSRenderProviderRegistry* registry,
                          QObject* parent = nullptr);
void setProject(IDOSProject* project);
void addView(IDOSRenderView* view);
void removeView(IDOSRenderView* view);
IDOSRenderView* currentView() const;
bool setCurrentView(IDOSRenderView* view);
bool setVisible(const QString& dataId, bool visible);
```

不采用 RenderServer(scene) 每场景一个服务的方案。Server 非拥有地关联 Registry、Project 和多个 View；工程事件连接一次。每个 View 拥有独立 Scene，每个 Scene 拥有自己的 RenderObject，每个 RenderObject 拥有自己的 RenderProvider。Provider 只引用源数据，Project 持有源数据。

新建 View：创建空 Scene，登记到 Server，尚未勾选不创建全部 RenderObject/Provider。工程新增数据仅表示数据可用，不自动在所有视图创建或显示它。恢复已保存的视图状态时按该视图的显示需求装配。

首次显示：Server 在当前 Scene 按 dataId 查找 → 未找到则从 Project 取数据 → Registry 查 Metadata → Metadata::createProvider(data) → Server 创建通用 RenderObject 接管 Provider → Scene::addObject(object) → 准备必要几何、设显隐并通知 View。已存在则复用；请求隐藏且对象不存在时不创建。Registry/Metadata 不复制源数据，不把工程数据“转换掉”。

取消勾选隐藏并保留对象、几何及显示设置；再次勾选复用。源数据修改时更新或标脏，隐藏时可延迟重建；删除前清理对应对象及其他 Provider 的非拥有引用。Scene 使用 addObject/removeObject，按源数据 ID 查询展示实例；源数据 ID 与渲染 ID 区分。RenderObject 与 Provider 不重复维护同义属性，具体唯一存储/委托方式在实现前统一。

RenderProvider 不含 icon，保留 dataSource、显隐、不透明度、高亮、几何 actors/legends 和必要数据依赖接口，刷新统一称 refresh。Metadata 只描述能力及创建 Provider。完整 Renderer/拾取/依赖协议沿用未被替代能力，不因此次命名清理而丢失。

### 6. 工具：每个工具在每个视图中有自己的 Provider

保留 IDOSTool、IDOSToolBar；IDOSToolMetadata/IDOSToolRegistry 分别更名为 IDOSToolProviderMetadata/IDOSToolProviderRegistry。Registry 持有 Metadata；ToolBar 按所属 View 创建并持有各工具的 Provider；每个 Provider 管一个工具能力的 QAction 和交互实例，不用一个大 Provider 根据 Mode 分派全部工具。

不同 View 的工具 Provider 和交互状态独立，不共享有状态实例。工具 Provider 绑定所属视图以操作画布，不机械套用树 Provider 的 Item 参数；准确构造签名在工具调用链审查后确定。工具拥有动作、交互实例及 View 引用的既有生命周期保护继续保留。工具本身不是源数据，不要求无意义的 dataId。

### 7. Interface、插件算法及非模态 Widget

平台通过 IDOSInterface 公开 Project、MainWindow、RenderServer 等必要服务；TreeServer 访问能力在树归属和实例关系确认后提供。公共 Registry 归属统一由 MainWindow 装配的方向保留，不再让公共命令能力依赖 AssistantWidget；多窗口注册隔离的细节待确定。

插件算法功能窗口统一使用 QWidget 派生、Qt::Window、show() 非模态窗口，不使用 QDialog::exec。窗口构造接收 IDOSInterface*，由它取得主窗口父对象和实际平台服务，不重复传 Project、MainWindow、RenderServer、TreeServer 等参数。不得以 parent 替代业务上下文或强制转换主窗口内部实现。

名称保留功能和角色，不刻意使用 Diff/Eval 等缩写，目标名称为：

```cpp
IDOSDfnGenerationWidget
IDOSFractureCheckWidget
IDOSCurveFitEvaluationWidget
IDOSFieldDifferenceWidget

explicit IDOSDfnGenerationWidget(IDOSInterface* interface);
```

Widget 负责输入和结果展示；命令准备数据、校验和提交；纯算法只接收计算输入，不依赖 IDOSInterface、Project、Widget 或 Server。报告显示与视图定位/工程修改职责分开。窗口打开、提交任务时绑定当时工程和稳定目标 ID，不能操作中途重新读取 Interface 当前工程而误操作新工程。工程切换、源数据失效、视图销毁、模态文件选择和插件卸载保护不能因缩短构造函数而删除。

### 8. 命令：一个类负责一个业务操作，保留 Metadata

保留 IDOSCommand、IDOSCommandMetadata、IDOSCommandRegistry、IDOSCommandManager。命令按操作拆分，禁止不同 Metadata 最后全部创建一个带 Mode 的大命令；不采用注册静态工厂函数替代 Metadata 的提议。

```cpp
bool registerMetadata(IDOSCommandMetadata* metadata);
bool unregisterMetadata(const QString& metadataId);
const IDOSCommandMetadata* metadata(const QString& metadataId) const;
IDOSCommand* createCommand(const QString& metadataId,
                           IDOSProject* project,
                           const QJsonObject& arguments) const;
```

CommandMetadata 提供名称、说明、参数规范，工厂统一称 createCommand。注册时 new Metadata，执行时才创建独立 Command；Manager 持有进入历史的命令，查询不入历史。命令绑定明确 Project，不把 GUI 的当前工程动态查找带入执行中途。

导入命令目标拆分：IDOSImportCaseCommand、IDOSImportGridCommand、IDOSImportGridPropertyCommand、IDOSImportWellDataCommand、IDOSImportFluidRockTablesCommand、IDOSImportDataCommand、IDOSImportSummaryResultsCommand；现有独立 IDOSImportRestartResultsCommand 保留。每个命令对应一个 Metadata，放在该命令同名 .h/.cpp 文件对中，删除 IDOSImportObjectsCommand 的 Mode 分派和旧总文件引用。井数据和通用数据的最终拆分粒度按业务提交规则继续核对。

一个业务操作可以包含必要的关联修改，例如网格及配套结果整体导入、删除时清理引用；仍应原子成功、整体撤销/回滚。不同格式由 DataProvider 负责，不按扩展名复制命令；批量是否共用一次历史、失败回滚边界须明确。保留失败不进入历史、不破坏 redo，以及重做复用缓存的约定。

### 9. 目标目录与待收口事项

目录草案见 [源码目录中的目标布局](IDOS源码目录.md#2026-10-16-目标目录草案待迁移)。core/data 保留业务源数据及其类型 Metadata；顶层 providers 保留具体格式解析；render 保留通用场景、服务、视图、对象和工具框架，Provider/Metadata 在 render/providers；具体内置展示在 app/providers 各业务目录，具体工具在 app/providers/render；插件自有算法、Widget、命令及扩展均在所属插件目录。

树体系拟集中到 app/tree，以允许 TreeServer 协调渲染；具体根 Provider 分别在 app/providers/data 和 app/providers/case。该项是目录草案，尚未实施；直接依赖方案与目录迁移须一起最后确认。固定旧目录不能在实现途中随意变更。gui 业务窗口物理存放与 app 链接目标的既有划分不机械搬动。

实施前还需收口：树服务与渲染服务的最终连接；三态请求与实际状态通知细节；多选/批量和空白菜单；Provider/展示对象基本属性唯一归属和构造顺序；固定/业务子节点扩展匹配规则；注册失败所有权与活动实例注销；多窗口 Registry 隔离。数据类型工厂与格式工厂暂不合并，HDF5 编码职责迁移另行确认，不删除现有无源文件恢复能力。

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
| providers | 文件和外部数据源的具体格式解析实现；通用格式接口与注册位于 core |
| gui | 通用树 View、GuiProvider 和通用面板；平台公共接口位于 gui 根部 |
| render | 通用渲染对象、Provider、Metadata、Registry、Server、Scene、View、VTK 与视图工具框架；不保留 RenderGuiProvider |
| app | 服务装配、内置具体树/渲染实现、导入命令和 UI 协调 |
| plugins | 专业数据、展示、交互和命令的具体扩展 |
| assistant | 模型进程、请求服务、命令调用及助手界面 |
| analysis / python | 保留的接口目标；算法及 Python 运行时待实现，未来业务修改走命令 |

providers、gui、render、assistant 面向 core。core 不依赖界面、渲染、app 或插件。gui 与 render 不互相依赖，通过工程事件和应用层协作。具体实现放在 app 或插件，通用模块不反向依赖具体实现。VTK 构造限于 render 和具体 RenderProvider。

源码目录以 [IDOS源码目录.md](IDOS源码目录.md) 的固定文件布局为准。用户于 2026-10-14 确认按桌面 src 方案重新归类。同名头文件与源文件成对放置：core 根部保存基础对象、工程、命令和 TreeServer，core/tree 保存 Tree、TreeItem、TreeModel，core/providers 保存通用数据/树 Provider、Metadata、Registry，日志位于 core/logger；TreeView 与 TreeGuiProvider 均位于 gui/tree；渲染 Provider/Metadata 位于 render/providers，Registry 和通用框架位于 render 根部。具体展示实现按 well、grid、case、fluidrock、seismic、data 分入 app/providers 的业务目录，内置工具位于 app/providers/render；gui 按 tree、well、grid、simulation 分类；工况及通用窗口位于 gui 根部，IDOSInterface 位于 gui 根部并由 idos_gui 导出。具体命令位于 app/command，插件管理界面位于 app/plugin，插件 Metadata/Registry 位于 app 根部。不保留 IDOSRenderController。目录调整须经用户确认。

物理目录不等同于链接目标。gui 根部及业务目录归档的 20 组业务界面由 app/CMakeLists.txt 明确列入 idos_app，继续使用 APP_EXPORT；其余通用界面编入 idos_gui。业务界面可使用具体 app Provider，idos_gui 自身不依赖 app 或 render，通用公开头不得包含这些业务界面。边界检查依据 CMake 的明确文件清单判断归属，不按 gui 整目录放开依赖。此轮保持既有业务绑定，不引入额外适配层。

## 3. 数据与工程

| 类 | 职责与主要接口 |
|---|---|
| IDOSDataObject | 数据 ID、名称、类型及通知；继承 objectId()、name()、setName()，提供 typeId() |
| IDOSProject | 数据与关系管理；add()、take()、remove()、object()、objects()、beginUpdate()、endUpdate() |
| IDOSDataMetadata | 类型描述及工厂；typeId()、displayName()、create() |
| IDOSDataRegistry | 类型注册；add()、remove()、metadata() |

工况也是具体业务数据，保存配置和数据引用。共享原始数据与工况专属配置分别表达；引用使用稳定数据 ID。数据拥有关系、引用关系和展示父子关系不能混用。

独立导入的流体表与岩石物理函数表属于工程共享数据，在数据树的对应分类中各自作为数据对象管理；工况仅保存其对象 ID 引用，并在工况树中按引用展示。表数据不由工况拥有。

微地震事件集、压裂泵注程序、天然裂缝集和生产历史均作为工程数据对象，由各自的 Provider 解析后经导入命令加入工程，并由数据树具体 Metadata/Provider 放入对应分类。解析器保留来源文件和样例字段；格式兼容范围由工作进度记录。

这些数据对象及其 DataMetadata 位于已确认的 `core/data/fluidrock`；流体/岩石表的 TreeMetadata/TreeProvider 位于 `app/providers/fluidrock`，ECLIPSE 表文本解析 Provider 位于 `providers/fluidrock`。树节点的所属树不决定数据对象的目录或所有权。

工程发布新增、删除前、删除、业务数据变化和批量通知。删除前通知在批量操作内仍立即发送，以解除展示关联。beginUpdate/endUpdate 只合并通知，不提供业务回滚。

数据、树节点、渲染对象分别管理 ID。同一数据可有多个引用节点和各视图独立的渲染对象。

天然裂缝集的只读几何检查由现有 IDOSNaturalFractureSet 提供领域报告；具体 IDOSNaturalFractureRenderProvider/Metadata 位于 app/providers/data，由平台注册，在各视图构建独立的有效裂缝面及属性标识。裂缝建模插件的独立 IDOSFractureGeometryReportDialog 位于现有插件目录，负责报告布局、填充及行激活信号；插件入口负责检查动作、源对象选中与目标视图定位，复用上述检查，绑定发起时工程、数据 ID 与目标视图；卸载插件先清理报告及动作，不移除平台渲染能力。不向通用 TreeModel、RenderServer、Scene 或 View 添加裂缝类型判断。

## 4. 数据读写

| 类 | 职责与主要接口 |
|---|---|
| IDOSDataProvider | 一次解析；read(filePath)、lastError() |
| IDOSDataProviderMetadata | 格式描述、匹配和工厂；id()、displayName()、fileExtensions()、canHandle(filePath)、create() |
| IDOSDataProviderRegistry | 格式注册和匹配；instance()、add()、remove()、metadata()、create(id)、providers(filePath) |

通用 DataProvider、DataProviderMetadata、DataProviderRegistry 与 DataMetadata、DataRegistry 位于 core/providers；具体格式实现位于顶层 providers 的业务目录，依赖 core。

类型工厂回答“创建什么数据”，格式工厂回答“如何读取数据”。一个类型可有多个格式，一个格式可产生多种数据。手工或算法产生的数据不必经过 DataProvider。

每次解析创建独立实例，结果由调用方接管；解析器不修改工程、不创建树或渲染对象。导入命令负责校验、合并、替换、引用建立和提交。候选格式按明确规则选择，不依赖哈希遍历顺序。导出能力按实际需求在这些类内补充。

独立 ECLIPSE 网格导入由具体 GridEclipseProvider 按同目录、同完整主文件名收集 INIT、UNRST 及 SMSPEC/UNSMRY，返回网格、静态/动态属性与汇总数据。UNRST 的报告步解析位于既有 GridPropertyEclipseProvider，手动结果导入命令复用该接口。Provider 只绑定解析结果之间的数据 ID，不访问工程；grid.import 一次提交整组对象及工况引用，整组撤销与重做。配套文件错误或维度不匹配时释放未提交对象，缺失可选文件则只导入已存在的数据；不把日志或其他未支持文件当作业务数据。

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
| IDOSRenderServer | 工程同步与当前视图；setProject()、addView()、removeView()、currentView()、setCurrentView() |

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

- 对实际节点创建右键菜单时，先将该节点设为当前节点，再由工程同步当前对象；工况树还同步明确的当前工况 ID。空白处不进入数据树菜单流程，也不回退到已选节点。通用 TreeView 默认关闭空白菜单，由应用仅为工况树开启必要的新建和导入入口。多个 GUI 实现共同贡献节点菜单，排序明确；通用视图动作由 View 或注册的通用 GUI 实现提供。
- 稳定服务依赖在构造或绑定时设置；populateMenu 只接收菜单和目标 View，通过明确关联获取目标及工程，不使用隐式全局“当前对象”。
- 局部显示操作以实际目标 View 和对象稳定 ID 为目标，不依赖事后变化的活动视图。
- 工程保存当前对象和明确的当前工况 ID；渲染服务保存当前视图。结果播放依据当前工况定位共享网格及属性。显示设置面板以空构造和分步 setter 绑定工程、对象、视图及属性/筛选目标，菜单动作仍保存打开时的目标快照。
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

工况菜单的生成输入与属性曲线请求由主窗口直接连接至 IDOSSimulation，执行时校验目标工程与工况 ID；不经过插件接口信号中转。IDOSSimulation 由主窗口拥有，先于 Registry 和共享主页控件销毁，负责取消任务、关闭业务窗口和移除自有动作。通用视图工具及井显示菜单由主窗口注册。

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

编码、头文件自包含及翻译规则遵循 AGENTS.md。配置、编译及构建测试统一使用 D:\CMakeCoder\IDOS-Build，禁止源码内构建。

## 13. 视图工具框架接口设计（2026-10-13，通用框架已实现）

本节记录已确定并实现的通用工具框架；五个类 IDOSToolRegistry、IDOSToolBar、IDOSTool 位于 src/render 根部，IDOSToolMetadata、IDOSToolProvider 位于 src/render/providers。具体工具由所属 app 功能或插件注册；当前导航工具实现位于 app/providers/render，通用 IDOSRenderInteractionTool 位于 render 根部，由平台注册。工具条仍由应用随视图装配，无业务注册时自动隐藏。不增加 ToolManager、ToolServer、Context、包装层或命名空间，不恢复已删除的 RenderGuiProvider 和渲染右键菜单。验收状态以工作进度文档为准。

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

工具条叠放在画布顶部居中，不占布局高度，不是独立 Dock。View 按画布宽度和工具条 sizeHint 定位；没有适用且可见入口时隐藏。用户主动隐藏不停止工具；自动显示不能覆盖用户隐藏偏好。左侧 10px 为拖动把手，绘制右移 2px 的双列四行点阵并使用移动光标提示，按住左键可移动工具条；用户移动后保留位置，超出画布时约束到画布边界。当前工具数量较少，使用舒展图标直接展示，不增加自定义“更多工具”按钮和菜单。工具条外不设阻挡整块画布输入的覆盖层。

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

通用框架已实现，应用持有 Registry，创建 View 时装配 ToolBar，插件入口已开放。平台注册内置视图工具 Provider，提供视图复位、图例、适配全部对象、缩放至选中对象、三维标准视角、顺/逆时针旋转、正交投影、光标坐标追踪、视图联动、点缩放、点探针、距离测量、网格透视、背景颜色、隐藏/显示全部、截图及坐标轴操作；Provider 操作绑定所在 View，并从该 View 更新状态。交互工具由 Provider 持有并通过通用 View 生命周期接口启停。主页移除重复的视图分组。程序退出时同步注销 Metadata，清理所有视图中的工具实例；无工具注册时工具条自动隐藏。

交互验收包括多个返回 false 的工具与相机操作并行、消费后停止分发、互斥切换、拖动捕获和跨边界释放、失焦取消、回调内启停及注销保护、打开菜单时卸载。真实工具若需要几何拾取等能力，按需求扩充现有 View 的明确接口，不暴露全部 VTK 成员。

## 14. 实施状态（截至 2026-10-13）

通用工具框架、成员函数 VTK 监听、回调注销保护及空工具条隐藏已实现；视图工具由平台直接注册。工程持久化、Python 运行时和拟合评价仍待实施。测试结果按工作进度中的具体日期和范围解释，不将历史全量通过记录套用于后续全部改动。

- 数据对象及井、网格、工况数据已迁入 core/data；新增独立 DataMetadata、DataRegistry 与内置类型 Metadata，由应用注册。删除旧 TypeRegistry、ObjectTypeMetadata 及其模板工厂。
- Project 的旧 addObject/takeObject/removeObject/objectById 接口已替换为 add/take/remove/object，全部源码和测试调用同步修改，不保留别名。
- CommandManager 直接拥有命令历史，失败不提交历史或清除重做记录，撤销失败不移动历史位置，结果在命令销毁后保留。删除 QUndoStack 和 QUndoCommand 依赖；命令使用 execute/undo，注册表通过 setRegistry 绑定。具体业务命令迁入 app 对应目录。
- RenderObject 已成为通用具体类，RenderMetadata 通过 create 创建 Provider，RenderServer 负责装配对象。删除井和网格的 RenderObject 子类，具体井/网格渲染 Provider/Metadata 分别位于 app/providers/well 和 app/providers/grid。
- 平台插件接口、加载与管理设施已迁入固定目录；工况对话框位于 gui 根部，编入 idos_app。源码目录边界检查已同步功能目录规则。
- 通用 Tree、TreeItem、TreeProvider、TreeMetadata、TreeRegistry、TreeModel 与 TreeServer 已实现；井、网格及工况分支由对应 Provider 构建，引用节点使用源数据注册的 Provider。旧专属模型、节点类、构建器、注册表及集中菜单接口已删除。
- 数据更新调用已有 TreeProvider 的 update，保留分支根节点的 ID、Provider 和持久索引；新分支调用 populate，分类或数据实例改变时重新装配。默认 update 调用 populate，具体 Provider 可覆盖以实现更细粒度更新。
- 工况统一使用 DataObject 的 typeId；模拟工况标识为 idos.case.simulation，删除重复 caseTypeId。删除无调用的 pendingImportPaths、IDOSItemState、IDOSColorMap 及旧 GUI 分层清单，目录回归检查采用当前模块边界。
- TreeGuiProvider 已实现，节点菜单保留实际目标视图和稳定对象标识；井、网格、工况菜单迁入对应业务目录。RenderController 和 DataObjectHandling 已删除。按当前需求删除渲染菜单及 RenderGuiProvider，属性依赖更新和引用清理由 RenderServer 与具体 Provider 处理。
- AssistantService 负责模型请求、工具协议和命令执行，Widget 负责界面。异步回复校验请求身份与绑定工程，切换工程取消旧请求。
- 数据解析 Registry 已删除 Impl 包装类，按注册顺序查找，拒绝重复类型并支持注销；内置格式由应用注册。旧 DataLoadService 已删除，LAS、ECLIPSE 与测井视图迁入固定目录。
- 井导入通过命令合并已有数据，保存修改前后快照，撤销恢复原通道，重做不重新读取文件。导入失败读取 CommandManager 保存的错误，避免访问已销毁的命令。
- 框架主体改造已落地，固定构建目录的 Debug 构建及 9 项回归测试全部通过。回归包含动态树注册、节点所有权、菜单目标、工程切换、渲染生命周期、井合并撤销重做及助手旧工程响应拒绝；独立头文件编译和目录边界检查通过，中文翻译无未完成项。这些检查覆盖现有内置实现，第三方新数据类型及插件卸载仍应按第 11 节逐项验收。
- idosexampleplugin 已完成点集扩展验收，只依赖公共 core/gui/render 接口，不链接 app。专项测试覆盖 DLL 加载、工厂和命令注册、失败命令不入历史、八个子节点、工况引用、树更新、实际三维截图、各视图状态隔离、删除和撤销重做、旧树菜单目标、跨工程及延迟删除的卸载保护、未关闭树菜单时注销、工厂创建的游离实例与重新加载。渲染菜单及相应测试已按当前需求移除；本次验证结果和未完成事项以工作进度文档为准。此示例验证点集链路，其他新工况和业务扩展仍按第 11 节验收。

## 14. 平台数值模拟与主页入口

- 数值模拟是默认业务模块，专业插件计算模型、参数、属性场或分析指标，核心模拟流程负责工况输入组织、检查、运行和结果管理。模拟服务位于 app 根部，模拟窗口位于 gui/simulation并编入 idos_app，由主窗口直接装配 IDOSSimulation，不写入通用 Model、View、Scene 或 Registry。
- 数值模拟是平台内置功能，不参与插件发现、启用和卸载。通用插件 required 契约保留供其他扩展使用；原数值模拟 DLL 导出、独立构建目标和必需插件配置已删除。
- 主页固定顺序为“工程、窗口、模拟、结果、输出、扩展”。工况的新建及导入保留在工况树。移除独立数值模拟 RibbonPage 和主页视图分组。
- IDOSMainWindow.homeRibbonPanel(groupId) 提供主页 simulation、results、output 分组；IDOSSimulation 拥有模拟动作、播放控件及业务窗口。IDOSInterface 仍为其他插件开放主页 Panel 查询。模拟退出清理由平台直接执行，不涉及动态库卸载；其他插件继续执行既有安全卸载规则。
- 模拟分组承载 Deck、解析摘要、预检、初始化、运行状态/启动、停止和工作流；结果分组承载表格、属性曲线、井控查看、结果文件、报告步播放、曲线对比和拟合评价入口；输出分组承载 DATA 导出、运行归档和工况报告，并保留平台截图/图片导出。未实现的拟合评价继续禁用。
- 移除主页视图入口后，复位、图例、方向标、视角、适配及背景均由目标视图的悬浮工具条提供。共享主页模拟动作继续根据当前工况及任务状态更新，不将平台业务逻辑搬入 Ribbon 创建函数。

### 网格显示设置

- `IDOSGridSettingWidget` 位于 `src/gui/grid` 并编入 idos_app，直接替换旧着色面板；`IDOSGridTreeGuiProvider` 合并旧着色菜单职责并保留原有菜单和勾选，删除插件中的专用着色 GuiProvider 及注册链。通用 TreeView 不判断网格业务。
- 网格及网格属性共用面板。网格入口打开外观页并读取当前显示状态，属性入口打开着色页并预选属性。外观页提供表面、线框、表面加边线、点显示、显隐、不透明度、边线颜色和像素线宽；着色页保留属性、22 种色带、反转、范围、无效值颜色及图例。表面边线开关对应已有 Surface/SurfaceWithEdges 模式，不维护第二份边线显隐状态。
- 每视图 `IDOSGridRenderProvider` 保存显示模式、透明度、边线颜色和线宽，以及原有着色设置。高亮临时覆盖边线颜色和宽度，取消后恢复用户设置；属性、报告步、数据更新和网格透视恢复不重置外观。局部显示直接调用目标 Provider 并通知 Scene，不进入业务历史。
- 菜单保存发起时的工程、树、网格/属性 ID 与目标视图弱引用，执行时重新核对身份。改变活动视图不重定向旧菜单或面板。工程切换、网格/渲染对象移除或视图关闭使面板失效；属性移除回退单色。菜单 Provider 注销时同步销毁其面板和显示动作。
- App 内部直接调用具体 GridRenderProvider，不增加通用网格协议或包装层。删除旧插件着色代码的 app 依赖例外，井显示界面归档 gui 并编入 idos_app，不再需要跨插件依赖例外。剖切、裁剪和专业网格编辑不在本阶段实现。
## 15. 井输入数据解析

井分层、实测射孔和 Checkshot 文件解析由 `src/providers/well` 中的具体 Provider/Metadata 提供。新增 `IDOSWellTopsProvider/Metadata`、`IDOSWellCompletionProvider/Metadata`、`IDOSWellCheckshotProvider/Metadata`；应用启动注册后沿用已有井导入命令。`IDOSWellCheckshot` 位于 `core/data/well`，作为 Well 的值类型成员保存原始时深采样、单位及声明的深度/时间类型，不把时深记录转换为三维点。实测射孔深度统一为米，未映射网格层号为 0，日期保持源文本；导入合并和快照撤销包含射孔及校深数据。射孔工作簿的阶段、簇、孔数、孔径和其他来源列保存在完井记录扩展属性中，不映射为网格层号；井完井数据表追加显示这些来源列。查看入口由现有井 GuiProvider 提供。

## 16. 井显示设置

井口默认使用旧版 0～27 专用符号，保留点、球体和圆锥选项。Provider 在类内实现符号几何及预览，不增加符号工厂或包装层；自动模式优先读取已有 WellHead.symbol，无井头时使用井类型和开关状态回退，手动覆盖只改变目标视图。按旧编号划分生产、注入、关闭、停井配色；实测关闭射孔采用关闭配色，停井符号下沿用停井配色。显示设置提供四类井色、四类射孔色及沿用井色开关，也可关闭状态着色以使用各部分单色。井口符号、轨迹和射孔线默认启用 VTK 管状线及旧版光照参数，线宽仍使用像素，不引入物理管径几何。当前数据模型没有独立停井字段，停井由文件/显示符号表达；本阶段不迁入二维井组样式，不改变 MD 裁剪规则。

具体井节点及 idos.well.trajectory 节点提供“显示设置…”入口，其他井数据分类不增加该入口。app/providers/well 中的 IDOSWellDisplayTreeGuiProvider 组合菜单，IDOSWellDisplayWidget 提供共用面板，菜单与井渲染实现位于 app/providers/well，面板位于 gui/well 并编入 idos_app，直接调用具体 Provider。每视图 IDOSWellRenderProvider 保存权威显示状态，提供 displaySettings/setDisplaySettings；面板绑定打开时的工程、井身份和目标视图，局部修改不进入命令历史。支持井口形状、颜色和大小，轨迹颜色、宽度、平滑及 MD 裁剪，实测射孔颜色与宽度，以及井名标签。源数据保持不变；COMPDAT 网格层号不转换为实测射孔深度。数据删除、工程切换、视图关闭和平台清理时关闭面板并解除关联。

## 17. 网格筛选

IDOSGridFilter 继承 IDOSDataObject，保存源网格 ID 与 I/J/K 范围／勾选项、属性比较、条件组合及分别的网格／属性显示操作；属性比较支持七种方式、区间边界包含、无效值及相等容差。方向范围与勾选项取并集，三方向按并集／交集组合，网格和属性结果分别应用显示／隐藏／反选后取交集；IDOSGridFilterMetadata 注册数据工厂。两组成对文件位于 core/data/grid。原始网格几何不复制，单元选择为计算结果；缺失属性使筛选失效，不删除条件，也不因反选而显示所有单元。

IDOSGridFilterWidget 为独立筛选窗口，成对文件位于 gui/grid并编入 idos_app；沿用旧版网格筛选／属性筛选两页、方向列表和间隔选择、属性列表与条件编辑、范围拖动和直方图。IDOSGridSettingWidget 只保留外观／着色；网格右键“网格筛选…”打开筛选窗口进入临时预览；保存及另存为通过 IDOSGridFilterCommand/Metadata 的 create/update/delete 命令，命令成对文件位于 app/command，重命名使用现有命令。保存的结果由现有 IDOSGridTreeProvider 装配在源网格“筛选”分类下，现有 GridTreeGuiProvider 提供编辑、重命名、删除和独立勾选，不新增树模型或菜单层。

IDOSGridFilterRenderProvider 继承现有 GridRenderProvider 复用网格几何和外观，不包装另一 Provider，关联 Filter 数据并拥有各视图独立状态；IDOSGridFilterRenderMetadata 创建该 Provider，两组成对文件位于 app/providers/grid。RenderProvider 增加 dependencyIds/setDataReference 非拥有接口，由 RenderServer 统一解析和绑定依赖后调用 update，不识别具体子类。Grid Provider 的临时筛选也使用这条依赖绑定路径。动态条件按属性当前报告步重新计算，绘制单元附加原始单元 ID 和 I/J/K 数组。删除网格命令同时脱离筛选数据，撤销恢复；源数据重新绑定由 Server 处理。项目持久化仍未实现，保存仅进入当前工程。

主网格的临时预览保存成结果时，GridRenderProvider 记录对应结果 ID（仅目标视图的非拥有引用）；取消该结果勾选清除对应预览，恢复主网格完整活跃单元，保留显隐、着色和外观。后续应用其他临时条件解除旧关联，不误清其他预览。删除结果或撤销创建时由 RenderServer 的既有依赖移除流程通知 Provider 清理；通用层不增加业务判断。

## 18. 对象复制、剪切、粘贴与拖放（2026-10-14，需求纠正）

用户确认操作目标是数据、工况、网格和属性对象本身。先前“复制引用／剪切引用”菜单及其拖放语义不符合需求，已经替换。添加已有数据到工况使用独立“添加到工况…”菜单。

- TreeView 处理选中集合、菜单组合、Ctrl+C/X/V、Qt MIME 及拖放目标提示。TreeGuiProvider 缺省不支持业务操作；新增 populateSelectionMenu 供具体业务实现补充多选菜单。通用框架不判断领域类型。
- 具体 CaseTreeGuiProvider 解析同工程 application/x-idos-objects 载荷，确认实际对象和目标归属；objects.copy / objects.move 经 CommandManager 调用 IDOSTransferObjectsCommand。重命名共用 object.rename。关联入口调用 CaseReferenceCommand，既有 case.fluidRock.attach 仍复用它。
- DataObject 提供缺省不支持的 canClone/clone，具体数据类复制全部值类型字段并由构造分配新 ID，不复制 QObject 所有权及连接。工况不复制运行状态和输出目录。业务命令负责递归复制关系和重映射；Metadata 和 Server 不参与业务修改。
- 工况副本复制关联数据和私有网格，网格副本复制属性、动态报告步和筛选。筛选条件中的属性 ID 指向新属性。独立副本修改不影响源数据；同一复制事务内的重复依赖复用同一新对象。
- 网格可在工况间移动，属性可在相同尺寸网格间移动；依赖属性的筛选存在时拒绝属性移动。剪切只记源，粘贴或放置成功才改变归属。普通/Ctrl 拖动复制，Shift 拖动移动。失败不进入历史；批量通知不替代预检和回滚。
- 数据树分类和工况均直接属于工程，不新增未保存的任意父子关系和行间排序。固定分类之间不移动对象。井部件、跨工程、外部拖入及筛选单独跨网格复制不开放。
- 重命名使用对象 name，动态属性树也显示 name，保留用于模拟器的 keyword。相同类型、相同容器中拒绝同名对象。
- 生命周期补齐：删除工况私有网格时一并接管属性及筛选，撤销恢复。

验收结果及后续事项以工作进度文档最新记录为准；旧的引用交互验收不代表本次对象操作。

## 19. 单文件二进制工程持久化

用户确认只保留一个 .idos 工程文件，保存解析后的实际数据，不嵌入原始导入文件。工程信息和关系由 IDOS 定义，底层使用现有 VTK 随附 HDF5 库，不新增 ProjectStorage、读写管理器或另一套对象管理体系。

- IDOSProject::save / load 负责工程级流程、版本、身份、文件路径和未保存状态。load 返回独立临时工程，所有对象读完后调用具体数据校验关系，再由 MainWindow 切换。原工程不参与加载修改。
- IDOSDataRegistry 按 typeId 查找 IDOSDataMetadata。Metadata 的 writeProjectData / readProjectData 负责 HDF5 分发；具体对象和组合值类型通过 projectData / restoreProjectData 明确列出字段。未知或未实现的类型保存失败，不静默跳过。
- IDOSObject 只授予 Project 恢复原 ID 的友元权限，正常创建继续生成新 ID。新增具体 IDOSGridPropertyMetadata，补齐属性恢复的类型注册。
- 文件 /IDOS 保存格式 ID、版本 2、工程元信息、当前选择和对象索引；/Objects/<序号>/Data 保存具体对象数据。对象索引包含稳定 ID、typeId、name 和 visible。当前只写版本 2，兼容读取版本 1。
- 版本 2 的复合数据使用 Kind 21：Metadata 保存固定 Qt_5_15、小端 QDataStream 元数据，魔数为 0x49444f32；原生数组独立放入 A0、A1 等数据集。相同字段的记录列表按列编码，数值列直接作为原生数组，避免逐记录、逐字段创建 HDF5 对象。版本 1 的 Kind 子分组读取继续保留。解码限定内置标量类型，并校验深度、计数和数组引用。
- 浮点／整型数组直接保存为 HDF5 原生数据集，固定小端格式；达到 1024 项时分块、Deflate 1 压缩并带 Fletcher32 校验。Qt 容器不按 C++ 内存布局转储，不使用 JSON 或 ZIP 包装。
- 网格保留 COORD/ZCORN/ACTNUM，属性保留静态数值及全部动态报告步；井保留井头存在标记、空间参考、轨迹、曲线、层位、时深、完井、模拟井控及扩展字段；地震保存实际采样数组。HDF5 作为工程容器；当前没有生成标准 /VTKHDF 数据集或提供 ParaView 直接读取，后续互通另按 VTKHDF 标准实现，不能将两者混称为已完成。
- 保存先生成同目录临时 HDF5 文件，确认关闭成功后原子替换目标；正常磁盘路径不再完整复制临时文件。Windows 使用 MoveFileExW，Qt 临时文件先释放底层句柄；失败保留原目标。中文路径下，窄字符 HDF5 库无法打开时使用 Qt 文件读写与 HDF5 内存映像兼容，内存回退通过 QSaveFile 提交，仍受 QByteArray 的 2 GB 大小限制且额外占用内存。
- 加载在独立工程中批量恢复，校验后一次切换展示；日志分别记录保存准备、写入、提交，读取头部、恢复、校验以及界面重建耗时。当前读写仍为同步执行，尚未实现后台任务或进度取消。
- 加载校验文件版本、对象身份、字段类型、枚举值、数组尺寸、报告步和关系；拒绝外部链接、外部原始数据和虚拟数据集，确保工程数据自包含。筛选允许保留已丢失属性的条件，以支持既有“失效但保留编辑内容”的行为。
- MainWindow 接入打开、保存、另存为与 Save/Discard/Cancel 提示。Server 根据加载后的工程重建树和渲染；不增加具体业务特判或 Metadata 同步调用。UI 状态扩展见下节，业务撤销历史不持久化。
- 新增字段必须同步 projectData/restoreProjectData、类型版本与保存恢复测试；不依赖源文件重新导入来弥补未保存字段。

## 20. 工程 UI 状态与本机窗口设置

- IDOSProject 的 presentationState 为可选 QVariantMap，复用 .idos 版本 2 容器；UI 子版本为 1。兼容没有该字段的版本 1/2 工程，不新增存储管理类。工程数据先完成读取与校验，MainWindow 再装配视图集合，RenderServer 统一收集和恢复渲染状态。
- RenderServer 按稳定视图 ID、数据 ID、数据 typeId 定位，恢复时不使用重新生成的 RenderObject ID。RenderProvider 的 presentationState/restorePresentationState 提供通用显隐、不透明度和显示模式；具体井、网格和地震 Provider 扩展各自参数。Server 负责再次绑定属性等非拥有依赖，再更新展示；Metadata 不同步工程。
- RenderView 保存 2D/3D 模式、背景、方向、坐标轴、图例、光标跟踪、活动与主页相机、工具栏位置和显隐。联动在所有独立相机恢复后建立，并保留解除联动时的独立相机。相机数值和正尺度校验，异常 UI 条目回退默认或前一状态，不影响业务数据加载。
- TreeServer 提供由稳定 key 组成的路径，不持久化树节点临时 ID；TreeView 保存展开、多选、当前项及滚动位置，MainWindow 协调恢复，不向 core 引入 QWidget。树和渲染模块继续互不依赖。测井曲线窗口由现有 App 界面保存井 ID、通道和深度范围，切换工程时清理旧窗口再恢复。
- 恢复期间屏蔽工程 UI 修改标记与树选择联动；恢复完成后工程保持干净。用户修改显示、相机、视图集合或树状态会标记需要保存，不加入业务撤销栈。保存前再次收集快照；按稳定 ID 排序，避免运行时对象顺序差异误报变更。旧工程缺少 UI 快照本身不触发未保存提示，显式保存会补齐。
- 本机 ui.ini 位于 QStandardPaths::AppConfigLocation；保存主窗口几何、主窗口工具条状态、按视图集合区分的 ADS 停靠布局，以及最多 10 个最近工程。恢复窗口时检查屏幕可见范围；最近工程显示在启动页。IDOS_UI_SETTINGS_PATH 可指定隔离设置文件，IDOS_DISABLE_UI_SETTINGS 可禁用本机设置，供测试使用。
- IDOSProjectStartWidget 是 gui 根部的独立通用 QWidget，由 idos_gui 编译；无工程时在主窗口中央展示新建、打开和最近工程入口，工程打开后由 app 主窗口切换为渲染视图。该类只展示最近路径并通过信号请求操作，不读取本机设置或依赖 app。
- 本阶段不保存业务撤销历史、运行中的计算任务或临时业务对话框；读写仍同步执行，后台任务与崩溃恢复另行实施。

共享导入数据的删除由 app/command 中 IDOSDeleteDataCommand 及 Metadata 提供 data.delete 命令；具体 IDOSImportedDataTreeGuiProvider 提供单选、多选菜单和确认，捕获目标工程与数据 ID。命令预校验全部目标、保存并清理受影响工况引用，持有脱离工程的数据以支持整批撤销/重做；失败回滚，不增加历史。TreeServer、RenderServer 继续根据工程信号独立清理和重建展示，通用 Model/View 不判断具体数据类型。井、网格和属性保留其专用删除命令。

数据树分类的批量显示由 app/providers/data 中 IDOSDataGroupTreeGuiProvider 提供，只支持 data 树的分类组。它按其他具体 GuiProvider 的可勾选节点收集后代数据 ID，并在目标视图批量设置现有 RenderObject 显隐、聚合三态；遇到可显示对象时不继续操作其内部属性节点。通用 TreeItem 保存 Qt::CheckState，TreeModel 透传，TreeView 从 GuiProvider::checkState 获取状态，默认接口保持已有布尔行为；不在通用框架判断数据类型或树分类，也不对工况树增加分类勾选。

DFN 生成的 IDOSDfnGenerationWidget、IDOSDfnGenerator、IDOSDfnGenerateCommand 及 Metadata 全部位于裂缝建模插件原目录。Widget 管理参数输入与结果提示，Generator 仅计算 double 坐标的矩形裂缝记录，命令负责预校验、生成后几何验证、工程提交及缓存结果的撤销/重做。通过 fracture.dfn.generate 向平台提交已有 IDOSNaturalFractureSet，不增加平台专属数据、算法或渲染包装层；参数随裂缝记录持久化，发布到平台前复制嵌套字符串存储以脱离插件静态字面量生命周期。插件维护窗口生命周期，注册/注销命令；命令实例计数覆盖撤销、重做及外部持有的未执行命令，实例存活时禁止常规卸载 DLL，平台裂缝数据本身不阻止卸载。


### 多尺度裂缝插件分析功能的当前实现（2026-10-16）

用户确认的三场差异、生产曲线拟合评价分别使用插件内独立 Widget，菜单动作仅负责打开窗口。差异计算通过 `analysis.field.difference` Action 命令修改工程，生成平台普通网格属性，按命令历史撤销/重做；评价通过 `analysis.curve.evaluate` Query 命令返回指标及配对样本，不修改工程、不进入撤销历史。曲线读取和数值评价属于插件算法类。Core 只提供报告步的只读访问，不依赖插件。

插件持有窗口弱引用，工程切换和卸载时清理窗口，卸载前注销命令 Metadata；仍存活的命令（含撤销/重做历史）或文件对话框阻止 DLL 卸载，避免悬空调用。差异结果通过平台 `IDOSGridPropertyMetadata::create()` 创建，避免 MSVC 在插件内直接构造导入类时生成指向插件的局部虚函数表。结果为平台数据类型，清空历史并卸载插件后可继续显示、保存及关闭工程。
