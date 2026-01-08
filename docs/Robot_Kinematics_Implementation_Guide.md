# 机器人正向运动学实现指南

## 概述

本指南说明如何为Aubo i5H机器人实现正向运动学，包括DH参数的获取、参数配置、以及与STEP模型的集成。

## 第一阶段：获取官方DH参数

### 1.1 获取Aubo i5H的DH参数

目前已创建的框架使用标准的6轴协作机器人参数。要获得准确的Aubo i5H参数，请：

#### 方法1: 官方文档
- 查阅 Aubo i5 用户手册 (AUBO i5 USER MANUAL V4.3.1 或更新版本)
- 联系 Aubo 技术支持: support@aubo-robotics.cn
- 访问 Aubo 官方网站: http://www.aubo-robotics.cn/

#### 方法2: ROS包
- 查看 Aubo 官方 ROS 包: https://github.com/AuboRobot/aubo_robot
- 查找 URDF 文件中的关节定义
- 从 URDF 中提取 DH 参数

#### 方法3: 学术资源
- 搜索使用 Aubo i5 的研究论文
- 查看 GitHub 上的相关项目 (如 LARA_AUBOi5_AG95)
- 参考 ROS 社区的讨论

### 1.2 DH参数格式

获得的参数应该包含以下信息（对于每个关节 i = 1 to 6）：

```
J1: a=?, d=?, alpha=?, theta_offset=?, type=R, part=NAUO1
J2: a=?, d=?, alpha=?, theta_offset=?, type=R, part=NAUO2
J3: a=?, d=?, alpha=?, theta_offset=?, type=R, part=NAUO3
J4: a=?, d=?, alpha=?, theta_offset=?, type=R, part=NAUO4
J5: a=?, d=?, alpha=?, theta_offset=?, type=R, part=NAUO5
J6: a=?, d=?, alpha=?, theta_offset=?, type=R, part=NAUO6
```

其中：
- **a**: 连杆长度 (mm)
- **d**: 连杆偏移 (mm)
- **alpha**: 连杆扭角 (度数，需要转换为弧度)
- **theta_offset**: 关节零位偏移 (度数，需要转换为弧度)
- **type**: 关节类型 (R=旋转, P=平移)
- **part**: STEP模型中对应的部件名称

## 第二阶段：更新DH参数

### 2.1 修改DHParameters.cpp

当获得官方参数后，更新 `initializeAuboI5HParameters()` 函数：

```cpp
void AuboI5HDHParameters::initializeAuboI5HParameters()
{
    m_parameters.clear();
    
    // 用实际参数替换以下内容
    m_parameters.append(DHParameter(
        "J1", "NAUO1",
        0,              // a (实际值)
        330,            // d (实际值)
        0,              // alpha (实际值，转换为弧度)
        0,              // theta_offset (实际值，转换为弧度)
        -M_PI,          // min_angle
        M_PI,           // max_angle
        M_PI,           // max_velocity
        'R'
    ));
    
    // ... 对其他关节重复 ...
}
```

### 2.2 度数到弧度的转换

```cpp
// 度数转弧度
double alpha_rad = alpha_deg * M_PI / 180.0;

// 例如: 90度 = π/2 弧度
double alpha_90 = 90.0 * M_PI / 180.0;  // = M_PI / 2
```

## 第三阶段：测试DH参数

### 3.1 创建测试程序

```cpp
#include "DHParameters.h"
#include <QDebug>

void testDHParameters() {
    AuboI5HDHParameters dh_params;
    
    // 测试1: 验证参数加载
    qDebug() << "关节数量:" << dh_params.getJointCount();
    for (int i = 0; i < dh_params.getJointCount(); ++i) {
        const DHParameter& param = dh_params.getParameter(i);
        qDebug() << "J" << (i+1) << ":" << param.partName 
                 << "a=" << param.a << "d=" << param.d;
    }
    
    // 测试2: 计算零位正向运动学
    QVector<double> zero_angles;
    for (int i = 0; i < 6; ++i) {
        zero_angles.append(0.0);
    }
    
    Eigen::Matrix4d T_zero = dh_params.computeForwardKinematics(zero_angles);
    qDebug() << "零位末端执行器位置:";
    Eigen::Vector3d pos = MatrixUtils::extractPosition(T_zero);
    qDebug() << "X=" << pos(0) << "Y=" << pos(1) << "Z=" << pos(2);
    
    // 测试3: 验证工作半径
    // 对于Aubo i5H，工作半径应该约为886.5mm
    double reach = pos.norm();
    qDebug() << "计算的工作半径:" << reach << "mm";
    qDebug() << "预期工作半径: 886.5 mm";
    
    // 测试4: 计算单个关节变换
    for (int i = 0; i < 6; ++i) {
        Eigen::Matrix4d T_i = dh_params.computeTransformToJoint(i, zero_angles);
        Eigen::Vector3d pos_i = MatrixUtils::extractPosition(T_i);
        qDebug() << "J" << (i+1) << "位置:" << pos_i(0) << pos_i(1) << pos_i(2);
    }
}
```

