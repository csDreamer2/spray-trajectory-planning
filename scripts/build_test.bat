@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ========================================
echo 环境配置验证脚本
echo ========================================
echo.

set "passed=0"
set "failed=0"

REM ========================================
REM 第1部分: 检查基础工具
REM ========================================
echo [INFO] 检查基础工具...
echo.

REM 检查CMake
cmake --version >nul 2>&1
if errorlevel 1 (
    echo [FAIL] CMake: 未找到
    set /a failed+=1
) else (
    echo [OK] CMake: 已安装
    set /a passed+=1
)

REM 检查Git
git --version >nul 2>&1
if errorlevel 1 (
    echo [FAIL] Git: 未找到
    set /a failed+=1
) else (
    echo [OK] Git: 已安装
    set /a passed+=1
)

REM 检查Visual Studio
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
    echo [OK] Visual Studio 2022: 已安装
    set /a passed+=1
) else (
    echo [FAIL] Visual Studio 2022: 未找到
    set /a failed+=1
)

echo.

REM ========================================
REM 第2部分: 检查第三方库
REM ========================================
echo [INFO] 检查第三方库...
echo.

REM 检查VTK GUISupportQt库
if exist "K:\Tools\vtkQT\build\lib\vtkGUISupportQt-9.2-gd.lib" (
    echo [OK] VTK GUISupportQt Debug库: 存在
    set /a passed+=1
) else (
    echo [FAIL] VTK GUISupportQt Debug库: 不存在
    set /a failed+=1
)

REM 检查VTK IOXML库
if exist "K:\Tools\vtkQT\build\lib\vtkIOXML-9.2-gd.lib" (
    echo [OK] VTK IOXML Debug库: 存在
    set /a passed+=1
) else (
    echo [FAIL] VTK IOXML Debug库: 不存在
    set /a failed+=1
)

REM 检查OpenCASCADE CMake配置
if exist "K:\Tools\OpenCasCade\install\cmake\OpenCASCADEConfig.cmake" (
    echo [OK] OpenCASCADE CMake配置: 存在
    set /a passed+=1
) else (
    echo [FAIL] OpenCASCADE CMake配置: 不存在
    set /a failed+=1
)

REM 检查Qt6 CMake配置
if exist "K:\Kapps\Qt\6.10.1\msvc2022_64\lib\cmake\Qt6\Qt6Config.cmake" (
    echo [OK] Qt6 CMake配置: 存在
    set /a passed+=1
) else (
    echo [FAIL] Qt6 CMake配置: 不存在
    set /a failed+=1
)

REM 检查PCL CMake配置
if exist "F:\PCL 1.13.0\cmake\PCLConfig.cmake" (
    echo [OK] PCL CMake配置: 存在
    set /a passed+=1
) else (
    echo [FAIL] PCL CMake配置: 不存在
    set /a failed+=1
)

echo.

REM ========================================
REM 第3部分: 检查项目结构
REM ========================================
echo [INFO] 检查项目结构...
echo.

REM 检查src目录
if exist "src\UI\MainWindow.cpp" (
    echo [OK] src目录: 存在
    set /a passed+=1
) else (
    echo [FAIL] src目录: 不存在
    set /a failed+=1
)

REM 检查CMakeLists.txt
if exist "CMakeLists.txt" (
    echo [OK] CMakeLists.txt: 存在
    set /a passed+=1
) else (
    echo [FAIL] CMakeLists.txt: 不存在
    set /a failed+=1
)

REM 检查paths.cmake
if exist "config\paths.cmake" (
    echo [OK] config/paths.cmake: 存在
    set /a passed+=1
) else (
    echo [FAIL] config/paths.cmake: 不存在
    set /a failed+=1
)

REM 检查main.cpp
if exist "src\main.cpp" (
    echo [OK] src/main.cpp: 存在
    set /a passed+=1
) else (
    echo [FAIL] src/main.cpp: 不存在
    set /a failed+=1
)

echo.

REM ========================================
REM 第4部分: 总结
REM ========================================
echo ========================================
echo 验证结果总结
echo ========================================
echo.
set /a total=passed+failed
echo 总检查项: %total%
echo 通过: %passed%
echo 失败: %failed%
echo.

if %failed% equ 0 (
    echo [OK] 所有检查通过！环境配置正确。
    echo.
    echo 下一步:
    echo 1. 运行编译脚本: build_project.bat
    echo 2. 或手动编译: cmake --build build --config Debug
    echo.
    pause
    exit /b 0
) else (
    echo [FAIL] 发现 %failed% 个问题。
    echo.
    echo 问题排查:
    echo 1. VTK库文件: K:\Tools\vtkQT\build\lib\
    echo 2. OpenCASCADE: K:\Tools\OpenCasCade\install\cmake\
    echo 3. Qt6: K:\Kapps\Qt\6.10.1\msvc2022_64\lib\cmake\Qt6\
    echo 4. PCL: F:\PCL 1.13.0\cmake\
    echo.
    pause
    exit /b 1
)
