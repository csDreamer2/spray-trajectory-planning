@echo off
chcp 65001 > nul
echo 测试控制台编码修复
echo ==================

echo 启动程序并测试中文输出...
cd /d "%~dp0"

if exist "build\bin\Debug\SprayTrajectoryPlanning.exe" (
    echo 找到Debug版本程序
    "build\bin\Debug\SprayTrajectoryPlanning.exe"
) else if exist "build\bin\Release\SprayTrajectoryPlanning.exe" (
    echo 找到Release版本程序
    "build\bin\Release\SprayTrajectoryPlanning.exe"
) else (
    echo 错误：找不到可执行文件
    echo 请先编译项目
    pause
    exit /b 1
)

pause