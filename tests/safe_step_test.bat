@echo off
echo === 安全STEP模型树测试 ===
echo 这个测试增加了递归深度限制和更多保护措施
echo.

cd /d "%~dp0\.."

echo 编译测试程序...
cmake --build build --target safe_step_test --config Debug

if %ERRORLEVEL% NEQ 0 (
    echo 编译失败！
    pause
    exit /b 1
)

echo.
echo 运行测试程序...
echo 注意：大文件可能需要几分钟时间
echo 程序增加了递归深度限制，应该能避免栈溢出
echo.

build\bin\Debug\safe_step_test.exe

echo.
echo 程序结束，返回码: %ERRORLEVEL%
pause