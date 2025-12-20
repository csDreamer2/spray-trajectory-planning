@echo off
echo OpenCASCADE Configuration Test
echo ==============================

cd /d "%~dp0"

if exist "build\bin\Debug\OpenCASCADETest.exe" (
    echo Testing basic OpenCASCADE functionality...
    echo.
    "build\bin\Debug\OpenCASCADETest.exe"
    echo.
    echo Testing with MPX3500.STEP file...
    echo WARNING: This will take about 90 seconds
    echo.
    "build\bin\Debug\OpenCASCADETest.exe" "data\model\MPX3500.STEP"
) else (
    echo ERROR: OpenCASCADETest.exe not found
    echo Please compile the project first
    pause
    exit /b 1
)

pause