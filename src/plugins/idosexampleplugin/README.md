# 点集扩展示例

本插件用立方体的八个顶点验证新数据类型的注册、树节点、菜单、命令和渲染扩展。只链接 `idos_core`、`idos_gui`、`idos_render` 等公共依赖，不链接 `idos_app`。

| 职责 | 实现 |
| --- | --- |
| 数据与工厂 | IDOSPointSet、IDOSPointSetMetadata |
| 树结构与注册 | IDOSPointSetTreeProvider、IDOSPointSetTreeMetadata |
| 树菜单 | IDOSPointSetTreeGuiProvider |
| 渲染与注册 | IDOSPointSetRenderProvider、IDOSPointSetRenderMetadata |
| 视图菜单 | IDOSPointSetRenderGuiProvider |
| 业务命令 | IDOSCreatePointSetCommand、IDOSDeletePointSetCommand 及各自 Metadata |
| 注册和注销 | IDOSExamplePlugin |

## 使用

1. 创建工程，启用“示例”插件。
2. 在“示例”页点击“创建点集”，或在数据树空白菜单中创建。
3. 数据树显示点集和八个点，三维视图显示八个顶点。
4. 在视图右键菜单中“增大点尺寸”，只修改当前视图。
5. 树或视图菜单中“删除点集”，再通过平台撤销、重做验证同步。

脚本和助手可通过同一命令注册表调用 `pointset.create`（参数 `name`）和 `pointset.delete`（参数 `objectId`）。创建命令返回 `objectId`、`pointCount`。

## 生命周期

插件跟踪切换过的工程和数据工厂创建的实例。有点集数据或点集命令历史存活时，拒绝卸载，包含已撤销但仍可重做的命令、未加入工程的实例和等待延迟删除的实例。卸载检查会说明阻止卸载的具体原因；命令历史清空前，插件管理器保持其勾选状态。清除相关数据和命令历史且对象完成销毁后才能注销并卸载 DLL。删除点集保留工况引用，显示为缺失对象；撤销恢复数据后引用重新显示。

GuiProvider 注销时同步删除自己添加的 QAction，避免存活菜单保留插件回调。传递到平台并可能跨越 DLL 生命周期的标识使用独立字符串存储。

Windows x64/MSVC 导出表将插件类的静态成员映射到现有加载器入口，避免增加自由函数。

## 验证

`case_workflow` 中的 `onExamplePlugin` 从 DLL 加载插件，通过公共命令接口创建、删除和恢复点集，检查树节点和 VTK 数据、独立视图菜单、旧菜单目标、跨工程卸载保护、注销与重新加载。渲染截图保存到构建目录的 `tests/example-pointset.png`。
