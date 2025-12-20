@echo off
setlocal enabledelayedexpansion
chcp 936 > nul

:: Change to project root directory
cd /d "%~dp0\.."

echo ========================================
echo Spray Trajectory Planning System - Environment Setup
echo Author: Wang Rui (Zhejiang University)
echo Version: v1.0
echo ========================================
echo.

echo Script location: %~dp0
echo Project root: %CD%
echo This script will help you configure the development environment
echo.

echo Please ensure you have completed the following steps:
echo 1. Install Visual Studio 2019/2022
echo 2. Install Qt 6.10+
echo 3. Compile VTK 9.2
echo 4. Compile OpenCASCADE 7.8
echo 5. Install PCL 1.13
echo.

pause

echo.
echo ========================================
echo Step 1: Configure Tool Paths
echo ========================================
echo.

set /p TOOLS_DIR="Please enter your tools installation directory (e.g., K:/Tools): "

if not exist "%TOOLS_DIR%" (
    echo Error: Directory %TOOLS_DIR% does not exist!
    pause
    exit /b 1
)

echo Tools directory set to: %TOOLS_DIR%
echo.

echo ========================================
echo Step 2: Verify VTK Installation
echo ========================================
echo.

set VTK_DIR=%TOOLS_DIR%/vtkQT/build
if exist "%VTK_DIR%" (
    echo [OK] VTK directory found: %VTK_DIR%
) else (
    echo [ERROR] VTK directory not found: %VTK_DIR%
    echo Please ensure VTK is compiled to this directory
    pause
    exit /b 1
)

echo.
echo ========================================
echo Step 3: Verify OpenCASCADE Installation
echo ========================================
echo.

set OPENCASCADE_DIR=%TOOLS_DIR%/OpenCasCade/install
if exist "%OPENCASCADE_DIR%" (
    echo [OK] OpenCASCADE directory found: %OPENCASCADE_DIR%
) else (
    echo [ERROR] OpenCASCADE directory not found: %OPENCASCADE_DIR%
    echo Please ensure OpenCASCADE is installed to this directory
    pause
    exit /b 1
)

echo.
echo ========================================
echo Step 4: Update Configuration File
echo ========================================
echo.

echo Updating config/paths.cmake...

