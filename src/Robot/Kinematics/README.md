# 机器人正向运动学模块 (Robot Kinematics)

## 概述

这个模块实现了Aubo i5H机器人的DH参数和正向运动学计算。它提供了以下功能：

- **DH参数管理** - 存储和管理每个关节的DH参数
- **变换矩阵计算** - 根据DH参数计算关节变换矩阵
- **正向运动学** - 计算末端执行器的位置和姿态
- **关节变换应用** - 对STEP模型中的每个部件应用变换

## 文件结构

```
src/Robot/Kinematics/
├── DHParameters.h          # DH参数类定义
├── DHParameters.cpp        # DH参数类实现
├── CMakeLists.txt          # CMake构建配置
└── README.md               # 本文件
```

## 主要类

### AuboI5HDHParameters

管理Aubo i5H机器人的DH参数和运动学计算。

#### 主要方法

```cpp
// 获取DH参数
const DHParameter& getParameter(int jointIndex) const;
int getJointIndexByPartName(const QString& partName) const;

// 计算变换矩阵
Eigen::Matrix4d computeDHTransform(int jointIndex, double theta) const;
Eigen::Matrix4d computeForwardKinematics(const QVector<double>& jointAngles) const;
Eigen::Matrix4d computeTransformToJoint(int jointIndex, const QVector<double>& jointAngles) const;

// 关节角度验证
bool isJointAngleValid(int jointIndex, double theta) const;
double clampJointAngle(int jointIndex, double theta) const;
```

## 使用示例

### 1. 初始化DH参数

```cpp
#include "DHParameters.h"

// 创建DH参数对象
AuboI5HDHParameters dh_params;

// 获取关节数量
int num_joints = dh_params.getJointCount();
qDebug() << "关节数量:" << num_joints;
```

### 2. 计算正向运动学

```cpp
// 定义关节角度 (弧度)
QVector<double> joint_angles;
joint_angles << 0.0, -M_PI/4, M_PI/2, 0.0, M_PI/4, 0.0;

// 计算末端执行器的变换矩阵
Eigen::Matrix4d T_ee = dh_params.computeForwardKinematics(joint_angles);

// 提取位置 (mm)
Eigen::Vector3d position = MatrixUtils::extractPosition(T_ee);
qDebug() << "末端执行器位置:" << position(0) << position(1) << position(2);

// 提取欧拉角 (弧度)
Eigen::Vector3d euler = MatrixUtils::extractEulerAngles(T_ee);
qDebug() << "末端执行器姿态:" << euler(0) << euler(1) << euler(2);
```

### 3. 计算单个关节的变换

```cpp
// 计算从基座到J3关节的变换矩阵
Eigen::Matrix4d T_j3 = dh_params.computeTransformToJoint(2, joint_angles);

// 打印矩阵用于调试
MatrixUtils::printMatrix(T_j3, "T_0^3");
```

### 4. 验证关节角度

```cpp
// 检查关节角度是否有效
double theta = M_PI / 2;
if (dh_params.isJointAngleValid(0, theta)) {
    qDebug() << "关节角度有效";
} else {
    qDebug() << "关节角度超出范围";
}

// 限制关节角度在有效范围内
double clamped_theta = dh_params.clampJointAngle(0, theta);
```

## DH参数更新

当获得官方的Aubo i5H DH参数后，需要更新 `initializeAuboI5HParameters()` 函数中的参数值。

### 参数格式

```cpp
m_parameters.append(DHParameter(
    "J1",           // 关节名称
    "NAUO1",        // STEP模型部件名称
    0,              // a (连杆长度, mm)
    330,            // d (连杆偏移, mm)
    0,              // alpha (连杆扭角, 弧度)
    0,              // theta_offset (关节零位偏移, 弧度)
    -M_PI,          // min_angle (最小关节角, 弧度)
    M_PI,           // max_angle (最大关节角, 弧度)
    M_PI,           // max_velocity (最大角速度, 弧度/秒)
    'R'             // joint_type ('R' 旋转 或 'P' 平移)
));
```

## 与STEP模型集成

### 应用关节变换到STEP模型部件

