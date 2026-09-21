# =====================================================================
# check_includes.ps1 —— include 架构边界守护脚本
#
# 规则（违例即退出码 1，构建/CI 失败）：
#   1. 全项目禁止 "../" 目录穿越式 include
#   2. core/ 禁止 include gui 侧头文件（tree/ inputtree/ modeltree/ 等）
#      —— 这条同时被 CMake 依赖图兜底（core 未链接 Widgets），双保险
#   3. gui/tree/（机制层）禁止 include 业务层（inputtree/ modeltree/ 主窗口）
#   4. gui 业务层互不依赖（inputtree 不 include modeltree，反之亦然）
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

        # 规则 2：core 不依赖 gui
        if ($isCoreFile -and $inc -match $guiSidePattern) {
            $violations.Add("${location}: core 禁止 include gui 侧头文件: $inc")
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
