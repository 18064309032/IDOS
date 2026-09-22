# IDOS UML 与架构图

适用：`D:\CMakeCoder\IDOS`。2026-09-22，目标设计，不是当前源码自动生成图。

配套：[目录与类文件组织](IDOS-目录与类文件组织.md) · [架构设计方案](IDOS-架构设计方案.md)。

以下采用 Mermaid。模块图是依赖图；类图和时序图表达 UML 关系。实心菱形表示生命周期所有权，普通关联和带说明箭头表示引用/使用，不代表复制数据。各图为分视角表达，省略 Qt 基类及部分成员。

## 1. 模块依赖图

箭头表示“依赖/调用”，不是对象变化通知方向。仅绘制主要直接依赖；App 的装配依赖还包括 Core 和 Render 公共类型。

```mermaid
flowchart TB
    MAIN["main.cpp"] --> APP["app：装配、会话、操作、命令、任务"]
    APP --> GUI["gui：树、属性面板、工作区、专业窗口"]
    APP --> PROVIDERS["providers：格式识别与解析"]
    APP --> CORE["core：工程、专业数据、工况、工程读写"]
    APP --> RENDER["render：唯一渲染模块"]
    GUI --> CORE
    GUI --> RENDER
    RENDER --> CORE
    PROVIDERS --> CORE
    RENDER --> VTK["VTK"]
    PROVIDERS --> OPM["OPM 等格式库"]
    APP -.-> ANALYSIS["analysis：后续专业算法"]
    ANALYSIS -.-> CORE
    APP -.-> PY["python：后续脚本运行时"]
    APP -.-> AI["assistant：后续助手工具"]
```

Python/Assistant 不反向链接 App。App 在装配时向它们注册回调或其公共接口的实现，避免双向模块依赖。Render 内部包含 adapters，不另建渲染适配模块。

## 2. 领域对象 UML 类图

```mermaid
classDiagram
    class IDOSObject {
        +objectId() QString
        +name() QString
    }
    class IDOSDataObject {
        <<abstract>>
        +typeId() QString
        +revision() quint64
    }
    class IDOSProject {
        +objectById(objectId) IDOSDataObject*
        +addObject(object, errorMessage) bool
        +removeObject(objectId, errorMessage) bool
        +objectChanged(objectId)
    }
    class IDOSTypeRegistry {
        +create(typeId) IDOSDataObject*
    }
    class IDOSWell
    class IDOSWellHead {
        <<value>>
    }
    class IDOSWellPath {
        <<value>>
    }
    class IDOSWellPathPoint {
        <<value>>
    }
    class IDOSWellLogSet {
        <<value>>
    }
    class IDOSWellLogChannel {
        <<value>>
    }
    class IDOSWellMarkerSet {
        <<value>>
    }
    class IDOSWellMarker {
        <<value>>
    }
    class IDOSGrid {
        +cell(i, j, k) IDOSGridCell
    }
    class IDOSGridProperty {
        +gridId() QString
        +gridRevision() quint64
    }
    class IDOSSimulationCase {
        +gridId() QString
        +wells() CaseWellValues
    }
    class IDOSCaseWell {
        <<value>>
        +wellId QString
        +gridLocation
    }
    class IDOSWellControl {
        <<value>>
    }
    class IDOSWellCompletion {
        <<value>>
    }
    class IDOSWellSegment {
        <<value>>
    }

    IDOSObject <|-- IDOSDataObject
    IDOSDataObject <|-- IDOSWell
    IDOSDataObject <|-- IDOSGrid
    IDOSDataObject <|-- IDOSGridProperty
    IDOSDataObject <|-- IDOSSimulationCase
    IDOSProject "1" *-- "0..*" IDOSDataObject : 拥有
    IDOSTypeRegistry ..> IDOSDataObject : 创建
    IDOSWell *-- IDOSWellHead
    IDOSWell *-- IDOSWellPath
    IDOSWellPath *-- "0..*" IDOSWellPathPoint
    IDOSWell *-- IDOSWellLogSet
    IDOSWellLogSet *-- "0..*" IDOSWellLogChannel
    IDOSWell *-- IDOSWellMarkerSet
    IDOSWellMarkerSet *-- "0..*" IDOSWellMarker
    IDOSGridProperty --> IDOSGrid : gridId 与修订号
    IDOSSimulationCase --> IDOSGrid : ID 引用
    IDOSSimulationCase --> IDOSGridProperty : 属性 ID 引用
    IDOSSimulationCase *-- "0..*" IDOSCaseWell
    IDOSCaseWell --> IDOSWell : wellId 引用
    IDOSCaseWell *-- IDOSWellControl
    IDOSCaseWell *-- "0..*" IDOSWellCompletion
    IDOSCaseWell *-- "0..*" IDOSWellSegment
```

