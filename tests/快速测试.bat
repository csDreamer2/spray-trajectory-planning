@echo off
chcp 936 > nul
setlocal enabledelayedexpansion

echo ========================================
echo STEP模型树快速测试
echo ========================================
echo.

cd /d "%~dp0\.."

echo [1/4] 检查测试文件...
set TEST_FILE=data\model\MPX3500.STEP
if not exist "%TEST_FILE%" (
    echo 警告: 测试文件 %TEST_FILE% 不存在
    echo 请将MPX3500.STEP文件放入data/model/目录
    echo.
    set /p CONTINUE="是否继续测试其他功能？(y/n): "
    if /i "!CONTINUE!" neq "y" (
        pause
        exit /b 1
    )
) else (
    echo ✓ 找到测试文件: %TEST_FILE%
)

echo [2/4] 检查构建环境...
if not exist "build" (
    echo 创建构建目录...
    mkdir build
)

cd build

echo [3/4] 快速编译测试...
echo 配置CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 -DBUILD_TESTS=ON -DQt6_DIR="K:/Kapps/Qt/6.10.1/msvc2022_64/lib/cmake/Qt6" > cmake_output.log 2>&1

if %ERRORLEVEL% neq 0 (
    echo CMake配置失败，查看详细错误：
    type cmake_output.log
    pause
    exit /b 1
)

echo 编译简单测试程序...
cmake --build . --config Debug --target simple_step_test > build_output.log 2>&1

if %ERRORLEVEL% neq 0 (
    echo 编译失败，查看详细错误：
    type build_output.log
    echo.
    echo 常见问题解决：
    echo 1. 确保OpenCASCADE库已正确安装和配置
    echo 2. 检查config/paths.cmake中的路径设置
    echo 3. 确保Qt6路径正确
    pause
    exit /b 1
)

echo [4/4] 运行测试...
echo.
echo ========================================
echo 开始功能测试
echo ========================================

if exist "bin\Debug\simple_step_test.exe" (
    echo 运行简单测试程序...
    echo.
    bin\Debug\simple_step_test.exe "..\%TEST_FILE%"
    echo.
    echo ========================================
    echo 测试完成
    echo ========================================
) else (
    echo 错误: 找不到测试程序 bin\Debug\simple_step_test.exe
    echo 请检查编译输出
)

echo.
set /p VIEW_LOG="查看编译日志？(y/n): "
if /i "!VIEW_LOG!" == "y" (
    echo.
    echo === CMake配置日志 ===
    type cmake_output.log
    echo.
    echo === 编译日志 ===
    type build_output.log
)

pause