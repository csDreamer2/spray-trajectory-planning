#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
创建一个简单的STEP测试文件
"""

import FreeCAD
import Part

def create_simple_step():
    """创建一个简单的立方体STEP文件用于测试"""
    try:
        # 创建新文档
        doc = FreeCAD.newDocument('SimpleTest')
        
        # 创建一个简单的立方体
        box = doc.addObject("Part::Box", "TestBox")
        box.Length = 100
        box.Width = 100
        box.Height = 100
        
        # 重新计算
        doc.recompute()
        
        # 导出为STEP文件
        step_path = "data/model/simple_test.STEP"
        Part.export([box], step_path)
        
        print(f"简单STEP测试文件已创建: {step_path}")
        
        # 关闭文档
        FreeCAD.closeDocument(doc.Name)
        
        return True
        
    except Exception as e:
        print(f"创建测试文件失败: {e}")
        return False

if __name__ == '__main__':
    create_simple_step()