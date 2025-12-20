#include "StatusPanel.h"
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QScrollBar>

namespace UI {

StatusPanel::StatusPanel(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_startTime(QDateTime::currentDateTime())
{
    setupUI();
    
    // 启动定时器更新时间和状态
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &StatusPanel::updateDateTime);
    m_updateTimer->start(1000); // 每秒更新
    
    // 初始化状态
    updateSystemStatus("系统就绪");
    updateRobotStatus("未连接", false);
    addLogMessage("INFO", "状态监控面板已初始化");
}

void StatusPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    
    setupSystemStatus();
    setupTaskProgress();
    setupConnectionStatus();
    setupLogViewer();
    
    m_mainLayout->addStretch();
}

void StatusPanel::setupSystemStatus()
{
    m_systemGroup = new QGroupBox("系统状态", this);
    QVBoxLayout* systemLayout = new QVBoxLayout(m_systemGroup);
    
    // 系统状态
    m_systemStatusLabel = new QLabel("系统: 就绪");
    m_systemStatusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; }");
    systemLayout->addWidget(m_systemStatusLabel);
    
    // 当前时间
    m_dateTimeLabel = new QLabel();
    systemLayout->addWidget(m_dateTimeLabel);
    
    // 运行时间
    m_uptimeLabel = new QLabel();
    systemLayout->addWidget(m_uptimeLabel);
    
    // 刷新按钮
    m_refreshButton = new QPushButton("刷新状态");
    connect(m_refreshButton, &QPushButton::clicked, this, &StatusPanel::onRefreshStatus);
    systemLayout->addWidget(m_refreshButton);
    
    m_mainLayout->addWidget(m_systemGroup);
}

void StatusPanel::setupTaskProgress()
{
    m_taskGroup = new QGroupBox("任务进度", this);
    QVBoxLayout* taskLayout = new QVBoxLayout(m_taskGroup);
    
    // 当前任务
    m_currentTaskLabel = new QLabel("当前任务: 无");
    taskLayout->addWidget(m_currentTaskLabel);
    
    // 进度条
    m_taskProgressBar = new QProgressBar();
    m_taskProgressBar->setRange(0, 100);
    m_taskProgressBar->setValue(0);
    taskLayout->addWidget(m_taskProgressBar);
    
    // 任务详情
    m_taskDetailsLabel = new QLabel("等待任务...");
    m_taskDetailsLabel->setWordWrap(true);
    taskLayout->addWidget(m_taskDetailsLabel);
    
    m_mainLayout->addWidget(m_taskGroup);
}

void StatusPanel::setupConnectionStatus()
{
    m_connectionGroup = new QGroupBox("连接状态", this);
    QVBoxLayout* connectionLayout = new QVBoxLayout(m_connectionGroup);
    
    // 机器人连接状态
    m_robotStatusLabel = new QLabel("机器人: 未连接");
    m_robotStatusLabel->setStyleSheet("QLabel { color: red; }");
    connectionLayout->addWidget(m_robotStatusLabel);
    
    // 数据库连接状态
    m_databaseStatusLabel = new QLabel("数据库: 已连接");
    m_databaseStatusLabel->setStyleSheet("QLabel { color: green; }");
    connectionLayout->addWidget(m_databaseStatusLabel);
    
    m_mainLayout->addWidget(m_connectionGroup);
}

void StatusPanel::setupLogViewer()
{
    m_logGroup = new QGroupBox("系统日志", this);
    QVBoxLayout* logLayout = new QVBoxLayout(m_logGroup);
    
    // 日志文本框
    m_logTextEdit = new QTextEdit();
    m_logTextEdit->setMaximumHeight(200);
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setFont(QFont("Consolas", 9));
    logLayout->addWidget(m_logTextEdit);
    
    // 按钮布局
    m_logButtonLayout = new QHBoxLayout();
    
    m_clearLogsButton = new QPushButton("清空日志");
    connect(m_clearLogsButton, &QPushButton::clicked, this, &StatusPanel::onClearLogs);
    m_logButtonLayout->addWidget(m_clearLogsButton);
    
    m_exportLogsButton = new QPushButton("导出日志");
    connect(m_exportLogsButton, &QPushButton::clicked, this, &StatusPanel::onExportLogs);
    m_logButtonLayout->addWidget(m_exportLogsButton);
    
    m_logButtonLayout->addStretch();
    logLayout->addLayout(m_logButtonLayout);
    
    m_mainLayout->addWidget(m_logGroup);
}

