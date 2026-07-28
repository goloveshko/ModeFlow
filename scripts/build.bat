@echo off
setlocal EnableDelayedExpansion

rem ============================================================================
rem ModeFlow – Master Build & Packaging Automation Script
rem Default variables target the official production GitHub repository.
rem ============================================================================

rem --- Default Production (GitHub & Standard Paths) Configurations ---
set "MODEFLOW_UPDATE_URL=https://raw.githubusercontent.com/goloveshko/ModeFlow/main/metadata/update.json"
set "GIT_HOST_URL=https://github.com/goloveshko/ModeFlow"
set "QT_DIR=C:\Qt\6.11.1\msvc2022_64"
set "VS_PATH=C:\Program Files\Microsoft Visual Studio\18\Community\VC"
set "VCPKG_EXE=C:\dev\vcpkg\vcpkg.exe"

rem --- Local Environment Override ---
rem If a local environment.bat exists in the scripts directory,
rem we call it to override the default paths and Gitea debug URLs.
if exist "%~dp0environment.bat" (
    call "%~dp0environment.bat"
)

rem Get the initial time in seconds from the beginning of the day
for /f "tokens=1-3 delims=:.," %%a in ("%TIME%") do (
    set /a "START_TIME=(((%%a*60)+1%%b-100)*60)+1%%c-100"
)

pushd "%~dp0.."

set "PROJECT_ROOT=%CD%"
set "BUILD_ROOT=%PROJECT_ROOT%\build"
set "APP_NAME_BASE=ModeFlow"
set "APP_NAME=%APP_NAME_BASE%.exe"
set "APP_NAME_TESTS=%APP_NAME_BASE%Tests.exe"
set "VERSION_HEADER=%PROJECT_ROOT%\src\utils\VersionInfo.h"
set "APP_VERSION=0.0.0"
set "VS_VARS=%VS_PATH%\Auxiliary\Build\vcvars64.bat"

rem Variables for i18n
set "I18N_DIR=%PROJECT_ROOT%\i18n"
set "TS_FILE=%I18N_DIR%\%APP_NAME_BASE%_ru_RU.ts"
set "QM_FILE=%I18N_DIR%\%APP_NAME_BASE%_ru_RU.qm"

set "CLEAN_BUILD=NO"
set "STATIC_BUILD=NO"
set "BUILD_QT=NO"
set "USE_NINJA=NO"
set "BUILD_DEBUG=NO"
set "BUILD_RELEASE=NO"
set "ONLY_LUPDATE=NO"
set "ONLY_LRELEASE=NO"
set "NOOBSOLETE=NO"
set "KILL_RUNNING=NO"
set "PACKAGE_BUILD=NO"
set "RUN_DEPLOY=NO"
set "RUN_FORMAT=NO"

:parse_args
if "%~1"=="" goto :after_parse
if /i "%~1"=="--clean"       set "CLEAN_BUILD=YES"
if /i "%~1"=="--static"      set "STATIC_BUILD=YES"
if /i "%~1"=="--build-qt"    set "BUILD_QT=YES"
if /i "%~1"=="--ninja"       set "USE_NINJA=YES"
if /i "%~1"=="--debug"       set "BUILD_DEBUG=YES"
if /i "%~1"=="--release"     set "BUILD_RELEASE=YES"
if /i "%~1"=="--lupdate"     set "ONLY_LUPDATE=YES"
if /i "%~1"=="--lrelease"    set "ONLY_LRELEASE=YES"
if /i "%~1"=="--noobsolete"  set "NOOBSOLETE=YES" & set "ONLY_LUPDATE=YES"
if /i "%~1"=="--kill-running" set "KILL_RUNNING=YES"
if /i "%~1"=="--package"     set "PACKAGE_BUILD=YES"
if /i "%~1"=="--format"      set "RUN_FORMAT=YES"
if /i "%~1"=="--deploy"      set "RUN_DEPLOY=YES"
if /i "%~1"=="--help"        goto :show_help
rem Short options
if /i "%~1"=="-c"            set "CLEAN_BUILD=YES"
if /i "%~1"=="-s"            set "STATIC_BUILD=YES"
if /i "%~1"=="-bq"           set "BUILD_QT=YES"
if /i "%~1"=="-n"            set "USE_NINJA=YES"
if /i "%~1"=="-d"            set "BUILD_DEBUG=YES"
if /i "%~1"=="-r"            set "BUILD_RELEASE=YES"
if /i "%~1"=="-lu"           set "ONLY_LUPDATE=YES"
if /i "%~1"=="-lr"           set "ONLY_LRELEASE=YES"
if /i "%~1"=="-no"           set "NOOBSOLETE=YES" & set "ONLY_LUPDATE=YES"
if /i "%~1"=="-k"            set "KILL_RUNNING=YES"
if /i "%~1"=="-p"            set "PACKAGE_BUILD=YES"
if /i "%~1"=="-f"            set "RUN_FORMAT=YES"
if /i "%~1"=="-dp"           set "RUN_DEPLOY=YES"
if /i "%~1"=="-h"            goto :show_help

