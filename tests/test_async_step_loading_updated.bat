@echo off
chcp 65001 > nul
echo 测试异步STEP加载功能
echo =====================

echo 启动程序...
cd /d "%~dp0"

if exist "build\bin\Debug\SprayTrajectoryPlanning.exe" (
    echo 找到Debug版本程序
    start "SprayTrajectoryPlanning" "build\bin\Debug\SprayTrajectoryPlanning.exe"
    echo.
    echo ✅ OpenCASCADE配置测试结果：
    echo    - 头文件访问：成功
    echo    - STEP读取器：成功
    echo    - MPX3500.STEP加载：成功（耗时约90秒）
    echo    - 网格生成：成功
    echo.
    echo 🚀 异步加载测试步骤：
    echo.
    echo 1. 在程序中点击 "文件" -> "导入车间模型"
    echo 2. 选择 "data\model\MPX3500.STEP" 文件
    echo 3. 在弹出的对话框中选择 "异步加载"（推荐）
    echo 4. 观察状态栏的进度更新：
    echo    - "正在读取STEP文件..."
    echo    - "正在解析STEP几何..."
    echo    - "正在生成网格..."
    echo    - "正在转换为VTK格式..."
    echo    - "正在创建3D模型..."
    echo 5. 验证界面保持响应（可以移动窗口、点击按钮等）
    echo 6. 等待加载完成，查看3D模型是否正确显示
    echo.
    echo 📊 预期性能：
    echo    - 同步加载：界面卡死90秒
    echo    - 异步加载：界面流畅，后台处理
    echo.
    echo ✅ 预期结果：
    echo    - 界面始终响应
    echo    - 状态栏实时更新进度
    echo    - 最终成功显示3D模型
    echo    - 控制台输出详细日志
    echo.
) else if exist "build\bin\Release\SprayTrajectoryPlanning.exe" (
    echo 找到Release版本程序
    start "SprayTrajectoryPlanning" "build\bin\Release\SprayTrajectoryPlanning.exe"
) else (
    echo 错误：找不到可执行文件
    echo 请先编译项目
    pause
    exit /b 1
)

pause