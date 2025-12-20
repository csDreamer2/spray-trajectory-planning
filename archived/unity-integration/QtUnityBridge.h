#ifndef QTUNITYBRIDGE_H
#define QTUNITYBRIDGE_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>

namespace UI {

/**
 * @brief Qt与Unity 3D通信桥梁
 * 
 * 负责Qt应用与Unity 3D引擎之间的双向通信
 * 使用TCP Socket进行数据交换
 */
class QtUnityBridge : public QObject
{
    Q_OBJECT

public:
    explicit QtUnityBridge(QObject *parent = nullptr);
    ~QtUnityBridge();

    // 连接管理
    bool StartServer(quint16 port = 12346);
    void StopServer();
    bool IsConnected() const;
    quint16 GetServerPort() const { return m_serverPort; }

    // 数据发送到Unity
    void SendWorkpieceData(const QJsonObject& workpieceData);
    void SendTrajectoryData(const QJsonObject& trajectoryData);
    void SendSimulationCommand(const QString& command, const QJsonObject& parameters = QJsonObject());
    void SendCameraCommand(const QString& command, const QJsonObject& parameters = QJsonObject());

    // 仿真控制
    void StartSimulation();
    void StopSimulation();
    void PauseSimulation();
    void ResetSimulation();
    void SetSimulationSpeed(float speed);

    // 场景控制
    void LoadWorkpiece(const QString& filePath);
    void ShowTrajectory(const QJsonObject& trajectoryData);
    void SetCameraView(const QString& viewType); // "front", "side", "top", "perspective"
    void ResetCamera();

signals:
    // 连接状态
    void UnityConnected();
    void UnityDisconnected();
    void ConnectionError(const QString& error);

    // Unity反馈信号
    void CollisionDetected(const QJsonObject& collisionData);
    void SimulationComplete(const QJsonObject& resultData);
    void QualityPrediction(const QJsonObject& qualityData);
    void SceneClicked(const QJsonObject& clickData);
    void CameraViewChanged(const QString& viewType);

    // 数据接收
    void WorkpieceLoaded(bool success, const QString& message);
    void TrajectoryDisplayed(bool success, const QString& message);

private slots:
    void OnNewConnection();
    void OnClientConnected();
    void OnClientDisconnected();
    void OnDataReceived();
    void OnSocketError(QAbstractSocket::SocketError error);
    void CheckConnection();

private:
    void ProcessUnityMessage(const QJsonObject& message);
    void SendMessage(const QJsonObject& message);
    void SetupHeartbeat();

private:
    QTcpServer* m_server;
    QTcpSocket* m_unitySocket;
    QTimer* m_heartbeatTimer;
    
    bool m_isConnected;
    quint16 m_serverPort = 12346;
    
    // 消息缓冲
    QByteArray m_messageBuffer;
};

} // namespace UI

#endif // QTUNITYBRIDGE_H