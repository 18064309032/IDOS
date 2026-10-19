param(
    [string]$CMake = 'cmake',
    [ValidateSet('Debug', 'Release')][string[]]$Configurations = @('Debug', 'Release')
)
$ErrorActionPreference = 'Stop'
$sourceRoot = (Split-Path -Parent $PSScriptRoot).Replace('\', '/')
$vendorRoot = Join-Path $sourceRoot 'thirdparty/proj'
$buildRoot = [IO.Path]::GetFullPath((Join-Path $sourceRoot '../IDOS-Build/thirdparty-proj')).Replace('\', '/')
New-Item -ItemType Directory -Force $buildRoot | Out-Null
$archives = @('proj-9.9.0.tar.gz', 'sqlite-amalgamation-3500400.zip', 'curl-8.17.0.tar.gz', 'tiff-4.7.1.tar.gz', 'zlib-1.3.1.tar.gz')
$hashes = Get-Content -Raw -LiteralPath (Join-Path $vendorRoot 'sha256.json') | ConvertFrom-Json
$urls = @{
    'proj-9.9.0.tar.gz' = 'https://download.osgeo.org/proj/proj-9.9.0.tar.gz'
    'sqlite-amalgamation-3500400.zip' = 'https://www.sqlite.org/2025/sqlite-amalgamation-3500400.zip'
    'curl-8.17.0.tar.gz' = 'https://curl.se/download/curl-8.17.0.tar.gz'
    'tiff-4.7.1.tar.gz' = 'https://download.osgeo.org/libtiff/tiff-4.7.1.tar.gz'
    'zlib-1.3.1.tar.gz' = 'https://zlib.net/fossils/zlib-1.3.1.tar.gz'
}
$downloadRoot = Join-Path $buildRoot 'downloads'
New-Item -ItemType Directory -Force $downloadRoot | Out-Null
foreach ($archive in $archives) {
    $archivePath = Join-Path $downloadRoot $archive
    if (!(Test-Path -LiteralPath $archivePath)) {
        $partialPath = "$archivePath.partial"
        Invoke-WebRequest -Uri $urls[$archive] -OutFile $partialPath -UseBasicParsing
        if ((Get-FileHash -LiteralPath $partialPath -Algorithm SHA256).Hash -ne $hashes.$archive) {
            throw "Downloaded source archive checksum mismatch: $archive"
        }
        Move-Item -LiteralPath $partialPath -Destination $archivePath -Force
    }
    if ((Get-FileHash -LiteralPath $archivePath -Algorithm SHA256).Hash -ne $hashes.$archive) {
        throw "Source archive checksum mismatch: $archive"
    }
    $extractedName = $archive -replace '\.tar\.gz$|\.zip$', ''
    if (Test-Path -LiteralPath (Join-Path $buildRoot $extractedName)) { continue }
    & $CMake -E chdir $buildRoot $CMake -E tar xf $archivePath
    if ($LASTEXITCODE -ne 0) { throw "Extraction failed: $archive" }
}
foreach ($configuration in $Configurations) {
    $deps = Join-Path $buildRoot "deps/$configuration"
    $common = @('-G', 'Visual Studio 17 2022', '-A', 'x64', "-DCMAKE_INSTALL_PREFIX=$deps", '-DCMAKE_INSTALL_LIBDIR=lib', '-DBUILD_SHARED_LIBS=OFF', '-DBUILD_TESTING=OFF')
    $sqliteBuild = Join-Path $buildRoot "sqlite-$configuration"
    & $CMake -S (Join-Path $sourceRoot 'cmake/sqlite') -B $sqliteBuild @common "-DSQLITE_SOURCE=$buildRoot/sqlite-amalgamation-3500400"
    if ($LASTEXITCODE -ne 0) { throw 'SQLite configure failed' }
    & $CMake --build $sqliteBuild --config $configuration --target install --parallel 8
    if ($LASTEXITCODE -ne 0) { throw 'SQLite build failed' }
    foreach ($dependency in @('zlib', 'curl', 'tiff')) {
        $dependencyBuild = Join-Path $buildRoot "$dependency-$configuration"
        if ($dependency -eq 'zlib') {
            $dependencySource = Join-Path $buildRoot 'zlib-1.3.1'
            $options = @()
        } elseif ($dependency -eq 'curl') {
            $dependencySource = Join-Path $buildRoot 'curl-8.17.0'
            $options = @('-DBUILD_CURL_EXE=OFF', '-DCURL_USE_SCHANNEL=ON', '-DCURL_USE_OPENSSL=OFF', '-DCURL_USE_LIBPSL=OFF', '-DCURL_USE_LIBSSH2=OFF', '-DCURL_BROTLI=OFF', '-DCURL_ZSTD=OFF', '-DCURL_ZLIB=OFF', '-DCURL_DISABLE_LDAP=ON', '-DCURL_DISABLE_LDAPS=ON')
        } else {
            $dependencySource = Join-Path $buildRoot 'tiff-4.7.1'
            $options = @("-DCMAKE_PREFIX_PATH=$deps", '-Dtiff-tools=OFF', '-Dtiff-tests=OFF', '-Dtiff-contrib=OFF', '-Dtiff-docs=OFF', '-Dtiff-opengl=OFF', '-Djpeg=OFF', '-Dold-jpeg=OFF', '-Dlzma=OFF', '-Dwebp=OFF', '-Dzstd=OFF', '-Djbig=OFF', '-Dlibdeflate=OFF', '-DCMAKE_DEBUG_POSTFIX=d')
        }
        & $CMake -S $dependencySource -B $dependencyBuild @common @options
        if ($LASTEXITCODE -ne 0) { throw "$dependency configure failed" }
        & $CMake --build $dependencyBuild --config $configuration --target install --parallel 8
        if ($LASTEXITCODE -ne 0) { throw "$dependency build failed" }
    }
    $projBuild = Join-Path $buildRoot "proj-$configuration"
    $suffix = if ($configuration -eq 'Debug') { 'd' } else { '' }
    & $CMake -S (Join-Path $buildRoot 'proj-9.9.0') -B $projBuild -G 'Visual Studio 17 2022' -A x64 "-DCMAKE_INSTALL_PREFIX=$vendorRoot" "-DCMAKE_PREFIX_PATH=$deps" "-DCMAKE_PROJECT_PROJ_INCLUDE=$sourceRoot/cmake/ProjDependencies.cmake" "-DZLIB_LIBRARY=$deps/lib/zlibstatic$suffix.lib" "-DZLIB_INCLUDE_DIR=$deps/include" "-DSQLite3_LIBRARY=$deps/lib/sqlite3$suffix.lib" "-DSQLite3_INCLUDE_DIR=$deps/include" "-DCMAKE_INSTALL_DATADIR=$sourceRoot/resources" '-DCMAKE_INSTALL_LIBDIR=lib' '-DBUILD_SHARED_LIBS=ON' '-DBUILD_TESTING=OFF' '-DENABLE_CURL=ON' '-DENABLE_TIFF=ON' '-DCMAKE_DEBUG_POSTFIX=d' "-DEXE_SQLITE3=$deps/bin/sqlite3.exe"
    if ($LASTEXITCODE -ne 0) { throw 'PROJ configure failed' }
    & $CMake --build $projBuild --config $configuration --target install --parallel 8
    if ($LASTEXITCODE -ne 0) { throw 'PROJ build failed' }
}
