#include "DHParameters.h"
#include <QDebug>
#include <cmath>

AuboI5HDHParameters::AuboI5HDHParameters()
{
    initializeAuboI5HParameters();
}

void AuboI5HDHParameters::initializeAuboI5HParameters()
{
    // 清空之前的参数
    m_parameters.clear();
    
    // 初始化Aubo i5H的DH参数
    // 注意: 这些是基于标准6轴协作机器人的典型参数
    // 实际参数应该从官方文档获取
    
    // J1 - 基座旋转关节
    m_parameters.append(DHParameter(
        "J1", "NAUO1",
        0,      // a (mm)
        330,    // d (mm)
        0,      // alpha (rad)
        0,      // theta_offset (rad)
        -M_PI,  // min_angle (rad)
        M_PI,   // max_angle (rad)
        M_PI,   // max_velocity (rad/s)
        'R'     // joint_type
    ));
    
    // J2 - 肩部关节
    m_parameters.append(DHParameter(
        "J2", "NAUO2",
        0,
        0,
        M_PI / 2,
        0,
        -M_PI,
        M_PI,
        M_PI,
        'R'
    ));
    
    // J3 - 上臂关节
    m_parameters.append(DHParameter(
        "J3", "NAUO3",
        400,
        0,
        0,
        0,
        -M_PI,
        M_PI,
        M_PI,
        'R'
    ));
    
    // J4 - 肘部关节
    m_parameters.append(DHParameter(
        "J4", "NAUO4",
        0,
        400,
        M_PI / 2,
        0,
        -M_PI,
        M_PI,
        M_PI,
        'R'
    ));
    
    // J5 - 腕部关节1
    m_parameters.append(DHParameter(
        "J5", "NAUO5",
        0,
        0,
        -M_PI / 2,
        0,
        -M_PI,
        M_PI,
        M_PI,
        'R'
    ));
    
    // J6 - 腕部关节2
    m_parameters.append(DHParameter(
        "J6", "NAUO6",
        0,
        100,
        M_PI / 2,
        0,
        -M_PI,
        M_PI,
        M_PI,
        'R'
    ));
    
    // 末端执行器 (不是关节，但用于计算)
    m_parameters.append(DHParameter(
        "EE", "NAUO7",
        0,
        100,
        0,
        0,
        0,
        0,
        0,
        'R'
    ));
    
    qDebug() << "AuboI5HDHParameters: 初始化完成，共" << m_parameters.size() << "个关节";
}

const DHParameter& AuboI5HDHParameters::getParameter(int jointIndex) const
{
    if (jointIndex < 0 || jointIndex >= m_parameters.size()) {
        qWarning() << "AuboI5HDHParameters: 关节索引超出范围:" << jointIndex;
        static DHParameter dummy;
        return dummy;
    }
    return m_parameters[jointIndex];
}

int AuboI5HDHParameters::getJointIndexByPartName(const QString& partName) const
{
    for (int i = 0; i < m_parameters.size(); ++i) {
        if (m_parameters[i].partName == partName) {
            return i;
        }
    }
    qWarning() << "AuboI5HDHParameters: 找不到部件:" << partName;
    return -1;
}

Eigen::Matrix4d AuboI5HDHParameters::computeDHTransform(int jointIndex, double theta) const
{
    if (jointIndex < 0 || jointIndex >= m_parameters.size()) {
        qWarning() << "AuboI5HDHParameters: 关节索引超出范围:" << jointIndex;
        return Eigen::Matrix4d::Identity();
    }
    
    const DHParameter& param = m_parameters[jointIndex];
    
    // 应用关节零位偏移
    double theta_actual = theta + param.theta_offset;
    
    // DH变换矩阵 = Rot_Z(theta) * Trans_Z(d) * Trans_X(a) * Rot_X(alpha)
    Eigen::Matrix4d T = MatrixUtils::rotationZ(theta_actual);
    T = T * MatrixUtils::translationZ(param.d);
    T = T * MatrixUtils::translationX(param.a);
    T = T * MatrixUtils::rotationX(param.alpha);
    
    return T;
}

Eigen::Matrix4d AuboI5HDHParameters::computeForwardKinematics(const QVector<double>& jointAngles) const
{
    if (jointAngles.size() < 6) {
        qWarning() << "AuboI5HDHParameters: 关节角度数量不足，需要6个，得到" << jointAngles.size();
        return Eigen::Matrix4d::Identity();
    }
    
    // 从基座到末端执行器的变换矩阵
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    
    // 计算每个关节的变换矩阵并相乘
    for (int i = 0; i < 6; ++i) {
        Eigen::Matrix4d Ti = computeDHTransform(i, jointAngles[i]);
        T = T * Ti;
    }
    
    // 加上末端执行器的偏移 (如果有的话)
    if (m_parameters.size() > 6) {
        Eigen::Matrix4d T_ee = computeDHTransform(6, 0);
        T = T * T_ee;
    }
    
    return T;
}

