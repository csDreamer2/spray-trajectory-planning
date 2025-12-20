@echo off
echo === 最小OpenCASCADE STEP测试 ===
echo 这个测试只做基本的STEP文件读取，不做复杂的树结构解析
echo 用于验证OpenCASCADE库本身是否工作正常
echo.

cd /d "%~dp0\.."

echo 编译测试程序...
cmake --build build --target minimal_occt_test --config Debug

if %ERRORLEVEL% NEQ 0 (
    echo 编译失败！
    pause
    exit /b 1
)

echo.
echo 运行测试程序...
echo.

build\bin\Debug\minimal_occt_test.exe

echo.
echo 程序结束，返回码: %ERRORLEVEL%
pause