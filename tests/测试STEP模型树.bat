@echo off
chcp 936 > nul
setlocal enabledelayedexpansion

echo ========================================
echo STEP模型树功能测试
echo ========================================
echo.

cd /d "%~dp0\.."

echo [1/4] 检查构建目录...
if not exist "build" (
    echo 错误: build目录不存在，请先编译主项目
    pause
    exit /b 1
)

echo [2/4] 创建测试构建目录...
if not exist "build\tests" mkdir "build\tests"

echo [3/4] 编译测试程序...
cd build

:: 使用CMake编译测试程序
cmake --build . --target test_step_model_tree --config Debug

if %ERRORLEVEL% neq 0 (
    echo.
    echo 编译失败！请检查以下内容：
    echo 1. 确保主项目已成功编译
    echo 2. 检查OpenCASCADE库是否正确配置
    echo 3. 检查Qt库是否正确配置
    echo.
    pause
    exit /b 1
)

echo [4/4] 运行测试程序...
echo.
echo ========================================
echo 测试程序启动
echo ========================================
echo.

:: 运行测试程序
if exist "bin\Debug\test_step_model_tree.exe" (
    bin\Debug\test_step_model_tree.exe
) else if exist "tests\Debug\test_step_model_tree.exe" (
    tests\Debug\test_step_model_tree.exe
) else (
    echo 错误: 找不到测试程序可执行文件
    echo 请检查编译输出目录
    pause
    exit /b 1
)

echo.
echo ========================================
echo 测试完成
echo ========================================
pause