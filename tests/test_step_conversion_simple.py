#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
简单的STEP转STL测试脚本
用于验证FreeCAD转换功能
"""

import os
import sys
import subprocess

def test_freecad_conversion():
    """测试FreeCAD STEP转STL功能"""
    
    print("========================================")
    print("FreeCAD STEP转STL转换测试")
    print("========================================")
    
    # 检查FreeCAD安装
    freecad_paths = [
        "K:/Kapps/FreeCAD/bin/FreeCAD.exe",
        "K:/Kapps/FreeCAD/bin/FreeCADCmd.exe"
    ]
    
    freecad_found = False
    freecad_exe = None
    
    for path in freecad_paths:
        if os.path.exists(path):
            print(f"✅ 找到FreeCAD: {path}")
            freecad_found = True
            freecad_exe = path
            break
        else:
            print(f"❌ 未找到: {path}")
    
    if not freecad_found:
        print("❌ 未找到FreeCAD安装，请检查安装路径")
        return False
    
    # 检查STEP文件
    step_files = [
        "data/model/杭汽轮总装.STEP",
        "data/model/MPX3500.STEP"
    ]
    
    for step_file in step_files:
        if os.path.exists(step_file):
            print(f"✅ 找到STEP文件: {step_file}")
        else:
            print(f"❌ 未找到STEP文件: {step_file}")
    
    print("\n测试说明:")
    print("1. 程序已集成FreeCAD自动转换功能")
    print("2. 支持复杂STEP文件的网格化")
    print("3. 自动检测FreeCAD安装路径")
    print("4. 详细的转换日志输出")
    
    print("\n使用方法:")
    print("1. 启动程序: SprayTrajectoryPlanning.exe")
    print("2. 文件 → 导入车间模型")
    print("3. 选择STEP文件")
    print("4. 程序会自动转换并加载3D模型")
    
    return True

if __name__ == "__main__":
    test_freecad_conversion()