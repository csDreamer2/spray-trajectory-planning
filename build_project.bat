@echo off
chcp 65001 >nul
REM STEP模型加载功能重构后的编译脚本
REM 作者: 王睿 (浙江大学)
REM 日期: 2026-01-07

echo ========================================
echo STEP模型加载功能 - 编译脚本
echo ========================================
echo.

REM 检查build目录是否存在
if not exist "build" (
    echo [错误] build目录不存在，请先运行CMake配置
    echo 运行命令: cmake -B build -S .
    pause
    exit /b 1
)

cd build

echo [步骤1/3] 清理旧的构建文件...
cmake --build . --target clean
if errorlevel 1 (
    echo [警告] 清理失败，继续编译...
)
echo.

echo [步骤2/3] 配置CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 -DQt6_DIR="K:/Kapps/Qt/6.10.1/msvc2022_64/lib/cmake/Qt6"
if errorlevel 1 (
    echo [错误] CMake配置失败
    cd ..
    pause
    exit /b 1
)
echo.

echo [步骤3/3] 编译项目 (Debug模式)...
cmake --build . --config Debug
if errorlevel 1 (
    echo [错误] 编译失败
    cd ..
    pause
    exit /b 1
)
echo.

cd ..

echo ========================================
echo 编译成功！
echo ========================================
echo.
echo 可执行文件位置: build\Debug\SprayTrajectoryPlanning.exe
echo.
echo 测试步骤:
echo 1. 运行程序: build\Debug\SprayTrajectoryPlanning.exe
echo 2. 点击 "文件" -^> "导入STEP模型"
echo 3. 选择 data\model\MPX3500.step
echo 4. 观察模型树和3D视图
echo.
echo 详细测试指南: docs\step-loading-test-guide.md
echo.
pause
