@echo off
setlocal EnableDelayedExpansion

rem ============================================================================
rem ModeFlow – Fast Test Runner Convenience Script
rem Builds ModeFlowTests and executes the CTest suite.
rem ============================================================================

echo [TESTS] Building ModeFlow and tests with Ninja...
call "%~dp0build.bat" --debug --ninja --tests %*
if errorlevel 1 (
    echo [ERROR] Build failed. Aborting test execution.
    exit /b 1
)

set "PRESET_DIR=%~dp0..\build\ninja-shared"

echo.
echo ============================================================================
echo [TESTS] Running CTest suite...
echo ============================================================================

ctest --test-dir "%PRESET_DIR%" --output-on-failure -C Debug

if errorlevel 1 (
    echo.
    echo [ERROR] One or more unit tests failed!
    exit /b 1
) else (
    echo.
    echo [SUCCESS] All unit tests passed successfully!
)

endlocal
exit /b 0