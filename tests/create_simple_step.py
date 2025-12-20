#!/usr/bin/env python3
"""
创建一个简单的STEP文件用于测试OpenCASCADE配置
"""

def create_simple_step():
    """创建一个简单的立方体STEP文件"""
    step_content = """ISO-10303-21;
HEADER;
FILE_DESCRIPTION(('Simple Cube for Testing'),'2;1');
FILE_NAME('test_cube.step','2024-12-19T10:00:00',('Test'),('Test'),'','','');
FILE_SCHEMA(('AUTOMOTIVE_DESIGN'));
ENDSEC;

DATA;
#1 = CARTESIAN_POINT('Origin',(0.0,0.0,0.0));
#2 = DIRECTION('X',(1.0,0.0,0.0));
#3 = DIRECTION('Y',(0.0,1.0,0.0));
#4 = DIRECTION('Z',(0.0,0.0,1.0));
#5 = AXIS2_PLACEMENT_3D('',#1,#4,#2);
#6 = CARTESIAN_POINT('P1',(0.0,0.0,0.0));
#7 = CARTESIAN_POINT('P2',(10.0,0.0,0.0));
#8 = CARTESIAN_POINT('P3',(10.0,10.0,0.0));
#9 = CARTESIAN_POINT('P4',(0.0,10.0,0.0));
#10 = CARTESIAN_POINT('P5',(0.0,0.0,10.0));
#11 = CARTESIAN_POINT('P6',(10.0,0.0,10.0));
#12 = CARTESIAN_POINT('P7',(10.0,10.0,10.0));
#13 = CARTESIAN_POINT('P8',(0.0,10.0,10.0));
#14 = LINE('',#6,#2);
#15 = LINE('',#7,#3);
#16 = LINE('',#8,#2);
#17 = LINE('',#9,#3);
#18 = BLOCK('TestCube',#5,10.0,10.0,10.0);
#19 = MANIFOLD_SOLID_BREP('Cube',#18);
#20 = PRODUCT_DEFINITION_SHAPE('','',#21);
#21 = PRODUCT_DEFINITION('','',#22,#23);
#22 = PRODUCT_DEFINITION_FORMATION('','',#24);
#23 = PRODUCT_DEFINITION_CONTEXT('part definition',#25,'design');
#24 = PRODUCT('TestCube','TestCube','',(#26));
#25 = APPLICATION_CONTEXT('automotive design');
#26 = PRODUCT_CONTEXT('',#25,'mechanical');
#27 = SHAPE_DEFINITION_REPRESENTATION(#20,#28);
#28 = SHAPE_REPRESENTATION('TestCube',(#19),#29);
#29 = GEOMETRIC_REPRESENTATION_CONTEXT(3);
ENDSEC;
END-ISO-10303-21;
"""
    
    with open('data/model/simple_cube.step', 'w', encoding='utf-8') as f:
        f.write(step_content)
    
    print("✅ 简单STEP文件已创建: data/model/simple_cube.step")

if __name__ == "__main__":
    create_simple_step()