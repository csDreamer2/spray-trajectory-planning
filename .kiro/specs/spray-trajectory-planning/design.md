# 自动喷涂轨迹规划系统设计文档

## 概述

自动喷涂轨迹规划系统是一个专为大型复杂工件喷涂作业设计的桌面软件应用。系统采用模块化架构，重点实现仿真模块、机器人控制模块、轨迹规划模块三个核心功能，同时提供清晰美观的用户界面和完善的数据接口。

**核心设计原则:**
- **模块化架构**: 各模块独立开发，接口清晰
- **桌面软件**: 基于Windows平台的原生应用程序
- **接口导向**: 与外部扫描系统通过标准接口集成
- **用户友好**: 90%以上操作通过图形界面完成

**系统工作流程:**
1. **数据接收**: 通过接口接收思看扫描系统的点云数据和位置信息
2. **轨迹规划**: 基于点云数据生成优化的喷涂路径
3. **Unity仿真验证**: 专业3D可视化仿真，预测质量和检测碰撞
4. **机器人控制**: 生成安川机器人可执行程序并监控执行状态

**Unity 3D集成优势:**
- **专业3D渲染**: 支持PBR材质、实时光照、后处理效果
- **强大物理引擎**: 精确的碰撞检测和物理模拟
- **丰富的可视化**: 粒子系统模拟喷涂效果，LineRenderer显示轨迹
- **成熟的相机系统**: Cinemachine提供电影级相机控制
- **跨平台支持**: 同时支持Windows和Linux
- **插件生态**: 丰富的第三方插件和资源
- **性能优化**: GPU加速渲染，支持大规模场景

**技术栈选择:**
- **开发平台**: 跨平台桌面应用 (Qt 6 C++ + Unity 3D)
- **3D引擎**: Unity 3D (专业3D渲染和物理引擎)
- **UI框架**: Qt Widgets (主界面) + Unity UI (3D视图)
- **数据库**: MySQL + 本地SQLite
- **通信协议**: TCP/IP, MotoPlus TCP, Profinet
- **文件格式**: PLY, STL, OBJ, PCD, JBO/OUT
- **集成方式**: Unity作为Qt应用的嵌入式3D组件

## 架构

系统采用分层模块化架构，分为表示层、业务逻辑层、数据访问层和硬件接口层：

```
┌─────────────────────────────────────────────────────────────────────────────────────────┐
│                                    表示层                                                │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐       │
│  │ 主界面UI    │ │ 轨迹规划UI  │ │ 仿真显示UI  │ │ 机器人控制UI│ │ 数据管理UI  │       │
│  │             │ │             │ │             │ │             │ │             │       │
│  │ 工件导入    │ │ 区域划分    │ │ 3D可视化    │ │ 状态监控    │ │ 参数配置    │       │
│  │ 布局推荐    │ │ 参数设置    │ │ 碰撞检测    │ │ 程序下发    │ │ 历史记录    │       │
│  │ 多工件管理  │ │ 路径优化    │ │ 质量预测    │ │ 异常处理    │ │ 用户权限    │       │
│  └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘       │
├─────────────────────────────────────────────────────────────────────────────────────────┤
│                                  业务逻辑层                                              │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐       │
│  │数据接收模块 │ │轨迹规划模块 │ │仿真计算模块 │ │机器人控制   │ │系统管理模块 │       │
│  │             │ │             │ │             │ │             │ │             │       │
│  │ 点云解析    │ │ 点云处理    │ │ 3D渲染引擎  │ │ 通信管理    │ │ 用户管理    │       │
│  │ 格式转换    │ │ 区域划分    │ │ 碰撞检测    │ │ 程序生成    │ │ 参数管理    │       │
│  │ 数据验证    │ │ 路径生成    │ │ 质量预测    │ │ 状态监控    │ │ 数据备份    │       │
│  │ 多工件管理  │ │ 批量轨迹    │ │ 批量仿真    │ │ 异常处理    │ │ 日志记录    │       │
│  └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘       │
├─────────────────────────────────────────────────────────────────────────────────────────┤
│                                  数据访问层                                              │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐       │
│  │ 点云数据管理│ │ 轨迹数据管理│ │ 仿真数据管理│ │ 配置数据管理│ │ 历史数据管理│       │
│  └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘       │
├─────────────────────────────────────────────────────────────────────────────────────────┤
│                                  硬件接口层                                              │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐       │
│  │ 扫描系统接口│ │ 机器人接口  │ │ PLC通信接口 │ │ 数据库接口  │ │ 文件系统接口│       │
│  └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘       │
└─────────────────────────────────────────────────────────────────────────────────────────┘
```

