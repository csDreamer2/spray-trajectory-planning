@echo off
echo ========================================
echo STEP导入崩溃问题修复验证
echo ========================================
echo.

echo 1. 测试参考版本（safe_step_test.exe）
echo    - 使用STEPModelTreeWidget完整功能
echo    - 应该正常运行并显示树状结构
echo.
pause

echo 运行参考版本测试...
.\build\bin\Debug\safe_step_test.exe

echo.
echo ========================================
echo.

echo 2. 测试修复版本（safe_tree_gui_fixed.exe）
echo    - 使用STEPModelTreeWidget异步处理 + 自定义UI
echo    - 应该正常运行并显示自定义树状界面
echo.
pause

echo 运行修复版本测试...
.\build\bin\Debug\safe_tree_gui_fixed.exe

echo.
echo ========================================
echo 测试完成！
echo.
echo 如果两个程序都正常运行，说明STEP导入崩溃问题已彻底解决。
echo 现在可以将修复方案应用到主程序中。
echo.
pause