void StatusPanel::updateSystemStatus(const QString& status)
{
    m_systemStatusLabel->setText(QString("系统: %1").arg(status));
    
    // 根据状态设置颜色
    if (status.contains("就绪") || status.contains("正常")) {
        m_systemStatusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; }");
    } else if (status.contains("警告") || status.contains("忙碌")) {
        m_systemStatusLabel->setStyleSheet("QLabel { color: orange; font-weight: bold; }");
    } else if (status.contains("错误") || status.contains("故障")) {
        m_systemStatusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    } else {
        m_systemStatusLabel->setStyleSheet("QLabel { color: blue; font-weight: bold; }");
    }
}

void StatusPanel::updateTaskProgress(const QString& taskName, int progress)
{
    m_currentTaskLabel->setText(QString("当前任务: %1").arg(taskName));
    m_taskProgressBar->setValue(progress);
    
    if (progress == 0) {
        m_taskDetailsLabel->setText("等待任务开始...");
    } else if (progress == 100) {
        m_taskDetailsLabel->setText("任务已完成");
    } else {
        m_taskDetailsLabel->setText(QString("进度: %1% - 正在执行中...").arg(progress));
    }
}

void StatusPanel::updateRobotStatus(const QString& status, bool connected)
{
    m_robotStatusLabel->setText(QString("机器人: %1").arg(status));
    m_robotStatusLabel->setStyleSheet(connected ? 
        "QLabel { color: green; }" : "QLabel { color: red; }");
}

void StatusPanel::addLogMessage(const QString& level, const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString logEntry = QString("[%1] %2: %3").arg(timestamp, level, message);
    
    // 添加到日志文本框
    m_logTextEdit->append(logEntry);
    
    // 自动滚动到底部
    QScrollBar* scrollBar = m_logTextEdit->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
    
    // 限制日志行数（保持最近1000行）
    QTextDocument* doc = m_logTextEdit->document();
    if (doc->blockCount() > 1000) {
        QTextCursor cursor = m_logTextEdit->textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, 100);
        cursor.removeSelectedText();
    }
}

void StatusPanel::clearLogs()
{
    m_logTextEdit->clear();
    addLogMessage("INFO", "日志已清空");
}

void StatusPanel::onRefreshStatus()
{
    addLogMessage("INFO", "手动刷新系统状态");
    emit statusClicked("refresh");
}

void StatusPanel::onClearLogs()
{
    clearLogs();
}

void StatusPanel::onExportLogs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "导出日志", 
        QString("system_log_%1.txt").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "文本文件 (*.txt)");
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << m_logTextEdit->toPlainText();
            file.close();
            
            addLogMessage("INFO", QString("日志已导出到: %1").arg(fileName));
            QMessageBox::information(this, "导出成功", "日志文件已成功导出!");
        } else {
            addLogMessage("ERROR", QString("无法导出日志到: %1").arg(fileName));
            QMessageBox::warning(this, "导出失败", "无法写入日志文件!");
        }
    }
}

void StatusPanel::updateDateTime()
{
    // 更新当前时间
    QDateTime now = QDateTime::currentDateTime();
    m_dateTimeLabel->setText(QString("当前时间: %1").arg(now.toString("yyyy-MM-dd hh:mm:ss")));
    
    // 更新运行时间
    qint64 uptime = m_startTime.secsTo(now);
    int hours = uptime / 3600;
    int minutes = (uptime % 3600) / 60;
    int seconds = uptime % 60;
    m_uptimeLabel->setText(QString("运行时间: %1:%2:%3")
                          .arg(hours, 2, 10, QChar('0'))
                          .arg(minutes, 2, 10, QChar('0'))
                          .arg(seconds, 2, 10, QChar('0')));
}

} // namespace UI