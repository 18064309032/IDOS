# 工作记录-CJW

## 2026-09-24

本次主要整理并固化 IDOS 新架构方向，删除上一版架构文档中的旧设计表述，改为当前确定的方案。

完成内容：

- 明确平台采用“通用平台 + 业务扩展”的设计方向。
- 明确数据树和工况树是两棵不同业务树，不再使用 Input/Model 旧分类表达平台结构。
- 明确数据树、工况树共用基础树节点和 Builder，但分别使用自己的 Model 和 Provider。
- 明确不使用自定义命名空间。
- 明确不再使用 `domain`、`IDOSTreeBuildContext`、`AppContext`、`BuiltinRegistrar`、`registerBuiltinTreeProviders` 等不必要的中间层。
- 明确工况对象需要基类：`IDOSCaseObject`。
- 明确工况引用通过 `IDOSCaseItemRef` 表达，工况树可以引用数据树对象或对象内部子项。
- 明确 `IDOSImportCoordinator` 更名为 `IDOSDataLoadService`，数据进入工程统一称为数据加载。
- 明确 `src/python` 是 C++ Python 模块，根目录 `python` 只放纯 Python SDK 脚本。
- 明确 `src/assistant` 是 AI 助手能力模块，助手 UI 放在 `src/gui`。
- 明确 `src/auto` 是待拆分旧模块，后续能力分别进入 `src/app/action`、`src/python` 和 `src/assistant`。
- 明确插件系统放在 `src/app/plugin`，对外插件导出头使用 `idos_plugin.h`。
- 更新 `docs/IDOS-架构设计方案.md`，去掉上一版旧内容，改为当前有效设计。

下一步：

- 按架构文档继续清理源码目录和旧命名。
- 保证 CMake、工程筛选器和源码目录一致。
- 继续把旧 `auto` 模块拆到目标模块中。
- 接入 `IDOSDataLoadService` 到实际加载入口。