:: Create temporary configuration file
(
echo # =============================================================================
echo # Spray Trajectory Planning System - Path Configuration File ^(Auto-generated^)
echo # =============================================================================
echo #
echo # Author: Wang Rui ^(Zhejiang University^)
echo # Version: v1.0
echo # Date: 2025-12-20
echo #
echo # This file contains absolute path configurations for all third-party libraries
echo # New developers need to modify these paths according to their environment
echo #
echo # Modification Instructions:
echo # 1. Replace all "K:/Tools" paths with your tools installation directory
echo # 2. Ensure all paths exist and are correct
echo # 3. Keep path format consistent ^(use forward slashes /^)
echo #
echo # =============================================================================
echo.
echo # -----------------------------------------------------------------------------
echo # Main Tools Directory Configuration
echo # -----------------------------------------------------------------------------
echo # Modify this base path to your tools installation directory
echo set^(TOOLS_BASE_DIR "%TOOLS_DIR%" CACHE PATH "Tools base installation directory"^)
echo.
echo # -----------------------------------------------------------------------------
echo # VTK Configuration ^(Version 9.2^)
echo # -----------------------------------------------------------------------------
echo # VTK build output directory
echo set^(VTK_BUILD_DIR "${TOOLS_BASE_DIR}/vtkQT/build" CACHE PATH "VTK build output directory"^)
echo set^(VTK_CMAKE_DIR "${VTK_BUILD_DIR}/lib/cmake/vtk-9.2" CACHE PATH "VTK CMake config directory"^)
echo.
echo # VTK path validation
echo if^(NOT EXISTS "${VTK_BUILD_DIR}"^)
echo     message^(FATAL_ERROR "[ERROR] VTK directory not found: ${VTK_BUILD_DIR}"^)
echo endif^(^)
echo.
echo message^(STATUS "[OK] VTK Configuration:"^)
echo message^(STATUS "   Build Directory: ${VTK_BUILD_DIR}"^)
echo message^(STATUS "   CMake Directory: ${VTK_CMAKE_DIR}"^)
echo.
echo # -----------------------------------------------------------------------------
echo # OpenCASCADE Configuration ^(Version 7.8^)
echo # -----------------------------------------------------------------------------
echo # OpenCASCADE installation directory
echo set^(OPENCASCADE_INSTALL_DIR "${TOOLS_BASE_DIR}/OpenCasCade/install" CACHE PATH "OpenCASCADE install directory"^)
echo set^(OPENCASCADE_CMAKE_DIR "${OPENCASCADE_INSTALL_DIR}/cmake" CACHE PATH "OpenCASCADE CMake config directory"^)
echo set^(OPENCASCADE_INCLUDE_DIR "${OPENCASCADE_INSTALL_DIR}/inc" CACHE PATH "OpenCASCADE include directory"^)
echo set^(OPENCASCADE_LIB_DIR "${OPENCASCADE_INSTALL_DIR}/win64/vc14/libd" CACHE PATH "OpenCASCADE library directory"^)
echo.
echo # OpenCASCADE path validation
echo if^(NOT EXISTS "${OPENCASCADE_INSTALL_DIR}"^)
echo     message^(FATAL_ERROR "[ERROR] OpenCASCADE directory not found: ${OPENCASCADE_INSTALL_DIR}"^)
echo endif^(^)
echo.
echo if^(NOT EXISTS "${OPENCASCADE_CMAKE_DIR}/OpenCASCADEConfig.cmake"^)
echo     message^(FATAL_ERROR "[ERROR] OpenCASCADE config file not found: ${OPENCASCADE_CMAKE_DIR}/OpenCASCADEConfig.cmake"^)
echo endif^(^)
echo.
echo message^(STATUS "[OK] OpenCASCADE Configuration:"^)
echo message^(STATUS "   Install Directory: ${OPENCASCADE_INSTALL_DIR}"^)
echo message^(STATUS "   Include Directory: ${OPENCASCADE_INCLUDE_DIR}"^)
echo message^(STATUS "   Library Directory: ${OPENCASCADE_LIB_DIR}"^)
echo message^(STATUS "   CMake Directory: ${OPENCASCADE_CMAKE_DIR}"^)
echo.
echo # -----------------------------------------------------------------------------
echo # Qt Configuration ^(Auto-detection^)
echo # -----------------------------------------------------------------------------
echo # Qt is usually auto-detected via environment variables or CMake, but you can specify a specific version here
echo # set^(Qt6_DIR "C:/Qt/6.10.0/msvc2022_64/lib/cmake/Qt6" CACHE PATH "Qt6 CMake directory"^)
echo.
echo message^(STATUS "[OK] Qt Configuration: Using system-detected Qt version"^)
echo.
echo # -----------------------------------------------------------------------------
echo # Apply CMAKE Prefix Paths
echo # -----------------------------------------------------------------------------
echo # Add configured paths to CMAKE_PREFIX_PATH
echo list^(APPEND CMAKE_PREFIX_PATH 
echo     "${VTK_BUILD_DIR}"
echo     "${OPENCASCADE_INSTALL_DIR}"
echo ^)
echo.
echo # Set specific directory variables for CMake use
echo set^(VTK_DIR "${VTK_CMAKE_DIR}" CACHE PATH "VTK directory" FORCE^)
echo set^(OpenCASCADE_DIR "${OPENCASCADE_CMAKE_DIR}" CACHE PATH "OpenCASCADE directory" FORCE^)
echo.
echo # -----------------------------------------------------------------------------
echo # Compiler and Linker Configuration
echo # -----------------------------------------------------------------------------
echo # Add library directories
echo link_directories^("${OPENCASCADE_LIB_DIR}"^)
echo.
echo # Add include directories
echo include_directories^("${OPENCASCADE_INCLUDE_DIR}"^)
echo.
echo # -----------------------------------------------------------------------------
echo # Configuration Summary
echo # -----------------------------------------------------------------------------
echo message^(STATUS "=== Path Configuration Summary ==="^)
echo message^(STATUS "Tools Base Directory: ${TOOLS_BASE_DIR}"^)
echo message^(STATUS "VTK Directory: ${VTK_BUILD_DIR}"^)
echo message^(STATUS "OpenCASCADE Directory: ${OPENCASCADE_INSTALL_DIR}"^)
echo message^(STATUS "=================================="^)
) > config\paths_temp.cmake

:: Replace original configuration file
if exist "config\paths.cmake" (
    copy "config\paths.cmake" "config\paths.cmake.backup"
    echo Original config file backed up as paths.cmake.backup
)

move "config\paths_temp.cmake" "config\paths.cmake"
echo [OK] Configuration file updated

echo.
echo ========================================
echo Step 5: Test Configuration
echo ========================================
echo.

echo Testing CMake configuration...
mkdir build_test 2>nul
cd build_test

cmake .. -G "Visual Studio 17 2022" -A x64 -DQt6_DIR="K:/Kapps/Qt/6.10.1/msvc2022_64/lib/cmake/Qt6" > cmake_output.txt 2>&1

if %ERRORLEVEL% == 0 (
    echo [OK] CMake configuration test successful!
    echo Detailed output saved to build_test/cmake_output.txt
) else (
    echo [ERROR] CMake configuration test failed!
    echo Error details:
    type cmake_output.txt
    echo.
    echo Please check:
    echo 1. Visual Studio 2019/2022 is properly installed
    echo 2. Qt is in system PATH or specify Qt6_DIR
    echo 3. All third-party library paths are correct
    cd ..
    pause
    exit /b 1
)

cd ..

echo.
echo ========================================
echo Configuration Complete!
echo ========================================
echo.

echo Environment configuration completed. You can now:
echo.
echo 1. Compile project:
echo    mkdir build
echo    cd build
echo    cmake .. -G "Visual Studio 17 2022" -A x64 -DQt6_DIR="K:/Kapps/Qt/6.10.1/msvc2022_64/lib/cmake/Qt6"
echo    cmake --build . --config Debug
echo.
echo 2. Or open in Visual Studio:
echo    build/SprayTrajectoryPlanning.sln
echo.
echo 3. Run tests:
echo    build/bin/Debug/SprayTrajectoryPlanning.exe
echo.

echo If you encounter issues, please refer to docs/Development_Environment_Guide.md
echo.

pause