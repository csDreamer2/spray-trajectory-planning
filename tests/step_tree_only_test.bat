@echo off
echo === STEP模型树单独测试 ===
echo 这个测试只加载STEP模型树，不涉及VTK，用于排查崩溃问题
echo.

cd /d "%~dp0\.."

echo 编译测试程序...
cmake --build build --target step_tree_only_test --config Debug

if %ERRORLEVEL% NEQ 0 (
    echo 编译失败！
    pause
    exit /b 1
)

echo.
echo 运行测试程序...
echo 观察是否在网格生成完成后崩溃
echo.

build\bin\Debug\step_tree_only_test.exe

echo.
echo 程序结束，返回码: %ERRORLEVEL%
pause