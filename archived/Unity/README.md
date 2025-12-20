# Unity 3D 集成说明

## 概述

本目录包含Unity 3D项目的相关文件和说明，用于与Qt应用程序进行集成。

## 文件结构

```
Unity/
├── README.md                 # 本说明文件
├── UnityTestClient.py       # Python测试客户端（模拟Unity通信）
├── SpraySimulation/         # Unity项目目录（需要创建）
│   ├── Assets/
│   │   ├── Scripts/
│   │   │   ├── QtCommunication.cs    # Qt通信脚本
│   │   │   ├── WorkpieceLoader.cs    # 工件加载脚本
│   │   │   ├── TrajectoryRenderer.cs # 轨迹渲染脚本
│   │   │   └── SimulationController.cs # 仿真控制脚本
│   │   ├── Models/
│   │   │   ├── Robot/               # 机器人模型
│   │   │   └── Environment/         # 环境模型
│   │   └── Scenes/
│   │       └── MainScene.unity      # 主场景
│   └── ProjectSettings/
└── Build/                   # Unity构建输出目录
    └── SpraySimulation.exe  # 构建的可执行文件
```

## Unity项目要求

### 1. Unity版本
- 推荐使用Unity 2022.3 LTS或更高版本
- 支持Windows平台构建

### 2. 核心功能模块

#### QtCommunication.cs
负责与Qt应用程序的TCP通信：
- 连接到Qt应用程序的TCP服务器（默认端口12346）
- 发送和接收JSON格式的消息
- 处理心跳机制保持连接

#### WorkpieceLoader.cs
工件加载和显示：
- 支持PLY、STL、OBJ等格式的3D模型加载
- 动态创建和销毁工件对象
- 工件位置和姿态调整

#### TrajectoryRenderer.cs
轨迹可视化：
- 使用LineRenderer显示喷涂轨迹
- 支持轨迹动画播放
- 轨迹颜色和样式配置

#### SimulationController.cs
仿真控制：
- 机械臂运动仿真
- 碰撞检测
- 仿真状态管理（开始/暂停/停止/重置）

### 3. 通信协议

Unity与Qt之间使用TCP Socket进行通信，消息格式为JSON：

#### 从Qt发送到Unity的消息类型：

```json
{
  "type": "workpiece_data",
  "data": {
    "file_path": "path/to/workpiece.stl",
    "position": {"x": 0, "y": 0, "z": 0},
    "rotation": {"x": 0, "y": 0, "z": 0}
  }
}
```

```json
{
  "type": "trajectory_data",
  "data": {
    "points": [
      {"x": 100, "y": 200, "z": 300, "speed": 50},
      {"x": 110, "y": 210, "z": 310, "speed": 50}
    ]
  }
}
```

```json
{
  "type": "simulation_command",
  "command": "start|stop|pause|reset",
  "parameters": {}
}
```

#### 从Unity发送到Qt的消息类型：

```json
{
  "type": "workpiece_loaded",
  "success": true,
  "message": "工件加载成功"
}
```

```json
{
  "type": "collision_detected",
  "data": {
    "message": "检测到碰撞",
    "position": {"x": 100, "y": 200, "z": 150},
    "severity": "high"
  }
}
```

```json
{
  "type": "simulation_complete",
  "data": {
    "status": "completed",
    "duration": 5.2,
    "quality_score": 0.95
  }
}
```

## 测试方法

### 1. 使用Python测试客户端

运行测试客户端模拟Unity应用程序：

```bash
cd Unity
python UnityTestClient.py
```

### 2. 测试步骤

1. 启动Qt应用程序
2. 运行Python测试客户端
3. 在Qt应用程序中点击"初始化Unity引擎"
4. 测试各种功能：
   - 工件加载
   - 轨迹显示
   - 仿真控制
   - 碰撞检测

## Unity项目创建步骤

### 1. 创建新的Unity项目
```
1. 打开Unity Hub
2. 创建新项目，选择3D模板
3. 项目名称：SpraySimulation
4. 保存位置：Unity/SpraySimulation/
```

### 2. 导入必要的包
```
- Cinemachine（相机控制）
- ProBuilder（场景建模）
- Universal Render Pipeline（渲染管线）
```

### 3. 场景设置
```
1. 创建喷烘室环境
2. 导入机器人模型（安川MPX3500）
3. 设置光照和材质
4. 配置相机系统
```

### 4. 脚本开发
```
1. 实现TCP通信脚本
2. 开发工件加载系统
3. 创建轨迹渲染器
4. 实现碰撞检测
```

### 5. 构建设置
```
1. 平台：Windows
2. 架构：x86_64
3. 构建类型：Development Build
4. 输出目录：Unity/Build/
```

## 部署说明

### 1. Unity应用程序部署
- 将构建的Unity应用程序放置在Qt应用程序的相对路径：`Unity/SpraySimulation.exe`
- 确保Unity应用程序具有网络通信权限

### 2. 启动参数
Unity应用程序支持以下启动参数：
- `-parentHWND <窗口句柄>`: 嵌入到指定窗口
- `-popupwindow`: 无边框窗口模式
- `-qt-port <端口号>`: 指定Qt通信端口（默认12346）

### 3. 错误处理
- 网络连接失败：检查防火墙设置
- 模型加载失败：检查文件路径和格式
- 性能问题：调整渲染质量设置

## 开发注意事项

1. **性能优化**：大型工件模型需要进行LOD优化
2. **内存管理**：及时释放不用的模型资源
3. **线程安全**：网络通信在主线程中处理
4. **错误恢复**：实现网络断线重连机制
5. **日志记录**：详细记录通信和错误信息

## 扩展功能

未来可以扩展的功能：
- VR/AR支持
- 实时光线追踪
- 物理仿真增强
- 多机器人协作
- 云端渲染支持