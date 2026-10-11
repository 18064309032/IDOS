# Plugin framework

This directory contains the host-side C++ plugin contract, abstract host interface, metadata and
dynamic-library registry. `IDOSAppInterface` implements `IDOSInterface` for the desktop app. The
interface exposes the main window. Additional app services can be added to the abstract interface
as plugin requirements become clear.

The plugin lifecycle follows `IDOSPlugin::initGui()` and `IDOSPlugin::unload()`. The registry calls
`unload()` and destroys the instance while its DLL is still loaded. Concrete plugin classes inherit
`QObject` directly alongside `IDOSPlugin`; instances without an existing parent are parented to the
host main window. `IDOSPlugin` is a plain C++ base class and does not depend on Qt's object system.

Plugins export `classFactory`, `unload`, `name`, `description`, `category`, `type`, `version` and
`icon` as C-linkage functions. Their function signatures are declared in `idosplugin.h`, following
the QGIS plugin API pattern. The registry and plugin manager can query metadata before creating a
plugin instance. `classFactory(IDOSInterface*)` creates the concrete plugin. The registry calls
the virtual `unload()` lifecycle method and destroys the instance while its DLL is still loaded;
the exported `unload(IDOSPlugin*)` is available to API consumers that need to destroy an instance.
A plugin sets its display name, description, category and version through the `IDOSPlugin` base constructor. Use
`IDOS_PLUGIN_EXPORT` on each entry point. The host searches the executable's `plugins` directory
and its immediate package subdirectories.

Each plugin has a separate `<plugin-name>_zh_CN.ts` catalog in the repository's `i18n` directory.
The build places the compiled catalogs in the runtime `i18n` directory beside the executable. Each
plugin creates and installs its own translator in `initGui()` before creating translated interface
text, then removes the translator during `unload()`. Plugin metadata is initialized by the
`IDOSPlugin` base constructor and is independent of GUI initialization.
