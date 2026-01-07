#ifndef ROBOTKINEMATICS_H
#define ROBOTKINEMATICS_H

#include <QObject>
#include <QVector3D>
#include <QMatrix4x4>
#include <array>
#include <vector>

namespace Robot {

/**
 * @brief 机器人DH参数结构
 */
struct DHParameter {
    double a;       // 连杆长度 (mm)
    double alpha;   // 连杆扭角 (rad)
    double d;       // 连杆偏距 (mm)
    double theta;   // 关节角度 (rad)
};

/**
 * @brief 关节限位结构
 */
struct JointLimit {
    double min;     // 最小角度 (度)
    double max;     // 最大角度 (度)
    double maxVel;  // 最大速度 (度/秒)
};

/**
 * @brief 机器人末端位姿
 */
struct EndEffectorPose {
    QVector3D position;     // 位置 (mm)
    QVector3D orientation;  // 姿态 (RPY角度，度)
};

/**
 * @brief 安川MPX3500机器人运动学类
 * 
 * 6轴工业机器人运动学计算
 * - 正运动学：关节角度 -> 末端位姿
 * - 逆运动学：末端位姿 -> 关节角度
 */
class RobotKinematics : public QObject
{
    Q_OBJECT

public:
    static constexpr int NUM_JOINTS = 6;
    
    explicit RobotKinematics(QObject* parent = nullptr);
    ~RobotKinematics();

    /**
     * @brief 设置关节角度
     * @param jointIndex 关节索引 (0-5)
     * @param angle 角度 (度)
     * @return 是否在限位范围内
     */
    bool setJointAngle(int jointIndex, double angle);

    /**
     * @brief 设置所有关节角度
     * @param angles 6个关节角度 (度)
     * @return 是否全部在限位范围内
     */
    bool setJointAngles(const std::array<double, NUM_JOINTS>& angles);

    /**
     * @brief 获取关节角度
     * @param jointIndex 关节索引 (0-5)
     * @return 角度 (度)
     */
    double getJointAngle(int jointIndex) const;

    /**
     * @brief 获取所有关节角度
     * @return 6个关节角度 (度)
     */
    std::array<double, NUM_JOINTS> getJointAngles() const;

    /**
     * @brief 获取关节限位
     * @param jointIndex 关节索引 (0-5)
     * @return 关节限位
     */
    JointLimit getJointLimit(int jointIndex) const;

    /**
     * @brief 正运动学计算
     * @return 末端位姿
     */
    EndEffectorPose forwardKinematics() const;

    /**
     * @brief 获取各关节的变换矩阵
     * @return 7个变换矩阵 (基座 + 6个关节)
     */
    std::vector<QMatrix4x4> getJointTransforms() const;

    /**
     * @brief 逆运动学计算
     * @param targetPose 目标位姿
     * @param solution 输出的关节角度解
     * @return 是否找到有效解
     */
    bool inverseKinematics(const EndEffectorPose& targetPose, 
                          std::array<double, NUM_JOINTS>& solution) const;

    /**
     * @brief 重置到零位
     */
    void resetToHome();

    /**
     * @brief 获取机器人名称
     */
    QString getRobotName() const { return m_robotName; }

signals:
    /**
     * @brief 关节角度变化信号
     * @param jointIndex 关节索引
     * @param angle 新角度 (度)
     */
    void jointAngleChanged(int jointIndex, double angle);

    /**
     * @brief 末端位姿变化信号
     * @param pose 新位姿
     */
    void endEffectorPoseChanged(const EndEffectorPose& pose);

    /**
     * @brief 关节超限警告
     * @param jointIndex 关节索引
     * @param angle 尝试设置的角度
     */
    void jointLimitWarning(int jointIndex, double angle);

private:
    void initDHParameters();
    void initJointLimits();
    QMatrix4x4 computeDHMatrix(const DHParameter& dh) const;
    double degToRad(double deg) const;
    double radToDeg(double rad) const;

private:
    QString m_robotName;
    std::array<DHParameter, NUM_JOINTS> m_dhParams;
    std::array<JointLimit, NUM_JOINTS> m_jointLimits;
    std::array<double, NUM_JOINTS> m_jointAngles;  // 当前关节角度 (度)
};

} // namespace Robot

#endif // ROBOTKINEMATICS_H