井和网格是 Project 的独立对象。工况的井配置按值持有，不能通过 QObject parent 把共享井改挂到工况。曲线通道第一版是井内部值数据；视图定位使用井 ID 加持久化子记录键，不将可编辑的显示名当永久键。

图中的 `revision` 用于变化检测和缓存失效，不代表已经实现可回溯版本仓库。第一版方案隔离采用显式派生对象，未来的运行复现另需保存输入快照。

## 3. 窗口与渲染 UML 类图

```mermaid
classDiagram
    class IDOSMainWindow
    class IDOSWorkspace {
        +activeView() IDOSDocumentView*
        +openView(viewType)
        +closeView(viewId)
    }
    class IDOSViewFactory {
        +create(viewType) IDOSDocumentView*
    }
    class IDOSDocumentView {
        <<abstract>>
        +viewId() QString
        +viewType() QString
        +canDisplay(objectId) bool
        +displayObject(objectId) bool
        +saveState() QByteArray
    }
    class IDOSRenderView
    class IDOSTableView
    class IDOSWellLogView
    class IDOSRenderWidget {
        +showObject(objectId) bool
        +setObjectStyle(objectId, style) bool
        +setCamera(camera)
        +selectionChanged(selection)
    }
    class IDOSRenderScene
    class IDOSRenderObject {
        -objectId
        -sourceRevision
        -vtkResources
    }
    class IDOSRenderStyle {
        <<value>>
        +visible
        +propertyId
        +colorRange
    }
    class IDOSGridRenderAdapter
    class IDOSWellRenderAdapter
    class IDOSProject
    class IDOSSelection

    IDOSMainWindow *-- IDOSWorkspace
    IDOSWorkspace *-- "0..*" IDOSDocumentView
    IDOSWorkspace --> IDOSViewFactory
    IDOSViewFactory ..> IDOSDocumentView : 创建
    IDOSDocumentView <|-- IDOSRenderView
    IDOSDocumentView <|-- IDOSTableView
    IDOSDocumentView <|-- IDOSWellLogView
    IDOSRenderView *-- IDOSRenderWidget
    IDOSRenderWidget *-- IDOSRenderScene
    IDOSRenderScene *-- "0..*" IDOSRenderObject
    IDOSRenderObject *-- IDOSRenderStyle
    IDOSRenderScene ..> IDOSGridRenderAdapter : 内部转换
    IDOSRenderScene ..> IDOSWellRenderAdapter : 内部转换
    IDOSRenderWidget --> IDOSProject : 非拥有绑定
    IDOSDocumentView --> IDOSProject : 非拥有绑定
    IDOSWorkspace --> IDOSSelection : 同步选择
```

上图的组合表示逻辑生命周期所有权，具体 QObject 使用 Qt 父子关系，非 QObject 使用 RAII。存在异步销毁风险的非拥有 QObject 引用使用 QPointer。

每个 RenderWidget 创建自己的 RenderScene 和样式；不同窗口引用同一业务对象时，不共享可变 Actor 或样式。可在后续共享不可变几何缓存。GUI 的 IDOSRenderView 是工作区窗口，不是第二套渲染实现。

## 4. 应用会话、导入与命令 UML 类图

