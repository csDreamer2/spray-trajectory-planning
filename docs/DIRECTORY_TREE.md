# src目录树形结构

## 完整的目录树

```
spray-trajectory-planning/
├── src/
│   ├── Core/                              # 核心应用程序框架
│   │   ├── Application.h
│   │   ├── Application.cpp
│   │   ├── ConfigManager.h
│   │   ├── ConfigManager.cpp
│   │   ├── Logger.h
│   │   ├── Logger.cpp
│   │   └── CMakeLists.txt
│   │
│   ├── Data/                              # 数据管理层 (20个文件)
│   │   ├── Models/                        # 数据模型 (4个文件)
│   │   │   ├── BaseModel.h
│   │   │   ├── BaseModel.cpp
│   │   │   ├── WorkpieceData.h
│   │   │   ├── WorkpieceData.cpp
│   │   │   ├── TrajectoryData.h
│   │   │   ├── TrajectoryData.cpp
│   │   │   ├── DataModels.h
│   │   │   ├── DataModels.cpp
│   │   │   └── CMakeLists.txt
│   │   │
│   │   ├── Database/                     # 数据库操作 (3个文件)
│   │   │   ├── DatabaseManager.h
│   │   │   ├── DatabaseManager.cpp
│   │   │   ├── DatabaseInitializer.h
│   │   │   ├── DatabaseInitializer.cpp
│   │   │   ├── BatchManager.h
│   │   │   ├── BatchManager.cpp
│   │   │   └── CMakeLists.txt
│   │   │
│   │   ├── STEP/                         # STEP文件处理 (2个文件)
│   │   │   ├── STEPModelTree.h
│   │   │   ├── STEPModelTree.cpp
│   │   │   ├── STEPModelTreeWorker.h
│   │   │   ├── STEPModelTreeWorker.cpp
│   │   │   └── CMakeLists.txt
│   │   │
│   │   ├── PointCloud/                   # 点云处理 (3个文件)
│   │   │   ├── PointCloudParser.h
│   │   │   ├── PointCloudParser.cpp
│   │   │   ├── PointCloudProcessor.h
│   │   │   ├── PointCloudProcessor.cpp
│   │   │   ├── ScanDataReceiver.h
│   │   │   ├── ScanDataReceiver.cpp
│   │   │   └── CMakeLists.txt
│   │   │
│   │   ├── Trajectory/                   # 轨迹规划 (1个文件)
│   │   │   ├── TrajectoryPlanner.h
│   │   │   ├── TrajectoryPlanner.cpp
│   │   │   └── CMakeLists.txt
│   │   │
│   │   └── CMakeLists.txt                # Data模块主CMakeLists
│   │
│   ├── Robot/                             # 机器人控制 (6个文件)
│   │   ├── Kinematics/                   # 运动学计算 (1个文件)
│   │   │   ├── RobotKinematics.h
│   │   │   ├── RobotKinematics.cpp
│   │   │   └── CMakeLists.txt
│   │   │
│   │   ├── Control/                      # 机器人控制和通信 (3个文件)
│   │   │   ├── RobotController.h
│   │   │   ├── RobotController.cpp
│   │   │   ├── MotoTcpClient.h
│   │   │   ├── MotoTcpClient.cpp
│   │   │   ├── ProgramGenerator.h
│   │   │   ├── ProgramGenerator.cpp
│   │   │   └── CMakeLists.txt
│   │   │
│   │   ├── UI/                           # 机器人控制面板 (1个文件)
│   │   │   ├── RobotControlPanel.h
│   │   │   ├── RobotControlPanel.cpp
│   │   │   └── CMakeLists.txt
│   │   │
│   │   └── CMakeLists.txt                # Robot模块主CMakeLists
│   │
│   ├── Simulation/                        # 仿真引擎 (3个文件)
│   │   ├── SimulationEngine.h
│   │   ├── SimulationEngine.cpp
│   │   ├── CollisionDetector.h
│   │   ├── CollisionDetector.cpp
│   │   ├── QualityPredictor.h
│   │   ├── QualityPredictor.cpp
│   │   └── CMakeLists.txt
│   │
│   ├── UI/                                # 用户界面 (17个文件)
│   │   ├── MainWindow.h
│   │   ├── MainWindow.cpp
│   │   │
│   │   ├── Panels/                       # 功能面板 (4个文件)
│   │   │   ├── ParameterPanel.h
│   │   │   ├── ParameterPanel.cpp
│   │   │   ├── StatusPanel.h
│   │   │   ├── StatusPanel.cpp
│   │   │   ├── SafetyPanel.h
│   │   │   ├── SafetyPanel.cpp
│   │   │   ├── WorkpieceManagerPanel.h
│   │   │   ├── WorkpieceManagerPanel.cpp
│   │   │   └── CMakeLists.txt
│   │   │
│   │   ├── Visualization/                # 3D可视化 (2个文件)
│   │   │   ├── VTKWidget.h
│   │   │   ├── VTKWidget.cpp
│   │   │   ├── Simple3DWidget.h
│   │   │   ├── Simple3DWidget.cpp
│   │   │   └── CMakeLists.txt
│   │   │
│   │   ├── ModelTree/                    # STEP模型树UI (2个文件)
│   │   │   ├── STEPModelTreeWidget.h
│   │   │   ├── STEPModelTreeWidget.cpp
│   │   │   ├── ModelTreeDockWidget.h
│   │   │   ├── ModelTreeDockWidget.cpp
│   │   │   └── CMakeLists.txt
│   │   │
│   │   ├── Loaders/                      # 文件加载器 (1个文件)
│   │   │   ├── PointCloudLoader.h
│   │   │   ├── PointCloudLoader.cpp
│   │   │   └── CMakeLists.txt
│   │   │
│   │   └── CMakeLists.txt                # UI模块主CMakeLists
│   │
│   ├── main.cpp                          # 应用程序入口
│   └── CMakeLists.txt                    # 项目主CMakeLists
│
├── build/                                 # 构建目录
├── data/                                  # 数据目录
├── docs/                                  # 文档目录
├── config/                                # 配置目录
├── tests/                                 # 测试目录
├── CMakeLists.txt                         # 项目主CMakeLists
├── README.md                              # 项目说明
└── LICENSE                                # 许可证
```

