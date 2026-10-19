# PROJ dependency

PROJ 9.9.0 is built independently using the project's established `include/lib/bin` layout. The IDOS root CMake file imports `PROJ::proj`; it does not use the private, symbol-renamed PROJ implementation inside VTK.

Run from the repository root on Windows with the Visual Studio 2022 C++ workload and CMake available:

```powershell
./tools/BuildProj.ps1 -CMake cmake
```

The script verifies `sha256.json`, downloads upstream source archives to `IDOS-Build/thirdparty-proj/downloads` as needed, checks their hashes, and extracts them into the source root's sibling `IDOS-Build/thirdparty-proj`, builds Debug and Release separately, then installs headers/libraries/tools into this directory and runtime data directly into the repository's `resources/proj`. No absolute drive or machine path is required. Source archives are not included in the repository. Rebuilding PROJ requires network access on the first run; cached archives support subsequent offline rebuilds. Normal IDOS builds use the bundled binaries and require no downloads.

Dependencies: SQLite 3.50.4 (database and database generation tool), curl 8.17.0 (Windows Schannel TLS), libtiff 4.7.1 and zlib 1.3.1 (compressed transformation grids). These dependencies are statically linked into the PROJ DLL; the application deploys the matching PROJ DLL alongside its existing runtime files. PROJ's complete coordinate operation implementation is enabled, including TIFF and curl support. The upstream command-line tools are retained here but are not required by IDOS.

Upstream archives:

- https://download.osgeo.org/proj/proj-9.9.0.tar.gz
- https://www.sqlite.org/2025/sqlite-amalgamation-3500400.zip
- https://curl.se/download/curl-8.17.0.tar.gz
- https://download.osgeo.org/libtiff/tiff-4.7.1.tar.gz
- https://zlib.net/fossils/zlib-1.3.1.tar.gz

PROJ's installed license is `share/doc/proj/COPYING`; dependency licenses and notices are retained in `share/doc/proj/dependencies`. Full upstream source licenses are also available in the downloaded archives.

IDOS copies the repository's `resources/proj` to `<executable>/resources/proj`, including `proj.db` and configuration files. Runtime contexts explicitly use that relative location and disable network access. High-accuracy regional grids are separate resources; add required licensed grids to the repository's `resources/proj` before building/deployment. IDOS rejects unavailable best transformations and ballpark fallbacks rather than downloading data or silently reducing accuracy.

The generated upstream CMake/pkg-config files are not consumed by IDOS; they may reference the dependency build prefixes. Use IDOS's root imported target, or rebuild dependencies when moving to another machine. Do not use a copied CMake cache.
