#include "QtUnityBridge.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <QHostAddress>
#include <QThread>

namespace UI {

QtUnityBridge::QtUnityBridge(QObject *parent)
    : QObject(parent)
    , m_server(nullptr)
    , m_unitySocket(nullptr)
    , m_heartbeatTimer(nullptr)
    , m_isConnected(false)
    , m_serverPort(12346)
{
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, &QtUnityBridge::OnNewConnection);
    
    SetupHeartbeat();
}

QtUnityBridge::~QtUnityBridge()
{
    StopServer();
}

bool QtUnityBridge::StartServer(quint16 port)
{
    m_serverPort = port;
    
    // 强制关闭现有服务器
    if (m_server->isListening()) {
        m_server->close();
        // 等待一小段时间确保端口释放
        QThread::msleep(100);
    }
    
    // 设置服务器选项，允许地址重用
    m_server->setMaxPendingConnections(1);
    
    if (!m_server->listen(QHostAddress::LocalHost, port)) {
        QString error = m_server->errorString();
        qWarning() << "Unity Bridge: 无法启动服务器，端口:" << port << "错误:" << error;
        emit ConnectionError(QString("无法启动服务器: %1").arg(error));
        return false;
    }
    
    qDebug() << "Unity Bridge: 服务器已启动，监听端口:" << port;
    return true;
}

void QtUnityBridge::StopServer()
{
    // 断开Unity连接
    if (m_unitySocket) {
        if (m_unitySocket->state() == QAbstractSocket::ConnectedState) {
            m_unitySocket->disconnectFromHost();
            // 等待断开完成
            if (!m_unitySocket->waitForDisconnected(1000)) {
                m_unitySocket->abort();
            }
        }
        m_unitySocket->deleteLater();
        m_unitySocket = nullptr;
    }
    
    // 关闭服务器
    if (m_server && m_server->isListening()) {
        m_server->close();
        qDebug() << "Unity Bridge: TCP服务器已关闭";
    }
    
    m_isConnected = false;
    qDebug() << "Unity Bridge: 服务器已停止";
}

bool QtUnityBridge::IsConnected() const
{
    return m_isConnected && m_unitySocket && m_unitySocket->state() == QAbstractSocket::ConnectedState;
}

void QtUnityBridge::SendWorkpieceData(const QJsonObject& workpieceData)
{
    QJsonObject message;
    message["type"] = "workpiece_data";
    
    // 将workpieceData转换为JSON字符串
    QJsonDocument dataDoc(workpieceData);
    message["data"] = QString::fromUtf8(dataDoc.toJson(QJsonDocument::Compact));
    
    SendMessage(message);
}

void QtUnityBridge::SendTrajectoryData(const QJsonObject& trajectoryData)
{
    QJsonObject message;
    message["type"] = "trajectory_data";
    
    // 将trajectoryData转换为JSON字符串
    QJsonDocument dataDoc(trajectoryData);
    message["data"] = QString::fromUtf8(dataDoc.toJson(QJsonDocument::Compact));
    
    SendMessage(message);
}

void QtUnityBridge::SendSimulationCommand(const QString& command, const QJsonObject& parameters)
{
    QJsonObject message;
    message["type"] = "simulation_command";
    message["command"] = command;
    message["parameters"] = parameters;
    SendMessage(message);
}

void QtUnityBridge::SendCameraCommand(const QString& command, const QJsonObject& parameters)
{
    QJsonObject message;
    message["type"] = "camera_command";
    message["command"] = command;
    message["parameters"] = parameters;
    SendMessage(message);
}

void QtUnityBridge::StartSimulation()
{
    SendSimulationCommand("start");
}

void QtUnityBridge::StopSimulation()
{
    SendSimulationCommand("stop");
}

void QtUnityBridge::PauseSimulation()
{
    SendSimulationCommand("pause");
}

void QtUnityBridge::ResetSimulation()
{
    SendSimulationCommand("reset");
}

void QtUnityBridge::SetSimulationSpeed(float speed)
{
    QJsonObject params;
    params["speed"] = speed;
    SendSimulationCommand("set_speed", params);
}

void QtUnityBridge::LoadWorkpiece(const QString& filePath)
{
    QJsonObject params;
    params["file_path"] = filePath;
    SendSimulationCommand("load_workpiece", params);
}

void QtUnityBridge::ShowTrajectory(const QJsonObject& trajectoryData)
{
    SendTrajectoryData(trajectoryData);
}

void QtUnityBridge::SetCameraView(const QString& viewType)
{
    QJsonObject params;
    params["view_type"] = viewType;
    SendCameraCommand("set_view", params);
}

void QtUnityBridge::ResetCamera()
{
    SendCameraCommand("reset");
}

