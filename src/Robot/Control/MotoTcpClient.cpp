#include "MotoTcpClient.h"
#include <QDebug>

namespace Robot {

MotoTcpClient::MotoTcpClient(QObject* parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_reconnectTimer(new QTimer(this))
    , m_heartbeatTimer(new QTimer(this))
    , m_hostPort(10040)
    , m_autoReconnect(false)
    , m_reconnectInterval(5000)
    , m_heartbeatInterval(0)
{
    // 连接socket信号
    connect(m_socket, &QTcpSocket::connected, this, &MotoTcpClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &MotoTcpClient::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &MotoTcpClient::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &MotoTcpClient::onError);
    
    // 连接定时器信号
    connect(m_reconnectTimer, &QTimer::timeout, this, &MotoTcpClient::onReconnectTimer);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &MotoTcpClient::onHeartbeatTimer);
    
    qDebug() << "MotoTcpClient: 初始化完成";
}

MotoTcpClient::~MotoTcpClient()
{
    disconnectFromHost();
}

bool MotoTcpClient::connectToHost(const QString& ip, int port)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        qWarning() << "MotoTcpClient: 已经连接";
        return true;
    }
    
    m_hostIp = ip;
    m_hostPort = port;
    
    qDebug() << "MotoTcpClient: 正在连接到" << ip << ":" << port;
    
    m_socket->connectToHost(ip, port);
    
    return true;
}

void MotoTcpClient::disconnectFromHost()
{
    m_reconnectTimer->stop();
    m_heartbeatTimer->stop();
    
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
        if (m_socket->state() != QAbstractSocket::UnconnectedState) {
            m_socket->waitForDisconnected(1000);
        }
    }
}

bool MotoTcpClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

bool MotoTcpClient::sendCommand(const QString& command)
{
    if (!isConnected()) {
        m_lastError = "未连接到机器人";
        return false;
    }
    
    m_lastCommand = command;
    
    // MotoPlus协议格式：命令 + 换行符
    QByteArray data = command.toUtf8() + "\r\n";
    
    qint64 written = m_socket->write(data);
    m_socket->flush();
    
    if (written != data.size()) {
        m_lastError = "发送数据不完整";
        return false;
    }
    
    qDebug() << "MotoTcpClient: 发送命令:" << command;
    return true;
}

bool MotoTcpClient::sendData(const QByteArray& data)
{
    if (!isConnected()) {
        m_lastError = "未连接到机器人";
        return false;
    }
    
    qint64 written = m_socket->write(data);
    m_socket->flush();
    
    return written == data.size();
}

void MotoTcpClient::setAutoReconnect(bool enable, int interval)
{
    m_autoReconnect = enable;
    m_reconnectInterval = interval;
    
    if (!enable) {
        m_reconnectTimer->stop();
    }
}

void MotoTcpClient::setHeartbeatInterval(int interval)
{
    m_heartbeatInterval = interval;
    
    if (interval > 0 && isConnected()) {
        m_heartbeatTimer->start(interval);
    } else {
        m_heartbeatTimer->stop();
    }
}

QString MotoTcpClient::lastError() const
{
    return m_lastError;
}

void MotoTcpClient::onConnected()
{
    qDebug() << "MotoTcpClient: 连接成功";
    
    m_reconnectTimer->stop();
    
    if (m_heartbeatInterval > 0) {
        m_heartbeatTimer->start(m_heartbeatInterval);
    }
    
    emit connected();
}

void MotoTcpClient::onDisconnected()
{
    qDebug() << "MotoTcpClient: 连接断开";
    
    m_heartbeatTimer->stop();
    
    if (m_autoReconnect) {
        m_reconnectTimer->start(m_reconnectInterval);
    }
    
    emit disconnected();
}

void MotoTcpClient::onReadyRead()
{
    m_receiveBuffer.append(m_socket->readAll());
    processReceivedData();
}

void MotoTcpClient::onError(QAbstractSocket::SocketError error)
{
    m_lastError = m_socket->errorString();
    
    qCritical() << "MotoTcpClient: Socket错误:" << error << m_lastError;
    
    emit errorOccurred(m_lastError);
    
    if (m_autoReconnect && error != QAbstractSocket::RemoteHostClosedError) {
        m_reconnectTimer->start(m_reconnectInterval);
    }
}

void MotoTcpClient::onReconnectTimer()
{
    qDebug() << "MotoTcpClient: 尝试重连...";
    connectToHost(m_hostIp, m_hostPort);
}

void MotoTcpClient::onHeartbeatTimer()
{
    // 发送心跳命令
    sendCommand("PING");
}

void MotoTcpClient::processReceivedData()
{
    // 按换行符分割数据
    while (true) {
        int index = m_receiveBuffer.indexOf('\n');
        if (index < 0) {
            break;
        }
        
        QByteArray line = m_receiveBuffer.left(index).trimmed();
        m_receiveBuffer.remove(0, index + 1);
        
        if (!line.isEmpty()) {
            qDebug() << "MotoTcpClient: 收到数据:" << line;
            
            emit dataReceived(line);
            emit commandResponse(m_lastCommand, QString::fromUtf8(line));
        }
    }
}

} // namespace Robot
