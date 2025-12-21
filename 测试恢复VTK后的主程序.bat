@echo off
echo === 测试恢复VTK后的主程序 ===
echo.
echo 当前状态：
echo - ✅ VTK组件已恢复
echo - ✅ 保持了信号处理的修复
echo - ✅ 保持了异常处理的增强
echo - ✅ 保持了延迟信号处理机制
echo.
echo 测试目标：
echo - 验证VTK恢复后是否影响STEP导入稳定性
echo - 确认之前的修复是否仍然有效
echo.
echo 测试步骤：
echo 1. 启动主程序 (VTK已恢复)
echo 2. 导入STEP模型: data/model/MPX3500.STEP
echo 3. 观察STEP模型树是否正常显示
echo 4. 检查是否还会在完成后崩溃
echo.
echo 启动主程序...
build\bin\Debug\SprayTrajectoryPlanning.exe
echo.
echo === 测试完成 ===
pause