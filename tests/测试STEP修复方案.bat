@echo off
echo ========================================
echo STEP导入崩溃问题修复方案测试
echo ========================================
echo.
echo 可用的测试程序：
echo.
echo 1. safe_step_test.exe          - 标准版本（参考实现）
echo 2. step_tree_only_test.exe     - 独立测试版本  
echo 3. safe_tree_gui_fixed.exe     - 修复版本（最终解决方案）
echo.
echo ========================================

:menu
echo.
echo 请选择要运行的测试：
echo [1] 运行标准版本测试
echo [2] 运行独立测试版本
echo [3] 运行修复版本测试（推荐）
echo [4] 运行所有测试
echo [0] 退出
echo.
set /p choice=请输入选择 (0-4): 

if "%choice%"=="1" goto test1
if "%choice%"=="2" goto test2  
if "%choice%"=="3" goto test3
if "%choice%"=="4" goto testall
if "%choice%"=="0" goto end
echo 无效选择，请重新输入
goto menu

:test1
echo.
echo ========================================
echo 运行标准版本测试...
echo ========================================
.\build\bin\Debug\safe_step_test.exe
goto menu

:test2
echo.
echo ========================================
echo 运行独立测试版本...
echo ========================================
.\build\bin\Debug\step_tree_only_test.exe
goto menu

:test3
echo.
echo ========================================
echo 运行修复版本测试（最终解决方案）...
echo ========================================
.\build\bin\Debug\safe_tree_gui_fixed.exe
goto menu

:testall
echo.
echo ========================================
echo 依次运行所有测试...
echo ========================================
echo.
echo [1/3] 标准版本测试...
.\build\bin\Debug\safe_step_test.exe
echo.
echo [2/3] 独立测试版本...
.\build\bin\Debug\step_tree_only_test.exe
echo.
echo [3/3] 修复版本测试...
.\build\bin\Debug\safe_tree_gui_fixed.exe
echo.
echo 所有测试完成！
goto menu

:end
echo.
echo 测试结束，感谢使用！
pause