@echo off
echo 测试STEP文件加载到VTK...
echo.

cd /d "%~dp0"

echo 启动主程序测试STEP文件加载...
echo 请在程序中尝试导入以下STEP文件：
echo 1. data\model\杭汽轮总装.STEP
echo 2. data\model\MPX3500.STEP
echo.

start "" "build\bin\Debug\SprayTrajectoryPlanning.exe"

echo 程序已启动，请手动测试STEP文件导入功能
pause