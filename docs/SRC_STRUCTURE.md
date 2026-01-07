# src目录结构优化说明

## 概述

本文档说明了src目录的优化重构，将代码按功能模块进行了更细致的分类，提高了代码的可维护性和可读性。

**优化时间**: 2026年1月7日  
**优化范围**: 删除过时文件，创建子目录结构  
**总文件数**: 46个（删除3个过时文件）

---

## 1. 目录结构总览

```
src/
├── Core/                          # 核心应用程序框架
│   ├── Application.h/cpp
│   ├── ConfigManager.h/cpp
│   ├── Logger.h/cpp
│   └── CMakeLists.txt
│
├── Data/                          # 数据管理层
│   ├── Models/                    # 数据模型
│   │   ├── BaseModel.h/cpp
│   │   ├── WorkpieceData.h/cpp
│   │   ├── TrajectoryData.h/cpp
│   │   └── DataModels.h/cpp
│   │
│   ├── Database/                  # 数据库操作
│   │   ├── DatabaseManager.h/cpp
│   │   ├── DatabaseInitializer.h/cpp
│   │   ├── BatchManager.h/cpp
│   │   └── CMakeLists.txt
│   │
│   ├── STEP/                      # STEP文件处理
│   │   ├── STEPModelTree.h/cpp
│   │   ├── STEPModelTreeWorker.h/cpp
│   │   └── CMakeLists.txt
│   │
│   ├── PointCloud/                # 点云处理
│   │   ├── PointCloudParser.h/cpp
│   │   ├── PointCloudProcessor.h/cpp
│   │   ├── ScanDataReceiver.h/cpp
│   │   └── CMakeLists.txt
│   │
│   ├── Trajectory/                # 轨迹规划
│   │   ├── TrajectoryPlanner.h/cpp
│   │   └── CMakeLists.txt
│   │
│   └── CMakeLists.txt             # Data模块主CMakeLists
│
├── Robot/                         # 机器人控制
│   ├── Kinematics/                # 运动学计算
│   │   ├── RobotKinematics.h/cpp
│   │   └── CMakeLists.txt
│   │
│   ├── Control/                   # 机器人控制和通信
│   │   ├── RobotController.h/cpp
│   │   ├── MotoTcpClient.h/cpp
│   │   ├── ProgramGenerator.h/cpp
│   │   └── CMakeLists.txt
│   │
│   ├── UI/                        # 机器人控制面板
│   │   ├── RobotControlPanel.h/cpp
│   │   └── CMakeLists.txt
│   │
│   └── CMakeLists.txt             # Robot模块主CMakeLists
│
├── Simulation/                    # 仿真引擎
│   ├── SimulationEngine.h/cpp
│   ├── CollisionDetector.h/cpp
│   ├── QualityPredictor.h/cpp
│   └── CMakeLists.txt
│
├── UI/                            # 用户界面
│   ├── MainWindow.h/cpp           # 主窗口
│   │
│   ├── Panels/                    # 功能面板
│   │   ├── ParameterPanel.h/cpp
│   │   ├── StatusPanel.h/cpp
│   │   ├── SafetyPanel.h/cpp
│   │   ├── WorkpieceManagerPanel.h/cpp
│   │   └── CMakeLists.txt
│   │
│   ├── Visualization/             # 3D可视化
│   │   ├── VTKWidget.h/cpp
│   │   ├── Simple3DWidget.h/cpp
│   │   └── CMakeLists.txt
│   │
│   ├── ModelTree/                 # STEP模型树UI
│   │   ├── STEPModelTreeWidget.h/cpp
│   │   ├── ModelTreeDockWidget.h/cpp
│   │   └── CMakeLists.txt
│   │
│   ├── Loaders/                   # 文件加载器
│   │   ├── PointCloudLoader.h/cpp
│   │   └── CMakeLists.txt
│   │
│   └── CMakeLists.txt             # UI模块主CMakeLists
│
├── main.cpp                       # 应用程序入口
└── CMakeLists.txt                 # 项目主CMakeLists
```

---

## 2. 模块详细说明

### 2.1 Core模块 - 核心应用程序框架

