@echo off
chcp 65001 > nul
echo 测试Qt信号阻塞问题修复
echo.
echo 问题描述:
echo - TransferRoots完成后在emit progressPercentage(60)处卡住
echo - 需要Ctrl+C才能继续执行
echo.
echo 修复方案:
echo - 使用QTimer::singleShot延迟发送信号
echo - 避免直接在工作线程中阻塞信号发送
echo.
echo 测试步骤:
echo 1. 启动主程序
echo 2. 加载大型STEP文件
echo 3. 选择任意质量模式
echo 4. 观察几何体解析阶段
echo 5. 验证程序是否自动继续（不需要Ctrl+C）
echo.
echo 预期结果:
echo - TransferRoots完成后自动继续
echo - 进度更新正常
echo - 无需用户干预
echo.
echo 如果仍然卡住:
echo - 等待1-2分钟
echo - 在控制台窗口按Ctrl+C一次
echo - 记录问题并报告
echo.
echo 启动主程序...
.\build\bin\Debug\SprayTrajectoryPlanning.exe
pause