shift
goto :parse_args

:show_help
rem Long options
echo Usage: build.bat [OPTIONS]
echo.
echo   -c, --clean     Clean build directory before building
echo   -s, --static    Build with vcpkg (static linking)
echo   -bq, --build-qt Build/install Qt static components via vcpkg
echo   -n, --ninja     Use Ninja generator (default: Visual Studio)
echo   -d, --debug     Build Debug configuration
echo   -r, --release   Build Release configuration
echo   -lu, --lupdate  Run ONLY lupdate (extract strings to .ts)
echo   -lr, --lrelease Run ONLY lrelease (compile .ts to .qm)
echo   -no, --noobsolete  Run lupdate only, removing obsolete translations
echo   -k, --kill-running  Terminate %APP_NAME% before build if it is running
echo   -p, --package   Create ZIP package and SHA-256 checksum after build
echo   -f, --format    Run clang-format on all .cpp/.h files in src folder
echo   -dp, --deploy   Run windeployqt for shared builds
echo   -h, --help      Show this help message
echo.
echo Convenience wrappers:
echo   build_debug.bat    ^<^= build.bat --debug   [...]
echo   build_release.bat  ^<^= build.bat --release --static --ninja [...]
echo   build_package.bat  ^<^= build.bat --release --static --ninja --package [...]
echo   build_lrelease.bat ^<^= build.bat --lrelease [...]
echo   build_lupdate.bat  ^<^= build.bat --lupdate  [...]

exit /b 0

:after_parse

rem If build-qt is selected, ensure static build is also turned on
if "%BUILD_QT%"=="YES" (
    set "STATIC_BUILD=YES"
)

rem Check if ONLY translation tools should be run
set "RUN_ONLY_TOOLS=NO"
if "%ONLY_LUPDATE%"=="YES" (
    call :run_lupdate
    set "RUN_ONLY_TOOLS=YES"
)
if "%ONLY_LRELEASE%"=="YES" (
    call :run_lrelease
    set "RUN_ONLY_TOOLS=YES"
)
rem If only translation tools were run, skip the build process and jump to the end
if "%RUN_ONLY_TOOLS%"=="YES" goto :end_script

rem === STANDARD BUILD ===
call "%VS_VARS%"

if "%STATIC_BUILD%"=="YES" (
    call :check_and_prepare_vcpkg
    if errorlevel 1 goto :end_script
)

rem If format requested, run clang-format and exit
if "%RUN_FORMAT%"=="YES" (
    call :run_clang_format
    goto :end_script
)

echo [GIT] Updating submodules...
git submodule update --init --recursive
if errorlevel 1 (
    echo [ERROR] Failed to update git submodules.
    exit /b 1
)

if "%BUILD_DEBUG%"=="NO" if "%BUILD_RELEASE%"=="NO" (
    set "BUILD_DEBUG=YES"
    set "BUILD_RELEASE=YES"
)

call :load_version

if "%USE_NINJA%"=="YES" (
    set "GEN=ninja"
) else (
    set "GEN=vs"
)

if "%STATIC_BUILD%"=="YES" (
    set "TYPE=static"
) else (
    set "TYPE=shared"
)

set "PRESET_NAME=%GEN%-%TYPE%"
set "PRESET_BUILD_DIR=%BUILD_ROOT%\%PRESET_NAME%"