**职责**: 应用程序初始化、配置管理、日志记录

| 文件 | 职责 | 关键类 |
|------|------|--------|
| Application.h/cpp | 应用程序主类，单例模式 | `Core::Application` |
| ConfigManager.h/cpp | 配置文件读写和管理 | `Core::ConfigManager` |
| Logger.h/cpp | 日志系统，支持文件和控制台输出 | `Core::Logger` |

**依赖**: Qt6::Core, Qt6::Network

**使用示例**:
```cpp
// 初始化应用
Core::Application::instance()->initialize();

// 读取配置
QString value = Core::ConfigManager::instance()->getValue("key");

// 记录日志
Core::Logger::instance()->log("信息", "消息内容");
```

---

### 2.2 Data模块 - 数据管理层

**职责**: 数据模型定义、数据库操作、文件解析、轨迹规划

#### 2.2.1 Models子模块 - 数据模型

| 文件 | 职责 |
|------|------|
| BaseModel.h/cpp | 所有数据模型的基类 |
| WorkpieceData.h/cpp | 工件数据模型 |
| TrajectoryData.h/cpp | 轨迹数据模型 |
| DataModels.h/cpp | 数据模型集合 |

**依赖**: Qt6::Core

#### 2.2.2 Database子模块 - 数据库操作

| 文件 | 职责 |
|------|------|
| DatabaseManager.h/cpp | MySQL和SQLite数据库管理 |
| DatabaseInitializer.h/cpp | 数据库初始化和表创建 |
| BatchManager.h/cpp | 批处理任务管理 |

**依赖**: Qt6::Sql, Models

**使用示例**:
```cpp
// 获取数据库单例
auto db = Data::DatabaseManager::instance();

// 保存工件数据
db->saveWorkpiece(workpieceData);

// 查询轨迹数据
auto trajectories = db->queryTrajectories();
```

#### 2.2.3 STEP子模块 - STEP文件处理

| 文件 | 职责 |
|------|------|
| STEPModelTree.h/cpp | STEP文件解析，构建模型树 |
| STEPModelTreeWorker.h/cpp | 异步STEP加载工作线程 |

**依赖**: OpenCASCADE, VTK, Qt6::Core

**关键特性**:
- 支持异步加载大型STEP文件
- 构建模型树结构
- 生成VTK Actor用于3D显示
- 支持加载进度信号

**使用示例**:
```cpp
// 创建STEP模型树
auto stepTree = new Data::STEPModelTree();

// 连接信号
connect(stepTree, &Data::STEPModelTree::modelTreeLoaded,
        this, &MyClass::onModelLoaded);

// 加载STEP文件
stepTree->loadFromSTEPFile("model.step");
```

#### 2.2.4 PointCloud子模块 - 点云处理

| 文件 | 职责 |
|------|------|
| PointCloudParser.h/cpp | 点云文件解析 |
| PointCloudProcessor.h/cpp | 点云处理算法 |
| ScanDataReceiver.h/cpp | 扫描数据接收 |

**依赖**: PCL, Qt6::Network

**支持格式**: PCD, PLY, XYZ

#### 2.2.5 Trajectory子模块 - 轨迹规划

| 文件 | 职责 |
|------|------|
| TrajectoryPlanner.h/cpp | 轨迹规划算法 |

**依赖**: Robot::RobotKinematics, Models

**状态**: 框架阶段，需要实现具体算法

---

### 2.3 Robot模块 - 机器人控制

**职责**: 机器人运动学、控制、通信

#### 2.3.1 Kinematics子模块 - 运动学计算

| 文件 | 职责 |
|------|------|
| RobotKinematics.h/cpp | 6轴机器人正逆运动学 |

**功能**:
- 正运动学: 关节角度 → 末端位姿
- 逆运动学: 末端位姿 → 关节角度
- 关节限位检查
- 奇异点检测

**使用示例**:
```cpp
// 创建运动学计算器
Robot::RobotKinematics kinematics;

// 正运动学
std::array<double, 6> angles = {0, 0, 0, 0, 0, 0};
auto pose = kinematics.forwardKinematics(angles);

// 逆运动学
std::array<double, 6> solution;
bool success = kinematics.inverseKinematics(targetPose, solution);
```

