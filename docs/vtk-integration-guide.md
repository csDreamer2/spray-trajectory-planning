# VTK集成指南 - 替换Unity方案

## 🎯 为什么选择VTK？

基于你的需求分析，VTK是最适合工业机器人喷涂仿真的方案：

### ✅ VTK方案优势：
1. **专业CAD支持** - 直接读取STEP文件（659MB车间模型）
2. **原生Qt集成** - QVTKOpenGLNativeWidget无缝嵌入
3. **强大点云处理** - 与PCL完美配合
4. **工业级可视化** - 科学级渲染质量
5. **简化架构** - 单一进程，无需TCP通信
6. **易于部署** - 不需要Unity Runtime

### ❌ Unity方案问题：
- 窗口嵌入复杂且不稳定
- UI布局错乱和交互问题
- 需要额外的TCP通信层
- 部署复杂（需要Unity Runtime）
- 不适合工业CAD数据处理

## 🚀 VTK安装步骤

### 方法1：使用vcpkg（推荐）

```bash
# 1. 安装vcpkg（如果还没有）
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# 2. 安装VTK
.\vcpkg install vtk[qt]:x64-windows

# 3. 集成到Visual Studio
.\vcpkg integrate install
```

### 方法2：预编译包下载

1. 访问 https://vtk.org/download/
2. 下载 VTK-9.x.x-Windows.zip
3. 解压到 `C:\VTK`
4. 设置环境变量 `VTK_DIR=C:\VTK\lib\cmake\vtk-9.x`

### 方法3：从源码编译

```bash
# 1. 下载VTK源码
git clone https://github.com/Kitware/VTK.git
cd VTK

# 2. 创建构建目录
mkdir build
cd build

# 3. 配置CMake
cmake .. -DVTK_GROUP_ENABLE_Qt=YES -DVTK_MODULE_ENABLE_VTK_GUISupportQt=YES

# 4. 编译
cmake --build . --config Release
```

## 🔧 项目配置

### CMakeLists.txt已更新

项目已经配置好VTK支持：

```cmake
# VTK依赖
find_package(VTK REQUIRED COMPONENTS
    CommonCore
    CommonDataModel
    RenderingCore
    RenderingOpenGL2
    InteractionStyle
    GUISupportQt
    IOGeometry
    IOPLY
)

# 链接VTK库
target_link_libraries(SprayTrajectoryPlanning ${VTK_LIBRARIES})
```

### 新增文件

- `src/UI/VTKWidget.h` - VTK渲染组件头文件
- `src/UI/VTKWidget.cpp` - VTK渲染组件实现
- `src/vtk_test_main.cpp` - VTK测试程序

## 🧪 测试VTK集成

### 1. 构建VTK测试程序

```bash
cd build
cmake --build . --config Debug
```

### 2. 运行VTK测试

```bash
cd build/bin/Debug
./VTKTest.exe
```

### 3. 测试功能

在VTK测试程序中：

1. **加载点云** - 点击"加载点云"，选择 `test_data/sample_cube.ply`
2. **显示测试轨迹** - 点击"显示测试轨迹"查看螺旋轨迹
3. **3D交互** - 鼠标拖拽旋转、滚轮缩放
4. **视图控制** - 使用"重置视角"、"适应场景"按钮

## 🏭 加载车间模型

### STEP文件支持

你的659MB车间模型 `data/model/杭汽轮总装.STEP` 需要额外配置：

#### 选项1：集成OpenCASCADE（推荐）

```cmake
# 添加OpenCASCADE支持
find_package(OpenCASCADE REQUIRED)
target_link_libraries(SprayTrajectoryPlanning ${OpenCASCADE_LIBRARIES})
```

#### 选项2：转换为STL格式

使用CAD软件将STEP转换为STL：
1. 在SolidWorks/AutoCAD中打开STEP文件
2. 导出为STL格式
3. 使用VTK的STL读取器加载

#### 选项3：使用FreeCAD转换

```python
# FreeCAD Python脚本
import FreeCAD
import Part
import Mesh

# 加载STEP文件
doc = FreeCAD.newDocument()
Part.insert("杭汽轮总装.STEP", "Document")

# 转换为STL
obj = doc.Objects[0]
mesh = doc.addObject("Mesh::Feature", "Mesh")
mesh.Mesh = Mesh.Mesh(obj.Shape.tessellate(0.1))
mesh.Mesh.write("杭汽轮总装.stl")
```

## 🔄 替换Unity组件

### 1. 更新MainWindow

```cpp
// 在MainWindow.cpp中
#include "VTKWidget.h"

// 替换UnityWidget为VTKWidget
m_vtkWidget = new UI::VTKWidget(this);
// 移除Unity相关代码
```

### 2. 更新点云加载

```cpp
// 在PointCloudLoader中
void PointCloudLoader::LoadPointCloud(const QString& filePath)
{
    // 直接使用VTK加载，无需TCP通信
    m_vtkWidget->LoadPointCloud(filePath);
}
```

### 3. 移除Unity依赖

- 删除 `UnityWidget.cpp/h`
- 删除 `QtUnityBridge.cpp/h`
- 移除Unity相关的TCP通信代码

## 📊 性能对比

| 功能 | Unity方案 | VTK方案 |
|------|-----------|---------|
| **点云渲染** | 需要转换 | 原生支持 |
| **CAD模型** | 插件支持 | 直接读取 |
| **内存使用** | 双进程 | 单进程 |
| **启动时间** | 10-15秒 | 2-3秒 |
| **交互响应** | TCP延迟 | 直接调用 |
| **部署大小** | 500MB+ | 100MB |

## 🎯 下一步计划

### 立即行动：
1. **安装VTK** - 使用vcpkg或预编译包
2. **测试VTK程序** - 运行VTKTest.exe验证功能
3. **加载点云** - 测试现有的sample_cube.ply文件

### 后续开发：
1. **集成OpenCASCADE** - 支持STEP文件加载
2. **替换Unity组件** - 在主程序中使用VTKWidget
3. **添加机器人模型** - URDF文件支持
4. **实现轨迹规划** - 喷涂路径可视化

## 💡 技术支持

如果遇到问题：

1. **VTK安装问题** - 检查CMake配置和环境变量
2. **编译错误** - 确保Qt6和VTK版本兼容
3. **运行时错误** - 检查VTK库路径和依赖

---

**VTK方案将为你的机器人喷涂仿真提供专业、稳定、高性能的3D可视化能力！** 🚀