## 组件和接口

### 数据接收模块（扫描系统接口）

**设计原则**: 与思看扫描系统解耦，通过标准接口接收数据

**核心组件:**
- **点云数据接收器**: 接收PLY、STL、OBJ、PCD等格式的点云文件
- **位置信息解析器**: 解析工件在平板车上的精确位置和姿态
- **数据验证器**: 验证数据完整性和置信度
- **格式转换器**: 统一转换为内部标准格式

**主要接口:**
```csharp
public interface IScanDataReceiver
{
    Task<PointCloudData> ReceivePointCloudAsync(string filePath, PointCloudFormat format);
    Task<WorkpiecePosition> ReceivePositionDataAsync(string positionData);
    Task<BatchScanResult> ReceiveBatchDataAsync(List<ScanDataPackage> batchData);
    bool ValidateDataIntegrity(ScanDataPackage data);
    event EventHandler<ScanDataReceivedEventArgs> DataReceived;
}

public class ScanDataPackage
{
    public string WorkpieceId { get; set; }
    public PointCloudData PointCloud { get; set; }
    public WorkpiecePosition Position { get; set; }
    public double Confidence { get; set; }
    public DateTime ScanTimestamp { get; set; }
}
```

### 轨迹规划模块（核心开发重点）

**设计原则**: 高效算法，支持复杂工件和批量处理

**核心组件:**
- **点云处理引擎**: 去噪、滤波、轻量化处理
- **区域划分引擎**: 手动、自动、混合三种模式
- **路径生成算法**: 基于几何特征和工艺参数
- **批量轨迹规划器**: 多工件优化和碰撞避免

**主要接口:**
```csharp
public interface ITrajectoryPlanner
{
    Task<ProcessedPointCloud> ProcessPointCloudAsync(PointCloudData rawData);
    Task<List<SprayRegion>> DivideRegionsAsync(ProcessedPointCloud cloud, DivisionMode mode);
    Task<Trajectory> GenerateTrajectoryAsync(List<SprayRegion> regions, SprayParameters parameters);
    Task<BatchTrajectory> GenerateBatchTrajectoryAsync(List<WorkpieceData> workpieces);
    Task<RobotProgram> ExportRobotProgramAsync(Trajectory trajectory, RobotType robotType);
}

public class SprayRegion
{
    public string RegionId { get; set; }
    public List<Point3D> BoundaryPoints { get; set; }
    public RegionType Type { get; set; }
    public SprayParameters Parameters { get; set; }
    public double Priority { get; set; }
}
```

### 仿真计算模块（核心开发重点）

**设计原则**: 高性能3D渲染，全方位碰撞检测，精确质量预测

**核心组件:**
- **Unity 3D渲染引擎**: 专业级3D渲染，支持PBR材质和实时光照
- **Unity物理引擎**: 利用Unity内置物理系统进行碰撞检测
- **机械臂避障算法**: 基于Unity Collider系统实现精确碰撞检测
- **轨迹可视化**: Unity LineRenderer和粒子系统展示喷涂效果
- **质量预测模型**: 膜厚分布和覆盖率计算
- **相机控制系统**: Unity Cinemachine实现平滑的多视角切换
- **安全区域管理**: Unity Trigger系统定义安全边界

