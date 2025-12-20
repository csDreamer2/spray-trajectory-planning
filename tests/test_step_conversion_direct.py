#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
直接测试STEP转STL转换
"""

import sys
import os
import FreeCAD
import Import
import Mesh
import Part

def convert_step_to_stl(step_file, stl_file):
    try:
        print('开始转换STEP文件:', step_file)
        
        # 创建新文档
        doc = FreeCAD.newDocument('StepToStl')
        
        # 导入STEP文件
        Import.insert(step_file, doc.Name)
        
        # 获取所有对象
        objects = doc.Objects
        print('找到对象数量:', len(objects))
        
        if not objects:
            print('ERROR: 没有找到任何对象')
            return False
        
        # 合并所有对象为一个形状
        shapes = []
        for obj in objects:
            if hasattr(obj, 'Shape') and obj.Shape.isValid():
                shapes.append(obj.Shape)
                print('添加形状:', obj.Label)
        
        if not shapes:
            print('ERROR: 没有找到有效的形状')
            return False
        
        # 创建复合形状
        if len(shapes) == 1:
            compound_shape = shapes[0]
        else:
            compound_shape = Part.makeCompound(shapes)
        
        # 网格化参数
        linear_deflection = 0.1  # 线性偏差
        angular_deflection = 0.1  # 角度偏差
        
        # 创建网格
        mesh_obj = doc.addObject('Mesh::Feature', 'MeshExport')
        mesh_obj.Mesh = Mesh.Mesh(compound_shape.tessellate(linear_deflection, angular_deflection))
        
        # 导出STL文件
        mesh_obj.Mesh.write(stl_file)
        
        print('STL文件已保存:', stl_file)
        print('网格点数:', mesh_obj.Mesh.CountPoints)
        print('网格面数:', mesh_obj.Mesh.CountFacets)
        
        # 关闭文档
        FreeCAD.closeDocument(doc.Name)
        
        print('SUCCESS')
        return True
        
    except Exception as e:
        print('ERROR:', str(e))
        import traceback
        traceback.print_exc()
        return False

# 测试转换
if __name__ == '__main__':
    step_path = r'data/model/MPX3500.STEP'
    stl_path = r'data/model/MPX3500.stl'
    
    print('STEP文件路径:', step_path)
    print('STL输出路径:', stl_path)
    
    if os.path.exists(step_path):
        convert_step_to_stl(step_path, stl_path)
    else:
        print('ERROR: STEP文件不存在')