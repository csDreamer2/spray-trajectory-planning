@echo off
echo 测试机械臂控制功能...
echo.

cd /d "%~dp0"

echo ========================================
echo 机械臂控制测试指南
echo ========================================
echo.
echo 1. 首先需要将STEP文件转换为STL格式：
echo    - data\model\杭汽轮总装.STEP
echo    - data\model\MPX3500.STEP
echo.
echo 2. 转换方法：
echo    - 安装FreeCAD（推荐）
echo    - 或使用在线转换工具
echo    - 或手动用CAD软件转换
echo.
echo 3. 程序功能测试：
echo    - 导入STL机械臂模型
echo    - 点击"机械臂控制"按钮
echo    - 观察圆周运动动画
echo.
echo 4. 当前实现的控制功能：
echo    - 整体位置控制（X,Y,Z）
echo    - 姿态控制（RX,RY,RZ）
echo    - 平滑动画插值
echo    - 实时渲染更新
echo.
echo 5. 限制说明：
echo    - STL文件无关节信息
echo    - 只能做整体变换
echo    - 无法控制单个关节
echo    - 需要URDF文件才能做真正的机械臂仿真
echo.

echo 启动程序进行测试...
start "" "build\bin\Debug\SprayTrajectoryPlanning.exe"

echo.
echo 程序已启动，请按照上述步骤测试机械臂控制功能
echo 详细说明请查看：docs\step-to-stl-guide.md
pause