**主要接口:**
```cpp
class ISimulationEngine
{
public:
    virtual bool InitializeEnvironment(const SimulationEnvironment& environment) = 0;
    virtual SimulationResult SimulateTrajectory(const Trajectory& trajectory, const SprayParameters& parameters) = 0;
    virtual std::vector<CollisionEvent> DetectCollisions(const Trajectory& trajectory) = 0;
    virtual CollisionCheckResult CheckRobotCollisions(const RobotPose& pose, const WorkpieceGeometry& workpiece) = 0;
    virtual SafetyValidationResult ValidateTrajectoryPath(const Trajectory& trajectory) = 0;
    virtual QualityPrediction PredictQuality(const Trajectory& trajectory, const SprayParameters& parameters) = 0;
    virtual BatchSimulationResult SimulateBatch(const BatchTrajectory& batchTrajectory) = 0;
    
    // 信号槽机制
    Q_SIGNALS:
    void ProgressUpdated(int percentage);
    void CollisionDetected(const CollisionEvent& event);
    void SafetyWarning(const QString& message);
};

// 碰撞检测结果
struct CollisionCheckResult
{
    bool HasCollision;
    CollisionType Type;  // 与工件碰撞、自身碰撞、环境碰撞
    QVector3D CollisionPoint;
    QString Description;
    double MinDistance;  // 最小安全距离
};

// 安全验证结果
struct SafetyValidationResult
{
    bool IsSafe;
    std::vector<SafetyIssue> Issues;
    std::vector<QVector3D> RiskPoints;  // 风险点位置
    double SafetyMargin;  // 安全余量
};

public class QualityPrediction
{
    public double CoverageRate { get; set; }
    public FilmThicknessDistribution ThicknessDistribution { get; set; }
    public double Uniformity { get; set; }
    public List<Point3D> UncoveredAreas { get; set; }
    public double EstimatedMaterialUsage { get; set; }
}
```

### 机器人控制模块（核心开发重点）

**设计原则**: 实时通信，状态监控，异常处理

**核心组件:**
- **通信管理器**: MotoPlus TCP协议实现
- **程序生成器**: JBO/OUT格式文件生成
- **状态监控器**: 实时监控机器人状态
- **异常处理器**: 断点续喷和错误恢复

**主要接口:**
```csharp
public interface IRobotController
{
    Task<bool> ConnectAsync(RobotConnectionConfig config);
    Task<bool> UploadProgramAsync(RobotProgram program);
    Task<ExecutionResult> ExecuteTrajectoryAsync(string programName);
    Task<RobotStatus> GetStatusAsync();
    Task<bool> PauseExecutionAsync();
    Task<bool> ResumeFromBreakpointAsync(Breakpoint breakpoint);
    event EventHandler<RobotStatusChangedEventArgs> StatusChanged;
    event EventHandler<RobotErrorEventArgs> ErrorOccurred;
}

public class RobotStatus
{
    public bool IsConnected { get; set; }
    public RobotState State { get; set; }
    public Point3D CurrentPosition { get; set; }
    public double[] JointAngles { get; set; }
    public double Speed { get; set; }
    public List<string> ActiveAlarms { get; set; }
}
```

### 用户界面模块（清晰美观简洁可修改）

**设计原则**: 现代化Qt界面，响应式布局，高度可定制

**核心组件:**
- **Qt主界面框架**: 模块化布局，工具栏和菜单系统
- **Unity 3D视图**: 嵌入式Unity窗口，专业3D渲染和交互
- **参数配置面板**: Qt Widgets实现的直观参数设置界面
- **状态监控面板**: 实时显示系统和设备状态
- **安全监控面板**: 显示碰撞检测和安全状态
- **Qt-Unity通信桥**: 实现Qt界面与Unity 3D场景的数据交互

