# PLY点云文件上传与Unity3D展示集成指南

## 概述

本文档说明如何在Qt应用程序中上传PLY点云文件，并在集成的Unity3D环境中进行可视化展示。

## 系统架构

```
┌─────────────────┐         TCP Socket         ┌──────────────────┐
│   Qt应用程序     │ ◄──────────────────────► │  Unity3D应用     │
│                 │      JSON消息通信          │                  │
│  - 文件上传      │                           │  - 点云渲染       │
│  - PCL解析      │                           │  - 3D可视化       │
│  - 数据处理      │                           │  - 相机控制       │
└─────────────────┘                           └──────────────────┘
```

## 功能特性

### Qt端功能
1. **文件上传**: 支持PLY、STL、OBJ、PCD格式
2. **点云解析**: 使用PCL库解析点云数据
3. **数据处理**: 
   - 边界框计算
   - 点云采样（避免数据过大）
   - 数据验证
4. **Unity通信**: 通过TCP Socket发送JSON格式数据

### Unity端功能
1. **点云渲染**: 使用Mesh系统渲染大量点
2. **性能优化**:
   - 自动分割大型点云（每个Mesh最多65000点）
   - LOD（细节层次）支持
   - 点云采样显示
3. **可视化增强**:
   - 边界框显示
   - 自动相机聚焦
   - 颜色支持
4. **交互控制**: 相机旋转、缩放、平移

## 使用步骤

### 1. 启动Qt应用程序

```bash
# 编译并运行
cd build
.\bin\Debug\SprayTrajectoryPlanning.exe
```

### 2. 启动Unity应用程序

Unity应用程序会自动连接到Qt的TCP服务器（默认端口12346）。

### 3. 上传点云文件

在Qt应用程序中：
1. 点击菜单 **文件 → 导入工件**
2. 选择PLY文件（或其他支持的格式）
3. 等待解析完成

### 4. 查看Unity中的点云

点云数据会自动发送到Unity并进行渲染：
- 点云以点的形式显示
- 黄色边界框显示点云范围
- 相机自动聚焦到点云中心

## 数据流程

### 1. Qt端处理流程

```cpp
// 1. 用户选择文件
QString fileName = QFileDialog::getOpenFileName(...);

// 2. 创建点云解析器
Data::PointCloudParser* parser = new Data::PointCloudParser();
Data::PointCloudData pointCloudData;

// 3. 解析点云文件
parser->parseFile(filePath, pointCloudData);

// 4. 准备JSON数据
QJsonObject workpieceJson;
workpieceJson["type"] = "pointcloud";
workpieceJson["fileName"] = pointCloudData.fileName;
workpieceJson["pointCount"] = pointCloudData.pointCount;
// ... 添加点数据和边界框信息

// 5. 发送到Unity
m_unityBridge->SendWorkpieceData(workpieceJson);
```

### 2. Unity端处理流程

```csharp
// 1. 接收Qt消息
void HandleWorkpieceData(QtMessage message)
{
    // 2. 解析JSON数据
    var pointCloudData = JsonUtility.FromJson<PointCloudData>(message.data);
    
    // 3. 创建点云渲染器
    PointCloudRenderer renderer = GetComponent<PointCloudRenderer>();
    
    // 4. 加载并渲染点云
    renderer.LoadPointCloudFromJson(message.data);
    
    // 5. 发送加载成功反馈
    SendMessage("workpiece_loaded", new { success = true });
}
```

## JSON消息格式

### Qt发送到Unity的点云数据

```json
{
  "type": "workpiece_data",
  "data": {
    "type": "pointcloud",
    "fileName": "example.ply",
    "format": "PLY",
    "pointCount": 50000,
    "fileSize": 2.5,
    "boundingBoxMin": [-100, -50, 0],
    "boundingBoxMax": [100, 50, 200],
    "points": [
      [x1, y1, z1],
      [x2, y2, z2],
      ...
    ],
    "sampleStep": 5
  }
}
```

### Unity发送到Qt的反馈

```json
{
  "type": "workpiece_loaded",
  "success": true,
  "message": "Unity成功加载点云: example.ply",
  "fileName": "example.ply"
}
```

## 性能优化

### 点云采样

为避免传输和渲染大量数据，Qt端会自动采样：

