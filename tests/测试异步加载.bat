@echo off
chcp 936 > nul
echo ========================================
echo 测试异步STEP加载功能
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
echo 启动异步STEP加载测试...
echo 注意: 这将测试大型STEP文件的异步加载功能
echo.

start "" "build\bin\Debug\SprayTrajectoryPlanning.exe"

echo.
echo 测试说明:
echo 1. 程序启动后，选择 "文件" -^> "导入车间模型"
echo 2. 选择 data\model\MPX3500.STEP 文件
echo 3. 在质量选择对话框中选择 "平衡模式"
echo 4. 观察异步加载进度条和系统日志输出
echo 5. 验证加载完成后模型正确显示
echo.
echo 预期结果:
echo - 界面不会冻结
echo - 进度条显示加载进度
echo - 系统日志输出各阶段时间统计
echo - 最终成功显示3D模型
echo.

pause