# src目录快速参考指南

## 1. 文件位置速查表

### 需要修改UI？
```
src/UI/MainWindow.cpp          - 主窗口布局和菜单
src/UI/Panels/                 - 各种功能面板
src/UI/Visualization/          - 3D显示相关
```

### 需要修改机器人控制？
```
src/Robot/RobotController.cpp  - 机器人主控制逻辑
src/Robot/RobotKinematics.cpp  - 运动学计算
src/Robot/MotoTcpClient.cpp    - 与机器人通信
```

### 需要处理STEP文件？
```
src/Data/STEPModelTree.cpp     - STEP文件解析
src/UI/STEPModelTreeWidget.cpp - STEP树形显示
src/UI/VTKWidget.cpp           - 3D显示
```

### 需要处理数据库？
```
src/Data/DatabaseManager.cpp   - 数据库操作
src/Data/Models/               - 数据模型
```

### 需要处理点云？
```
src/Data/PointCloud/           - 点云处理
src/UI/PointCloudLoader.cpp    - 点云加载UI
```

---

## 2. 常见任务速查

### 任务: 添加新的UI面板

**步骤**:
1. 在 `src/UI/Panels/` 创建 `NewPanel.h/cpp`
2. 继承 `QWidget`
3. 在 `MainWindow.cpp` 中创建实例
4. 添加到停靠窗口
5. 连接信号/槽

**示例**:
```cpp
// NewPanel.h
class NewPanel : public QWidget {
    Q_OBJECT
public:
    explicit NewPanel(QWidget *parent = nullptr);
signals:
    void parameterChanged(const QString& value);
};

// MainWindow.cpp
m_newPanel = new UI::NewPanel(this);
m_newDock = new QDockWidget("新面板", this);
m_newDock->setWidget(m_newPanel);
addDockWidget(Qt::RightDockWidgetArea, m_newDock);
```

### 任务: 添加新的机器人功能

**步骤**:
1. 在 `RobotController` 中添加公共方法
2. 在 `RobotControlPanel` 中添加UI控件
3. 连接信号/槽
4. 在 `MotoTcpClient` 中添加通信协议

**示例**:
```cpp
// RobotController.h
void moveToNamedPosition(const QString& positionName);

// RobotController.cpp
void RobotController::moveToNamedPosition(const QString& positionName) {
    // 查找预设位置
    // 调用 moveToJointAngles()
}

// RobotControlPanel.cpp
connect(m_namedPosButton, &QPushButton::clicked, 
        m_controller, &RobotController::moveToNamedPosition);
```

### 任务: 添加新的数据模型

**步骤**:
1. 在 `src/Data/Models/` 创建 `NewData.h/cpp`
2. 继承 `BaseModel`
3. 实现序列化/反序列化
4. 在 `DatabaseManager` 中添加CRUD操作

**示例**:
```cpp
// NewData.h
class NewData : public BaseModel {
public:
    QString name;
    double value;
    
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& json) override;
};

// DatabaseManager.cpp
bool DatabaseManager::saveNewData(const NewData& data) {
    QSqlQuery query;
    query.prepare("INSERT INTO new_data (name, value) VALUES (?, ?)");
    query.addBindValue(data.name);
    query.addBindValue(data.value);
    return query.exec();
}
```

### 任务: 修改STEP文件加载

**步骤**:
1. 修改 `STEPModelTree::loadFromSTEPFile()`
2. 修改 `STEPModelTreeWorker::run()`
3. 测试加载进度信号
4. 更新 `STEPModelTreeWidget` 显示

**关键代码**:
```cpp
// STEPModelTree.cpp
bool STEPModelTree::loadFromSTEPFile(const QString& filePath) {
    // 1. 打开STEP文件
    STEPCAFControl_Reader reader;
    if (reader.ReadFile(filePath.toStdString().c_str()) != IFSelect_RetDone) {
        return false;
    }
    
    // 2. 获取文档
    reader.Transfer(m_stepDocument);
    
    // 3. 构建模型树
    parseSTEPLabel(m_stepDocument->Main(), m_rootNode);
    
    // 4. 发送信号
    emit modelTreeLoaded(true, "加载成功");
    return true;
}
```

### 任务: 添加新的仿真功能

**步骤**:
1. 在 `Simulation` 模块中实现功能
2. 在 `SimulationEngine` 中集成
3. 在 `MainWindow` 中添加UI触发
4. 显示仿真结果

**示例**:
```cpp
// SimulationEngine.cpp
bool SimulationEngine::simulate(const TrajectoryData& trajectory) {
    // 1. 初始化仿真环境
    // 2. 逐步执行轨迹
    for (const auto& point : trajectory.points) {
        // 3. 碰撞检测
        if (m_collisionDetector->checkCollision(point)) {
            emit collisionDetected(point);
            return false;
        }
        // 4. 质量预测
        double quality = m_qualityPredictor->predict(point);
        emit qualityUpdated(quality);
    }
    return true;
}
```