### 3.2 验证结果

运行测试程序并检查：

1. **工作半径**: 计算的末端执行器位置到基座的距离应该约为886.5mm
2. **关节位置**: 每个关节的位置应该逐步增加
3. **参数一致性**: 所有参数应该正确加载

## 第四阶段：与STEP模型集成

### 4.1 创建关节变换管理器

```cpp
#include "DHParameters.h"
#include "../../../UI/ModelTree/STEPModelTreeWidget.h"

class RobotJointTransformManager {
public:
    RobotJointTransformManager(STEPModelTreeWidget* model_tree)
        : m_model_tree(model_tree), m_dh_params() {}
    
    /**
     * @brief 更新所有关节的变换
     * @param joint_angles 关节角度 (弧度)
     */
    void updateJointTransforms(const QVector<double>& joint_angles) {
        if (joint_angles.size() < 6) {
            qWarning() << "关节角度数量不足";
            return;
        }
        
        // 对每个关节应用变换
        for (int i = 0; i < 6; ++i) {
            // 计算该关节的变换矩阵
            Eigen::Matrix4d T = m_dh_params.computeTransformToJoint(i, joint_angles);
            
            // 转换为VTK变换
            vtkSmartPointer<vtkTransform> vtk_transform = 
                eigenMatrixToVTKTransform(T);
            
            // 获取部件名称
            const DHParameter& param = m_dh_params.getParameter(i);
            
            // 应用变换到STEP模型部件
            m_model_tree->applyTransformToActor(param.partName, vtk_transform);
        }
    }
    
    /**
     * @brief 获取末端执行器的位置和姿态
     */
    void getEndEffectorPose(const QVector<double>& joint_angles,
                           Eigen::Vector3d& position,
                           Eigen::Vector3d& euler_angles) {
        Eigen::Matrix4d T_ee = m_dh_params.computeForwardKinematics(joint_angles);
        position = MatrixUtils::extractPosition(T_ee);
        euler_angles = MatrixUtils::extractEulerAngles(T_ee);
    }

private:
    STEPModelTreeWidget* m_model_tree;
    AuboI5HDHParameters m_dh_params;
    
    /**
     * @brief 将Eigen矩阵转换为VTK变换
     */
    vtkSmartPointer<vtkTransform> eigenMatrixToVTKTransform(
        const Eigen::Matrix4d& matrix) {
        vtkSmartPointer<vtkTransform> transform = 
            vtkSmartPointer<vtkTransform>::New();
        
        vtkMatrix4x4* vtk_matrix = vtkMatrix4x4::New();
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                vtk_matrix->SetElement(row, col, matrix(row, col));
            }
        }
        
        transform->SetMatrix(vtk_matrix);
        vtk_matrix->Delete();
        
        return transform;
    }
};
```

### 4.2 在MainWindow中使用

```cpp
#include "Robot/Kinematics/DHParameters.h"

class MainWindow : public QMainWindow {
    // ...
private:
    RobotJointTransformManager* m_joint_transform_manager;
    
    void setupRobotKinematics() {
        // 创建关节变换管理器
        m_joint_transform_manager = new RobotJointTransformManager(m_modelTreePanel);
    }
    
    void onJointAngleChanged(int joint_index, double angle) {
        // 获取当前所有关节角度
        QVector<double> joint_angles = getCurrentJointAngles();
        
        // 更新指定关节的角度
        joint_angles[joint_index] = angle;
        
        // 更新所有关节的变换
        m_joint_transform_manager->updateJointTransforms(joint_angles);
        
        // 刷新VTK渲染
        m_vtkView->RefreshRender();
    }
};
```

## 第五阶段：实现关节控制UI