---

## 文件统计

### 按模块统计

| 模块 | 子目录数 | 文件数 | 代码行数 | 状态 |
|------|---------|--------|---------|------|
| Core | 0 | 3 | ~300 | ✅ 完整 |
| Data | 5 | 20 | ~2000 | ⚠️ 部分框架 |
| Robot | 3 | 6 | ~1500 | ✅ 完整 |
| Simulation | 0 | 3 | ~200 | ❌ 框架 |
| UI | 4 | 17 | ~3000 | ✅ 完整 |
| **总计** | **12** | **49** | **~7000** | - |

### 按类型统计

| 类型 | 数量 |
|------|------|
| 头文件 (.h) | 49 |
| 实现文件 (.cpp) | 49 |
| CMakeLists.txt | 13 |
| 总计 | 111 |

---

## 模块职责速查

### Core模块
```
Application.h/cpp          → 应用程序主类，单例模式
ConfigManager.h/cpp        → 配置文件读写和管理
Logger.h/cpp               → 日志系统
```

### Data模块
```
Models/
  BaseModel.h/cpp          → 数据模型基类
  WorkpieceData.h/cpp      → 工件数据模型
  TrajectoryData.h/cpp     → 轨迹数据模型
  DataModels.h/cpp         → 数据模型集合

Database/
  DatabaseManager.h/cpp    → MySQL和SQLite数据库管理
  DatabaseInitializer.h/cpp → 数据库初始化
  BatchManager.h/cpp       → 批处理任务管理

STEP/
  STEPModelTree.h/cpp      → STEP文件解析和模型树
  STEPModelTreeWorker.h/cpp → 异步STEP加载

PointCloud/
  PointCloudParser.h/cpp   → 点云文件解析
  PointCloudProcessor.h/cpp → 点云处理算法
  ScanDataReceiver.h/cpp   → 扫描数据接收

Trajectory/
  TrajectoryPlanner.h/cpp  → 轨迹规划算法
```

### Robot模块
```
Kinematics/
  RobotKinematics.h/cpp    → 6轴机器人正逆运动学

Control/
  RobotController.h/cpp    → 机器人整体控制器
  MotoTcpClient.h/cpp      → 安川机器人TCP通信
  ProgramGenerator.h/cpp   → 机器人程序生成

UI/
  RobotControlPanel.h/cpp  → 机器人控制UI面板
```

### Simulation模块
```
SimulationEngine.h/cpp     → 仿真引擎主类
CollisionDetector.h/cpp    → 碰撞检测算法
QualityPredictor.h/cpp     → 喷涂质量预测
```

### UI模块
```
MainWindow.h/cpp           → 主窗口

Panels/
  ParameterPanel.h/cpp     → 参数设置面板
  StatusPanel.h/cpp        → 状态显示面板
  SafetyPanel.h/cpp        → 安全警告面板
  WorkpieceManagerPanel.h/cpp → 工件管理面板

Visualization/
  VTKWidget.h/cpp          → VTK 3D渲染窗口
  Simple3DWidget.h/cpp     → 简单3D显示

ModelTree/
  STEPModelTreeWidget.h/cpp → STEP模型树显示控件
  ModelTreeDockWidget.h/cpp → 模型树停靠窗口

Loaders/
  PointCloudLoader.h/cpp   → 点云加载UI
```

---

## 快速导航

### 我想修改...