#### 2.3.2 Control子模块 - 机器人控制和通信

| 文件 | 职责 |
|------|------|
| RobotController.h/cpp | 机器人整体控制器 |
| MotoTcpClient.h/cpp | 安川机器人TCP通信 |
| ProgramGenerator.h/cpp | 机器人程序生成 |

**RobotController功能**:
- 连接管理
- 运动控制
- 状态监控
- 模式切换（仿真/远程/示教）

**MotoTcpClient功能**:
- TCP/IP通信
- 命令编码/解码
- 心跳检测
- 错误处理

**使用示例**:
```cpp
// 创建机器人控制器
auto controller = new Robot::RobotController();

// 连接机器人
controller->connectToRobot("192.168.1.100", 10040);

// 设置关节角度
std::array<double, 6> angles = {0, 45, 90, 0, 0, 0};
controller->setJointAngles(angles);

// 获取末端位姿
auto pose = controller->getEndEffectorPose();
```

#### 2.3.3 UI子模块 - 机器人控制面板

| 文件 | 职责 |
|------|------|
| RobotControlPanel.h/cpp | 机器人控制UI面板 |

**功能**:
- 关节角度控制（滑动条和数值输入）
- 末端位姿显示
- 连接管理
- 运动控制按钮
- 模式切换

---

### 2.4 Simulation模块 - 仿真引擎

**职责**: 仿真、碰撞检测、质量预测

| 文件 | 职责 | 状态 |
|------|------|------|
| SimulationEngine.h/cpp | 仿真引擎主类 | 框架 |
| CollisionDetector.h/cpp | 碰撞检测算法 | 框架 |
| QualityPredictor.h/cpp | 喷涂质量预测 | 框架 |

**依赖**: Robot, Data, OpenCASCADE

**计划功能**:
- 轨迹仿真
- 碰撞检测
- 质量评估
- 结果可视化

---

### 2.5 UI模块 - 用户界面

**职责**: 主窗口、各种功能面板、3D可视化

#### 2.5.1 主窗口

| 文件 | 职责 |
|------|------|
| MainWindow.h/cpp | 主窗口，整合所有面板 |

**功能**:
- 菜单栏
- 工具栏
- 停靠窗口管理
- 信号/槽连接
- 布局管理

#### 2.5.2 Panels子模块 - 功能面板

| 文件 | 职责 |
|------|------|
| ParameterPanel.h/cpp | 参数设置面板 |
| StatusPanel.h/cpp | 状态显示面板 |
| SafetyPanel.h/cpp | 安全警告面板 |
| WorkpieceManagerPanel.h/cpp | 工件管理面板 |

**依赖**: Qt6::Widgets, Data

#### 2.5.3 Visualization子模块 - 3D可视化

| 文件 | 职责 |
|------|------|
| VTKWidget.h/cpp | VTK 3D渲染窗口 |
| Simple3DWidget.h/cpp | 简单3D显示（OpenGL） |

**VTKWidget功能**:
- 3D模型显示
- 轨迹可视化
- 机器人显示
- 交互操作（旋转、缩放、平移）
- 相机管理

**依赖**: VTK, Qt6::OpenGL, Data

#### 2.5.4 ModelTree子模块 - STEP模型树UI

| 文件 | 职责 |
|------|------|
| STEPModelTreeWidget.h/cpp | STEP模型树显示控件 |
| ModelTreeDockWidget.h/cpp | 模型树停靠窗口 |

**功能**:
- 树形显示模型结构
- 节点选择
- 可见性控制
- 属性显示

**依赖**: Data::STEPModelTree, Qt6::Widgets

#### 2.5.5 Loaders子模块 - 文件加载器

| 文件 | 职责 |
|------|------|
| PointCloudLoader.h/cpp | 点云加载UI |

**功能**:
- 文件选择对话框
- 加载进度显示
- 错误处理

**依赖**: Data::PointCloudParser, Qt6::Widgets

---

## 3. 文件移动清单

