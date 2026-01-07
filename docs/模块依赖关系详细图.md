# 模块依赖关系详细图

## 1. 整体架构图

```
┌─────────────────────────────────────────────────────────────────┐
│                        应用程序入口                              │
│                       src/main.cpp                              │
└────────────────────────────┬────────────────────────────────────┘
                             │
                ┌────────────┴────────────┐
                │                         │
        ┌───────▼────────┐      ┌────────▼──────────┐
        │ Core::          │      │ UI::              │
        │ Application     │      │ MainWindow        │
        └────────┬────────┘      └────────┬──────────┘
                 │                        │
        ┌────────┴────────┐       ┌───────┴──────────────────────┐
        │                 │       │                              │
   ┌────▼────┐      ┌────▼────┐  │  ┌──────────────────────────┐ │
   │Config   │      │Logger   │  │  │ UI::VTKWidget            │ │
   │Manager  │      │         │  │  │ (3D Visualization)       │ │
   └─────────┘      └─────────┘  │  └──────────────────────────┘ │
                                 │  ┌──────────────────────────┐ │
                                 │  │ UI::各种Panel            │ │
                                 │  │ (Parameter/Status/Safety)│ │
                                 │  └──────────────────────────┘ │
                                 │  ┌──────────────────────────┐ │
                                 │  │ Robot::RobotControlPanel │ │
                                 │  └──────────────────────────┘ │
                                 └──────────────────────────────┘
```

## 2. 详细模块依赖树

### 2.1 Core模块
```
Core::Application (单例)
├── Core::ConfigManager
│   └── 读取: config/paths.cmake
└── Core::Logger
    └── 输出: logs/*.log
```

### 2.2 Data模块
```
Data (数据层)
├── Models (数据模型)
│   ├── BaseModel (基类)
│   ├── WorkpieceData (工件数据)
│   └── TrajectoryData (轨迹数据)
│
├── Database (数据库)
│   ├── DatabaseManager (MySQL + SQLite)
│   │   ├── 依赖: BaseModel, WorkpieceData, TrajectoryData
│   │   └── 连接: MySQL服务器 + SQLite本地缓存
│   ├── DatabaseInitializer
│   │   └── 依赖: DatabaseManager
│   └── BatchManager (批处理)
│
├── STEP (STEP文件处理)
│   ├── STEPModelTree (核心)
│   │   ├── 依赖: OpenCASCADE库, VTK库
│   │   ├── 输入: *.step/*.stp文件
│   │   └── 输出: 模型树结构 + VTK Actor
│   └── STEPModelTreeWorker (异步加载)
│       └── 依赖: STEPModelTree, QThread
│
├── PointCloud (点云处理)
│   ├── PointCloudParser
│   │   ├── 依赖: PCL库
│   │   └── 输入: *.pcd/*.ply文件
│   ├── PointCloudProcessor
│   │   └── 依赖: PCL库, PointCloudParser
│   └── ScanDataReceiver
│       └── 依赖: Qt6::Network
│
└── Trajectory (轨迹规划)
    └── TrajectoryPlanner (框架)
        └── 依赖: RobotKinematics
```

### 2.3 Robot模块
```
Robot (机器人控制)
├── Kinematics (运动学)
│   └── RobotKinematics
│       ├── 输入: 关节角度 (6个)
│       ├── 计算: 正/逆运动学
│       └── 输出: 末端位姿 (x,y,z,r,p,y)
│
├── Control (控制)
│   ├── RobotController (主控制器)
│   │   ├── 依赖: RobotKinematics, MotoTcpClient
│   │   ├── 功能: 连接管理, 运动控制, 状态监控
│   │   └── 信号: connectionStateChanged, jointAnglesChanged等
│   ├── MotoTcpClient (通信)
│   │   ├── 依赖: Qt6::Network
│   │   ├── 协议: TCP/IP (安川机器人)
│   │   └── 端口: 10040 (默认)
│   └── ProgramGenerator
│       └── 生成: 机器人程序代码
│
└── UI (控制面板)
    └── RobotControlPanel
        ├── 依赖: RobotController
        ├── 功能: 关节控制, 位姿显示, 连接管理
        └── 信号: 连接到MainWindow
```

