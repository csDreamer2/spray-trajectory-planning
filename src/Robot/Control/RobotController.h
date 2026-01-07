#ifndef ROBOTCONTROLLER_H
#define ROBOTCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <memory>
#include "../Kinematics/RobotKinematics.h"

namespace Robot {

class MotoTcpClient;

/**
 * @brief 机器人连接状态
 */
enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Error
};

/**
 * @brief 机器人运行模式
 */
enum class OperationMode {
    Simulation,     // 仿真模式
    Remote,         // 远程控制模式
    Teach           // 示教模式
};

/**
 * @brief 机器人状态结构
 */
struct RobotState {
    ConnectionState connectionState = ConnectionState::Disconnected;
    OperationMode operationMode = OperationMode::Simulation;
    bool isServoOn = false;
    bool isMoving = false;
    bool hasError = false;
    QString errorMessage;
    std::array<double, 6> jointAngles = {0, 0, 0, 0, 0, 0};
    std::array<double, 6> jointVelocities = {0, 0, 0, 0, 0, 0};
    EndEffectorPose endEffectorPose;
};

/**
 * @brief 机器人控制器
 * 
 * 负责机器人的整体控制，包括：
 * - 运动学计算
 * - 与真实机器人通信
 * - 状态监控
 * - 运动控制
 */
class RobotController : public QObject
{
    Q_OBJECT

public:
    explicit RobotController(QObject* parent = nullptr);
    ~RobotController();

    // ========== 连接管理 ==========
    
    /**
     * @brief 连接到真实机器人
     * @param ip 机器人IP地址
     * @param port 端口号
     * @return 是否开始连接
     */
    bool connectToRobot(const QString& ip, int port = 10040);

    /**
     * @brief 断开连接
     */
    void disconnectFromRobot();

    /**
     * @brief 获取连接状态
     */
    ConnectionState getConnectionState() const;

    /**
     * @brief 是否已连接
     */
    bool isConnected() const;

    // ========== 模式控制 ==========
    
    /**
     * @brief 设置操作模式
     */
    void setOperationMode(OperationMode mode);

    /**
     * @brief 获取操作模式
     */
    OperationMode getOperationMode() const;

    /**
     * @brief 是否为仿真模式
     */
    bool isSimulationMode() const;

    // ========== 关节控制 ==========
    
    /**
     * @brief 设置单个关节角度
     * @param jointIndex 关节索引 (0-5)
     * @param angle 角度 (度)
     */
    void setJointAngle(int jointIndex, double angle);

    /**
     * @brief 设置所有关节角度
     * @param angles 6个关节角度 (度)
     */
    void setJointAngles(const std::array<double, 6>& angles);

    /**
     * @brief 获取关节角度
     */
    double getJointAngle(int jointIndex) const;

    /**
     * @brief 获取所有关节角度
     */
    std::array<double, 6> getJointAngles() const;

    /**
     * @brief 获取关节限位
     */
    JointLimit getJointLimit(int jointIndex) const;

    // ========== 运动控制 ==========
    
    /**
     * @brief 移动到目标位姿
     * @param pose 目标位姿
     * @param speed 速度百分比 (0-100)
     */
    void moveToPose(const EndEffectorPose& pose, double speed = 50);

    /**
     * @brief 移动到目标关节角度
     * @param angles 目标关节角度
     * @param speed 速度百分比 (0-100)
     */
    void moveToJointAngles(const std::array<double, 6>& angles, double speed = 50);

    /**
     * @brief 停止运动
     */
    void stopMotion();

    /**
     * @brief 回零位
     */
    void moveToHome();

    // ========== 状态查询 ==========
    
    /**
     * @brief 获取机器人状态
     */
    RobotState getRobotState() const;

    /**
     * @brief 获取末端位姿
     */
    EndEffectorPose getEndEffectorPose() const;

    /**
     * @brief 获取运动学对象
     */
    RobotKinematics* getKinematics() const;

    // ========== 伺服控制 ==========
    
    /**
     * @brief 伺服上电
     */
    bool servoOn();

    /**
     * @brief 伺服下电
     */
    bool servoOff();

    /**
     * @brief 清除报警
     */
    bool clearAlarm();

signals:
    /**
     * @brief 连接状态变化
     */
    void connectionStateChanged(ConnectionState state);

    /**
     * @brief 操作模式变化
     */
    void operationModeChanged(OperationMode mode);

    /**
     * @brief 关节角度变化
     */
    void jointAnglesChanged(const std::array<double, 6>& angles);

    /**
     * @brief 末端位姿变化
     */
    void endEffectorPoseChanged(const EndEffectorPose& pose);

    /**
     * @brief 机器人状态变化
     */
    void robotStateChanged(const RobotState& state);

    /**
     * @brief 错误发生
     */
    void errorOccurred(const QString& errorMessage);

    /**
     * @brief 运动完成
     */
    void motionCompleted();

private slots:
    void onTcpConnected();
    void onTcpDisconnected();
    void onTcpError(const QString& error);
    void onTcpDataReceived(const QByteArray& data);
    void onKinematicsJointChanged(int jointIndex, double angle);
    void onKinematicsPoseChanged(const EndEffectorPose& pose);
    void updateRobotState();

private:
    void initConnections();
    void sendJointCommand(const std::array<double, 6>& angles);
    void parseRobotResponse(const QByteArray& data);

private:
    std::unique_ptr<RobotKinematics> m_kinematics;
    std::unique_ptr<MotoTcpClient> m_tcpClient;
    
    RobotState m_state;
    QTimer* m_stateUpdateTimer;
    
    QString m_robotIp;
    int m_robotPort;
};

} // namespace Robot

#endif // ROBOTCONTROLLER_H