Eigen::Matrix4d AuboI5HDHParameters::computeTransformToJoint(int jointIndex, const QVector<double>& jointAngles) const
{
    if (jointIndex < 0 || jointIndex >= 6) {
        qWarning() << "AuboI5HDHParameters: 关节索引超出范围:" << jointIndex;
        return Eigen::Matrix4d::Identity();
    }
    
    if (jointAngles.size() < 6) {
        qWarning() << "AuboI5HDHParameters: 关节角度数量不足";
        return Eigen::Matrix4d::Identity();
    }
    
    // 从基座到指定关节的变换矩阵
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    
    for (int i = 0; i <= jointIndex; ++i) {
        Eigen::Matrix4d Ti = computeDHTransform(i, jointAngles[i]);
        T = T * Ti;
    }
    
    return T;
}

bool AuboI5HDHParameters::isJointAngleValid(int jointIndex, double theta) const
{
    if (jointIndex < 0 || jointIndex >= m_parameters.size()) {
        return false;
    }
    
    const DHParameter& param = m_parameters[jointIndex];
    return theta >= param.min_angle && theta <= param.max_angle;
}

double AuboI5HDHParameters::clampJointAngle(int jointIndex, double theta) const
{
    if (jointIndex < 0 || jointIndex >= m_parameters.size()) {
        return theta;
    }
    
    const DHParameter& param = m_parameters[jointIndex];
    if (theta < param.min_angle) return param.min_angle;
    if (theta > param.max_angle) return param.max_angle;
    return theta;
}

// ==================== MatrixUtils 实现 ====================

Eigen::Matrix4d MatrixUtils::rotationZ(double angle)
{
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    double c = std::cos(angle);
    double s = std::sin(angle);
    
    T(0, 0) = c;   T(0, 1) = -s;  T(0, 2) = 0;
    T(1, 0) = s;   T(1, 1) = c;   T(1, 2) = 0;
    T(2, 0) = 0;   T(2, 1) = 0;   T(2, 2) = 1;
    
    return T;
}

Eigen::Matrix4d MatrixUtils::rotationX(double angle)
{
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    double c = std::cos(angle);
    double s = std::sin(angle);
    
    T(0, 0) = 1;   T(0, 1) = 0;   T(0, 2) = 0;
    T(1, 0) = 0;   T(1, 1) = c;   T(1, 2) = -s;
    T(2, 0) = 0;   T(2, 1) = s;   T(2, 2) = c;
    
    return T;
}

Eigen::Matrix4d MatrixUtils::translationZ(double distance)
{
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T(2, 3) = distance;
    return T;
}

Eigen::Matrix4d MatrixUtils::translationX(double distance)
{
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T(0, 3) = distance;
    return T;
}

Eigen::Vector3d MatrixUtils::extractPosition(const Eigen::Matrix4d& transform)
{
    return Eigen::Vector3d(transform(0, 3), transform(1, 3), transform(2, 3));
}

Eigen::Vector3d MatrixUtils::extractEulerAngles(const Eigen::Matrix4d& transform)
{
    // 从旋转矩阵提取欧拉角 (ZYX顺序)
    Eigen::Vector3d euler;
    
    // 提取旋转矩阵的3x3部分
    Eigen::Matrix3d R = transform.block<3, 3>(0, 0);
    
    // 计算欧拉角
    double sy = std::sqrt(R(0, 0) * R(0, 0) + R(1, 0) * R(1, 0));
    
    bool singular = sy < 1e-6;
    
    if (!singular) {
        euler(0) = std::atan2(R(2, 1), R(2, 2));  // roll (绕X轴)
        euler(1) = std::atan2(-R(2, 0), sy);      // pitch (绕Y轴)
        euler(2) = std::atan2(R(1, 0), R(0, 0));  // yaw (绕Z轴)
    } else {
        euler(0) = std::atan2(-R(1, 2), R(1, 1));
        euler(1) = std::atan2(-R(2, 0), sy);
        euler(2) = 0;
    }
    
    return euler;
}

void MatrixUtils::printMatrix(const Eigen::Matrix4d& matrix, const QString& name)
{
    qDebug() << name << ":";
    for (int i = 0; i < 4; ++i) {
        qDebug() << QString("[%1 %2 %3 %4]")
            .arg(matrix(i, 0), 8, 'f', 4)
            .arg(matrix(i, 1), 8, 'f', 4)
            .arg(matrix(i, 2), 8, 'f', 4)
            .arg(matrix(i, 3), 8, 'f', 4);
    }
}
