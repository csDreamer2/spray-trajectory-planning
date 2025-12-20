@echo off
chcp 936 > nul
echo ========================================
echo 测试作者信息显示功能
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

echo.
echo 启动作者信息显示测试...
echo.

start "" "build\bin\Debug\SprayTrajectoryPlanning.exe"

echo.
echo 测试说明:
echo 1. 程序启动后，检查窗口标题是否显示作者信息
echo 2. 选择 "帮助" -^> "关于" 菜单
echo 3. 验证关于对话框中的作者信息
echo.
echo 预期结果:
echo - 窗口标题: "机器人喷涂轨迹规划系统 - 王睿 (浙江大学)"
echo - 关于对话框显示完整的作者信息和系统信息
echo - 包含: 作者: 王睿 (浙江大学)
echo - 包含: 3D引擎: VTK 9.2
echo - 包含: CAD内核: OpenCASCADE 7.8
echo.

pause