### 2.4 Simulation模块
```
Simulation (仿真引擎) - 框架阶段
├── SimulationEngine (主引擎)
│   ├── 依赖: RobotKinematics, CollisionDetector
│   └── 功能: 仿真循环, 时间管理
├── CollisionDetector (碰撞检测)
│   ├── 依赖: OpenCASCADE库
│   └── 功能: 机器人-工件碰撞检测
└── QualityPredictor (质量预测)
    ├── 依赖: TrajectoryData
    └── 功能: 喷涂质量评估
```

### 2.5 UI模块
```
UI (用户界面)
├── MainWindow (主窗口)
│   ├── 依赖: 所有其他UI组件
│   ├── 菜单: 文件, 编辑, 视图, 工具, 帮助
│   ├── 工具栏: 常用操作
│   └── 停靠窗口: 各种面板
│
├── Visualization (可视化)
│   ├── VTKWidget (3D渲染)
│   │   ├── 依赖: VTK库, STEPModelTree, STEPModelTreeWidget
│   │   ├── 功能: 显示模型, 轨迹, 机器人
│   │   └── 交互: 旋转, 缩放, 平移
│   └── Simple3DWidget (简单3D) - 可能重复
│       └── 依赖: Qt6::OpenGL
│
├── Panels (功能面板)
│   ├── ParameterPanel (参数设置)
│   │   └── 功能: 喷涂参数, 轨迹参数
│   ├── StatusPanel (状态显示)
│   │   └── 功能: 系统状态, 日志显示
│   ├── SafetyPanel (安全警告)
│   │   └── 功能: 碰撞警告, 安全提示
│   └── WorkpieceManagerPanel (工件管理)
│       └── 功能: 工件导入, 工件列表
│
├── ModelTree (模型树)
│   ├── STEPModelTreeWidget (树形控件)
│   │   ├── 依赖: STEPModelTree
│   │   ├── 功能: 显示模型树, 节点选择
│   │   └── 信号: nodeVisibilityChanged
│   └── ModelTreeDockWidget (停靠窗口)
│       └── 依赖: STEPModelTreeWidget
│
└── Loaders (加载器)
    └── PointCloudLoader (点云加载)
        ├── 依赖: PointCloudParser
        └── 功能: 点云文件加载UI
```

## 3. 数据流向

### 3.1 STEP文件加载流程
```
用户选择STEP文件
    ↓
MainWindow::OnImportSTEPModel()
    ↓
STEPModelTreeWidget::loadSTEPFile()
    ↓
STEPModelTree::loadFromSTEPFile() (主线程)
    ↓
STEPModelTreeWorker::run() (工作线程)
    ├─ 解析STEP文件 (OpenCASCADE)
    ├─ 构建模型树
    ├─ 创建VTK Actor
    └─ 发送信号: loadProgress, modelTreeLoaded
    ↓
VTKWidget::onModelTreeLoaded()
    ├─ 添加Actor到场景
    └─ 更新显示
    ↓
STEPModelTreeWidget::updateTreeView()
    └─ 显示树形结构
```

### 3.2 机器人控制流程
```
用户输入关节角度
    ↓
RobotControlPanel::onJointChanged()
    ↓
RobotController::setJointAngles()
    ├─ 验证限位
    ├─ 计算正运动学
    └─ 发送TCP命令
    ↓
MotoTcpClient::sendCommand()
    ├─ 编码命令
    └─ 发送到机器人
    ↓
机器人执行运动
    ↓
MotoTcpClient::onDataReceived()
    ├─ 解析反馈
    └─ 更新状态
    ↓
RobotController::updateRobotState()
    ├─ 更新关节角度
    ├─ 计算末端位姿
    └─ 发送信号
    ↓
RobotControlPanel::onRobotStateChanged()
    └─ 更新UI显示
```

