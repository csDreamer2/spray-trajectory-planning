@echo off
echo ========================================
echo STEP文件加载安全测试
echo ========================================
echo.

cd /d "%~dp0"

echo 修复内容:
echo ✅ 添加用户选择对话框，避免自动转换卡死
echo ✅ 缩短超时时间到30秒
echo ✅ 提供详细的手动转换指导
echo ✅ 优先使用FreeCADCmd命令行版本
echo ✅ 创建了测试用的STL文件
echo.

echo 测试文件状态:
if exist "data\model\test_cube.stl" (
    echo ✅ 测试STL文件: test_cube.stl
) else (
    echo ❌ 测试STL文件不存在
)

if exist "data\model\杭汽轮总装.STEP" (
    echo ✅ 大型STEP文件: 杭汽轮总装.STEP
) else (
    echo ❌ 大型STEP文件不存在
)

if exist "data\model\MPX3500.STEP" (
    echo ✅ 机械臂STEP文件: MPX3500.STEP
) else (
    echo ❌ 机械臂STEP文件不存在
)

echo.
echo 推荐测试顺序:
echo.
echo 1. 测试STL文件加载 (最安全):
echo    - 文件 → 导入车间模型
echo    - 选择 data\model\test_cube.stl
echo    - 应该立即加载成功
echo.
echo 2. 测试STEP文件对话框:
echo    - 文件 → 导入车间模型  
echo    - 选择 data\model\MPX3500.STEP
echo    - 会弹出选择对话框
echo    - 建议选择"手动转换指导"
echo.
echo 3. 手动转换STEP文件:
echo    - 按照指导使用FreeCAD手动转换
echo    - 或使用在线转换工具
echo    - 然后导入生成的STL文件
echo.

echo 启动程序进行测试...
start "" "build\bin\Debug\SprayTrajectoryPlanning.exe"

echo.
echo 注意事项:
echo - 如果选择自动转换，可能需要等待30秒
echo - 大型文件建议手动转换
echo - 程序不会再卡死，有超时保护
echo.

pause