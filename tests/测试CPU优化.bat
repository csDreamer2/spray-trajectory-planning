@echo off
chcp 936 > nul
echo ========================================
echo 测试CPU性能优化功能
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
echo 启动CPU优化测试...
echo 注意: 这将测试CPU优化对STEP加载性能的影响
echo.

start "" "build\bin\Debug\SprayTrajectoryPlanning.exe"

echo.
echo 测试说明:
echo 1. 程序启动后，选择 "文件" -^> "导入车间模型"
echo 2. 选择 data\model\MPX3500.STEP 文件
echo 3. 选择 "平衡模式" 进行加载
echo 4. 观察系统日志中的性能统计信息
echo 5. 对比不同质量模式的加载时间
echo.
echo CPU优化特性:
echo - 工作线程设置为高优先级
echo - Windows平台CPU亲和性优化
echo - OpenCASCADE内存和并行优化
echo - 智能网格精度调整
echo.
echo 预期结果:
echo - 加载时间相比之前版本有所改善
echo - 系统日志显示详细的时间统计
echo - CPU使用率更加均衡
echo.

pause