**UI设计规范:**
```cpp
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void LoadWorkpiece(const WorkpieceData& workpiece);
    void DisplayTrajectory(const Trajectory& trajectory);
    void ShowSimulation(const SimulationResult& result);
    void UpdateRobotStatus(const RobotStatus& status);
    void ShowNotification(NotificationType type, const QString& message);
    void ShowSafetyAlert(const CollisionEvent& event);

private slots:
    void OnCollisionDetected(const CollisionEvent& event);
    void OnSafetyWarning(const QString& message);
    void OnTrajectoryChanged();
    void OnUnitySceneReady();

private:
    UnityWidget* m_unityView;           // 嵌入式Unity窗口
    ParameterPanel* m_parameterPanel;
    StatusPanel* m_statusPanel;
    SafetyPanel* m_safetyPanel;
    QtUnityBridge* m_unityBridge;       // Qt-Unity通信桥
};

// Qt-Unity通信桥
class QtUnityBridge : public QObject
{
    Q_OBJECT

public:
    void SendWorkpieceToUnity(const WorkpieceData& workpiece);
    void SendTrajectoryToUnity(const Trajectory& trajectory);
    void SendRobotPoseToUnity(const RobotPose& pose);
    void StartSimulation();
    void StopSimulation();

public slots:
    void OnUnityCollisionDetected(const QString& collisionData);
    void OnUnitySimulationComplete(const QString& resultData);

signals:
    void CollisionDetected(const CollisionEvent& event);
    void SimulationComplete(const SimulationResult& result);

private:
    QProcess* m_unityProcess;           // Unity进程管理
    QTcpSocket* m_unitySocket;          // TCP通信
};

// UI主题和样式配置
class UIThemeConfig
{
public:
    QString StyleSheet;
    QColor PrimaryColor;
    QColor AccentColor;
    QFont DefaultFont;
    bool EnableAnimations;
    double UIScale;
    
    void ApplyTheme(QApplication* app);
    void SaveTheme(const QString& filePath);
    void LoadTheme(const QString& filePath);
};
```

## 数据模型

### 核心数据结构

```csharp
// 点云数据模型
public class PointCloudData
{
    public string Id { get; set; }
    public List<Point3D> Points { get; set; }
    public List<Vector3D> Normals { get; set; }
    public List<Color> Colors { get; set; }
    public PointCloudMetadata Metadata { get; set; }
    public BoundingBox BoundingBox { get; set; }
}

// 工件数据模型
public class WorkpieceData
{
    public string WorkpieceId { get; set; }
    public WorkpieceCategory Category { get; set; }
    public PointCloudData PointCloud { get; set; }
    public WorkpiecePosition Position { get; set; }
    public Dimensions3D Dimensions { get; set; }
    public CoatingRequirements CoatingRequirements { get; set; }
}

// 轨迹数据模型
public class Trajectory
{
    public string Id { get; set; }
    public string WorkpieceId { get; set; }
    public List<TrajectorySegment> Segments { get; set; }
    public SprayParameters Parameters { get; set; }
    public double EstimatedTime { get; set; }
    public double EstimatedMaterial { get; set; }
    public QualityPrediction QualityPrediction { get; set; }
}

// 喷涂参数模型
public class SprayParameters
{
    public double Pressure { get; set; }        // 喷涂压力 (bar)
    public double FlowRate { get; set; }       // 流量 (ml/min)
    public double SprayHeight { get; set; }    // 喷涂高度 (mm)
    public double SpraySpeed { get; set; }     // 喷涂速度 (mm/s)
    public double FanWidth { get; set; }       // 扇面宽度 (mm)
    public double OverlapRate { get; set; }    // 重叠率 (%)
    public MaterialType MaterialType { get; set; }
    public double TargetThickness { get; set; } // 目标膜厚 (μm)
}

// 机器人程序模型
public class RobotProgram
{
    public string ProgramName { get; set; }
    public RobotType RobotType { get; set; }
    public List<RobotInstruction> Instructions { get; set; }
    public string JBOContent { get; set; }     // JBO格式内容
    public string OUTContent { get; set; }     // OUT格式内容
    public DateTime CreatedAt { get; set; }
}

// 批量轨迹模型
public class BatchTrajectory
{
    public string BatchId { get; set; }
    public List<WorkpieceTrajectory> WorkpieceTrajectories { get; set; }
    public List<SpraySequenceItem> SpraySequence { get; set; }
    public double TotalEstimatedTime { get; set; }
    public double TotalEstimatedMaterial { get; set; }
    public bool CollisionFreeGuarantee { get; set; }
}
```

## 正确性属性

*属性是一个特征或行为，应该在系统的所有有效执行中保持为真——本质上，是关于系统应该做什么的正式声明。属性作为人类可读规范和机器可验证正确性保证之间的桥梁。*

