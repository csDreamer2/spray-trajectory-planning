@echo off
chcp 936 > nul
echo ========================================
echo 测试UI事件循环阻塞问题修复
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
echo 启动UI事件循环修复测试...
echo 注意: 这将测试复杂UI环境下的事件循环阻塞修复
echo.

start "" "build\bin\Debug\SprayTrajectoryPlanning.exe"

echo.
echo 测试说明:
echo 1. 程序启动后，选择 "文件" -^> "导入车间模型"
echo 2. 选择 data\model\MPX3500.STEP 文件
echo 3. 在加载过程中尝试操作其他UI元素
echo 4. 观察界面响应性和系统日志输出
echo 5. 验证加载过程中UI不会冻结
echo.
echo 修复内容:
echo - StatusPanel调用改为异步执行
echo - 使用QTimer::singleShot(0, ...)避免同步阻塞
echo - 分离VTK渲染和UI更新的执行时机
echo - 优化事件循环处理机制
echo.
echo 预期结果:
echo - 加载过程中界面保持响应
echo - 可以正常操作菜单和按钮
echo - 系统日志实时更新
echo - 进度条平滑更新
echo - 不会出现界面冻结现象
echo.

pause