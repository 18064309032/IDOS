# IDOS 框架设计文档

更新：2026-09-24。适用仓库：`E:\CJW-Archive\IDOS`。

架构方案描述目标设计，工作记录描述已完成事项。以源码和验证结果判断实现状态，不把目标目录中的类视为已经实现。

## 阅读顺序

1. [架构设计方案](IDOS-架构设计方案.md)：模块职责、公共 API、实施顺序和落地状态。
2. [工作记录](工作记录-CJW.md)：已完成工作、验证结果及下一步。

## 已确定的设计约束

- 采用 C++17、Qt 5、CMake，沿用现有技术基础。
- 渲染目标为 `render_core`、`render_qt`、`render_adapters/idos`；旧 `src/render` 暂保留占位。
- 同名 .h 与 .cpp 放在同一目录；公共类成对放在模块根目录，内部类在所属子目录内成对放置。跨模块只能包含公共头。
- 主窗口及应用装配归 `app`，树和界面控件归 `gui`。
- 工程拥有业务对象；树、工况和窗口通过稳定 ID 引用对象。
- 工作区管理窗口，每个渲染窗口拥有独立显示状态。
- 保留现有 Provider 和通用树机制，逐步补齐工程、操作和显示闭环。
- 本设计不要求立即创建全部目录、空类或 DLL。

## 当前实现

已有工程对象、数据树和工况树、Provider 注册及数据加载服务。主窗口位于 `src/app`，启动时不再注入演示数据，可通过“工程 → 新建工程”或 Ctrl+N 确认对话框后初始化默认数据树；暂不应用工程信息或修改窗口标题。已接通井头与批量井轨迹导入，工况引用显示尚待接通，Python、Assistant、分析和旧渲染模块仍为占位。

## 本地验证

配置本机 Qt 路径后执行：

```powershell
cmake --preset win-x64-debug-local -B E:/CJW-Archive/build/win-x64-debug-local
cmake --build E:/CJW-Archive/build/win-x64-debug-local --config Debug
ctest --test-dir E:/CJW-Archive/build/win-x64-debug-local -C Debug --output-on-failure
```

构建会执行 `tools/check_includes.ps1`。公共头显式列入 CMake target，Qt AUTOMOC 和 VS 筛选器与新布局保持一致。

GUI 的通用树、数据树与工况树逻辑分层记录在 `tools/gui_layers.json`，文件同目录后仍执行原有依赖限制。新增相关 GUI 类时需同步更新清单。
