# Isolated fixtures verify both accepted dependencies and rejected regressions.
param([Parameter(Mandatory=$true)][string]$ScratchRoot)
$ErrorActionPreference = 'Stop'
$runRoot = Join-Path $ScratchRoot ([Guid]::NewGuid().ToString('N'))
$checker = Join-Path $PSScriptRoot 'check_includes.ps1'
$cases = @(
    @{ Name='public_dependency'; Exit=0; Files=@{'core/idosdata.h'='class IDOSData {};'; 'providers/load.cpp'='#include "idosdata.h"'} },
    @{ Name='same_module_internal'; Exit=0; Files=@{'providers/well/private.h'=''; 'providers/load.cpp'='#include "well/private.h"'} },
    @{ Name='core_to_gui'; Exit=1; Files=@{'gui/idosview.h'=''; 'core/object.cpp'='#include "idosview.h"'} },
    @{ Name='gui_to_load_service'; Exit=1; Files=@{'providers/idosdataloadservice.h'=''; 'gui/view.cpp'='#include <idosdataloadservice.h>'} },
    @{ Name='app_to_internal'; Exit=1; Files=@{'gui/input/private.h'=''; 'app/window.cpp'='#include "input/private.h"'} },
    @{ Name='public_to_internal'; Exit=1; Files=@{'core/data/private.h'=''; 'core/idospublic.h'='#include "data/private.h"'} },
    @{ Name='internal_type_leak'; Exit=1; Files=@{'core/data/private.h'='class IDOSPrivate {};'; 'core/idospublic.h'='class IDOSPrivate; IDOSPrivate* data();'} },
    @{ Name='tree_to_data'; Exit=1; Files=@{'gui/tree/base.cpp'='#include "idosdata.h"'; 'gui/input/idosdata.cpp'=''; 'gui/idosdata.h'=''} },
    @{ Name='data_to_case'; Exit=1; Files=@{'gui/input/data.cpp'='#include "idoscase.h"'; 'gui/case/idoscase.cpp'=''; 'gui/idoscase.h'=''} },
    @{ Name='case_to_data'; Exit=1; Files=@{'gui/case/case.cpp'='#include "idosdata.h"'; 'gui/input/idosdata.cpp'=''; 'gui/idosdata.h'=''} },
    @{ Name='flat_tree_to_data'; Exit=1; Files=@{'gui/idostreemodel.cpp'='#include "idosdatatreemodel.h"'; 'gui/idosdatatreemodel.h'=''} },
    @{ Name='flat_data_to_case'; Exit=1; Files=@{'gui/idosdatatreemodel.cpp'='#include "idoscasetreemodel.h"'; 'gui/idoscasetreemodel.h'=''} },
    @{ Name='flat_data_to_tree'; Exit=0; Files=@{'gui/idosdatatreemodel.cpp'='#include "idostreemodel.h"'; 'gui/idostreemodel.h'=''} },
    @{ Name='app_to_render'; Exit=0; Files=@{'render/idos_render.h'=''; 'app/window.cpp'='#include "idos_render.h"'} },
    @{ Name='render_to_core'; Exit=1; Files=@{'core/idosdata.h'=''; 'render/view.cpp'='#include "idosdata.h"'} },
    @{ Name='new_render'; Exit=0; Files=@{'render_core/idosscene.h'=''; 'render_qt/view.cpp'='#include "idosscene.h"'} },
    @{ Name='plugin_sdk'; Exit=0; Files=@{'app/plugin/idos_plugin.h'=''; 'main.cpp'='#include "plugin/idos_plugin.h"'} },
    @{ Name='traversal'; Exit=1; Files=@{'core/internal.cpp'='#include "../gui/view.h"'} },
    @{ Name='stale_header'; Exit=1; Files=@{'app/window.cpp'='#include "idosimportcoordinator.h"'} },
    @{ Name='thirdparty_and_comment'; Exit=0; Files=@{'core/object.cpp'="#include <QObject>`n// #include `"missing.h`""} }
)
foreach ($case in $cases) {
    $sourceRoot = Join-Path $runRoot $case.Name
    foreach ($relative in $case.Files.Keys) {
        $path = Join-Path $sourceRoot $relative
        New-Item -ItemType Directory -Path (Split-Path $path) -Force | Out-Null
        [IO.File]::WriteAllText($path, $case.Files[$relative])
    }
    $output = & powershell -NoProfile -ExecutionPolicy Bypass -File $checker -SourceRoot $sourceRoot 2>&1
    if ($LASTEXITCODE -ne $case.Exit) { throw "$($case.Name): expected $($case.Exit), got $LASTEXITCODE`n$output" }
    Write-Host "PASS $($case.Name)"
}
Write-Host "$($cases.Count) boundary fixtures passed."

