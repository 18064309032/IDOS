# idos_analysis: 专业算法模块
#
# 职责判据（不满足任一条 = 不属于本模块）：
#   1. 读取明确的数据输入，产出新的计算数据
#   2. 不碰界面（不 include gui，不依赖 Qt Widgets）
#   3. 不修改工程对象关系（读数据走 IDOSProject 只读接口，
#      结果入库由调用方经 app 的统一操作入口提交）
#
# 典型成员：储量计算、网格粗化、递减拟合、属性插值、体积计算。
# 非典型（禁止）：导入解析在 providers、显示转换在 render、
#                业务编排在 app。
#
# 依赖链：idos_analysis → idos_core → (Qt5::Core Qt5::Gui)
add_library(idos_analysis SHARED dummy.cpp)
target_include_directories(idos_analysis PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(idos_analysis PUBLIC idos_core)
target_compile_definitions(idos_analysis PRIVATE IDOS_ANALYSIS_LIB)
