#ifndef MOTOTCPCLIENT_H
#define MOTOTCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QQueue>

namespace Robot {

/**
 * @brief 安川MotoPlus TCP通信客户端
 * 
 * 实现与安川机器人的TCP通信协议
 * - 支持命令发送和响应接收
 * - 自动重连机制
 * - 心跳检测
 */
class MotoTcpClient : public QObject
{
    Q_OBJECT

public:
    explicit MotoTcpClient(QObject* parent = nullptr);
    ~MotoTcpClient();

    /**
     * @brief 连接到机器人
     * @param ip IP地址
     * @param port 端口号
     * @return 是否开始连接
     */
    bool connectToHost(const QString& ip, int port);

    /**
     * @brief 断开连接
     */
    void disconnectFromHost();

    /**
     * @brief 是否已连接
     */
    bool isConnected() const;

    /**
     * @brief 发送命令
     * @param command 命令字符串
     * @return 是否发送成功
     */
    bool sendCommand(const QString& command);

    /**
     * @brief 发送原始数据
     * @param data 数据
     * @return 是否发送成功
     */
    bool sendData(const QByteArray& data);

    /**
     * @brief 设置自动重连
     * @param enable 是否启用
     * @param interval 重连间隔 (ms)
     */
    void setAutoReconnect(bool enable, int interval = 5000);

    /**
     * @brief 设置心跳间隔
     * @param interval 间隔 (ms)，0表示禁用
     */
    void setHeartbeatInterval(int interval);

    /**
     * @brief 获取最后错误信息
     */
    QString lastError() const;

signals:
    /**
     * @brief 连接成功
     */
    void connected();

    /**
     * @brief 连接断开
     */
    void disconnected();

    /**
     * @brief 错误发生
     */
    void errorOccurred(const QString& error);

    /**
     * @brief 收到数据
     */
    void dataReceived(const QByteArray& data);

    /**
     * @brief 命令响应
     */
    void commandResponse(const QString& command, const QString& response);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
    void onReconnectTimer();
    void onHeartbeatTimer();

private:
    void processReceivedData();

private:
    QTcpSocket* m_socket;
    QTimer* m_reconnectTimer;
    QTimer* m_heartbeatTimer;
    
    QString m_hostIp;
    int m_hostPort;
    
    bool m_autoReconnect;
    int m_reconnectInterval;
    int m_heartbeatInterval;
    
    QByteArray m_receiveBuffer;
    QString m_lastError;
    QString m_lastCommand;
};

} // namespace Robot

#endif // MOTOTCPCLIENT_H
