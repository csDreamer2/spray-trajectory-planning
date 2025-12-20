@echo off
chcp 65001 > nul
echo 测试STEP加载进度条功能
echo.
echo 功能说明:
echo 1. 启动调试程序
echo 2. 点击"Load STEP"按钮
echo 3. 选择加载质量
echo 4. 观察进度条显示
echo 5. 测试加载完成后界面响应性
echo.
echo 启动调试程序...
.\build\bin\Debug\DebugAsyncMain.exe
pause