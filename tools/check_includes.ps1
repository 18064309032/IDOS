# 检查真实头文件归属、模块依赖和公共 API 边界；不依赖旧目录名称猜测模块。
# 公共头在模块根目录；app/plugin 是方案明确规定的插件 SDK 例外。
# 检查直接 include 和已知内部类型引用，不替代 C++ 编译器的语义分析。
param([string]$SourceRoot = "")
$ErrorActionPreference = 'Stop'
if (!$SourceRoot) { $SourceRoot = Join-Path $PSScriptRoot '../src' }
$root = (Resolve-Path -LiteralPath $SourceRoot).Path
$modules = @(Get-ChildItem -LiteralPath $root -Directory)
$files = @(Get-ChildItem -LiteralPath $root -Recurse -File | Where-Object { $_.Extension -in '.h','.hpp','.cpp','.cc' })
$headers = @($files | Where-Object { $_.Extension -in '.h','.hpp' })
$violations = New-Object 'System.Collections.Generic.List[string]'
$allowed = @{
    core = @()
    gui = @('core','render_core','render_qt','render_adapters')
    providers = @('core')
    render = @()
    render_core = @()
    render_qt = @('render_core')
    render_adapters = @('core','render_core')
    analysis = @('core')
    python = @('core')
    assistant = @('core')
    app = @('core','gui','providers','render','render_core','render_qt','render_adapters','analysis','python','assistant')
}
function RelativePath($path) { $path.Substring($root.Length + 1).Replace('\','/') }
function IsPublic($rel) { ($rel -split '/').Count -eq 2 -or $rel -match '^app/plugin/[^/]+\.h(pp)?$' }
function WithoutComments($content) {
    [regex]::Replace($content, '(?s)/\*.*?\*/|(?m)//[^\r\n]*', { param($m) [regex]::Replace($m.Value, '[^\r\n]', ' ') })
}
# 同目录布局通过显式清单保留 GUI 分层；子目录布局仍按目录识别。
$guiLayers = @{}
$layerManifest = Join-Path $PSScriptRoot 'gui_layers.json'
if (Test-Path -LiteralPath $layerManifest) {
    $layerDefinitions = Get-Content -LiteralPath $layerManifest -Raw | ConvertFrom-Json
    foreach ($property in $layerDefinitions.PSObject.Properties) {
        $guiLayers[$property.Name] = $property.Value
    }
}
foreach ($file in $files) {
    $rel = RelativePath $file.FullName
    if ($rel -match '^gui/(tree|input|case)/') { $guiLayers[$file.BaseName] = $Matches[1] }
}
$internalTypes = @{}
foreach ($header in $headers) {
    $rel = RelativePath $header.FullName
    if (!(IsPublic $rel)) {
        $content = WithoutComments ([IO.File]::ReadAllText($header.FullName))
        foreach ($match in [regex]::Matches($content, '\b(?:class|struct)\s+(?:\w+_EXPORT\s+)?(IDOS\w+)\s*(?:final\s*)?(?::[^;{]+)?\{')) {
            $internalTypes[$match.Groups[1].Value] = $rel
        }
    }
}
foreach ($file in $files) {
    $rel = RelativePath $file.FullName
    $module = ($rel -split '/')[0]
    $content = WithoutComments ([IO.File]::ReadAllText($file.FullName))
    $public = $file.Extension -in '.h','.hpp' -and (IsPublic $rel)
    if ($public) {
        foreach ($type in $internalTypes.Keys) {
            if ($content -match "\b$([regex]::Escape($type))\b") {
                $violations.Add("${rel}: 公共头引用内部类型 $type ($($internalTypes[$type]))")
            }
        }
    }
    $lineNumber = 0
    foreach ($line in ($content -split '\r?\n')) {
        $lineNumber++
        if ($line -notmatch '^\s*#\s*include\s*(["<])([^">]+)[">]') { continue }
        $quoted = $Matches[1] -eq '"'
        $inc = $Matches[2].Replace('\','/')
        $location = "${rel}(${lineNumber})"
        if ($inc -match '(^|/)\.\.(/|$)' -or [IO.Path]::IsPathRooted($inc)) {
            $violations.Add("${location}: 禁止目录穿越或绝对路径 include: $inc")
            continue
        }
        $candidates = @()
        if ($quoted) {
            $local = Join-Path $file.DirectoryName $inc
            if (Test-Path -LiteralPath $local -PathType Leaf) { $candidates = @((Get-Item -LiteralPath $local)) }
        }
        if (!$candidates.Count -and $module -ne 'main.cpp') {
            $own = Join-Path (Join-Path $root $module) $inc
            if (Test-Path -LiteralPath $own -PathType Leaf) { $candidates = @((Get-Item -LiteralPath $own)) }
        }
        if (!$candidates.Count) {
            $candidates = @(foreach ($dir in $modules) {
                $path = Join-Path $dir.FullName $inc
                if (Test-Path -LiteralPath $path -PathType Leaf) { Get-Item -LiteralPath $path }
            })
            $qualified = Join-Path $root $inc
            if (Test-Path -LiteralPath $qualified -PathType Leaf) { $candidates += Get-Item -LiteralPath $qualified }
            $candidates = @($candidates | Sort-Object FullName -Unique)
        }
        if (!$candidates.Count) {
            if ($quoted -or $inc -match '(^|/)idos[^/]*\.h$') { $violations.Add("${location}: 无法解析项目头文件: $inc") }
            continue # 第三方尖括号头由编译器检查。
        }
        if ($candidates.Count -ne 1) { $violations.Add("${location}: 头文件归属不唯一: $inc"); continue }
        $target = $candidates[0]
        $targetRel = RelativePath $target.FullName
        $targetModule = ($targetRel -split '/')[0]
        if ($module -ne $targetModule) {
            if ($module -ne 'main.cpp' -and (!$allowed.ContainsKey($module) -or $targetModule -notin $allowed[$module])) {
                $violations.Add("${location}: $module 禁止依赖 $targetModule ($inc)")
            }
            if (!(IsPublic $targetRel)) { $violations.Add("${location}: 跨模块禁止包含内部头: $targetRel") }
        }
        if ($public -and !(IsPublic $targetRel)) { $violations.Add("${location}: 公共头禁止包含内部头: $targetRel") }
        if ($module -eq 'gui' -and $targetModule -eq 'gui') {
            $fromLayer = $guiLayers[$file.BaseName]
            $toLayer = $guiLayers[$target.BaseName]
            if (($fromLayer -eq 'tree' -and $toLayer -in 'input','case') -or
                ($fromLayer -eq 'input' -and $toLayer -eq 'case') -or
                ($fromLayer -eq 'case' -and $toLayer -eq 'input')) {
                $violations.Add("${location}: GUI $fromLayer 禁止依赖 $toLayer ($inc)")
            }
        }
    }
}
if ($violations.Count) {
    $violations | ForEach-Object { Write-Host $_ -ForegroundColor Red }
    Write-Host "check_includes: $($violations.Count) 处违规"
    exit 1
}
Write-Host "check_includes: 架构边界检查通过 ($($files.Count) 个源文件)"
exit 0
