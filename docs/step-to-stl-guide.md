# STEP文件转STL指南

## 问题说明
VTK无法直接读取STEP文件，需要转换为STL格式。系统已实现自动转换功能，但需要FreeCAD支持。

## 解决方案

### 方案1：使用已安装的FreeCAD（推荐）
1. FreeCAD已安装在：`K:/Kapps/FreeCAD/`
2. 系统会自动检测并使用该路径
3. 支持的转换功能：
   - 自动检测FreeCAD安装路径
   - 使用改进的Python脚本
   - 支持复杂STEP文件网格化
   - 自动合并多个对象为单一STL
   - 详细的转换日志输出

### 方案2：手动转换
使用任何CAD软件将STEP文件转换为STL：

#### 使用FreeCAD手动转换：
1. 打开FreeCAD
2. 文件 → 打开 → 选择STEP文件
3. 选择导入的对象
4. 文件 → 导出 → 选择STL格式
5. 保存到相同目录，文件名保持一致

#### 使用在线转换工具：
- https://www.convertio.co/step-stl/
- https://anyconv.com/step-to-stl-converter/

#### 使用其他CAD软件：
- SolidWorks：另存为 → STL
- Fusion 360：导出 → STL
- Inventor：导出 → STL

## 当前STEP文件
项目中有两个STEP文件：
1. `data/model/杭汽轮总装.STEP` - 车间总装模型
2. `data/model/MPX3500.STEP` - 机械臂模型

## 机械臂仿真功能

### STL文件的限制
- ✅ 可以显示机械臂外观
- ✅ 可以做整体位置控制
- ✅ 可以做简单动画演示
- ❌ 无法做真正的关节控制
- ❌ 无法做运动学解算

### 当前实现的功能
1. **STL模型加载** - 显示机械臂3D模型
2. **位姿控制** - 设置机械臂位置和姿态
3. **动画演示** - 机械臂圆周运动演示
4. **实时控制** - 通过界面按钮控制

### 使用方法
1. 将STEP文件转换为STL格式
2. 在程序中：文件 → 导入车间模型 → 选择STL文件
3. 点击"机械臂控制"按钮开始演示动画

### 未来扩展
要实现真正的机械臂仿真，需要：
1. **URDF文件** - 定义关节和连杆
2. **运动学库** - 如KDL、MoveIt等
3. **关节控制器** - 控制各个关节
4. **轨迹规划** - 路径规划算法

## 测试步骤
1. 运行测试脚本：`test_freecad_conversion.bat`
2. 或直接运行程序：`build/bin/Debug/SprayTrajectoryPlanning.exe`
3. 文件 → 导入车间模型 → 选择STEP文件
4. 程序会自动尝试转换为STL格式
5. 转换成功后自动加载3D模型
6. 点击"机械臂控制"测试动画

## 转换改进
基于CSDN文章实现的改进：
- 修复了FreeCAD参数错误（`--python-script` → `-c`）
- 优先使用FreeCADCmd命令行版本
- 改进的Python脚本支持复杂模型
- 自动检测用户的FreeCAD安装路径
- 详细的错误处理和日志输出
- 支持多对象合并和网格优化
- 增加了60秒转换超时（适应大文件）

## 故障排除
如果转换遇到问题，请查看：`docs/step-conversion-troubleshooting.md`

常见问题：
- FreeCAD参数错误 → 已修复
- 转换超时 → 增加了超时时间
- 内存不足 → 建议关闭其他程序
- 文件太大 → 可以手动转换或使用在线工具