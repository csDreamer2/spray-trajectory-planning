@echo off
echo === STEP崩溃调试测试 ===
echo.

cd /d "%~dp0\.."

echo 编译调试程序...
cmake --build build --target step_crash_debug --config Debug

if %ERRORLEVEL% NEQ 0 (
    echo 编译失败！
    pause
    exit /b 1
)

echo.
echo 运行调试程序...
echo 注意观察是否在网格生成完成后2秒左右崩溃
echo.

build\tests\Debug\step_crash_debug.exe

echo.
echo 程序结束，返回码: %ERRORLEVEL%
pause