### 5.1 创建关节滑块控制

```cpp
class RobotJointControlPanel : public QWidget {
    Q_OBJECT
    
public:
    RobotJointControlPanel(QWidget* parent = nullptr)
        : QWidget(parent) {
        setupUI();
    }
    
private slots:
    void onJointSliderChanged(int value) {
        // 获取发送者的滑块
        QSlider* slider = qobject_cast<QSlider*>(sender());
        if (!slider) return;
        
        // 获取关节索引
        int joint_index = m_sliders.indexOf(slider);
        
        // 转换为弧度
        double angle_rad = (value / 100.0) * M_PI;
        
        // 发送信号
        emit jointAngleChanged(joint_index, angle_rad);
    }
    
signals:
    void jointAngleChanged(int joint_index, double angle_rad);
    
private:
    QVector<QSlider*> m_sliders;
    
    void setupUI() {
        QVBoxLayout* layout = new QVBoxLayout(this);
        
        // 为每个关节创建滑块
        for (int i = 0; i < 6; ++i) {
            QHBoxLayout* h_layout = new QHBoxLayout();
            
            // 标签
            QLabel* label = new QLabel(QString("J%1").arg(i+1));
            h_layout->addWidget(label);
            
            // 滑块
            QSlider* slider = new QSlider(Qt::Horizontal);
            slider->setRange(-180, 180);
            slider->setValue(0);
            connect(slider, QOverload<int>::of(&QSlider::valueChanged),
                    this, &RobotJointControlPanel::onJointSliderChanged);
            m_sliders.append(slider);
            h_layout->addWidget(slider);
            
            // 数值显示
            QLabel* value_label = new QLabel("0°");
            h_layout->addWidget(value_label);
            
            layout->addLayout(h_layout);
        }
    }
};
```

## 第六阶段：测试和验证

### 6.1 测试清单

- [ ] DH参数正确加载
- [ ] 零位正向运动学计算正确
- [ ] 工作半径与规格相符
- [ ] 关节变换正确应用到STEP模型
- [ ] 关节滑块控制正常工作
- [ ] 末端执行器位置实时更新
- [ ] 关节限制正确应用

### 6.2 调试技巧

```cpp
// 打印DH参数
for (int i = 0; i < dh_params.getJointCount(); ++i) {
    const DHParameter& param = dh_params.getParameter(i);
    qDebug() << "J" << (i+1) << ":" 
             << "a=" << param.a 
             << "d=" << param.d 
             << "alpha=" << param.alpha 
             << "theta_offset=" << param.theta_offset;
}

// 打印变换矩阵
Eigen::Matrix4d T = dh_params.computeForwardKinematics(angles);
MatrixUtils::printMatrix(T, "Forward Kinematics");

// 打印末端执行器位置
Eigen::Vector3d pos = MatrixUtils::extractPosition(T);
qDebug() << "EE Position: X=" << pos(0) << "Y=" << pos(1) << "Z=" << pos(2);
```

## 常见问题

### Q1: 如何获得准确的DH参数？
A: 最好的方法是查阅官方文档或联系Aubo技术支持。也可以从ROS包或学术论文中获取。

### Q2: 为什么计算的工作半径与规格不符？
A: 可能是DH参数不准确。请检查参数值，特别是连杆长度(a)和连杆偏移(d)。

### Q3: 如何处理关节限制？
A: 使用 `clampJointAngle()` 方法自动限制关节角度在有效范围内。

### Q4: 如何实现逆向运动学？
A: 逆向运动学比较复杂，可以使用数值方法或解析方法。建议参考机器人学教科书。

## 参考资源

- [DH参数维基百科](https://en.wikipedia.org/wiki/Denavit%E2%80%93Hartenberg_parameters)
- [机器人学导论 - Craig](https://www.pearson.com/en-us/subject-catalog/p/introduction-to-robotics-mechanics-and-control/P200000003282)
- [Aubo官方文档](http://www.aubo-robotics.cn/)
- [Eigen文档](https://eigen.tuxfamily.org/)

## 下一步

1. 获取官方Aubo i5H DH参数
2. 更新DHParameters.cpp中的参数
3. 编译并测试
4. 实现关节控制UI
5. 集成到主应用程序

## 支持

如有问题，请：
1. 查看本指南的相关部分
2. 查看代码注释和文档
3. 参考参考资源
4. 联系Aubo技术支持
