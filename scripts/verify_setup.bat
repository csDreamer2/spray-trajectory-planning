@echo off
setlocal enabledelayedexpansion
chcp 936 > nul

:: Change to project root directory
cd /d "%~dp0\.."

echo ========================================
echo Development Environment Verification Script
echo Author: Wang Rui (Zhejiang University)
echo Version: v1.0
echo ========================================
echo.

echo Script location: %~dp0
echo Project root: %CD%
echo Verifying development environment configuration...
echo.

:: Check basic tools
echo [1/8] Checking CMake...
cmake --version >nul 2>&1
if %ERRORLEVEL% == 0 (
    echo [OK] CMake is installed
    cmake --version | findstr "cmake version"
) else (
    echo [ERROR] CMake not found, please install CMake 3.16+
    set HAS_ERROR=1
)

echo.
echo [2/8] Checking Git...
git --version >nul 2>&1
if %ERRORLEVEL% == 0 (
    echo [OK] Git is installed
    git --version
) else (
    echo [ERROR] Git not found, please install Git
    set HAS_ERROR=1
)

echo.
echo [3/8] Checking Visual Studio...
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
    "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property displayName > vs_info.txt 2>nul
    if exist vs_info.txt (
        set /p VS_NAME=<vs_info.txt
        echo [OK] Visual Studio found: !VS_NAME!
        del vs_info.txt
    ) else (
        echo [ERROR] Visual Studio not found via vswhere
        set HAS_ERROR=1
    )
) else (
    echo [ERROR] Visual Studio Installer not found
    echo Please install Visual Studio 2019/2022
    set HAS_ERROR=1
)

echo.
echo [4/8] Checking configuration files...
if exist "config\paths.cmake" (
    echo [OK] Path configuration file exists: config\paths.cmake
) else (
    echo [ERROR] Path configuration file not found: config\paths.cmake
    echo Current directory: %CD%
    echo Please run scripts\setup_environment.bat to configure
    set HAS_ERROR=1
)

echo.
echo [5/8] Checking project structure...
set REQUIRED_DIRS=src docs config scripts data
set MISSING_DIRS=

for %%d in (%REQUIRED_DIRS%) do (
    if not exist "%%d" (
        if "!MISSING_DIRS!" == "" (
            set MISSING_DIRS=%%d
        ) else (
            set MISSING_DIRS=!MISSING_DIRS! %%d
        )
    )
)

if "!MISSING_DIRS!" == "" (
    echo [OK] Project directory structure is complete
    echo Found directories: %REQUIRED_DIRS%
) else (
    echo [ERROR] Missing directories: !MISSING_DIRS!
    echo Current directory: %CD%
    dir /b | findstr /C:"src docs config scripts data"
    set HAS_ERROR=1
)

echo.
echo [6/8] Testing CMake configuration...
if exist "build_verify" rmdir /s /q "build_verify"
mkdir build_verify
cd build_verify

echo Running: cmake .. -G "Visual Studio 17 2022" -A x64 -DQt6_DIR="K:/Kapps/Qt/6.10.1/msvc2022_64/lib/cmake/Qt6"
cmake .. -G "Visual Studio 17 2022" -A x64 -DQt6_DIR="K:/Kapps/Qt/6.10.1/msvc2022_64/lib/cmake/Qt6" > cmake_test.log 2>&1

if %ERRORLEVEL% == 0 (
    echo [OK] CMake configuration successful
) else (
    echo [ERROR] CMake configuration failed (Exit code: %ERRORLEVEL%)
    echo Error details:
    type cmake_test.log
    set HAS_ERROR=1
)

cd ..

echo.
echo [7/8] Checking third-party library paths...

:: Read paths from configuration file (improved check)
if exist "config\paths.cmake" (
    echo Parsing config\paths.cmake...
    findstr "TOOLS_BASE_DIR" config\paths.cmake > temp_path.txt 2>nul
    if exist temp_path.txt (
        for /f "tokens=2 delims=\" %%i in (temp_path.txt) do (
            set TOOLS_DIR=%%i
            echo Found TOOLS_BASE_DIR: !TOOLS_DIR!
        )
        del temp_path.txt
        
        if defined TOOLS_DIR (
            if exist "!TOOLS_DIR!" (
                echo [OK] Tools base directory exists: !TOOLS_DIR!
                
                if exist "!TOOLS_DIR!\vtkQT\build" (
                    echo [OK] VTK directory exists: !TOOLS_DIR!\vtkQT\build
                ) else (
                    echo [ERROR] VTK directory not found: !TOOLS_DIR!\vtkQT\build
                    set HAS_ERROR=1
                )
                
                if exist "!TOOLS_DIR!\OpenCasCade\install" (
                    echo [OK] OpenCASCADE directory exists: !TOOLS_DIR!\OpenCasCade\install
                ) else (
                    echo [ERROR] OpenCASCADE directory not found: !TOOLS_DIR!\OpenCasCade\install
                    set HAS_ERROR=1
                )
            ) else (
                echo [ERROR] Tools base directory not found: !TOOLS_DIR!
                set HAS_ERROR=1
            )
        ) else (
            echo [WARNING] Could not parse TOOLS_BASE_DIR from config file
        )
    ) else (
        echo [WARNING] TOOLS_BASE_DIR not found in config file
    )
) else (
    echo [ERROR] Configuration file not found: config\paths.cmake
    set HAS_ERROR=1
)

echo.
echo [8/8] Checking test data...
if exist "data\model\MPX3500.STEP" (
    echo [OK] Test STEP file exists
) else (
    echo [WARNING] Test STEP file not found: data\model\MPX3500.STEP
    echo This will not affect compilation but will affect functional testing
)

echo.
echo ========================================
echo Verification Results
echo ========================================
echo.

if "!HAS_ERROR!" == "1" (
    echo [ERROR] Environment verification failed!
    echo.
    echo Please resolve the above issues and run verification again.
    echo For help, please refer to docs\Development_Environment_Guide.md
    echo.
    pause
    exit /b 1
) else (
    echo [OK] Environment verification successful!
    echo.
    echo Your development environment is properly configured and ready for development.
    echo.
    echo Next steps:
    echo 1. Compile project: mkdir build ^&^& cd build ^&^& cmake .. -G "Visual Studio 17 2022" -A x64 -DQt6_DIR="K:/Kapps/Qt/6.10.1/msvc2022_64/lib/cmake/Qt6"
    echo 2. Build project: cmake --build . --config Debug
    echo 3. Run program: build\bin\Debug\SprayTrajectoryPlanning.exe
    echo.
)

pause