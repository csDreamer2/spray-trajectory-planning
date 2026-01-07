#include "RobotKinematics.h"
#include <QtMath>
#include <QDebug>

namespace Robot {

RobotKinematics::RobotKinematics(QObject* parent)
    : QObject(parent)
    , m_robotName("YASKAWA MPX3500")
{
    initDHParameters();
    initJointLimits();
    resetToHome();
}

RobotKinematics::~RobotKinematics()
{
}

void RobotKinematics::initDHParameters()
{
    // 安川MPX3500机器人DH参数 (近似值，实际需要根据机器人手册调整)
    // 参数: a(mm), alpha(rad), d(mm), theta(rad)
    
    // J1: 基座旋转
    m_dhParams[0] = {0, -M_PI/2, 330, 0};
    
    // J2: 肩部
    m_dhParams[1] = {680, 0, 0, -M_PI/2};
    
    // J3: 肘部
    m_dhParams[2] = {0, -M_PI/2, 0, 0};
    
    // J4: 手腕旋转
    m_dhParams[3] = {0, M_PI/2, 680, 0};
    
    // J5: 手腕俯仰
    m_dhParams[4] = {0, -M_PI/2, 0, 0};
    
    // J6: 末端旋转
    m_dhParams[5] = {0, 0, 100, 0};
    
    qDebug() << "RobotKinematics: DH参数初始化完成 -" << m_robotName;
}

void RobotKinematics::initJointLimits()
{
    // 安川MPX3500关节限位 (度)
    // 参数: min, max, maxVel(度/秒)
    
    m_jointLimits[0] = {-180, 180, 180};   // J1
    m_jointLimits[1] = {-90, 155, 180};    // J2
    m_jointLimits[2] = {-175, 90, 180};    // J3
    m_jointLimits[3] = {-200, 200, 400};   // J4
    m_jointLimits[4] = {-150, 150, 400};   // J5
    m_jointLimits[5] = {-455, 455, 600};   // J6
    
    qDebug() << "RobotKinematics: 关节限位初始化完成";
}

void RobotKinematics::resetToHome()
{
    // 零位姿态
    for (int i = 0; i < NUM_JOINTS; ++i) {
        m_jointAngles[i] = 0.0;
    }
    
    qDebug() << "RobotKinematics: 重置到零位";
    
    EndEffectorPose pose = forwardKinematics();
    emit endEffectorPoseChanged(pose);
}

bool RobotKinematics::setJointAngle(int jointIndex, double angle)
{
    if (jointIndex < 0 || jointIndex >= NUM_JOINTS) {
        qWarning() << "RobotKinematics: 无效的关节索引:" << jointIndex;
        return false;
    }
    
    const JointLimit& limit = m_jointLimits[jointIndex];
    
    // 检查限位
    if (angle < limit.min || angle > limit.max) {
        qWarning() << "RobotKinematics: 关节" << (jointIndex + 1) 
                   << "角度超限:" << angle 
                   << "范围:[" << limit.min << "," << limit.max << "]";
        emit jointLimitWarning(jointIndex, angle);
        
        // 限制到有效范围
        angle = qBound(limit.min, angle, limit.max);
    }
    
    if (qFuzzyCompare(m_jointAngles[jointIndex], angle)) {
        return true;
    }
    
    m_jointAngles[jointIndex] = angle;
    
    emit jointAngleChanged(jointIndex, angle);
    
    // 计算并发送末端位姿
    EndEffectorPose pose = forwardKinematics();
    emit endEffectorPoseChanged(pose);
    
    return true;
}

bool RobotKinematics::setJointAngles(const std::array<double, NUM_JOINTS>& angles)
{
    bool allValid = true;
    
    for (int i = 0; i < NUM_JOINTS; ++i) {
        if (!setJointAngle(i, angles[i])) {
            allValid = false;
        }
    }
    
    return allValid;
}

double RobotKinematics::getJointAngle(int jointIndex) const
{
    if (jointIndex < 0 || jointIndex >= NUM_JOINTS) {
        return 0.0;
    }
    return m_jointAngles[jointIndex];
}

std::array<double, RobotKinematics::NUM_JOINTS> RobotKinematics::getJointAngles() const
{
    return m_jointAngles;
}

JointLimit RobotKinematics::getJointLimit(int jointIndex) const
{
    if (jointIndex < 0 || jointIndex >= NUM_JOINTS) {
        return {0, 0, 0};
    }
    return m_jointLimits[jointIndex];
}

double RobotKinematics::degToRad(double deg) const
{
    return deg * M_PI / 180.0;
}

double RobotKinematics::radToDeg(double rad) const
{
    return rad * 180.0 / M_PI;
}

QMatrix4x4 RobotKinematics::computeDHMatrix(const DHParameter& dh) const
{
    double ct = qCos(dh.theta);
    double st = qSin(dh.theta);
    double ca = qCos(dh.alpha);
    double sa = qSin(dh.alpha);
    
    QMatrix4x4 T;
    T.setToIdentity();
    
    // DH变换矩阵
    T(0, 0) = ct;
    T(0, 1) = -st * ca;
    T(0, 2) = st * sa;
    T(0, 3) = dh.a * ct;
    
    T(1, 0) = st;
    T(1, 1) = ct * ca;
    T(1, 2) = -ct * sa;
    T(1, 3) = dh.a * st;
    
    T(2, 0) = 0;
    T(2, 1) = sa;
    T(2, 2) = ca;
    T(2, 3) = dh.d;
    
    T(3, 0) = 0;
    T(3, 1) = 0;
    T(3, 2) = 0;
    T(3, 3) = 1;
    
    return T;
}

std::vector<QMatrix4x4> RobotKinematics::getJointTransforms() const
{
    std::vector<QMatrix4x4> transforms;
    transforms.reserve(NUM_JOINTS + 1);
    
    // 基座变换
    QMatrix4x4 T;
    T.setToIdentity();
    transforms.push_back(T);
    
    // 各关节变换
    for (int i = 0; i < NUM_JOINTS; ++i) {
        DHParameter dh = m_dhParams[i];
        dh.theta += degToRad(m_jointAngles[i]);  // 加上关节角度
        
        QMatrix4x4 Ti = computeDHMatrix(dh);
        T = T * Ti;
        transforms.push_back(T);
    }
    
    return transforms;
}

EndEffectorPose RobotKinematics::forwardKinematics() const
{
    std::vector<QMatrix4x4> transforms = getJointTransforms();
    QMatrix4x4 T = transforms.back();
    
    EndEffectorPose pose;
    
    // 提取位置
    pose.position = QVector3D(T(0, 3), T(1, 3), T(2, 3));
    
    // 提取姿态 (从旋转矩阵计算RPY角)
    double r11 = T(0, 0), r12 = T(0, 1), r13 = T(0, 2);
    double r21 = T(1, 0), r22 = T(1, 1), r23 = T(1, 2);
    double r31 = T(2, 0), r32 = T(2, 1), r33 = T(2, 2);
    
    double roll, pitch, yaw;
    
    // 计算RPY角 (ZYX顺序)
    if (qAbs(r31) < 0.9999) {
        pitch = qAsin(-r31);
        roll = qAtan2(r32, r33);
        yaw = qAtan2(r21, r11);
    } else {
        // 奇异点
        pitch = (r31 < 0) ? M_PI / 2 : -M_PI / 2;
        roll = 0;
        yaw = qAtan2(-r12, r22);
    }
    
    pose.orientation = QVector3D(radToDeg(roll), radToDeg(pitch), radToDeg(yaw));
    
    return pose;
}

bool RobotKinematics::inverseKinematics(const EndEffectorPose& targetPose,
                                        std::array<double, NUM_JOINTS>& solution) const
{
    // 逆运动学计算 - 简化版本
    // 实际应用中需要使用更复杂的算法（如数值迭代法）
    
    // TODO: 实现完整的逆运动学算法
    // 这里只是一个占位实现
    
    qWarning() << "RobotKinematics: 逆运动学尚未完全实现";
    
    // 返回当前角度作为默认解
    solution = m_jointAngles;
    
    return false;
}

} // namespace Robot