```cpp
// 最多发送10000个点
int sampleStep = qMax(1, pointCloudData.pointCount / 10000);

for (int i = 0; i < pointCloudData.points.size(); i += sampleStep) {
    // 添加采样点到JSON
}
```

### Unity网格分割

Unity会自动将大型点云分割成多个Mesh：

```csharp
// Unity Mesh顶点限制为65000
int maxPointsPerMesh = 65000;
int meshCount = Mathf.CeilToInt((float)totalPoints / maxPointsPerMesh);

// 创建多个Mesh对象
for (int i = 0; i < meshCount; i++) {
    CreateMesh(points, startIndex, endIndex);
}
```

## Unity场景设置

### 1. 创建Unity场景

1. 创建空GameObject命名为"QtCommunication"
2. 添加`QtCommunication.cs`脚本
3. 添加`PointCloudRenderer.cs`脚本

### 2. 配置相机

```csharp
// 主相机设置
Camera.main.clearFlags = CameraClearFlags.SolidColor;
Camera.main.backgroundColor = Color.black;
Camera.main.fieldOfView = 60;
```

### 3. 配置点云渲染器

在Inspector中设置：
- **Point Size**: 0.01 (点的大小)
- **Point Color**: White (默认点颜色)
- **Show Bounding Box**: true (显示边界框)
- **Enable LOD**: true (启用LOD优化)

## 故障排除

### 问题1: Unity无法连接到Qt

**解决方案**:
1. 检查Qt应用程序是否已启动Unity Bridge服务器
2. 检查防火墙设置，确保端口12346未被阻止
3. 查看Qt控制台输出，确认服务器已启动

### 问题2: 点云不显示

**解决方案**:
1. 检查Unity Console是否有错误信息
2. 确认点云数据已成功接收（查看Debug.Log）
3. 检查相机位置是否正确
4. 确认点云材质已正确设置

### 问题3: 点云解析失败

**解决方案**:
1. 检查文件格式是否支持（PLY、STL、OBJ、PCD）
2. 确认PCL库已正确安装和链接
3. 查看Qt控制台的错误信息
4. 检查文件是否损坏

### 问题4: 性能问题

**解决方案**:
1. 增加采样步长，减少发送的点数
2. 在Unity中启用LOD
3. 降低点的渲染大小
4. 使用更高效的Shader

## 扩展功能

### 1. 添加颜色支持

Qt端：
```cpp
// 添加颜色数据到JSON
QJsonArray colorsArray;
for (const QVector3D& color : pointCloudData.colors) {
    colorsArray.append(color.x());
    colorsArray.append(color.y());
    colorsArray.append(color.z());
}
workpieceJson["colors"] = colorsArray;
```

Unity端：
```csharp
// 使用顶点颜色
mesh.colors = vertexColors;
```

### 2. 添加法向量支持

```cpp
// Qt端添加法向量
QJsonArray normalsArray;
for (const QVector3D& normal : pointCloudData.normals) {
    normalsArray.append(normal.x());
    normalsArray.append(normal.y());
    normalsArray.append(normal.z());
}
workpieceJson["normals"] = normalsArray;
```

### 3. 点云编辑功能

- 点选择
- 区域裁剪
- 滤波处理
- 表面重建

## 相关文件

### Qt端
- `src/Data/PointCloudParser.h/cpp` - 点云解析器
- `src/UI/MainWindow.h/cpp` - 主窗口（文件上传）
- `src/UI/QtUnityBridge.h/cpp` - Unity通信桥

### Unity端
- `Unity/QtCommunication.cs` - Qt通信脚本
- `Unity/PointCloudRenderer.cs` - 点云渲染器

### 文档
- `Unity/README.md` - Unity集成说明
- `docs/Unity_Integration_Guide.md` - Unity集成指南

## 技术参考

### PCL (Point Cloud Library)
- 官网: https://pointclouds.org/
- 文档: https://pcl.readthedocs.io/

### Unity Mesh API
- 文档: https://docs.unity3d.com/ScriptReference/Mesh.html
- 顶点限制: 65535个顶点/Mesh

### Qt Network
- QTcpServer: https://doc.qt.io/qt-6/qtcpserver.html
- QTcpSocket: https://doc.qt.io/qt-6/qtcpsocket.html

## 总结

通过Qt的PCL点云解析和Unity的高性能渲染，实现了完整的点云文件上传和3D可视化流程。系统支持多种点云格式，具有良好的性能和扩展性。
