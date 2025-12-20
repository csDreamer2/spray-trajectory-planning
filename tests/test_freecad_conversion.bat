@echo off
echo 测试FreeCAD STEP转STL功能...
echo.

cd /d "%~dp0"

echo ========================================
echo FreeCAD STEP转STL测试
echo ========================================
echo.
echo FreeCAD安装路径: K:\Kapps\FreeCAD
echo.

echo 检查FreeCAD安装...
if exist "K:\Kapps\FreeCAD\bin\FreeCAD.exe" (
    echo ✅ 找到FreeCAD主程序: K:\Kapps\FreeCAD\bin\FreeCAD.exe
) else (
    echo ❌ 未找到FreeCAD主程序
)

if exist "K:\Kapps\FreeCAD\bin\FreeCADCmd.exe" (
    echo ✅ 找到FreeCAD命令行版本: K:\Kapps\FreeCAD\bin\FreeCADCmd.exe
) else (
    echo ❌ 未找到FreeCAD命令行版本
)

echo.
echo 检查STEP文件...
if exist "data\model\杭汽轮总装.STEP" (
    echo ✅ 找到STEP文件: 杭汽轮总装.STEP
) else (
    echo ❌ 未找到STEP文件: 杭汽轮总装.STEP
)

if exist "data\model\MPX3500.STEP" (
    echo ✅ 找到STEP文件: MPX3500.STEP
) else (
    echo ❌ 未找到STEP文件: MPX3500.STEP
)

echo.
echo 测试说明:
echo 1. 程序会自动检测FreeCAD安装路径
echo 2. 使用改进的Python脚本进行转换
echo 3. 支持复杂STEP文件的网格化
echo 4. 自动合并多个对象为单一STL
echo.

echo 启动程序进行测试...
start "" "build\bin\Debug\SprayTrajectoryPlanning.exe"

echo.
echo 测试步骤:
echo 1. 在程序中选择: 文件 → 导入车间模型
echo 2. 选择STEP文件 (杭汽轮总装.STEP 或 MPX3500.STEP)
echo 3. 程序会自动尝试转换为STL格式
echo 4. 观察控制台输出的转换日志
echo 5. 转换成功后会自动加载3D模型
echo.
echo 如果转换失败，请检查:
echo - FreeCAD是否正确安装在 K:\Kapps\FreeCAD
echo - STEP文件是否存在且格式正确
echo - 系统是否有足够的内存和磁盘空间
echo.

pause