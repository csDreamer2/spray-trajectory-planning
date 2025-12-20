#ifndef UNITYWIDGET_H
#define UNITYWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QProcess>
#include <QTimer>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace UI {

class QtUnityBridge;

/**
 * @brief Unity 3D视图嵌入组件
 * 
 * 负责在Qt界面中嵌入Unity 3D渲染窗口，
 * 提供3D场景显示和交互功能
 */
class UnityWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UnityWidget(QWidget *parent = nullptr);
    ~UnityWidget();

    // Unity场景控制
    bool InitializeUnity();
    void ShowWorkpiece(const QString& workpieceData);
    void ShowTrajectory(const QString& trajectoryData);
    void StartSimulation();
    void StopSimulation();
    void ResetView();
    
    // 设置通信桥
    void SetBridge(QtUnityBridge* bridge);

signals:
    void UnityReady();
    void UnityError(const QString& error);
    void SceneClicked(const QPoint& position);

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void OnUnityProcessStarted();
    void OnUnityProcessFinished(int exitCode);
    void OnUnityProcessError(QProcess::ProcessError error);

private slots:
    void OnBridgeConnected();
    void OnBridgeDisconnected();
    void OnBridgeError(const QString& error);
    void OnWorkpieceLoaded(bool success, const QString& message);
    void OnTrajectoryDisplayed(bool success, const QString& message);

private:
    void setupUI();
    void setupUnityBridge();
    void updateConnectionStatus();

private:
    QVBoxLayout* m_layout;
    QLabel* m_placeholderLabel;
    QPushButton* m_initButton;
    QLabel* m_statusLabel;
    
    // Unity进程管理
    QProcess* m_unityProcess;
    bool m_unityInitialized;
    
    // Unity窗口句柄 (Windows)
    WId m_unityWindowId;
    
    // Unity通信桥
    QtUnityBridge* m_bridge;
    
    // Unity独立窗口模式，不需要窗口句柄
};

} // namespace UI

#endif // UNITYWIDGET_H