### 属性 1: 点云数据解析一致性
*对于任何*有效的PLY、STL、OBJ或PCD格式文件，解析后重新导出应该产生等价的几何数据
**验证: 需求 11.1**

### 属性 2: 轨迹规划性能保证
*对于任何*输入的工件点云数据，轨迹规划时间应该不超过30分钟且覆盖率应该达到95%以上
**验证: 需求 1A.4**

### 属性 3: 仿真时间性能约束
*对于任何*有效的轨迹数据，仿真计算时间应该不超过30分钟
**验证: 需求 3.1**

### 属性 4: 碰撞检测可靠性
*对于任何*存在碰撞风险的轨迹，系统应该能够检测并暂停仿真
**验证: 需求 3.4**

### 属性 5: 机器人通信实时性
*对于任何*与机器人的数据交换，延迟应该保持在毫秒级别
**验证: 需求 5.1**

### 属性 6: 程序格式正确性
*对于任何*生成的机器人程序，JBO和OUT格式应该符合安川机器人规范
**验证: 需求 1A.5**

### 属性 7: 膜厚参数约束
*对于任何*喷涂参数计算，结果膜厚应该在目标值的80%-150%范围内
**验证: 需求 12.3**

### 属性 8: 批量轨迹无碰撞保证
*对于任何*批量轨迹规划，生成的路径应该确保无碰撞执行
**验证: 需求 10.4**

### 属性 9: 数据存储往返一致性
*对于任何*保存到数据库的轨迹数据，检索后应该与原始数据完全一致
**验证: 需求 13.3**

### 属性 10: 机械臂避障安全性
*对于任何*生成的轨迹路径，机械臂在执行过程中应该与工件、本体、环境保持安全距离
**验证: 需求 3.4, 需求 9.3**

### 属性 11: 用户权限控制有效性
*对于任何*用户角色，只能访问其权限范围内的功能
**验证: 需求 4.5**

## 错误处理

### 错误分类和处理策略

**1. 数据输入错误**
- 文件格式不支持: 显示支持格式列表，提供格式转换建议
- 点云数据损坏: 尝试自动修复，失败则提示重新扫描
- 工件尺寸超限: 警告用户并提供缩放选项

**2. 轨迹规划错误**
- 无法生成有效轨迹: 调整参数建议，提供手动干预选项
- 覆盖率不足: 自动增加路径密度或提示手动补充
- 计算超时: 提供简化算法选项

**3. 仿真计算错误**
- 内存不足: 自动降低模型精度，启用分块渲染
- 碰撞检测失败: 提供路径调整建议
- 渲染异常: 切换到备用渲染引擎

**4. 机器人通信错误**
- 连接失败: 自动重试，检查网络配置
- 程序上传失败: 验证程序格式，提供修复建议
- 执行异常: 记录断点，支持恢复执行

## 测试策略

### 单元测试策略

**测试范围:**
- 点云处理算法的正确性和性能
- 轨迹生成算法的覆盖率和质量
- 仿真引擎的精度和稳定性
- 机器人通信协议的可靠性
- UI组件的响应性和可用性

**测试工具:** NUnit + Moq，覆盖率目标 > 85%

### 属性基础测试策略

**测试框架:** 使用FsCheck进行属性基础测试，每个属性测试运行最少100次迭代

**测试生成器设计:**
- 点云数据生成器: 生成不同复杂度和噪声水平的点云
- 轨迹参数生成器: 生成有效范围内的随机参数组合
- 工件几何生成器: 生成符合实际尺寸约束的工件模型

**属性测试标记格式:**
每个属性测试必须使用以下格式标记：
`**Feature: spray-trajectory-planning, Property {number}: {property_text}**`

### 集成测试策略

**测试场景:**
- 完整的轨迹规划流程测试
- 仿真引擎集成测试
- 机器人通信集成测试
- 数据库集成测试

### 性能测试策略

**关键指标:**
- 轨迹规划时间 < 30分钟
- 仿真计算时间 < 30分钟
- UI响应时间 < 2秒
- 内存使用 < 8GB

**测试工具:** NBomber进行负载测试，PerfView进行性能分析