### 已删除的过时文件
- ❌ `src/UI/MainWindow_Simple.cpp` - 简化版本
- ❌ `src/UI/MainWindow_VTK.cpp` - VTK版本
- ❌ `src/Data/STEPModelTree_old.cpp` - 旧版本实现

### 待移动的文件（下一步）

**Data模块**:
```
src/Data/
├── Models/
│   ├── BaseModel.h/cpp
│   ├── WorkpieceData.h/cpp
│   ├── TrajectoryData.h/cpp
│   └── DataModels.h/cpp
├── Database/
│   ├── DatabaseManager.h/cpp
│   ├── DatabaseInitializer.h/cpp
│   └── BatchManager.h/cpp
├── STEP/
│   ├── STEPModelTree.h/cpp
│   └── STEPModelTreeWorker.h/cpp
├── PointCloud/
│   ├── PointCloudParser.h/cpp
│   ├── PointCloudProcessor.h/cpp
│   └── ScanDataReceiver.h/cpp
└── Trajectory/
    └── TrajectoryPlanner.h/cpp
```

**Robot模块**:
```
src/Robot/
├── Kinematics/
│   └── RobotKinematics.h/cpp
├── Control/
│   ├── RobotController.h/cpp
│   ├── MotoTcpClient.h/cpp
│   └── ProgramGenerator.h/cpp
└── UI/
    └── RobotControlPanel.h/cpp
```

**UI模块**:
```
src/UI/
├── Panels/
│   ├── ParameterPanel.h/cpp
│   ├── StatusPanel.h/cpp
│   ├── SafetyPanel.h/cpp
│   └── WorkpieceManagerPanel.h/cpp
├── Visualization/
│   ├── VTKWidget.h/cpp
│   └── Simple3DWidget.h/cpp
├── ModelTree/
│   ├── STEPModelTreeWidget.h/cpp
│   └── ModelTreeDockWidget.h/cpp
└── Loaders/
    └── PointCloudLoader.h/cpp
```

---

## 4. Include路径更新指南

### 旧的Include路径
```cpp
#include "STEPModelTree.h"
#include "RobotKinematics.h"
#include "ParameterPanel.h"
```

### 新的Include路径
```cpp
#include "Data/STEP/STEPModelTree.h"
#include "Robot/Kinematics/RobotKinematics.h"
#include "UI/Panels/ParameterPanel.h"
```

### 相对路径Include（推荐）
```cpp
// 在Data/STEP/STEPModelTree.cpp中
#include "../Models/BaseModel.h"
#include "STEPModelTreeWorker.h"

// 在UI/Panels/ParameterPanel.cpp中
#include "../MainWindow.h"
#include "StatusPanel.h"
```

---

## 5. CMakeLists.txt更新

### 主CMakeLists.txt结构
```cmake
# src/CMakeLists.txt
add_subdirectory(Core)
add_subdirectory(Data)
add_subdirectory(Robot)
add_subdirectory(Simulation)
add_subdirectory(UI)

add_executable(spray-trajectory-planning main.cpp)
target_link_libraries(spray-trajectory-planning
    Core Data Robot Simulation UI
)
```

### 子模块CMakeLists.txt示例

**Data/CMakeLists.txt**:
```cmake
add_subdirectory(Models)
add_subdirectory(Database)
add_subdirectory(STEP)
add_subdirectory(PointCloud)
add_subdirectory(Trajectory)

add_library(Data INTERFACE)
target_link_libraries(Data INTERFACE
    DataModels DataDatabase DataSTEP DataPointCloud DataTrajectory
)
```

**Data/STEP/CMakeLists.txt**:
```cmake
add_library(DataSTEP
    STEPModelTree.cpp
    STEPModelTreeWorker.cpp
)
target_include_directories(DataSTEP PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(DataSTEP PUBLIC
    Qt6::Core
    OpenCASCADE::TKernel
    VTK::vtkCommon
)
```

---

## 6. 依赖关系图

