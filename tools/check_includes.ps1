# =====================================================================
# check_includes.ps1 —— include 架构边界守护脚本
#
# 规则（违例即退出码 1，构建/CI 失败）：
#   1. 全项目禁止 "../" 目录穿越式 include
#   2. core/ 禁止 include 其他任何模块的头（gui/providers/render/app/扩展侧）
#      —— core 与 gui 的这条同时被 CMake 依赖图兜底（core 未链接 Widgets），双保险
#   3. gui/tree/（机制层）禁止 include 业务层（inputtree/ modeltree/ 主窗口）
#   4. gui 业务层互不依赖（inputtree 不 include modeltree，反之亦然）
#   5. providers/ 禁止 include gui 侧与 app 侧头（解析层不认识界面与装配）
#   6. render/ 禁止 include gui 侧、providers 侧与 app 侧头（渲染只依赖 core + VTK）
#   7. gui/ 禁止 include providers 侧、app 侧与扩展模块侧头
#      （允许 render 侧：设计约定 GUI 可依赖 Core/Render，需在 gui/CMakeLists 同步链接）
#   8. analysis/ python/ assistant/ 只允许 include core 侧与自身根头
#      —— 扩展/自动化模块不得依赖 gui/providers/render/app，也不得互相依赖
#      （对应设计约束：python/assistant 反向链接 App = 统一操作入口失效）
#   9. app/ 禁止 include gui、providers、render 的内部子目录
#      —— app 只走各模块公共头与工厂；app 自身是唯一可认识所有模块公共面的装配层
#
# 用法：
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools/check_includes.ps1
#   可选参数 -SourceRoot <路径>（默认 <脚本目录>/../src）
# 由根 CMakeLists.txt 的 check_includes 目标（ALL）在每次构建时自动执行。
# =====================================================================
param(
    [string]$SourceRoot = ""
)

if ($SourceRoot -eq "") {
    $SourceRoot = Join-Path $PSScriptRoot "..\src"
}

if (-not (Test-Path $SourceRoot)) {
    Write-Host "check_includes: 源码目录不存在: $SourceRoot" -ForegroundColor Red
    exit 1
}

$sourceRootResolved = (Resolve-Path $SourceRoot).Path

# gui 侧 include 目标（相对模块根的路径或 gui 根头文件）
$guiSidePattern = '^(tree/|inputtree/|modeltree/|idos_gui\.h$|idosmainwindow\.h$)'

$violations = New-Object System.Collections.Generic.List[string]
$fileCount = 0

