# Isolated fixtures verify both accepted dependencies and rejected regressions.
param([Parameter(Mandatory=$true)][string]$ScratchRoot)
$ErrorActionPreference = 'Stop'
$runRoot = Join-Path $ScratchRoot ([Guid]::NewGuid().ToString('N'))
$checker = Join-Path $PSScriptRoot 'check_includes.ps1'
$cases = @(
    @{ Name='gui_tree_public_interface'; Exit=0; Files=@{'gui/tree/idostreeguiprovider.h'=''; 'plugins/example/plugin.cpp'='#include "tree/idostreeguiprovider.h"'} },
    @{ Name='gui_plugin_public_interface'; Exit=0; Files=@{'gui/idosinterface.h'=''; 'plugins/example/plugin.cpp'='#include "idosinterface.h"'} },
    @{ Name='gui_output_public_interface'; Exit=0; Files=@{'gui/idosruntimeinfowidget.h'=''; 'app/window.cpp'='#include "idosruntimeinfowidget.h"'} },
    @{ Name='core_provider_interface'; Exit=0; Files=@{'core/providers/idosdataprovider.h'=''; 'providers/well/load.cpp'='#include "providers/idosdataprovider.h"'} },
    @{ Name='render_provider_interface'; Exit=0; Files=@{'render/providers/idosrenderprovider.h'=''; 'app/providers/display.cpp'='#include "providers/idosrenderprovider.h"'} },
    @{ Name='app_owned_ui_binding'; Exit=0; Files=@{'app/CMakeLists.txt'='"${PROJECT_SOURCE_DIR}/src/gui/grid/idosgridsettingwidget.cpp"'; 'app/providers/private.h'=''; 'gui/grid/idosgridsettingwidget.cpp'='#include "providers/private.h"'} },
    @{ Name='generic_gui_to_app_owned_ui'; Exit=1; Files=@{'app/CMakeLists.txt'='"${PROJECT_SOURCE_DIR}/src/gui/grid/idosgridsettingwidget.h"'; 'gui/grid/idosgridsettingwidget.h'=''; 'gui/general.cpp'='#include "grid/idosgridsettingwidget.h"'} },
    @{ Name='app_owned_ui_public_leak'; Exit=1; Files=@{'app/CMakeLists.txt'='"${PROJECT_SOURCE_DIR}/src/gui/grid/idosgridsettingwidget.h"'; 'gui/grid/idosgridsettingwidget.h'=''; 'app/public.h'='#include "grid/idosgridsettingwidget.h"'} },
    @{ Name='fluidrock_data'; Exit=0; Files=@{'core/data/fluidrock/idosfluidrocktable.h'=''; 'app/load.cpp'='#include "idosfluidrocktable.h"'} },
    @{ Name='grid_parser'; Exit=0; Files=@{'providers/grid/idosgridprovider.h'=''; 'app/load.cpp'='#include "idosgridprovider.h"'} },
    @{ Name='plugin_case_menu_boundary'; Exit=1; Files=@{'app/case/idossimulationcasetreeguiprovider.h'=''; 'plugins/example/plugin.cpp'='#include "case/idossimulationcasetreeguiprovider.h"'} },
    @{ Name='public_dependency'; Exit=0; Files=@{'core/idosdata.h'='class IDOSData {};'; 'providers/load.cpp'='#include "idosdata.h"'} },
    @{ Name='same_module_internal'; Exit=0; Files=@{'providers/well/private.h'=''; 'providers/load.cpp'='#include "well/private.h"'} },
    @{ Name='core_to_gui'; Exit=1; Files=@{'gui/idosview.h'=''; 'core/object.cpp'='#include "idosview.h"'} },
    @{ Name='gui_to_load_service'; Exit=1; Files=@{'providers/idosdataloadservice.h'=''; 'gui/view.cpp'='#include <idosdataloadservice.h>'} },
    @{ Name='app_to_internal'; Exit=1; Files=@{'gui/input/private.h'=''; 'app/window.cpp'='#include "input/private.h"'} },
    @{ Name='public_to_internal'; Exit=1; Files=@{'core/internal/private.h'=''; 'core/idospublic.h'='#include "internal/private.h"'} },
    @{ Name='internal_type_leak'; Exit=1; Files=@{'core/internal/private.h'='class IDOSPrivate {};'; 'core/idospublic.h'='class IDOSPrivate; IDOSPrivate* data();'} },
    @{ Name='gui_tree_to_app'; Exit=1; Files=@{'gui/tree/base.cpp'='#include "idoswelltreeprovider.h"'; 'app/idoswelltreeprovider.h'=''} },
    @{ Name='gui_tree_to_render'; Exit=1; Files=@{'gui/tree/view.cpp'='#include "idosrenderprovider.h"'; 'render/idosrenderprovider.h'=''} },
    @{ Name='render_to_gui_tree'; Exit=1; Files=@{'render/view.cpp'='#include "idostreeview.h"'; 'gui/tree/idostreeview.h'=''} },
    @{ Name='core_tree_to_app'; Exit=1; Files=@{'core/tree/idostreeserver.cpp'='#include "idoswelltreeprovider.h"'; 'app/idoswelltreeprovider.h'=''} },
    @{ Name='core_tree_to_gui'; Exit=1; Files=@{'core/tree/idostreemodel.cpp'='#include "idostreeview.h"'; 'gui/tree/idostreeview.h'=''} },
    @{ Name='app_to_tree'; Exit=0; Files=@{'app/well/idoswelltreeprovider.cpp'='#include "idostreeprovider.h"'; 'core/tree/idostreeprovider.h'=''} },
    @{ Name='app_to_render'; Exit=0; Files=@{'render/idos_render.h'=''; 'app/window.cpp'='#include "idos_render.h"'} },
    @{ Name='render_to_core'; Exit=0; Files=@{'core/idosdata.h'=''; 'render/view.cpp'='#include "idosdata.h"'} },
    @{ Name='core_to_render'; Exit=1; Files=@{'render/idosview.h'=''; 'core/data.cpp'='#include "idosview.h"'} },
    @{ Name='forward_declaration_is_not_definition'; Exit=0; Files=@{'core/idosdata.h'='class IDOSData {};'; 'app/render/private.h'='class IDOSData;'; 'render/provider.h'='class IDOSData;'} },
    @{ Name='same_module_render_header'; Exit=0; Files=@{'app/render/private.h'=''; 'app/window.cpp'='#include "private.h"'} },
    @{ Name='gui_to_assistant'; Exit=1; Files=@{'assistant/idosassistantservice.h'=''; 'gui/view.cpp'='#include <idosassistantservice.h>'} },
    @{ Name='plugin_to_app'; Exit=1; Files=@{'app/idosmainwindow.h'=''; 'plugins/example/plugin.cpp'='#include "idosmainwindow.h"'} },
    @{ Name='retired_coloring_provider_boundary'; Exit=1; Files=@{'app/grid/idosgridrenderprovider.h'=''; 'plugins/idossimulationplugin/idoscoloringwidget.cpp'='#include "grid/idosgridrenderprovider.h"'} },
    @{ Name='retired_coloring_menu_boundary'; Exit=1; Files=@{'app/grid/idosgridrenderprovider.h'=''; 'plugins/idossimulationplugin/idoscoloringtreeguiprovider.cpp'='#include "grid/idosgridrenderprovider.h"'} },
    @{ Name='grid_settings_same_module'; Exit=0; Files=@{'app/grid/idosgridrenderprovider.h'=''; 'app/grid/idosgridsettingwidget.cpp'='#include "idosgridrenderprovider.h"'} },
    @{ Name='retired_simulation_well_display'; Exit=1; Files=@{'app/well/idoswellrenderprovider.h'=''; 'plugins/idossimulationplugin/idoswelldisplaywidget.cpp'='#include "well/idoswellrenderprovider.h"'} },
    @{ Name='retired_simulation_well_display_menu'; Exit=1; Files=@{'app/well/idoswellrenderprovider.h'=''; 'plugins/idossimulationplugin/idoswelldisplaytreeguiprovider.cpp'='#include "well/idoswellrenderprovider.h"'} },
    @{ Name='other_plugin_well_provider'; Exit=1; Files=@{'app/well/idoswellrenderprovider.h'=''; 'plugins/example/plugin.cpp'='#include "well/idoswellrenderprovider.h"'} },
    @{ Name='well_display_public_header'; Exit=1; Files=@{'app/well/idoswellrenderprovider.h'=''; 'plugins/idossimulationplugin/idoswelldisplaywidget.h'='#include "well/idoswellrenderprovider.h"'} },
    @{ Name='other_plugin_grid_provider'; Exit=1; Files=@{'app/grid/idosgridrenderprovider.h'=''; 'plugins/example/plugin.cpp'='#include "grid/idosgridrenderprovider.h"'} },
    @{ Name='coloring_public_header_boundary'; Exit=1; Files=@{'app/grid/idosgridrenderprovider.h'=''; 'plugins/idossimulationplugin/idoscoloringwidget.h'='#include "grid/idosgridrenderprovider.h"'} },
    @{ Name='plugin_sdk'; Exit=0; Files=@{'plugins/idosplugin.h'=''; 'main.cpp'='#include "idosplugin.h"'} },
    @{ Name='plugin_to_core'; Exit=0; Files=@{'plugins/example/data.cpp'='#include "idosdataobject.h"'; 'core/data/idosdataobject.h'=''} },
    @{ Name='plugin_to_render'; Exit=0; Files=@{'plugins/example/provider.cpp'='#include "idosrenderprovider.h"'; 'render/idosrenderprovider.h'=''} },
    @{ Name='functional_public_data'; Exit=0; Files=@{'core/data/well/idoswell.h'=''; 'providers/load.cpp'='#include "idoswell.h"'} },
    @{ Name='functional_public_tree'; Exit=0; Files=@{'core/tree/idostree.h'=''; 'gui/tree/view.cpp'='#include "idostree.h"'} },
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