```cpp
#include "DHParameters.h"
#include "../../../UI/ModelTree/STEPModelTreeWidget.h"

// 假设有STEPModelTreeWidget实例
STEPModelTreeWidget* model_tree = ...;

// 获取DH参数
AuboI5HDHParameters dh_params;

// 定义关节角度
QVector<double> joint_angles;
joint_angles << 0.0, -M_PI/4, M_PI/2, 0.0, M_PI/4, 0.0;

// 对每个关节应用变换
for (int i = 0; i < 6; ++i) {
    // 计算该关节的变换矩阵
    Eigen::Matrix4d T = dh_params.computeTransformToJoint(i, joint_angles);
    
    // 转换为VTK变换
    vtkSmartPointer<vtkTransform> vtk_transform = vtkSmartPointer<vtkTransform>::New();
    
    // 设置矩阵 (注意: VTK使用行主序，Eigen使用列主序)
    vtkMatrix4x4* matrix = vtkMatrix4x4::New();
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            matrix->SetElement(row, col, T(row, col));
        }
    }
    vtk_transform->SetMatrix(matrix);
    matrix->Delete();
    
    // 获取部件名称
    const DHParameter& param = dh_params.getParameter(i);
    
    // 应用变换到STEP模型部件
    model_tree->applyTransformToActor(param.partName, vtk_transform);
}
```

## 坐标系约定

### 基座坐标系 (Frame 0)
- 原点: 机器人基座中心
- Z轴: 向上
- X轴: 向前
- Y轴: 向左 (右手坐标系)

### 关节坐标系
- 每个关节都有自己的坐标系，按照DH约定附加到相应的连杆上
- Z轴: 沿着关节旋转轴
- X轴: 沿着连杆长度方向

## 数学基础

### DH变换矩阵

从坐标系 i-1 到坐标系 i 的齐次变换矩阵为：

```
T_{i-1}^i = Rot_Z(θ_i) * Trans_Z(d_i) * Trans_X(a_i) * Rot_X(α_i)
```

其中：
- θ_i: 关节角度 (绕Z_{i-1}轴旋转)
- d_i: 连杆偏移 (沿Z_{i-1}轴平移)
- a_i: 连杆长度 (沿X_i轴平移)
- α_i: 连杆扭角 (绕X_i轴旋转)

### 正向运动学

末端执行器相对于基座的变换矩阵为：

```
T_0^6 = T_0^1 * T_1^2 * T_2^3 * T_3^4 * T_4^5 * T_5^6
```

## 依赖项

- **Qt6**: 用于字符串和容器
- **Eigen3**: 用于矩阵计算

## 编译

该模块已集成到主项目的CMake构建系统中。只需在主项目目录中运行：

```bash
cmake .. -G "Visual Studio 17 2022" -A x64 -DQt6_DIR="..." -DEigen3_DIR="..."
cmake --build . --config Debug
```

## 测试

可以创建一个简单的测试程序来验证DH参数和运动学计算：

```cpp
#include "DHParameters.h"
#include <QDebug>

int main() {
    AuboI5HDHParameters dh_params;
    
    // 测试1: 获取参数
    qDebug() << "关节数量:" << dh_params.getJointCount();
    
    // 测试2: 计算正向运动学
    QVector<double> angles;
    angles << 0, 0, 0, 0, 0, 0;
    Eigen::Matrix4d T = dh_params.computeForwardKinematics(angles);
    MatrixUtils::printMatrix(T, "T_0^6 (零位)");
    
    // 测试3: 验证关节角度
    qDebug() << "J1有效范围:" << dh_params.getParameter(0).min_angle 
             << "到" << dh_params.getParameter(0).max_angle;
    
    return 0;
}
```

## 参考资源

- [DH参数维基百科](https://en.wikipedia.org/wiki/Denavit%E2%80%93Hartenberg_parameters)
- [Aubo官方网站](http://www.aubo-robotics.cn/)
- [Aubo ROS包](https://github.com/AuboRobot/aubo_robot)
- [Eigen文档](https://eigen.tuxfamily.org/)

## 许可证

本模块遵循项目的许可证。

## 作者

Kiro AI Assistant

## 更新日志

### v1.0.0 (2026-01-07)
- 初始实现
- 支持DH参数管理
- 实现正向运动学计算
- 提供矩阵工具函数
