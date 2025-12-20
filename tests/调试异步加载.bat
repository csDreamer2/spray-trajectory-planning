@echo off
chcp 936 > nul
echo ========================================
echo 调试异步STEP加载功能
echo ========================================
echo.

cd /d "%~dp0\.."

echo 当前目录: %CD%
echo.

echo 检查调试可执行文件...
if not exist "build\bin\tests\DebugAsyncMain.exe" (
    echo 错误: 找不到调试可执行文件 build\bin\tests\DebugAsyncMain.exe
    echo 请先编译调试版本：
    echo   cd tests/programs
    echo   mkdir build
    echo   cd build
    echo   cmake .. -G "Visual Studio 17 2022" -A x64 -DQt6_DIR="K:/Kapps/Qt/6.10.1/msvc2022_64/lib/cmake/Qt6"
    echo   cmake --build . --config Debug
    pause
    exit /b 1
)

echo 检查测试数据...
if not exist "data\model\MPX3500.STEP" (
    echo 错误: 找不到测试文件 data\model\MPX3500.STEP
    echo 请确保测试数据存在
    pause
    exit /b 1
)

echo.
echo 启动异步加载调试程序...
echo 注意: 这是专门的调试版本，用于测试异步加载核心功能
echo.

echo 运行调试程序...
"build\bin\tests\DebugAsyncMain.exe"

echo.
echo 调试程序说明:
echo - 这是简化版本，专注于异步STEP加载测试
echo - 没有复杂的UI界面，便于调试核心功能
echo - 直接加载 data\model\MPX3500.STEP 文件
echo - 输出详细的调试信息和性能统计
echo.
echo 预期输出:
echo - 详细的加载阶段信息
echo - 各阶段耗时统计
echo - 线程状态和同步信息
echo - VTK转换结果
echo.

pause