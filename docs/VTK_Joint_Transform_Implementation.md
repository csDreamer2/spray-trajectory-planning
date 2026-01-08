# VTK 关节变换实现说明

## 核心方法

`VTKWidget::UpdateRobotJoints()` - 关键实现方法

### 功能

- 接收 6 个关节角度（单位：度）
- 使用 DH 参数计算每个关节的变换矩阵
- 累积变换从基座到末端
- 应用变换到 STEP 模型的各个部件
- 实时刷新 VTK 渲染

### 使用方式

```cpp
std::array<double, 6> jointAngles = {45, 30, -45, 0, 0, 0};
m_vtkWidget->UpdateRobotJoints(jointAngles);
```

## DH 参数配置

当前使用占位符参数，需要根据 MPX3500 规格更新：

```cpp
const DHParam dhParams[6] = {
    {a, alpha, d, theta_offset},  // J1
    {a, alpha, d, theta_offset},  // J2
    {a, alpha, d, theta_offset},  // J3
    {a, alpha, d, theta_offset},  // J4
    {a, alpha, d, theta_offset},  // J5
    {a, alpha, d, theta_offset}   // J6
};
```

## 部件映射

STEP 模型部件与关节的对应关系：

| 关节 | 部件名称 |
|------|---------|
| J1   | NAUO1   |
| J2   | NAUO2   |
| J3   | NAUO3   |
| J4   | NAUO4   |
| J5   | NAUO5   |
| J6   | NAUO6   |

## 后续工作

1. 获取 MPX3500 的实际 DH 参数
2. 更新 `dhParams` 数组中的参数值
3. 验证变换是否正确
4. 实现控制面板（RobotControlPanel）
5. 添加逆运动学计算
