#ifndef SAFETYPANEL_H
#define SAFETYPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QProgressBar>
#include <QTimer>
#include <QDateTime>

namespace UI {

/**
 * @brief 安全监控面板
 * 
 * 监控系统安全状态、碰撞检测、紧急停止等安全相关功能
 */
class SafetyPanel : public QWidget
{
    Q_OBJECT

public:
    explicit SafetyPanel(QWidget *parent = nullptr);

    // 安全状态更新接口
    void updateSafetyStatus(const QString& status);
    void addSafetyAlert(const QString& level, const QString& message);
    void updateCollisionStatus(bool detected, const QString& details = "");
    void updateEmergencyStopStatus(bool active);
    void updateSafeZoneStatus(const QString& zone, bool safe);
    void clearAlerts();

signals:
    void emergencyStopRequested();
    void safetyResetRequested();
    void alertAcknowledged(const QString& alertId);

private slots:
    void onEmergencyStop();
    void onSafetyReset();
    void onClearAlerts();
    void onAcknowledgeAlert();
    void updateSafetyMonitoring();

private:
    void setupUI();
    void setupSafetyStatus();
    void setupCollisionMonitoring();
    void setupEmergencyControls();
    void setupSafetyAlerts();
    void setupSafeZones();

private:
    QVBoxLayout* m_mainLayout;
    
    // 安全状态组
    QGroupBox* m_safetyStatusGroup;
    QLabel* m_overallStatusLabel;
    QLabel* m_lastCheckLabel;
    QPushButton* m_safetyResetButton;
    
    // 碰撞监控组
    QGroupBox* m_collisionGroup;
    QLabel* m_collisionStatusLabel;
    QLabel* m_collisionDetailsLabel;
    QProgressBar* m_collisionSensitivityBar;
    
    // 紧急控制组
    QGroupBox* m_emergencyGroup;
    QPushButton* m_emergencyStopButton;
    QLabel* m_emergencyStatusLabel;
    
    // 安全区域组
    QGroupBox* m_safeZoneGroup;
    QLabel* m_workspaceStatusLabel;
    QLabel* m_robotZoneStatusLabel;
    QLabel* m_humanZoneStatusLabel;
    
    // 安全警报组
    QGroupBox* m_alertGroup;
    QListWidget* m_alertListWidget;
    QHBoxLayout* m_alertButtonLayout;
    QPushButton* m_clearAlertsButton;
    QPushButton* m_acknowledgeButton;
    
    // 定时器
    QTimer* m_monitoringTimer;
    
    // 状态变量
    bool m_emergencyStopActive;
    bool m_collisionDetected;
    int m_alertCount;
};

} // namespace UI

#endif // SAFETYPANEL_H