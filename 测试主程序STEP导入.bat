@echo off
echo === 测试主程序STEP模型导入功能 ===
echo 注意：VTK 3D可视化已禁用，仅测试STEP模型树功能
echo.
echo 测试步骤：
echo 1. 启动主程序
echo 2. 点击菜单 "文件" -> "导入STEP模型"
echo 3. 选择 data/model/MPX3500.STEP 文件
echo 4. 观察STEP模型树面板是否正常显示
echo 5. 检查是否还会在2秒后崩溃
echo.
echo 启动主程序...
build\bin\Debug\SprayTrajectoryPlanning.exe
echo.
echo === 测试完成 ===
pause