$files = Get-ChildItem -Path $SourceRoot -Recurse -File -Include *.h, *.hpp, *.cpp, *.cc
foreach ($file in $files) {
    $fileCount++
    $rel = $file.FullName.Substring($sourceRootResolved.Length + 1).Replace('\', '/')
    $isCoreFile = $rel -like 'core/*'
    $isTreeMechanism = $rel -like 'gui/tree/*'
    $isInputTree = $rel -like 'gui/inputtree/*'
    $isModelTree = $rel -like 'gui/modeltree/*'
    $isGuiFile = $rel -like 'gui/*'
    $isProvidersFile = $rel -like 'providers/*'
    $isRenderFile = $rel -like 'render/*'
    $isAppFile = $rel -like 'app/*'
    $isAnalysisFile = $rel -like 'analysis/*'
    $isPythonFile = $rel -like 'python/*'
    $isAssistantFile = $rel -like 'assistant/*'
    $isExtFile = $isAnalysisFile -or $isPythonFile -or $isAssistantFile

    # 各模块的"侧"（include 一律从模块根出发写全路径，故按根相对路径匹配）
    $providersSide = '^(well/|grid/|model/|idos_providers\.h$|idosdataprovider\.h$|idosprovidermetadata\.h$|idosproviderregistry\.h$|idosimportcoordinator\.h$)'
    $renderSide = '^(idos_render\.h$|scene/|adapters/|interaction/|overlays/)'
    $appSide = '^(idos_app\.h$|action/|import/|command/|task/|automation/|plugin/)'
    $extSide = '^(idos_analysis\.h$|idos_python\.h$|idos_assistant\.h$)'

    # 内部子目录（仅规则 9 用：app 不深入模块内部，公共根头不受限）
    $guiInternal = '^(tree/|inputtree/|modeltree/|views/|properties/|dialogs/)'
    $providersInternal = '^(well/|grid/|model/)'
    $renderInternal = '^(scene/|adapters/|interaction/|overlays/)'

    # 扩展模块自身的根头（规则 8 允许 self-include）
    $ownHeader = ''
    if ($isAnalysisFile) { $ownHeader = 'idos_analysis.h' }
    elseif ($isPythonFile) { $ownHeader = 'idos_python.h' }
    elseif ($isAssistantFile) { $ownHeader = 'idos_assistant.h' }

    $lines = Get-Content -Path $file.FullName -Encoding UTF8
    for ($i = 0; $i -lt $lines.Count; $i++) {
        if ($lines[$i] -notmatch '^\s*#\s*include\s*"([^"]+)"') { continue }
        $inc = $Matches[1]
        $location = "$rel($($i + 1))"

        # 规则 1：禁止目录穿越
        if ($inc -like '*../*') {
            $violations.Add("${location}: 禁止 '../' 目录穿越: #include `"$inc`"")
            continue
        }

        # 规则 2：core 不依赖任何其他模块
        if ($isCoreFile -and ($inc -match $guiSidePattern -or $inc -match $providersSide `
                -or $inc -match $renderSide -or $inc -match $appSide -or $inc -match $extSide)) {
            $violations.Add("${location}: core 禁止 include 其他模块头文件: $inc")
            continue
        }

        # 规则 3：机制层不依赖业务层
        if ($isTreeMechanism -and $inc -match '^(inputtree/|modeltree/|idosmainwindow)') {
            $violations.Add("${location}: 机制层 tree/ 禁止 include 业务层头文件: $inc")
            continue
        }

        # 规则 4：业务层互不依赖
        if ($isInputTree -and $inc -like 'modeltree/*') {
            $violations.Add("${location}: inputtree 禁止 include modeltree: $inc")
            continue
        }
        if ($isModelTree -and $inc -like 'inputtree/*') {
            $violations.Add("${location}: modeltree 禁止 include inputtree: $inc")
        }

        # 规则 5：解析层不认识界面与装配
        if ($isProvidersFile -and ($inc -match $guiSidePattern -or $inc -match $appSide)) {
            $violations.Add("${location}: providers 禁止 include gui/app 侧头文件: $inc")
            continue
        }

        # 规则 6：渲染层只依赖 core（+VTK），不认识界面/解析/装配
        if ($isRenderFile -and ($inc -match $guiSidePattern -or $inc -match $providersSide `
                -or $inc -match $appSide -or $inc -match $extSide)) {
            $violations.Add("${location}: render 禁止 include gui/providers/app/扩展侧头文件: $inc")
            continue
        }

        # 规则 7：gui 不认识解析层、装配层与扩展模块（允许 render 侧，见头部注释）
        if ($isGuiFile -and ($inc -match $providersSide -or $inc -match $appSide `
                -or $inc -match $extSide)) {
            $violations.Add("${location}: gui 禁止 include providers/app/扩展模块侧头文件: $inc")
            continue
        }

        # 规则 8：扩展/自动化模块只认 core + 自身根头
        if ($isExtFile -and $inc -ne $ownHeader `
                -and ($inc -match $guiSidePattern -or $inc -match $providersSide `
                      -or $inc -match $renderSide -or $inc -match $appSide `
                      -or $inc -match $extSide)) {
            $violations.Add("${location}: analysis/python/assistant 只允许 include core 侧头文件: $inc")
            continue
        }

        # 规则 9：app 走公共头与工厂，不深入其他模块内部子目录
        if ($isAppFile -and ($inc -match $guiInternal -or $inc -match $providersInternal `
                -or $inc -match $renderInternal)) {
            $violations.Add("${location}: app 禁止 include 其他模块的内部子目录: $inc")
            continue
        }
    }
}

if ($violations.Count -gt 0) {
    Write-Host "check_includes: 发现 $($violations.Count) 处架构边界违规：" -ForegroundColor Red
    foreach ($v in $violations) {
        Write-Host "  $v" -ForegroundColor Red
    }
    exit 1
}

Write-Host "check_includes: 架构边界检查通过 ($fileCount 个源文件)"
exit 0
