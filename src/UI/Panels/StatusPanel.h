#ifndef STATUSPANEL_H
#define STATUSPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QTextEdit>
#include <QListWidget>
#include <QPushButton>
#include <QTimer>
#include <QDateTime>

namespace UI {

/**
 * @brief 状态监控面板
 * 
 * 显示系统运行状态、任务进度、日志信息等
 */
class StatusPanel : public QWidget
{
    Q_OBJECT

public:
    explicit StatusPanel(QWidget *parent = nullptr);

    // 状态更新接口
    void updateSystemStatus(const QString& status);
    void updateTaskProgress(const QString& taskName, int progress);
    void updateRobotStatus(const QString& status, bool connected);
    void addLogMessage(const QString& level, const QString& message);
    void clearLogs();

signals:
    void statusClicked(const QString& statusType);
    void logExportRequested();

private slots:
    void onRefreshStatus();
    void onClearLogs();
    void onExportLogs();
    void updateDateTime();

private:
    void setupUI();
    void setupSystemStatus();
    void setupTaskProgress();
    void setupConnectionStatus();
    void setupLogViewer();

private:
    QVBoxLayout* m_mainLayout;
    
    // 系统状态组
    QGroupBox* m_systemGroup;
    QLabel* m_systemStatusLabel;
    QLabel* m_dateTimeLabel;
    QLabel* m_uptimeLabel;
    QPushButton* m_refreshButton;
    
    // 任务进度组
    QGroupBox* m_taskGroup;
    QLabel* m_currentTaskLabel;
    QProgressBar* m_taskProgressBar;
    QLabel* m_taskDetailsLabel;
    
    // 连接状态组
    QGroupBox* m_connectionGroup;
    QLabel* m_robotStatusLabel;
    QLabel* m_databaseStatusLabel;
    
    // 日志查看器组
    QGroupBox* m_logGroup;
    QTextEdit* m_logTextEdit;
    QHBoxLayout* m_logButtonLayout;
    QPushButton* m_clearLogsButton;
    QPushButton* m_exportLogsButton;
    
    // 定时器
    QTimer* m_updateTimer;
    QDateTime m_startTime;
};

} // namespace UI

#endif // STATUSPANEL_H