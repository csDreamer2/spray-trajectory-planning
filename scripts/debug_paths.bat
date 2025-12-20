@echo off
setlocal enabledelayedexpansion
chcp 936 > nul

:: Change to project root directory
cd /d "%~dp0\.."

echo ========================================
echo Path Debug Script
echo ========================================
echo.

echo Script location: %~dp0
echo Project root: %CD%
echo.

echo Checking if directories exist:
for %%d in (src docs config scripts data) do (
    if exist "%%d" (
        echo [OK] %%d exists
    ) else (
        echo [ERROR] %%d not found
    )
)

echo.
echo Checking config file:
if exist "config\paths.cmake" (
    echo [OK] config\paths.cmake exists
    echo File size:
    dir "config\paths.cmake" | findstr "paths.cmake"
    echo.
    echo First few lines:
    type "config\paths.cmake" | more +1
) else (
    echo [ERROR] config\paths.cmake not found
)

echo.
echo Checking data directory:
if exist "data" (
    echo [OK] data directory exists
    echo Contents:
    dir /b data
    if exist "data\model" (
        echo [OK] data\model exists
        echo Model files:
        dir /b "data\model" 2>nul
    ) else (
        echo [ERROR] data\model not found
    )
) else (
    echo [ERROR] data directory not found
)

echo.
echo Visual Studio check:
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
    echo [OK] vswhere.exe found
    "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property displayName
) else (
    echo [ERROR] vswhere.exe not found
)

pause