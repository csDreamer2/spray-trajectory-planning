@echo off
echo === 测试极简主程序版本 ===
echo.
echo 简化内容：
echo - 禁用VTK组件
echo - 移除复杂的停靠窗口配置
echo - 只保留STEP模型树和系统日志面板
echo - 禁用构造函数中的定时器
echo.
echo 测试目标：
echo - 逐步排除可能导致崩溃的组件
echo - 找到与独立测试程序的关键差异
echo.
echo 测试步骤：
echo 1. 启动极简主程序
echo 2. 导入STEP模型: data/model/MPX3500.STEP
echo 3. 观察是否还会崩溃
echo 4. 对比独立测试程序的行为
echo.
echo 启动极简主程序...
build\bin\Debug\SprayTrajectoryPlanning.exe
echo.
echo === 测试完成 ===
pause