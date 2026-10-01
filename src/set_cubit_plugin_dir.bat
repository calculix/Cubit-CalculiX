@echo off
rem Resolve the script's directory without a trailing backslash for icacls.
for %%D in ("%~dp0.") do set "CUBIT_PLUGIN_DIR=%%~fD"
for /f "delims=" %%U in ('whoami') do set "CUBIT_PLUGIN_USER=%%U"
if not defined CUBIT_PLUGIN_USER (
    echo Failed to identify the current user.
    pause
    exit /b 1
)
icacls "%CUBIT_PLUGIN_DIR%" /grant "%CUBIT_PLUGIN_USER%:(OI)(CI)M" /T
if errorlevel 1 (
    echo Failed to grant write access to "%CUBIT_PLUGIN_DIR%".
    echo Run this script as administrator if Windows denies permission.
    pause
    exit /b 1
)
setx CUBIT_PLUGIN_DIR "%CUBIT_PLUGIN_DIR%"
if errorlevel 1 (
    echo Failed to save CUBIT_PLUGIN_DIR for the current user.
    pause
    exit /b 1
)
echo CUBIT_PLUGIN_DIR=%CUBIT_PLUGIN_DIR%
echo Restart Cubit to use the new value.
pause
