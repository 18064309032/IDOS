# Plugin framework

This directory contains the host-side C++ plugin contract, abstract host interface, metadata and
dynamic-library registry. `IDOSAppInterface` implements `IDOSInterface` for the desktop app. The
interface exposes the main window. Additional app services can be added to the abstract interface
as plugin requirements become clear.

The plugin lifecycle follows `IDOSPlugin::initGui()` and `IDOSPlugin::unload()`. The registry calls
`unload()` and destroys the instance while its DLL is still loaded. QObject-based plugin instances
without an existing parent are parented to the host main window.

Plugins export `name` and `classFactory` as C-linkage functions. The factory signature is
`IDOSPlugin* classFactory(IDOSInterface*)`. Use `IDOS_PLUGIN_EXPORT` from `idosplugin.h` on each
entry point. The host searches the executable's `plugins` directory and its immediate package
subdirectories.