### 模块依赖关系
```
┌─────────────────────────────────────────┐
│           UI (用户界面)                  │
│  ┌──────────────────────────────────┐  │
│  │ MainWindow                       │  │
│  │ ├─ Panels (参数、状态、安全)     │  │
│  │ ├─ Visualization (VTK显示)      │  │
│  │ ├─ ModelTree (模型树)           │  │
│  │ └─ Loaders (文件加载)           │  │
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

### 编译顺序
1. Core (无依赖)
2. Data (依赖Core)
3. Robot (依赖Core, Data)
4. Simulation (依赖Core, Data, Robot)
5. UI (依赖所有模块)
6. main.cpp (依赖所有模块)

---

## 7. 快速导航

### 需要修改什么？

| 需求 | 位置 |
|------|------|
| 修改UI布局 | `src/UI/MainWindow.cpp` |
| 添加新的参数面板 | `src/UI/Panels/` |
| 修改3D显示 | `src/UI/Visualization/VTKWidget.cpp` |
| 修改机器人控制 | `src/Robot/Control/RobotController.cpp` |
| 修改运动学计算 | `src/Robot/Kinematics/RobotKinematics.cpp` |
| 修改STEP加载 | `src/Data/STEP/STEPModelTree.cpp` |
| 修改数据库操作 | `src/Data/Database/DatabaseManager.cpp` |
| 添加新的数据模型 | `src/Data/Models/` |
| 修改日志系统 | `src/Core/Logger.cpp` |
| 修改配置管理 | `src/Core/ConfigManager.cpp` |

---

## 8. 优化收益

### ✅ 优点
1. **更清晰的模块划分**: 相关文件聚集在一起
2. **更容易找到文件**: 按功能分类，快速定位
3. **更容易管理依赖**: 子目录CMakeLists.txt独立管理
4. **更容易进行单元测试**: 每个子模块可独立测试
5. **更容易进行代码审查**: 模块职责明确
6. **更容易进行重构**: 模块间耦合度低

### 📊 统计数据
- **总文件数**: 46个（删除3个过时文件）
- **子目录数**: 13个
- **模块数**: 5个
- **代码行数**: ~7000行

---

## 9. 后续优化计划

### 第1阶段: 文件移动（优先级: 高）
- [ ] 移动Data模块文件到子目录
- [ ] 移动Robot模块文件到子目录
- [ ] 移动UI模块文件到子目录
- [ ] 更新所有CMakeLists.txt
- [ ] 更新所有include路径
- [ ] 重新编译验证

### 第2阶段: 代码优化（优先级: 中）
- [ ] 完成Simulation模块实现
- [ ] 审查Simple3DWidget和DataModels
- [ ] 添加单元测试
- [ ] 改进代码文档

### 第3阶段: 进一步优化（优先级: 低）
- [ ] 分离UI和业务逻辑
- [ ] 使用工厂模式
- [ ] 使用观察者模式
- [ ] 性能优化

---

## 10. 参考文档

- [src目录详细分析](src-directory-analysis.md)
- [模块依赖关系图](module-dependency-diagram.md)
- [快速参考指南](src-quick-reference.md)
- [清理建议](cleanup-recommendations.md)

---

## 11. 常见问题

### Q: 为什么要创建子目录？
A: 
1. 提高代码可读性
2. 更容易找到相关文件
3. 更容易管理依赖
4. 更容易进行单元测试
5. 更容易进行代码审查

### Q: 如何更新include路径？
A: 
1. 使用相对路径: `#include "../Models/BaseModel.h"`
2. 或使用绝对路径: `#include "Data/Models/BaseModel.h"`
3. 在CMakeLists.txt中配置include目录

### Q: 如何编译新的结构？
A:
```bash
cd build
cmake ..
cmake --build . --config Release
```

### Q: 如何验证编译成功？
A:
```bash
# 检查是否有编译错误
cmake --build . 2>&1 | grep -i error

# 运行应用程序
./spray-trajectory-planning
```

---

## 12. 总结

通过本次优化，src目录的结构变得更加清晰和有序：

- ✅ 删除了3个过时文件
- ✅ 创建了13个子目录
- ✅ 明确了各模块的职责
- ✅ 提高了代码的可维护性

下一步需要：
1. 移动文件到新的子目录
2. 更新所有include路径
3. 更新CMakeLists.txt
4. 重新编译验证

这将进一步提高代码的质量和可维护性。
