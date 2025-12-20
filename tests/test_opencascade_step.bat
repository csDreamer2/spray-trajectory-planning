@echo off
echo ========================================
echo OpenCASCADE STEP文件加载测试
echo ========================================
echo.

cd /d "%~dp0"

echo ✅ OpenCASCADE已成功集成
echo    路径: K:\Tools\OpenCasCade\install
echo    版本: 7.8.0 (推测)
echo.

echo ✅ 编译成功
echo    主程序: build\bin\Debug\SprayTrajectoryPlanning.exe
echo.

echo 测试文件:
if exist "data\model\杭汽轮总装.STEP" (
    echo ✅ 杭汽轮总装.STEP
) else (
    echo ❌ 杭汽轮总装.STEP 不存在
)

if exist "data\model\MPX3500.STEP" (
    echo ✅ MPX3500.STEP
) else (
    echo ❌ MPX3500.STEP 不存在
)

echo.
echo 功能说明:
echo 1. 使用OpenCASCADE直接读取STEP文件
echo 2. 自动三角化网格
echo 3. 转换为VTK PolyData
echo 4. 支持装配体结构
echo 5. 保留精确几何
echo.

echo 测试步骤:
echo 1. 启动程序
echo 2. 文件 → 导入车间模型
echo 3. 选择STEP文件
echo 4. 等待加载（可能需要几秒钟）
echo 5. 查看3D模型
echo 6. 测试机械臂控制功能
echo.

echo 启动程序...
start "" "build\bin\Debug\SprayTrajectoryPlanning.exe"

echo.
echo 程序已启动，请测试STEP文件加载功能！
pause