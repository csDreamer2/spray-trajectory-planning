@echo off
chcp 936 > nul
setlocal enabledelayedexpansion

echo ========================================
echo STEP模型树编译测试
echo ========================================
echo.

cd /d "%~dp0\.."

echo [1/3] 检查源文件...
if not exist "src\Data\STEPModelTree.h" (
    echo 错误: STEPModelTree.h 不存在
    pause
    exit /b 1
)

if not exist "src\Data\STEPModelTree.cpp" (
    echo 错误: STEPModelTree.cpp 不存在
    pause
    exit /b 1
)

echo ✓ 源文件检查通过

echo [2/3] 创建测试构建目录...
if not exist "build_test" mkdir "build_test"
cd build_test

echo [3/3] 配置CMake并编译...
cmake .. -G "Visual Studio 17 2022" -A x64 -DBUILD_TESTS=ON -DQt6_DIR="K:/Kapps/Qt/6.10.1/msvc2022_64/lib/cmake/Qt6"

if %ERRORLEVEL% neq 0 (
    echo.
    echo CMake配置失败！
    echo 请检查：
    echo 1. Visual Studio 2022是否安装
    echo 2. Qt路径是否正确
    echo 3. OpenCASCADE是否正确配置
    echo.
    pause
    exit /b 1
)

echo.
echo 开始编译...
cmake --build . --config Debug --target test_step_model_tree

if %ERRORLEVEL% neq 0 (
    echo.
    echo 编译失败！
    echo 常见问题：
    echo 1. 检查OpenCASCADE库路径
    echo 2. 检查Qt库版本兼容性
    echo 3. 检查C++17编译器支持
    echo.
    pause
    exit /b 1
)

echo.
echo ========================================
echo 编译成功！
echo ========================================
echo.
echo 可执行文件位置: build_test\bin\Debug\test_step_model_tree.exe
echo.

set /p RUN_TEST="是否运行测试程序？(y/n): "
if /i "!RUN_TEST!" == "y" (
    echo.
    echo 启动测试程序...
    if exist "bin\Debug\test_step_model_tree.exe" (
        bin\Debug\test_step_model_tree.exe
    ) else (
        echo 错误: 找不到可执行文件
    )
)

pause