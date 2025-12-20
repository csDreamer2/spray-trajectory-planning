@echo off
chcp 65001 > nul
echo 测试系统日志时间统计功能
echo.
echo 功能说明:
echo 1. 启动主程序 SprayTrajectoryPlanning.exe
echo 2. 在右侧面板中找到"系统日志"标签页
echo 3. 点击"文件" -> "导入点云" 或使用工具栏按钮
echo 4. 选择STEP文件并选择加载质量
echo 5. 观察系统日志中的时间统计输出:
echo    - 开始加载信息
echo    - 各阶段进度信息
echo    - 性能统计 (PERF): 每个阶段的耗时(秒)
echo    - 最终成功信息和总耗时
echo.
echo 启动主程序...
.\build\bin\Debug\SprayTrajectoryPlanning.exe
pause