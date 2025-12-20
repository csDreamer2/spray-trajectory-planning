@echo off
chcp 936 > nul
echo ========================================
echo 测试TransferRoots阻塞问题修复
echo ========================================
echo.

cd /d "%~dp0\.."

echo 当前目录: %CD%
echo.

echo 检查可执行文件...
if not exist "build\bin\Debug\SprayTrajectoryPlanning.exe" (
    echo 错误: 找不到可执行文件 build\bin\Debug\SprayTrajectoryPlanning.exe
    echo 请先编译项目
    pause
    exit /b 1
)

echo 检查测试数据...
if not exist "data\model\MPX3500.STEP" (
    echo 错误: 找不到测试文件 data\model\MPX3500.STEP
    echo 请确保测试数据存在
    pause
    exit /b 1
)

echo.
echo 启动TransferRoots修复测试...
echo 注意: 这将测试TransferRoots完成后是否还会卡住
echo.

start "" "build\bin\Debug\SprayTrajectoryPlanning.exe"

echo.
echo 测试说明:
echo 1. 程序启动后，选择 "文件" -^> "导入车间模型"
echo 2. 选择 data\model\MPX3500.STEP 文件
echo 3. 选择任意质量模式进行加载
echo 4. 重点观察TransferRoots阶段完成后的行为
echo 5. 验证程序不会在TransferRoots后卡住
echo.
echo 修复内容:
echo - 使用异步StatusPanel调用避免事件循环阻塞
echo - 添加QTimer::singleShot延迟执行UI更新
echo - 分离关键路径和UI更新逻辑
echo - 强制线程状态刷新防止卡死
echo.
echo 预期结果:
echo - TransferRoots完成后程序继续正常执行
echo - 不需要Ctrl+C来激活线程
echo - 系统日志正常输出后续阶段信息
echo - 最终成功完成模型加载
echo.

pause