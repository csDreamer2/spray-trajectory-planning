@echo off
chcp 65001 > nul
echo 测试主程序UI事件循环阻塞修复
echo.
echo 重要发现:
echo - DebugAsyncMain.exe 正常工作（简单UI）
echo - SprayTrajectoryPlanning.exe 会卡住（复杂UI）
echo - 问题在于StatusPanel的同步调用阻塞事件循环
echo.
echo 修复方案:
echo - 将StatusPanel的addLogMessage调用改为异步
echo - 使用QTimer::singleShot(0, ...)推迟UI更新
echo - 确保关键处理流程不被UI更新阻塞
echo.
echo 对比测试:
echo.
echo 1. 测试DebugAsyncMain.exe（应该仍然正常）
echo 2. 测试SprayTrajectoryPlanning.exe（应该不再卡住）
echo.
echo 预期结果:
echo - 两个程序都能正常加载STEP文件
echo - 无需Ctrl+C干预
echo - 系统日志正常更新
echo.
echo 开始测试...
echo.
echo [1/2] 测试DebugAsyncMain.exe
echo 按任意键启动...
pause > nul
.\build\bin\Debug\DebugAsyncMain.exe
echo.
echo [2/2] 测试SprayTrajectoryPlanning.exe
echo 按任意键启动...
pause > nul
.\build\bin\Debug\SprayTrajectoryPlanning.exe
echo.
echo 测试完成！
pause