```mermaid
classDiagram
    class IDOSApplication
    class IDOSMainWindow
    class IDOSSession {
        +project() IDOSProject*
        +generation() quint64
    }
    class IDOSProject
    class QUndoStack
    class QUndoCommand
    class IDOSImportOperation
    class IDOSImportObjectsCommand
    class IDOSRenameObjectCommand
    class IDOSRemoveObjectsCommand
    class IDOSTaskQueue
    class IDOSTask {
        <<abstract>>
        +requestCancel()
    }
    class IDOSImportTask
    class IDOSProviderRegistry
    class IDOSDataProvider {
        <<interface>>
        +read(request, batch, errorMessage) bool
    }
    class IDOSImportBatch {
        <<value>>
        +parsedRecords
        +externalReferences
    }

    IDOSApplication *-- IDOSMainWindow
    IDOSApplication *-- IDOSSession
    IDOSApplication *-- IDOSTaskQueue
    IDOSSession *-- IDOSProject
    IDOSSession *-- QUndoStack
    QUndoStack *-- "0..*" QUndoCommand
    QUndoCommand <|-- IDOSImportObjectsCommand
    QUndoCommand <|-- IDOSRenameObjectCommand
    QUndoCommand <|-- IDOSRemoveObjectsCommand
    IDOSImportOperation --> IDOSSession : 校验和提交
    IDOSImportOperation --> IDOSTaskQueue : 提交任务
    IDOSTaskQueue *-- "0..*" IDOSTask
    IDOSTask <|-- IDOSImportTask
    IDOSImportTask --> IDOSProviderRegistry
    IDOSImportTask --> IDOSDataProvider
    IDOSDataProvider ..> IDOSImportBatch : 填充纯数据
    IDOSImportOperation ..> IDOSImportObjectsCommand : 创建一次提交
    IDOSImportObjectsCommand --> IDOSProject : redo 与 undo
```

Provider 的 request 表示路径、取消与进度上下文的设计占位，具体接口在后台导入实现时收敛。纯数据包不携带工程、窗口或跨线程 QObject 指针。

## 5. 后台导入 UML 时序图

```mermaid
sequenceDiagram
    actor User as 用户
    participant UI as ImportActions（GUI线程）
    participant OP as ImportOperation（GUI线程）
    participant TASK as ImportTask（工作线程）
    participant P as DataProvider（工作线程）
    participant S as Session / UndoStack
    participant CORE as Project
    participant VIEW as 树与工作区窗口

    User->>UI: 选择文件与冲突策略
    UI->>OP: 发起导入
    OP->>S: 记录工程ID、会话代次、相关修订号
    OP->>TASK: 提交解析任务
    TASK->>P: 解析文件
    P-->>TASK: ImportBatch 或错误
    TASK-->>OP: 排队通知完成（纯数据）
    OP->>S: 校验工程仍打开、输入未过期
    alt 取消、解析失败、工程已切换或冲突未解决
        OP-->>UI: 释放暂存数据并反馈，工程不变
    else 校验通过
        OP->>OP: 构建对象、匹配身份、解析整批引用
        OP->>S: push ImportObjectsCommand
        S->>CORE: redo：提交一次完整修改
        CORE-->>VIEW: 批量对象变化通知
        OP-->>UI: 导入完成
    end
```

主线程不得等待工作线程完成解析。构建对象只做轻量封装，昂贵转换和大数组处理在后台完成；提交阶段使用移动/共享不可变块降低复制。发生提交失败时整批回退，不能留下半个工况。

## 6. 保存与恢复流程图

```mermaid
flowchart LR
    A["业务对象与引用"] --> S["一致的数据快照"]
    B["工作区窗口状态"] --> S
    S --> W["ProjectIO 写临时工程文件"]
    W --> C["校验并原子替换目标文件"]
    C --> R["读入临时 Project"]
    R --> V["校验 ID、引用、版本、数据块"]
    V --> P["Session 切换到新工程"]
    P --> U["按窗口类型恢复工作区"]
```

Core 只把窗口状态视为有版本的不透明字节数据，不认识 QWidget。未知窗口类型可以跳过并提示；未知业务对象类型不能静默丢弃后保存覆盖原工程。