if "%CLEAN_BUILD%"=="YES" (
    if exist "%PRESET_BUILD_DIR%" (
        echo [CLEAN] Removing build directory for preset %PRESET_NAME%...
        rmdir /s /q "%PRESET_BUILD_DIR%"
    ) else (
        echo [CLEAN] Build directory for preset %PRESET_NAME% does not exist, skipping cleanup.
    )
)

if "%KILL_RUNNING%"=="YES" (
    call :kill_running_app
)

rem Automatically compile .ts into binary .qm catalog on every standard build
call :run_lrelease

for %%C in (debug release) do (
    if "!BUILD_%%C!"=="YES" (
        echo[INFO] Configuring and Building: !PRESET_NAME!
        cmake --preset !PRESET_NAME! -DMODEFLOW_UPDATE_URL="%MODEFLOW_UPDATE_URL%"
        cmake --build --preset !PRESET_NAME! --config %%C --parallel

        if "!RUN_DEPLOY!"=="YES" (
            echo [DEPLOY] Running windeployqt for !PRESET_NAME!
            call :deploy_shared "%%C"
        )

        if "!PACKAGE_BUILD!"=="YES" (
            call :package_build "%%C"
        )
    )
)

:end_script
rem Get the final time
for /f "tokens=1-3 delims=:.," %%a in ("%TIME%") do (
    set /a "END_TIME=(((%%a*60)+1%%b-100)*60)+1%%c-100"
)

rem Midnight bug fix (if the build process crosses midnight)
if %END_TIME% LSS %START_TIME% set /a "END_TIME+=86400"

rem Calculate the difference
set /a "DURATION=END_TIME-START_TIME"
set /a "MINS=DURATION/60"
set /a "SECS=DURATION%%60"

echo.
echo ============================================================================
echo Process finished in %MINS% minutes %SECS% seconds.
echo ============================================================================

popd
endlocal
exit /b

rem ============================================================================
rem SUBROUTINES
rem ============================================================================

:check_and_prepare_vcpkg
echo [VCPKG] Checking for vcpkg...

if exist "!VCPKG_EXE!" goto :vcpkg_ready

if exist "%PROJECT_ROOT%\vcpkg\vcpkg.exe" (
    set "VCPKG_EXE=%PROJECT_ROOT%\vcpkg\vcpkg.exe"
    goto :vcpkg_ready
)

where vcpkg >nul 2>&1
if not errorlevel 1 (
    for /f "delims=" %%I in ('where vcpkg') do (
        set "TEMP_PATH=%%I"
        echo !TEMP_PATH! | findstr /I "Microsoft Visual Studio" >nul
        if errorlevel 1 (
            set "VCPKG_EXE=!TEMP_PATH!"
            goto :vcpkg_ready
        )
    )
)

echo [VCPKG] vcpkg was not found.
set /p "INSTALL_VCPKG=Would you like to clone and bootstrap vcpkg in "%PROJECT_ROOT%\vcpkg"? (y/n): "
if /i "!INSTALL_VCPKG!"=="y" (
    echo [VCPKG] Cloning vcpkg...
    git clone https://github.com/microsoft/vcpkg.git "%PROJECT_ROOT%\vcpkg"
    if errorlevel 1 (
        echo [ERROR] Failed to clone vcpkg.
        exit /b 1
    )
    echo [VCPKG] Bootstrapping vcpkg...
    pushd "%PROJECT_ROOT%\vcpkg"
    call bootstrap-vcpkg.bat
    popd
    if not exist "%PROJECT_ROOT%\vcpkg\vcpkg.exe" (
        echo [ERROR] Failed to bootstrap vcpkg.
        exit /b 1
    )
    set "VCPKG_EXE=%PROJECT_ROOT%\vcpkg\vcpkg.exe"
    goto :vcpkg_ready
) else (
    echo [ERROR] Static build requires vcpkg. Aborting.
    exit /b 1
)

:vcpkg_ready
for %%I in ("!VCPKG_EXE!") do set "VCPKG_ROOT=%%~dpI"
if "!VCPKG_ROOT:~-1!"=="\" set "VCPKG_ROOT=!VCPKG_ROOT:~0,-1!"

