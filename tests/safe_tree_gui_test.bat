@echo off
echo ========================================
echo 安全STEP树状界面测试
echo 基于safe_step_test.cpp + QTreeWidget
echo ========================================
echo.

cd /d %~dp0\..
.\build\bin\Debug\safe_tree_gui_test.exe

pause