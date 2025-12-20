#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
创建简单的STL测试文件
"""

import FreeCAD
import Mesh
import Part

def create_simple_stl():
    """创建简单的STL文件用于测试"""
    try:
        # 创建新文档
        doc = FreeCAD.newDocument('SimpleSTL')
        
        # 创建一个机械臂形状的简单模型
        # 底座
        base = doc.addObject("Part::Cylinder", "Base")
        base.Radius = 50
        base.Height = 20
        base.Placement.Base = FreeCAD.Vector(0, 0, 0)
        
        # 第一段臂
        arm1 = doc.addObject("Part::Box", "Arm1")
        arm1.Length = 200
        arm1.Width = 30
        arm1.Height = 30
        arm1.Placement.Base = FreeCAD.Vector(0, -15, 20)
        
        # 第二段臂
        arm2 = doc.addObject("Part::Box", "Arm2")
        arm2.Length = 150
        arm2.Width = 25
        arm2.Height = 25
        arm2.Placement.Base = FreeCAD.Vector(200, -12.5, 32.5)
        
        # 末端执行器
        end_effector = doc.addObject("Part::Cylinder", "EndEffector")
        end_effector.Radius = 15
        end_effector.Height = 40
        end_effector.Placement.Base = FreeCAD.Vector(350, 0, 40)
        
        # 重新计算
        doc.recompute()
        
        # 创建网格并导出STL
        objects = [base, arm1, arm2, end_effector]
        
        # 合并所有形状
        shapes = []
        for obj in objects:
            if hasattr(obj, 'Shape'):
                shapes.append(obj.Shape)
        
        if shapes:
            compound = Part.makeCompound(shapes)
            
            # 创建网格
            mesh_obj = doc.addObject('Mesh::Feature', 'RobotMesh')
            mesh_obj.Mesh = Mesh.Mesh(compound.tessellate(0.1))
            
            # 导出STL
            import os
            stl_path = os.path.abspath("data/model/simple_robot.stl")
            mesh_obj.Mesh.write(stl_path)
            
            print(f"简单机械臂STL文件已创建: {stl_path}")
            print(f"网格点数: {mesh_obj.Mesh.CountPoints}")
            print(f"网格面数: {mesh_obj.Mesh.CountFacets}")
        
        # 关闭文档
        FreeCAD.closeDocument(doc.Name)
        
        return True
        
    except Exception as e:
        print(f"创建STL文件失败: {e}")
        import traceback
        traceback.print_exc()
        return False

if __name__ == '__main__':
    create_simple_stl()