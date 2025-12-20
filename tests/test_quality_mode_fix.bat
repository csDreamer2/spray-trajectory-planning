@echo off
chcp 65001 > nul
echo 测试高质量模式优化和日志修复
echo.
echo 测试内容:
echo 1. 质量选择对话框显示时间预估
echo 2. 高质量模式确认对话框
echo 3. 系统日志时间格式正确
echo 4. 进度消息显示预估时间
echo.
echo 测试步骤:
echo 1. 启动主程序
echo 2. 导入STEP文件
echo 3. 观察质量选择对话框的改进说明
echo 4. 尝试选择高质量模式，验证确认对话框
echo 5. 选择其他模式，观察系统日志的时间统计
echo.
echo 启动主程序...
.\build\bin\Debug\SprayTrajectoryPlanning.exe
pause