---

## 3. 关键类和方法

### Core模块
```cpp
Core::Application::instance()           // 获取单例
Core::Application::initialize()         // 初始化应用
Core::ConfigManager::readConfig()       // 读取配置
Core::Logger::log()                     // 记录日志
```

### Data模块
```cpp
STEPModelTree::loadFromSTEPFile()       // 加载STEP文件
STEPModelTree::getVisibleShapes()       // 获取可见形状
DatabaseManager::instance()             // 获取数据库单例
PointCloudParser::parseFile()           // 解析点云文件
```

### Robot模块
```cpp
RobotController::connectToRobot()       // 连接机器人
RobotController::setJointAngles()       // 设置关节角度
RobotKinematics::forwardKinematics()    // 正运动学
RobotKinematics::inverseKinematics()    // 逆运动学
```

### UI模块
```cpp
MainWindow::LoadWorkpiece()             // 加载工件
VTKWidget::addActor()                   // 添加3D对象
STEPModelTreeWidget::loadSTEPFile()     // 加载STEP树
```

---

## 4. 编译和构建

### 编译单个模块
```bash
# 编译Core模块
cmake --build . --target Core

# 编译Data模块
cmake --build . --target Data

# 编译Robot模块
cmake --build . --target Robot

# 编译UI模块
cmake --build . --target UI
```

### 完整编译
```bash
# 在build目录中
cmake ..
cmake --build . --config Release
```

### 清理构建
```bash
# 删除build目录
rm -rf build
mkdir build
cd build
cmake ..
cmake --build .
```

---

## 5. 调试技巧

### 查看日志
```cpp
// 在代码中添加日志
Core::Logger::instance()->log("调试信息");

// 查看日志文件
// 位置: logs/application.log
```

### 查看配置
```cpp
// 读取配置值
QString value = Core::ConfigManager::instance()->getValue("key");

// 配置文件位置
// 位置: config/paths.cmake
```

### 查看数据库
```cpp
// 连接到SQLite数据库
// 位置: data/cache/application.db

// 使用SQLite浏览器查看
// 工具: DB Browser for SQLite
```

### 调试STEP加载
```cpp
// 在STEPModelTree.cpp中添加调试输出
qDebug() << "加载STEP文件:" << filePath;
qDebug() << "总标签数:" << m_totalLabels;
qDebug() << "已处理:" << m_processedLabels;

// 查看加载进度
// 信号: STEPModelTree::loadProgress(int progress, QString message)
```

### 调试机器人通信
```cpp
// 在MotoTcpClient.cpp中添加调试
qDebug() << "发送命令:" << command;
qDebug() << "接收数据:" << data;

// 查看TCP连接状态
// 信号: MotoTcpClient::connected(), disconnected(), error()
```

---

## 6. 常见错误和解决方案

### 错误: STEP文件加载失败
**原因**: OpenCASCADE库未正确链接
**解决**:
1. 检查 `CMakeLists.txt` 中的 OpenCASCADE 配置
2. 检查 `config/paths.cmake` 中的路径
3. 重新编译 Data 模块

### 错误: 机器人连接失败
**原因**: TCP连接参数错误或机器人离线
**解决**:
1. 检查机器人IP和端口
2. 检查网络连接
3. 查看 `MotoTcpClient` 的错误信号

### 错误: VTK显示为黑屏
**原因**: VTK库未正确初始化或没有添加Actor
**解决**:
1. 检查 `VTKWidget::initialize()` 是否调用
2. 检查是否添加了Actor
3. 检查相机设置

### 错误: 数据库连接失败
**原因**: MySQL服务未启动或SQLite文件损坏
**解决**:
1. 启动MySQL服务
2. 删除 `data/cache/application.db` 重新创建
3. 检查数据库连接参数

---

## 7. 性能优化建议

### STEP文件加载优化
```cpp
// 使用异步加载
STEPModelTreeWorker* worker = new STEPModelTreeWorker(filePath);
QThread* thread = new QThread();
worker->moveToThread(thread);
connect(thread, &QThread::started, worker, &STEPModelTreeWorker::run);
thread->start();
```

### 3D显示优化
```cpp
// 使用LOD (Level of Detail)
// 远处模型使用低精度，近处使用高精度

// 使用视锥体剔除
// 只渲染可见的对象

// 使用显示列表缓存
// 减少重复计算
```

### 数据库查询优化
```cpp
// 使用索引
CREATE INDEX idx_workpiece_id ON workpiece(id);

// 使用批量操作
// 减少数据库往返次数

// 使用连接池
// 复用数据库连接
```