### 3.3 轨迹规划流程
```
用户设置轨迹参数
    ↓
ParameterPanel::onParametersChanged()
    ↓
TrajectoryPlanner::planTrajectory()
    ├─ 读取工件数据
    ├─ 计算喷涂轨迹
    └─ 生成关键点
    ↓
TrajectoryData::saveTrajectory()
    ├─ 保存到数据库
    └─ 保存到文件
    ↓
VTKWidget::displayTrajectory()
    └─ 显示轨迹线
    ↓
SimulationEngine::simulate()
    ├─ 逆运动学求解
    ├─ 碰撞检测
    ├─ 质量预测
    └─ 生成仿真结果
```

## 4. 编译依赖关系

### 4.1 库依赖
```
Core库
├── Qt6::Core
└── Qt6::Network

Data库
├── Qt6::Core
├── Qt6::Sql
├── Qt6::Gui
├── Qt6::Network
├── PCL库
└── OpenCASCADE库

Robot库
├── Qt6::Core
├── Qt6::Network
├── Qt6::Widgets
└── Qt6::Gui

Simulation库
├── Qt6::Core
└── Qt6::OpenGL

UI库
├── Qt6::Core
├── Qt6::Widgets
├── Qt6::Network
├── Qt6::OpenGL
├── Qt6::OpenGLWidgets
├── VTK库
├── Data库 (新增)
└── Robot库 (隐含)
```

### 4.2 链接顺序
```
main.cpp
    ↓
UI库 (依赖所有其他库)
    ├─ Data库
    ├─ Robot库
    └─ Simulation库
        ├─ Core库
        └─ 外部库 (Qt6, VTK, PCL, OpenCASCADE)
```

## 5. 信号/槽连接图

### 5.1 主要信号流
```
STEPModelTree::modelTreeLoaded()
    ↓
STEPModelTreeWidget::onModelTreeLoaded()
    ↓
VTKWidget::onModelTreeLoaded()
    ↓
MainWindow::OnVTKSceneReady()

RobotController::jointAnglesChanged()
    ↓
RobotControlPanel::onJointAnglesChanged()
    ↓
VTKWidget::updateRobotPose()

STEPModelTree::nodeVisibilityChanged()
    ↓
VTKWidget::onNodeVisibilityChanged()
    ↓
VTKWidget::updateActorVisibility()
```

## 6. 文件包含关系

### 6.1 头文件包含树
```
MainWindow.h
├── VTKWidget.h
│   ├── STEPModelTreeWidget.h
│   │   └── STEPModelTree.h (Data)
│   └── StatusPanel.h
├── ParameterPanel.h
├── StatusPanel.h
├── SafetyPanel.h
├── PointCloudLoader.h
│   └── PointCloudParser.h (Data)
├── ModelTreeDockWidget.h
│   └── STEPModelTreeWidget.h
├── WorkpieceManagerPanel.h
├── RobotControlPanel.h (Robot)
│   └── RobotController.h
│       ├── RobotKinematics.h
│       └── MotoTcpClient.h
└── STEPModelTreeWidget.h
```

### 6.2 实现文件包含
```
MainWindow.cpp
├── MainWindow.h
├── VTKWidget.h
├── ParameterPanel.h
├── StatusPanel.h
├── SafetyPanel.h
├── PointCloudLoader.h
├── ModelTreeDockWidget.h
├── STEPModelTreeWidget.h
├── WorkpieceManagerPanel.h
├── RobotControlPanel.h
├── PointCloudParser.h (Data)
└── Qt6 headers
```

## 7. 循环依赖检查

### ✅ 无循环依赖
- Core → 无依赖其他模块
- Data → 无依赖UI/Robot/Simulation
- Robot → 无依赖UI/Data/Simulation
- Simulation → 依赖Robot/Data (单向)
- UI → 依赖所有其他模块 (单向)

### 结论
**架构设计良好，无循环依赖问题**

## 8. 模块间通信方式

### 8.1 信号/槽 (推荐)
- 用于异步通信
- 例: STEPModelTree::modelTreeLoaded() → VTKWidget::onModelTreeLoaded()

### 8.2 直接调用
- 用于同步操作
- 例: RobotController::setJointAngles()

### 8.3 数据库
- 用于持久化
- 例: DatabaseManager::saveTrajectory()

### 8.4 配置文件
- 用于启动参数
- 例: ConfigManager::readConfig()