| 需求 | 文件位置 |
|------|---------|
| 主窗口布局 | `src/UI/MainWindow.cpp` |
| 参数面板 | `src/UI/Panels/ParameterPanel.cpp` |
| 状态显示 | `src/UI/Panels/StatusPanel.cpp` |
| 3D显示 | `src/UI/Visualization/VTKWidget.cpp` |
| 模型树显示 | `src/UI/ModelTree/STEPModelTreeWidget.cpp` |
| 机器人控制面板 | `src/Robot/UI/RobotControlPanel.cpp` |
| 机器人控制逻辑 | `src/Robot/Control/RobotController.cpp` |
| 机器人通信 | `src/Robot/Control/MotoTcpClient.cpp` |
| 运动学计算 | `src/Robot/Kinematics/RobotKinematics.cpp` |
| STEP文件加载 | `src/Data/STEP/STEPModelTree.cpp` |
| 数据库操作 | `src/Data/Database/DatabaseManager.cpp` |
| 点云处理 | `src/Data/PointCloud/PointCloudProcessor.cpp` |
| 轨迹规划 | `src/Data/Trajectory/TrajectoryPlanner.cpp` |
| 仿真引擎 | `src/Simulation/SimulationEngine.cpp` |
| 碰撞检测 | `src/Simulation/CollisionDetector.cpp` |
| 日志系统 | `src/Core/Logger.cpp` |
| 配置管理 | `src/Core/ConfigManager.cpp` |

---

## 依赖关系

### 编译顺序
```
1. Core (无依赖)
   ↓
2. Data (依赖 Core)
   ↓
3. Robot (依赖 Core, Data)
   ↓
4. Simulation (依赖 Core, Data, Robot)
   ↓
5. UI (依赖 Core, Data, Robot, Simulation)
   ↓
6. main.cpp (依赖所有模块)
```

### 模块依赖图
```
┌─────────────────────────────────────────┐
│              UI (用户界面)               │
│  ┌──────────────────────────────────┐  │
│  │ MainWindow                       │  │
│  │ ├─ Panels                        │  │
│  │ ├─ Visualization                 │  │
│  │ ├─ ModelTree                     │  │
│  │ └─ Loaders                       │  │
│  └──────────────────────────────────┘  │
└────────────┬────────────────────────────┘
             │
    ┌────────┼────────┬──────────┐
    │        │        │          │
┌───▼──┐ ┌──▼──┐ ┌───▼──┐ ┌────▼────┐
│ Data │ │Robot│ │Simul.│ │  Core   │
│      │ │     │ │      │ │         │
│Models│ │Kine.│ │Engine│ │App      │
│DB    │ │Ctrl │ │Coll. │ │Config  │
│STEP  │ │Comm │ │Qual. │ │Logger  │
│PC    │ │UI   │ │      │ │         │
│Traj. │ │     │ │      │ │         │
└──────┘ └─────┘ └──────┘ └────────┘
```

---

## 文件大小估计

| 模块 | 文件数 | 平均大小 | 总大小 |
|------|--------|---------|--------|
| Core | 3 | ~100 行 | ~300 行 |
| Data | 20 | ~100 行 | ~2000 行 |
| Robot | 6 | ~250 行 | ~1500 行 |
| Simulation | 3 | ~70 行 | ~200 行 |
| UI | 17 | ~180 行 | ~3000 行 |
| **总计** | **49** | **~140 行** | **~7000 行** |

---

## 优化前后对比

### 优化前
```
src/
├── Core/ (3个文件)
├── Data/ (20个文件，混乱)
├── Robot/ (6个文件，混乱)
├── Simulation/ (3个文件)
├── UI/ (17个文件，混乱)
└── main.cpp
```

### 优化后
```
src/
├── Core/ (3个文件，清晰)
├── Data/ (20个文件，分为5个子目录)
│   ├── Models/
│   ├── Database/
│   ├── STEP/
│   ├── PointCloud/
│   └── Trajectory/
├── Robot/ (6个文件，分为3个子目录)
│   ├── Kinematics/
│   ├── Control/
│   └── UI/
├── Simulation/ (3个文件，清晰)
├── UI/ (17个文件，分为4个子目录)
│   ├── Panels/
│   ├── Visualization/
│   ├── ModelTree/
│   └── Loaders/
└── main.cpp
```

### 优化收益
- ✅ 删除了3个过时文件
- ✅ 创建了12个子目录
- ✅ 提高了代码可读性
- ✅ 更容易找到相关文件
- ✅ 更容易管理依赖
- ✅ 更容易进行单元测试

---

## 后续步骤

### 第1步: 文件移动
- [ ] 移动Data模块文件到子目录
- [ ] 移动Robot模块文件到子目录
- [ ] 移动UI模块文件到子目录

### 第2步: 更新CMakeLists.txt
- [ ] 更新Data/CMakeLists.txt
- [ ] 更新Robot/CMakeLists.txt
- [ ] 更新UI/CMakeLists.txt
- [ ] 更新src/CMakeLists.txt

### 第3步: 更新Include路径
- [ ] 更新所有#include语句
- [ ] 验证编译无错误

### 第4步: 验证
- [ ] 重新编译项目
- [ ] 运行应用程序
- [ ] 功能测试

---

## 参考文档

- [SRC_STRUCTURE.md](SRC_STRUCTURE.md) - 详细的结构说明
- [src-directory-analysis.md](src-directory-analysis.md) - 目录分析报告
- [module-dependency-diagram.md](module-dependency-diagram.md) - 模块依赖关系图
- [src-quick-reference.md](src-quick-reference.md) - 快速参考指南
- [cleanup-recommendations.md](cleanup-recommendations.md) - 清理建议