### 机器人通信优化
```cpp
// 使用缓冲区减少网络往返
// 批量发送命令

// 使用异步通信
// 不阻塞UI线程

// 使用心跳检测
// 及时发现连接断开
```

---

## 8. 代码规范

### 命名规范
```cpp
// 类名: PascalCase
class RobotController { };

// 方法名: camelCase
void setJointAngle(int index, double angle);

// 成员变量: m_camelCase
double m_jointAngle;

// 常量: UPPER_CASE
const int MAX_JOINTS = 6;

// 枚举: PascalCase
enum class ConnectionState { Connected, Disconnected };
```

### 注释规范
```cpp
/**
 * @brief 简短描述
 * @param param1 参数1说明
 * @return 返回值说明
 */
void functionName(int param1);

// 单行注释用于解释复杂逻辑
// 多行注释用于函数文档
```

### 文件组织
```cpp
// 头文件顺序
#include <system_headers>      // 系统头文件
#include <qt_headers>          // Qt头文件
#include <external_headers>    // 外部库头文件
#include "local_headers"       // 本地头文件

// 类成员顺序
public:
    // 构造/析构
    // 公共方法
    // 信号
private slots:
    // 私有槽
private:
    // 私有方法
    // 成员变量
```

---

## 9. 测试指南

### 单元测试
```cpp
// 在tests/目录中创建测试
#include <gtest/gtest.h>
#include "RobotKinematics.h"

TEST(RobotKinematicsTest, ForwardKinematics) {
    RobotKinematics kinematics;
    std::array<double, 6> angles = {0, 0, 0, 0, 0, 0};
    auto pose = kinematics.forwardKinematics();
    EXPECT_NEAR(pose.position.x(), 0, 0.01);
}
```

### 集成测试
```cpp
// 测试模块间的交互
TEST(IntegrationTest, STEPLoadingAndDisplay) {
    // 1. 加载STEP文件
    // 2. 验证模型树
    // 3. 验证VTK显示
    // 4. 验证交互
}
```

### 性能测试
```cpp
// 测试加载时间
QElapsedTimer timer;
timer.start();
stepModelTree->loadFromSTEPFile(filePath);
qDebug() << "加载耗时:" << timer.elapsed() << "ms";
```

---

## 10. 快速命令参考

### Git操作
```bash
# 查看修改的文件
git status

# 查看具体修改
git diff src/UI/MainWindow.cpp

# 提交修改
git add src/
git commit -m "修改说明"

# 查看日志
git log --oneline -10
```

### CMake操作
```bash
# 生成构建文件
cmake -B build -S .

# 构建项目
cmake --build build --config Release

# 清理构建
cmake --build build --target clean
```

### 调试操作
```bash
# 使用GDB调试
gdb ./build/spray-trajectory-planning

# 使用Qt Creator调试
# 直接在IDE中设置断点和调试
```

---

## 11. 资源链接

### 文档
- [src目录详细分析](src-directory-analysis.md)
- [模块依赖关系图](module-dependency-diagram.md)
- [开发环境部署指南](开发环境部署指南.md)

### 外部资源
- [Qt6文档](https://doc.qt.io/qt-6/)
- [VTK文档](https://vtk.org/doc/nightly/html/)
- [OpenCASCADE文档](https://dev.opencascade.org/doc/overview/html/)
- [PCL文档](https://pointclouds.org/documentation/)

---

## 12. 常见问题解答

### Q: 如何添加新的依赖库？
A: 
1. 在 `CMakeLists.txt` 中添加 `find_package()`
2. 在 `target_link_libraries()` 中添加库
3. 在 `target_include_directories()` 中添加头文件路径
4. 重新运行 `cmake`

### Q: 如何调试信号/槽连接？
A:
```cpp
// 启用信号/槽调试
QObject::connect(sender, &Sender::signal, receiver, &Receiver::slot,
                 Qt::AutoConnection);
// 如果连接失败，会输出警告信息
```

### Q: 如何处理线程安全？
A:
```cpp
// 使用QMutex保护共享数据
QMutex m_mutex;
m_mutex.lock();
// 访问共享数据
m_mutex.unlock();

// 或使用QMutexLocker自动解锁
{
    QMutexLocker locker(&m_mutex);
    // 访问共享数据
}
```

### Q: 如何优化内存使用？
A:
1. 使用智能指针 (`std::unique_ptr`, `std::shared_ptr`)
2. 及时释放不需要的对象
3. 使用对象池复用对象
4. 监控内存使用情况

### Q: 如何处理异常？
A:
```cpp
try {
    // 可能抛出异常的代码
    stepModelTree->loadFromSTEPFile(filePath);
} catch (const std::exception& e) {
    qWarning() << "异常:" << e.what();
}
```