void QtUnityBridge::OnNewConnection()
{
    if (m_unitySocket) {
        // 如果已有连接，断开旧连接
        m_unitySocket->disconnectFromHost();
    }
    
    m_unitySocket = m_server->nextPendingConnection();
    
    connect(m_unitySocket, &QTcpSocket::connected, this, &QtUnityBridge::OnClientConnected);
    connect(m_unitySocket, &QTcpSocket::disconnected, this, &QtUnityBridge::OnClientDisconnected);
    connect(m_unitySocket, &QTcpSocket::readyRead, this, &QtUnityBridge::OnDataReceived);
    connect(m_unitySocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &QtUnityBridge::OnSocketError);
    
    // TCP连接已经建立，直接设置连接状态
    m_isConnected = true;
    emit UnityConnected();
    
    qDebug() << "Unity Bridge: Unity客户端已连接，状态:" << (m_unitySocket->state() == QAbstractSocket::ConnectedState ? "已连接" : "未连接");
}

void QtUnityBridge::OnClientConnected()
{
    m_isConnected = true;
    emit UnityConnected();
    qDebug() << "Unity Bridge: 与Unity的连接已建立";
}

void QtUnityBridge::OnClientDisconnected()
{
    m_isConnected = false;
    emit UnityDisconnected();
    qDebug() << "Unity Bridge: Unity连接已断开";
}

void QtUnityBridge::OnDataReceived()
{
    if (!m_unitySocket) return;
    
    m_messageBuffer.append(m_unitySocket->readAll());
    
    // 处理完整的JSON消息（以换行符分隔）
    while (m_messageBuffer.contains('\n')) {
        int index = m_messageBuffer.indexOf('\n');
        QByteArray messageData = m_messageBuffer.left(index);
        m_messageBuffer.remove(0, index + 1);
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(messageData, &error);
        
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            ProcessUnityMessage(doc.object());
        } else {
            qWarning() << "Unity Bridge: JSON解析错误:" << error.errorString();
        }
    }
}

void QtUnityBridge::OnSocketError(QAbstractSocket::SocketError error)
{
    QString errorString = m_unitySocket ? m_unitySocket->errorString() : "未知错误";
    qWarning() << "Unity Bridge: Socket错误:" << error << errorString;
    emit ConnectionError(errorString);
}

void QtUnityBridge::CheckConnection()
{
    if (m_unitySocket && m_unitySocket->state() != QAbstractSocket::ConnectedState) {
        if (m_isConnected) {
            m_isConnected = false;
            emit UnityDisconnected();
        }
    }
}

void QtUnityBridge::ProcessUnityMessage(const QJsonObject& message)
{
    QString type = message["type"].toString();
    
    if (type == "collision_detected") {
        emit CollisionDetected(message["data"].toObject());
    }
    else if (type == "simulation_complete") {
        emit SimulationComplete(message["data"].toObject());
    }
    else if (type == "quality_prediction") {
        emit QualityPrediction(message["data"].toObject());
    }
    else if (type == "scene_clicked") {
        emit SceneClicked(message["data"].toObject());
    }
    else if (type == "camera_view_changed") {
        emit CameraViewChanged(message["view_type"].toString());
    }
    else if (type == "workpiece_loaded") {
        emit WorkpieceLoaded(message["success"].toBool(), message["message"].toString());
    }
    else if (type == "trajectory_displayed") {
        emit TrajectoryDisplayed(message["success"].toBool(), message["message"].toString());
    }
    else if (type == "heartbeat") {
        // 心跳响应，保持连接
        QJsonObject response;
        response["type"] = "heartbeat_response";
        SendMessage(response);
    }
    else {
        qDebug() << "Unity Bridge: 收到未知消息类型:" << type;
    }
}

void QtUnityBridge::SendMessage(const QJsonObject& message)
{
    if (!IsConnected()) {
        qWarning() << "Unity Bridge: 未连接到Unity，无法发送消息";
        return;
    }
    
    QJsonDocument doc(message);
    QByteArray data = doc.toJson(QJsonDocument::Compact) + "\n";
    
    // 添加调试信息
    QString messageType = message["type"].toString();
    qDebug() << "Unity Bridge: 发送消息类型:" << messageType << "大小:" << data.size() << "字节";
    
    qint64 written = m_unitySocket->write(data);
    if (written != data.size()) {
        qWarning() << "Unity Bridge: 消息发送不完整，期望:" << data.size() << "实际:" << written;
    } else {
        qDebug() << "Unity Bridge: 消息发送成功";
    }
}

void QtUnityBridge::SetupHeartbeat()
{
    m_heartbeatTimer = new QTimer(this);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &QtUnityBridge::CheckConnection);
    m_heartbeatTimer->start(5000); // 每5秒检查一次连接
}

} // namespace UI