echo [VCPKG] Using vcpkg at: !VCPKG_EXE!
echo [VCPKG] Overriding VCPKG_ROOT to: !VCPKG_ROOT!

set "QT_INSTALLED=YES"
for %%P in (qtbase qttools qtsvg qttranslations) do (
    "!VCPKG_EXE!" list | findstr /C:"%%P:x64-windows-static" >nul 2>&1
    if errorlevel 1 (
        echo [VCPKG] Missing package: %%P:x64-windows-static
        set "QT_INSTALLED=NO"
    )
)

if "!QT_INSTALLED!"=="NO" (
    set "PROCEED_BUILD_QT=NO"
    if "!BUILD_QT!"=="YES" (
        set "PROCEED_BUILD_QT=y"
    ) else (
        echo.
        echo [WARNING] Required Qt static components are missing in vcpkg.
        echo [WARNING] Building Qt from source can take from 30 minutes to several hours depending on your CPU.
        set /p "PROCEED_BUILD_QT=Do you want to build Qt now? (y/n): "
    )

    if /i "!PROCEED_BUILD_QT!"=="y" (
        echo [VCPKG] Installing Qt static libraries...
        "!VCPKG_EXE!" install "qtbase[openssl]" qttools qtsvg qttranslations --triplet=x64-windows-static
        if errorlevel 1 (
            echo [ERROR] Failed to install Qt packages via vcpkg.
            exit /b 1
        )
    ) else (
        echo [ERROR] Static build cannot proceed without required Qt packages.
        echo [INFO] You can run the build with the "--build-qt" flag to install them automatically.
        exit /b 1
    )
) else (
    echo [VCPKG] All required Qt static components are already installed.
)
goto :eof

:run_lupdate
echo [I18N] Running lupdate...
if not exist "%I18N_DIR%" mkdir "%I18N_DIR%"
set "LUPDATE_FLAGS=-locations none"
if "%NOOBSOLETE%"=="YES" (
    set "LUPDATE_FLAGS=-locations none -noobsolete"
    echo [I18N] Option: Removing obsolete strings
)
"%QT_DIR%\bin\lupdate.exe" "%PROJECT_ROOT%\src" %LUPDATE_FLAGS% -ts "%TS_FILE%"
goto :eof

:run_lrelease
if not exist "%TS_FILE%" (
    echo [I18N] TS file not found. Generating initial translation source via lupdate...
    call :run_lupdate
)
echo [I18N] Running lrelease...
"%QT_DIR%\bin\lrelease.exe" "%TS_FILE%"
goto :eof

:load_version
if not exist "%VERSION_HEADER%" goto :eof
for /f "usebackq tokens=*" %%A in (`powershell -NoProfile -Command "$t = [IO.File]::ReadAllText('%VERSION_HEADER%'); $t -match '#define APP_VERSION_MAJOR\s+(\d+)' | Out-Null; $maj = $Matches[1]; $t -match '#define APP_VERSION_MINOR\s+(\d+)' | Out-Null; $min = $Matches[1]; $t -match '#define APP_VERSION_PATCH\s+(\d+)' | Out-Null; $pat = $Matches[1]; '{0}.{1}.{2}' -f $maj,$min,$pat"`) do (
    set "APP_VERSION=%%A"
)
goto :eof

:deploy_shared
set "CONFIG=%~1"
set "BIN_DIR=%BUILD_ROOT%\bin\%CONFIG%"
echo [DEPLOY] Running windeployqt for %CONFIG%...
"%QT_DIR%\bin\windeployqt.exe" --%CONFIG% --compiler-runtime "%BIN_DIR%\%APP_NAME%"
"%QT_DIR%\bin\windeployqt.exe" --%CONFIG% --compiler-runtime "%BIN_DIR%\%APP_NAME_TESTS%"
goto :eof

:package_build
set "CONFIG=%~1"
set "BIN_DIR=%BUILD_ROOT%\bin\%CONFIG%"
set "ARTIFACTS_DIR=%BUILD_ROOT%\artifacts"

