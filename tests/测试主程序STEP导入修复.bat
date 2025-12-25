@echo off
echo === 测试主程序STEP导入修复 ===
echo 目标: 验证主程序中的STEP导入崩溃问题是否已修复
echo.

cd /d "%~dp0"

echo 启动主程序...
echo 请在主程序中测试STEP模型导入功能
echo 观察是否还会在第14个组件处理时崩溃
echo.

.\build\bin\Debug\SprayTrajectoryPlanning.exe

echo.
echo === 测试完成 ===
pause