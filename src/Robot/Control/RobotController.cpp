#include "RobotController.h"
#include "MotoTcpClient.h"
#include <QDebug>

namespace Robot {

RobotController::RobotController(QObject* parent)
    : QObject(parent)
    , m_kinematics(std::make_unique<RobotKinematics>(this))
    , m_tcpClient(std::make_unique<MotoTcpClient>(this))
    , m_stateUpdateTimer(new QTimer(this))
    , m_robotPort(10040)
{
    initConnections();
    
    // 状态更新定时器 (100ms)
    m_stateUpdateTimer->setInterval(100);
    connect(m_stateUpdateTimer, &QTimer::timeout, this, &RobotController::updateRobotState);
    
    qDebug() << "RobotController: 初始化完成";
}

RobotController::~RobotController()
{
    disconnectFromRobot();
}

void RobotController::initConnections()
{
    // 连接运动学信号
    connect(m_kinematics.get(), &RobotKinematics::jointAngleChanged,
            this, &RobotController::onKinematicsJointChanged);
    connect(m_kinematics.get(), &RobotKinematics::endEffectorPoseChanged,
            this, &RobotController::onKinematicsPoseChanged);
    
    // 连接TCP客户端信号
    connect(m_tcpClient.get(), &MotoTcpClient::connected,
            this, &RobotController::onTcpConnected);
    connect(m_tcpClient.get(), &MotoTcpClient::disconnected,
            this, &RobotController::onTcpDisconnected);
    connect(m_tcpClient.get(), &MotoTcpClient::errorOccurred,
            this, &RobotController::onTcpError);
    connect(m_tcpClient.get(), &MotoTcpClient::dataReceived,
            this, &RobotController::onTcpDataReceived);
}

// ========== 连接管理 ==========

bool RobotController::connectToRobot(const QString& ip, int port)
{
    if (m_state.connectionState == ConnectionState::Connected) {
        qWarning() << "RobotController: 已经连接到机器人";
        return true;
    }
    
    m_robotIp = ip;
    m_robotPort = port;
    
    m_state.connectionState = ConnectionState::Connecting;
    emit connectionStateChanged(m_state.connectionState);
    
    qDebug() << "RobotController: 正在连接到机器人" << ip << ":" << port;
    
    return m_tcpClient->connectToHost(ip, port);
}

void RobotController::disconnectFromRobot()
{
    if (m_state.connectionState == ConnectionState::Disconnected) {
        return;
    }
    
    m_stateUpdateTimer->stop();
    m_tcpClient->disconnectFromHost();
    
    m_state.connectionState = ConnectionState::Disconnected;
    emit connectionStateChanged(m_state.connectionState);
    
    qDebug() << "RobotController: 已断开连接";
}

ConnectionState RobotController::getConnectionState() const
{
    return m_state.connectionState;
}

bool RobotController::isConnected() const
{
    return m_state.connectionState == ConnectionState::Connected;
}

// ========== 模式控制 ==========

void RobotController::setOperationMode(OperationMode mode)
{
    if (m_state.operationMode == mode) {
        return;
    }
    
    m_state.operationMode = mode;
    emit operationModeChanged(mode);
    
    QString modeName;
    switch (mode) {
        case OperationMode::Simulation: modeName = "仿真"; break;
        case OperationMode::Remote: modeName = "远程"; break;
        case OperationMode::Teach: modeName = "示教"; break;
    }
    qDebug() << "RobotController: 切换到" << modeName << "模式";
}

OperationMode RobotController::getOperationMode() const
{
    return m_state.operationMode;
}

bool RobotController::isSimulationMode() const
{
    return m_state.operationMode == OperationMode::Simulation;
}

// ========== 关节控制 ==========

void RobotController::setJointAngle(int jointIndex, double angle)
{
    if (jointIndex < 0 || jointIndex >= 6) {
        return;
    }
    
    // 更新运动学模型
    m_kinematics->setJointAngle(jointIndex, angle);
    
    // 如果连接到真实机器人，发送命令
    if (isConnected() && !isSimulationMode()) {
        std::array<double, 6> angles = m_kinematics->getJointAngles();
        sendJointCommand(angles);
    }
}

void RobotController::setJointAngles(const std::array<double, 6>& angles)
{
    m_kinematics->setJointAngles(angles);
    
    if (isConnected() && !isSimulationMode()) {
        sendJointCommand(angles);
    }
}

double RobotController::getJointAngle(int jointIndex) const
{
    return m_kinematics->getJointAngle(jointIndex);
}

std::array<double, 6> RobotController::getJointAngles() const
{
    return m_kinematics->getJointAngles();
}

JointLimit RobotController::getJointLimit(int jointIndex) const
{
    return m_kinematics->getJointLimit(jointIndex);
}

// ========== 运动控制 ==========

void RobotController::moveToPose(const EndEffectorPose& pose, double speed)
{
    std::array<double, 6> solution;
    if (m_kinematics->inverseKinematics(pose, solution)) {
        moveToJointAngles(solution, speed);
    } else {
        emit errorOccurred("无法计算逆运动学解");
    }
}

void RobotController::moveToJointAngles(const std::array<double, 6>& angles, double speed)
{
    Q_UNUSED(speed)
    
    m_state.isMoving = true;
    
    // 在仿真模式下直接设置角度
    if (isSimulationMode()) {
        setJointAngles(angles);
        m_state.isMoving = false;
        emit motionCompleted();
    } else {
        // 发送运动命令到真实机器人
        sendJointCommand(angles);
    }
}

void RobotController::stopMotion()
{
    m_state.isMoving = false;
    
    if (isConnected() && !isSimulationMode()) {
        // 发送停止命令
        m_tcpClient->sendCommand("STOP");
    }
    
    qDebug() << "RobotController: 停止运动";
}

void RobotController::moveToHome()
{
    std::array<double, 6> homeAngles = {0, 0, 0, 0, 0, 0};
    moveToJointAngles(homeAngles, 30);
    
    qDebug() << "RobotController: 回零位";
}

// ========== 状态查询 ==========

RobotState RobotController::getRobotState() const
{
    return m_state;
}

EndEffectorPose RobotController::getEndEffectorPose() const
{
    return m_kinematics->forwardKinematics();
}

RobotKinematics* RobotController::getKinematics() const
{
    return m_kinematics.get();
}

// ========== 伺服控制 ==========

bool RobotController::servoOn()
{
    if (!isConnected()) {
        emit errorOccurred("未连接到机器人");
        return false;
    }
    
    m_tcpClient->sendCommand("SVON");
    m_state.isServoOn = true;
    
    qDebug() << "RobotController: 伺服上电";
    return true;
}

bool RobotController::servoOff()
{
    if (!isConnected()) {
        return false;
    }
    
    m_tcpClient->sendCommand("SVOFF");
    m_state.isServoOn = false;
    
    qDebug() << "RobotController: 伺服下电";
    return true;
}

bool RobotController::clearAlarm()
{
    if (!isConnected()) {
        return false;
    }
    
    m_tcpClient->sendCommand("RESET");
    m_state.hasError = false;
    m_state.errorMessage.clear();
    
    qDebug() << "RobotController: 清除报警";
    return true;
}

// ========== 私有槽函数 ==========

void RobotController::onTcpConnected()
{
    m_state.connectionState = ConnectionState::Connected;
    emit connectionStateChanged(m_state.connectionState);
    
    // 启动状态更新定时器
    m_stateUpdateTimer->start();
    
    qDebug() << "RobotController: TCP连接成功";
}

void RobotController::onTcpDisconnected()
{
    m_state.connectionState = ConnectionState::Disconnected;
    m_state.isServoOn = false;
    emit connectionStateChanged(m_state.connectionState);
    
    m_stateUpdateTimer->stop();
    
    qDebug() << "RobotController: TCP连接断开";
}

void RobotController::onTcpError(const QString& error)
{
    m_state.connectionState = ConnectionState::Error;
    m_state.hasError = true;
    m_state.errorMessage = error;
    
    emit connectionStateChanged(m_state.connectionState);
    emit errorOccurred(error);
    
    qCritical() << "RobotController: TCP错误:" << error;
}

void RobotController::onTcpDataReceived(const QByteArray& data)
{
    parseRobotResponse(data);
}

void RobotController::onKinematicsJointChanged(int jointIndex, double angle)
{
    m_state.jointAngles[jointIndex] = angle;
    emit jointAnglesChanged(m_state.jointAngles);
}

void RobotController::onKinematicsPoseChanged(const EndEffectorPose& pose)
{
    m_state.endEffectorPose = pose;
    emit endEffectorPoseChanged(pose);
}

void RobotController::updateRobotState()
{
    if (!isConnected()) {
        return;
    }
    
    // 请求机器人状态
    m_tcpClient->sendCommand("RPOSJ");  // 读取关节位置
    
    emit robotStateChanged(m_state);
}

// ========== 私有函数 ==========

void RobotController::sendJointCommand(const std::array<double, 6>& angles)
{
    if (!isConnected()) {
        return;
    }
    
    // 构建MotoPlus格式的关节运动命令
    QString cmd = QString("MOVJ %1,%2,%3,%4,%5,%6")
        .arg(angles[0], 0, 'f', 3)
        .arg(angles[1], 0, 'f', 3)
        .arg(angles[2], 0, 'f', 3)
        .arg(angles[3], 0, 'f', 3)
        .arg(angles[4], 0, 'f', 3)
        .arg(angles[5], 0, 'f', 3);
    
    m_tcpClient->sendCommand(cmd);
}

void RobotController::parseRobotResponse(const QByteArray& data)
{
    QString response = QString::fromUtf8(data).trimmed();
    
    // 解析关节位置响应
    if (response.startsWith("RPOSJ")) {
        QStringList parts = response.mid(6).split(',');
        if (parts.size() >= 6) {
            for (int i = 0; i < 6; ++i) {
                m_state.jointAngles[i] = parts[i].toDouble();
            }
            
            // 更新运动学模型
            m_kinematics->setJointAngles(m_state.jointAngles);
        }
    }
    // 解析其他响应...
}

} // namespace Robot