set "ARCHIVE_BASENAME=%APP_NAME_BASE%-v%APP_VERSION%-win-x64"
if /i not "%CONFIG%"=="release" set "ARCHIVE_BASENAME=%ARCHIVE_BASENAME%-%CONFIG%"

if not exist "%ARTIFACTS_DIR%" mkdir "%ARTIFACTS_DIR%"

echo [PACKAGE] Creating release package for %CONFIG%...

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
    "Copy-Item '%PROJECT_ROOT%\LICENSE' '%BIN_DIR%\LICENSE.txt' -ErrorAction SilentlyContinue;" ^
    "Copy-Item '%PROJECT_ROOT%\README.md' '%BIN_DIR%\README.md' -ErrorAction SilentlyContinue;" ^
    "Copy-Item '%PROJECT_ROOT%\THIRD_PARTY_NOTICES.md' '%BIN_DIR%\THIRD_PARTY_NOTICES.txt' -ErrorAction SilentlyContinue;" ^
    "if (Test-Path '%PROJECT_ROOT%\screenshots') { Copy-Item '%PROJECT_ROOT%\screenshots' '%BIN_DIR%\screenshots' -Recurse -Force | Out-Null };" ^
    "Compress-Archive -Path @('%BIN_DIR%\%APP_NAME%', '%BIN_DIR%\LICENSE.txt', '%BIN_DIR%\README.md', '%BIN_DIR%\THIRD_PARTY_NOTICES.txt', '%BIN_DIR%\screenshots') -DestinationPath '%ARTIFACTS_DIR%\%ARCHIVE_BASENAME%.zip' -Force;" ^
    "$hash = (Get-FileHash -Algorithm SHA256 '%ARTIFACTS_DIR%\%ARCHIVE_BASENAME%.zip').Hash.ToLower();" ^
    "('{0} *{1}' -f $hash, '%ARCHIVE_BASENAME%.zip') | Set-Content -Path '%ARTIFACTS_DIR%\%ARCHIVE_BASENAME%.zip.sha256' -Encoding ascii;" ^
    "$rawChangelog = if (Test-Path '%PROJECT_ROOT%\metadata\changelog.md') { [System.IO.File]::ReadAllText('%PROJECT_ROOT%\metadata\changelog.md') } else { 'See GitHub Releases for details.' };" ^
    "$changelog = $rawChangelog.Replace('{VERSION}', '%APP_VERSION%');" ^
    "$changelog | Set-Content -Path '%ARTIFACTS_DIR%\temp_changelog.md' -Encoding utf8;" ^
    "$json = [ordered]@{ version = '%APP_VERSION%'; url = '%GIT_HOST_URL%/releases/tag/v%APP_VERSION%'; changelog = $changelog };" ^
    "$json | ConvertTo-Json | Set-Content -Path '%PROJECT_ROOT%\metadata\update.json' -Encoding utf8"

if errorlevel 1 (
    echo [ERROR] Failed to package build for %CONFIG%.
    exit /b 1
)

echo [PACKAGE] Package ready: %ARTIFACTS_DIR%\%ARCHIVE_BASENAME%.zip
echo [PACKAGE] Checksum ready: %ARTIFACTS_DIR%\%ARCHIVE_BASENAME%.zip.sha256
echo [PACKAGE] Update manifest ready: %PROJECT_ROOT%\metadata\update.json
goto :eof

:run_clang_format
echo [FORMAT] Running clang-format on src folder...
set "CLANG_FORMAT=clang-format.exe"
where %CLANG_FORMAT% >nul 2>&1
if errorlevel 1 (
    set "CLANG_FORMAT=%VS_PATH%\Tools\Llvm\x64\bin\clang-format.exe"
    if not exist "!CLANG_FORMAT!" (
        echo [ERROR] clang-format.exe not found in PATH nor in VS default location.
        exit /b 1
    )
)
set "FORMAT_COUNT=0"
for /R "%PROJECT_ROOT%\src" %%F in (*.cpp *.h *.hpp) do (
    echo   Formatting %%~nxF
    "!CLANG_FORMAT!" -i "%%F"
    if not errorlevel 1 set /a FORMAT_COUNT+=1
)
echo [FORMAT] Formatted %FORMAT_COUNT% files.
goto :eof