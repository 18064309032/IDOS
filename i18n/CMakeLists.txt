# i18n: 翻译文件 (.ts / .qm)
#
# 命名遵循 Qt 标准：<应用名>_<语言代码小写>_<国家代码大写>.ts
#   语言代码 = ISO 639-1 小写（zh / en / ja）
#   国家代码 = ISO 3166-1 大写（CN / TW / US）
# 本项目：idos_zh_CN.ts（与 Qt 自带 qtbase_zh_CN 一致）
#
# 翻译工作流：
#   1. ninja lupdate  / cmake --build --target lupdate   扫描 src 提取 tr() 字符串
#   2. linguist i18n/idos_zh_CN.ts                            翻译
#   3. lrelease 由 translations 目标自动执行，产出 idos_zh_CN.qm
#   4. 主程序启动时 QTranslator::load(QLocale(), "idos", "_", ":/i18n") 加载

# lupdate / lrelease 来自 LinguistTools 组件
find_package(Qt5 COMPONENTS LinguistTools REQUIRED)

set(IDOS_TS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/idos_zh_CN.ts")
set(IDOS_QM_FILE "${CMAKE_CURRENT_BINARY_DIR}/idos_zh_CN.qm")

# 手动目标：源码有新增 tr() 后运行一次，更新 ts
add_custom_target(lupdate
    COMMAND Qt5::lupdate "${CMAKE_SOURCE_DIR}/src" -ts "${IDOS_TS_FILE}"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "lupdate: 提取 tr() 字符串到 idos_zh_CN.ts..."
    VERBATIM
)

# translations（ALL）：随每次构建重新生成 qm
add_custom_command(OUTPUT "${IDOS_QM_FILE}"
    COMMAND Qt5::lrelease "${IDOS_TS_FILE}" -qm "${IDOS_QM_FILE}"
    DEPENDS "${IDOS_TS_FILE}"
    COMMENT "lrelease: 生成 idos_zh_CN.qm..."
    VERBATIM
)
add_custom_target(translations ALL DEPENDS "${IDOS_QM_FILE}")
