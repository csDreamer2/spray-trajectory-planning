@echo off
chcp 936 > nul
echo 测试优化后的异步STEP加载
echo ============================

echo 启动应用程序...
cd /d "%~dp0"

if exist "build\bin\Debug\SprayTrajectoryPlanning.exe" (
    echo 找到Debug版本程序
    echo.
    echo 优化改进：
    echo 1. 延迟VTK Actor创建（50ms延迟）
    echo 2. 延迟渲染执行（100ms延迟）
    echo 3. 大型模型检测和优化
    echo 4. 非阻塞进度更新
    echo.
    echo 预期行为：
    echo - 加载期间界面保持响应
    echo - 进度更新平滑显示
    echo - 最终渲染在短暂延迟后进行
    echo - 任务管理器中不显示"未响应"
    echo.
    echo 观察调试输出：
    echo - "在延迟调用中创建VTK Actor..."
    echo - "检测到大型模型，正在优化渲染"
    echo - "安排延迟渲染..."
    echo - "为ModelType执行延迟渲染"
    echo.
    echo 测试步骤：
    echo 1. 点击"文件" - "导入车间模型"
    echo 2. 选择 data\model\MPX3500.STEP
    echo 3. 选择"异步加载"
    echo 4. 观察界面响应性和控制台输出
    echo.
    "build\bin\Debug\SprayTrajectoryPlanning.exe"
) else (
    echo 错误：找不到可执行文件
    pause
    exit /b 1
)

pause