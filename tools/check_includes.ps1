# 检查真实头文件归属、模块依赖和公共 API 边界；不依赖旧目录名称猜测模块。
# 公共边界以固定源码目录中的功能根目录为准。
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
    gui = @('core')
    providers = @('core')
    render = @()
    analysis = @('core')
    python = @('core')
    assistant = @('core')
    app = @('core','gui','providers','render','analysis','python','assistant')
    plugins = @('core','gui','render','providers')
}
function ModuleOf($rel) {
    return ($rel -split '/')[0]
}
function RelativePath($path) { $path.Substring($root.Length + 1).Replace('\','/') }
function IsPublic($rel) {
    ($rel -split '/').Count -eq 2 -or
    $rel -match '^core/(data|tree|providers|logger|command)/[^/]+\.h(pp)?$' -or
    $rel -match '^core/data/(well|grid|case|fluidrock|fracture|production|seismic)/[^/]+\.h(pp)?$' -or
    $rel -match '^gui/tree/[^/]+\.h(pp)?$' -or
    $rel -match '^render/tools/[^/]+\.h(pp)?$' -or
    $rel -match '^providers/(well|las|grid|case|fluidrock|fracture|production|seismic)/[^/]+\.h(pp)?$' -or
    $rel -match '^app/render/idosrender(provider(metadata|registry)|server)\.h$' -or
    $rel -match '^plugins/[^/]+/[^/]+\.h(pp)?$'
}
function WithoutComments($content) {
    [regex]::Replace($content, '(?s)/\*.*?\*/|(?m)//[^\r\n]*', { param($m) [regex]::Replace($m.Value, '[^\r\n]', ' ') })
}
$internalTypes = @{}
foreach ($header in $headers) {
    $rel = RelativePath $header.FullName
    if (!(IsPublic $rel)) {
        $content = WithoutComments ([IO.File]::ReadAllText($header.FullName))
        $typePattern = '\b(?:class|struct)\s+(?:\w+_EXPORT\s+)?(IDOS\w+)\s*(?:final\s*)?(?::[^;{]+)?\s*\{'
        foreach ($match in [regex]::Matches($content, $typePattern)) {
            $typeName = $match.Groups[1].Value
            $internalTypes[$typeName] = $rel
        }
    }
}
foreach ($file in $files) {
    $rel = RelativePath $file.FullName
    $module = ModuleOf $rel
    $content = WithoutComments ([IO.File]::ReadAllText($file.FullName))
    $public = $file.Extension -in '.h','.hpp' -and (IsPublic $rel)
    if ($public) {
        $publicContent = [regex]::Replace($content, '\bclass\s+IDOS\w+\s*;', '')
        $publicContent = [regex]::Replace($publicContent, '(?s)\bprivate(?:\s+slots)?\s*:.*', '')
        foreach ($type in $internalTypes.Keys) {
            if ($publicContent -match "\b$([regex]::Escape($type))\b") {
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
        if (!$candidates.Count -and $inc -notmatch '/') {
            $matchingHeaders = @($headers | Where-Object { $_.Name -eq $inc })
            $sameModule = @($matchingHeaders | Where-Object { (RelativePath $_.FullName) -like "$module/*" })
            if ($sameModule.Count) { $candidates = $sameModule }
            else { $candidates = $matchingHeaders }
        }
        if (!$candidates.Count) {
            if ($quoted -or $inc -match '(^|/)idos[^/]*\.h$') { $violations.Add("${location}: 无法解析项目头文件: $inc") }
            continue # 第三方尖括号头由编译器检查。
        }
        if ($candidates.Count -ne 1) { $violations.Add("${location}: 头文件归属不唯一: $inc"); continue }
        $target = $candidates[0]
        $targetRel = RelativePath $target.FullName
        $targetModule = ModuleOf $targetRel
        if ($module -ne $targetModule) {
            if ($module -ne 'main.cpp' -and
                (!$allowed.ContainsKey($module) -or $targetModule -notin $allowed[$module]) -and
                !($module -eq 'app' -and $targetRel -eq 'plugins/idosplugin.h') -and
                !($module -eq 'plugins' -and ($targetRel -eq 'app/idos_app.h' -or
                    $targetRel -match '^app/render/idosrender(provider(metadata|registry)|server)\.h$'))) {
                $violations.Add("${location}: $module 禁止依赖 $targetModule ($inc)")
            }
            if (!(IsPublic $targetRel)) { $violations.Add("${location}: 跨模块禁止包含内部头: $targetRel") }
        }
        if ($public -and !(IsPublic $targetRel)) { $violations.Add("${location}: 公共头禁止包含内部头: $targetRel") }
    }
}
if ($violations.Count) {
    $violations | ForEach-Object { Write-Host $_ -ForegroundColor Red }
    Write-Host "check_includes: $($violations.Count) violation(s)"
    exit 1
}
Write-Host "check_includes: passed ($($files.Count) source file(s))"
exit 0
