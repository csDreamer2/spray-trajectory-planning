#include "SafetyPanel.h"
#include <QMessageBox>
#include <QListWidgetItem>
#include <QFont>

namespace UI {

SafetyPanel::SafetyPanel(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_emergencyStopActive(false)
    , m_collisionDetected(false)
    , m_alertCount(0)
{
    setupUI();
    
    // 启动安全监控定时器
    m_monitoringTimer = new QTimer(this);
    connect(m_monitoringTimer, &QTimer::timeout, this, &SafetyPanel::updateSafetyMonitoring);
    m_monitoringTimer->start(500); // 每500ms检查一次
    
    // 初始化安全状态
    updateSafetyStatus("系统安全");
    updateCollisionStatus(false);
    updateEmergencyStopStatus(false);
    updateSafeZoneStatus("工作区", true);
    updateSafeZoneStatus("机器人区", true);
    updateSafeZoneStatus("人员区", true);
}

void SafetyPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    
    setupSafetyStatus();
    setupCollisionMonitoring();
    setupEmergencyControls();
    setupSafeZones();
    setupSafetyAlerts();
    
    m_mainLayout->addStretch();
}

void SafetyPanel::setupSafetyStatus()
{
    m_safetyStatusGroup = new QGroupBox("安全状态", this);
    QVBoxLayout* statusLayout = new QVBoxLayout(m_safetyStatusGroup);
    
    // 总体安全状态
    m_overallStatusLabel = new QLabel("系统安全");
    m_overallStatusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; font-size: 14px; }");
    statusLayout->addWidget(m_overallStatusLabel);
    
    // 最后检查时间
    m_lastCheckLabel = new QLabel("最后检查: --");
    statusLayout->addWidget(m_lastCheckLabel);
    
    // 安全复位按钮
    m_safetyResetButton = new QPushButton("安全复位");
    m_safetyResetButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
    connect(m_safetyResetButton, &QPushButton::clicked, this, &SafetyPanel::onSafetyReset);
    statusLayout->addWidget(m_safetyResetButton);
    
    m_mainLayout->addWidget(m_safetyStatusGroup);
}

void SafetyPanel::setupCollisionMonitoring()
{
    m_collisionGroup = new QGroupBox("碰撞监控", this);
    QVBoxLayout* collisionLayout = new QVBoxLayout(m_collisionGroup);
    
    // 碰撞状态
    m_collisionStatusLabel = new QLabel("碰撞检测: 正常");
    m_collisionStatusLabel->setStyleSheet("QLabel { color: green; }");
    collisionLayout->addWidget(m_collisionStatusLabel);
    
    // 碰撞详情
    m_collisionDetailsLabel = new QLabel("无碰撞风险");
    m_collisionDetailsLabel->setWordWrap(true);
    collisionLayout->addWidget(m_collisionDetailsLabel);
    
    // 灵敏度指示器
    QLabel* sensitivityLabel = new QLabel("检测灵敏度:");
    collisionLayout->addWidget(sensitivityLabel);
    
    m_collisionSensitivityBar = new QProgressBar();
    m_collisionSensitivityBar->setRange(0, 100);
    m_collisionSensitivityBar->setValue(75);
    m_collisionSensitivityBar->setFormat("75%");
    collisionLayout->addWidget(m_collisionSensitivityBar);
    
    m_mainLayout->addWidget(m_collisionGroup);
}

void SafetyPanel::setupEmergencyControls()
{
    m_emergencyGroup = new QGroupBox("紧急控制", this);
    QVBoxLayout* emergencyLayout = new QVBoxLayout(m_emergencyGroup);
    
    // 紧急停止按钮
    m_emergencyStopButton = new QPushButton("紧急停止");
    m_emergencyStopButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #f44336;"
        "   color: white;"
        "   font-weight: bold;"
        "   font-size: 16px;"
        "   min-height: 50px;"
        "   border-radius: 25px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #d32f2f;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #b71c1c;"
        "}"
    );
    connect(m_emergencyStopButton, &QPushButton::clicked, this, &SafetyPanel::onEmergencyStop);
    emergencyLayout->addWidget(m_emergencyStopButton);
    
    // 紧急停止状态
    m_emergencyStatusLabel = new QLabel("紧急停止: 未激活");
    m_emergencyStatusLabel->setStyleSheet("QLabel { color: green; }");
    emergencyLayout->addWidget(m_emergencyStatusLabel);
    
    m_mainLayout->addWidget(m_emergencyGroup);
}

void SafetyPanel::setupSafeZones()
{
    m_safeZoneGroup = new QGroupBox("安全区域", this);
    QVBoxLayout* zoneLayout = new QVBoxLayout(m_safeZoneGroup);
    
    // 工作区状态
    m_workspaceStatusLabel = new QLabel("工作区: 安全");
    m_workspaceStatusLabel->setStyleSheet("QLabel { color: green; }");
    zoneLayout->addWidget(m_workspaceStatusLabel);
    
    // 机器人区状态
    m_robotZoneStatusLabel = new QLabel("机器人区: 安全");
    m_robotZoneStatusLabel->setStyleSheet("QLabel { color: green; }");
    zoneLayout->addWidget(m_robotZoneStatusLabel);
    
    // 人员区状态
    m_humanZoneStatusLabel = new QLabel("人员区: 安全");
    m_humanZoneStatusLabel->setStyleSheet("QLabel { color: green; }");
    zoneLayout->addWidget(m_humanZoneStatusLabel);
    
    m_mainLayout->addWidget(m_safeZoneGroup);
}

void SafetyPanel::setupSafetyAlerts()
{
    m_alertGroup = new QGroupBox("安全警报", this);
    QVBoxLayout* alertLayout = new QVBoxLayout(m_alertGroup);
    
    // 警报列表
    m_alertListWidget = new QListWidget();
    m_alertListWidget->setMaximumHeight(150);
    alertLayout->addWidget(m_alertListWidget);
    
    // 按钮布局
    m_alertButtonLayout = new QHBoxLayout();
    
    m_acknowledgeButton = new QPushButton("确认警报");
    connect(m_acknowledgeButton, &QPushButton::clicked, this, &SafetyPanel::onAcknowledgeAlert);
    m_alertButtonLayout->addWidget(m_acknowledgeButton);
    
    m_clearAlertsButton = new QPushButton("清空警报");
    connect(m_clearAlertsButton, &QPushButton::clicked, this, &SafetyPanel::onClearAlerts);
    m_alertButtonLayout->addWidget(m_clearAlertsButton);
    
    m_alertButtonLayout->addStretch();
    alertLayout->addLayout(m_alertButtonLayout);
    
    m_mainLayout->addWidget(m_alertGroup);
}

void SafetyPanel::updateSafetyStatus(const QString& status)
{
    m_overallStatusLabel->setText(status);
    
    // 根据状态设置颜色
    if (status.contains("安全") || status.contains("正常")) {
        m_overallStatusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; font-size: 14px; }");
    } else if (status.contains("警告") || status.contains("注意")) {
        m_overallStatusLabel->setStyleSheet("QLabel { color: orange; font-weight: bold; font-size: 14px; }");
    } else if (status.contains("危险") || status.contains("紧急")) {
        m_overallStatusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; font-size: 14px; }");
    }
    
    m_lastCheckLabel->setText(QString("最后检查: %1").arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
}

void SafetyPanel::addSafetyAlert(const QString& level, const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString alertText = QString("[%1] %2: %3").arg(timestamp, level, message);
    
    QListWidgetItem* item = new QListWidgetItem(alertText);
    
    // 根据级别设置颜色
    if (level == "ERROR" || level == "CRITICAL") {
        item->setBackground(QBrush(QColor(255, 235, 235))); // 浅红色
        item->setForeground(QBrush(QColor(200, 0, 0))); // 深红色
    } else if (level == "WARNING") {
        item->setBackground(QBrush(QColor(255, 248, 220))); // 浅黄色
        item->setForeground(QBrush(QColor(200, 100, 0))); // 橙色
    } else {
        item->setBackground(QBrush(QColor(240, 248, 255))); // 浅蓝色
        item->setForeground(QBrush(QColor(0, 100, 200))); // 蓝色
    }
    
    m_alertListWidget->addItem(item);
    m_alertListWidget->scrollToBottom();
    
    m_alertCount++;
    
    // 限制警报数量
    if (m_alertListWidget->count() > 100) {
        delete m_alertListWidget->takeItem(0);
    }
}

void SafetyPanel::updateCollisionStatus(bool detected, const QString& details)
{
    m_collisionDetected = detected;
    
    if (detected) {
        m_collisionStatusLabel->setText("碰撞检测: 检测到碰撞风险!");
        m_collisionStatusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
        m_collisionDetailsLabel->setText(details.isEmpty() ? "检测到潜在碰撞" : details);
        
        addSafetyAlert("WARNING", "检测到碰撞风险");
    } else {
        m_collisionStatusLabel->setText("碰撞检测: 正常");
        m_collisionStatusLabel->setStyleSheet("QLabel { color: green; }");
        m_collisionDetailsLabel->setText("无碰撞风险");
    }
}

void SafetyPanel::updateEmergencyStopStatus(bool active)
{
    m_emergencyStopActive = active;
    
    if (active) {
        m_emergencyStatusLabel->setText("紧急停止: 已激活");
        m_emergencyStatusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
        m_emergencyStopButton->setText("复位紧急停止");
        m_emergencyStopButton->setStyleSheet(
            "QPushButton {"
            "   background-color: #ff9800;"
            "   color: white;"
            "   font-weight: bold;"
            "   font-size: 16px;"
            "   min-height: 50px;"
            "   border-radius: 25px;"
            "}"
        );
    } else {
        m_emergencyStatusLabel->setText("紧急停止: 未激活");
        m_emergencyStatusLabel->setStyleSheet("QLabel { color: green; }");
        m_emergencyStopButton->setText("紧急停止");
        m_emergencyStopButton->setStyleSheet(
            "QPushButton {"
            "   background-color: #f44336;"
            "   color: white;"
            "   font-weight: bold;"
            "   font-size: 16px;"
            "   min-height: 50px;"
            "   border-radius: 25px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #d32f2f;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #b71c1c;"
            "}"
        );
    }
}

void SafetyPanel::updateSafeZoneStatus(const QString& zone, bool safe)
{
    QString status = safe ? "安全" : "警告";
    QString color = safe ? "green" : "red";
    
    if (zone == "工作区") {
        m_workspaceStatusLabel->setText(QString("工作区: %1").arg(status));
        m_workspaceStatusLabel->setStyleSheet(QString("QLabel { color: %1; }").arg(color));
    } else if (zone == "机器人区") {
        m_robotZoneStatusLabel->setText(QString("机器人区: %1").arg(status));
        m_robotZoneStatusLabel->setStyleSheet(QString("QLabel { color: %1; }").arg(color));
    } else if (zone == "人员区") {
        m_humanZoneStatusLabel->setText(QString("人员区: %1").arg(status));
        m_humanZoneStatusLabel->setStyleSheet(QString("QLabel { color: %1; }").arg(color));
    }
    
    if (!safe) {
        addSafetyAlert("WARNING", QString("%1检测到异常").arg(zone));
    }
}

void SafetyPanel::clearAlerts()
{
    m_alertListWidget->clear();
    m_alertCount = 0;
}

void SafetyPanel::onEmergencyStop()
{
    if (m_emergencyStopActive) {
        // 复位紧急停止
        int ret = QMessageBox::question(this, "确认复位", 
            "确定要复位紧急停止吗？\n请确保现场安全后再进行复位操作。",
            QMessageBox::Yes | QMessageBox::No);
        
        if (ret == QMessageBox::Yes) {
            updateEmergencyStopStatus(false);
            addSafetyAlert("INFO", "紧急停止已复位");
            emit safetyResetRequested();
        }
    } else {
        // 激活紧急停止
        updateEmergencyStopStatus(true);
        addSafetyAlert("CRITICAL", "紧急停止已激活");
        emit emergencyStopRequested();
    }
}

void SafetyPanel::onSafetyReset()
{
    int ret = QMessageBox::question(this, "安全复位", 
        "确定要进行安全复位吗？\n这将重置所有安全状态。",
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        updateSafetyStatus("系统安全");
        updateCollisionStatus(false);
        updateEmergencyStopStatus(false);
        updateSafeZoneStatus("工作区", true);
        updateSafeZoneStatus("机器人区", true);
        updateSafeZoneStatus("人员区", true);
        
        addSafetyAlert("INFO", "安全系统已复位");
        emit safetyResetRequested();
    }
}

void SafetyPanel::onClearAlerts()
{
    clearAlerts();
    addSafetyAlert("INFO", "警报列表已清空");
}

void SafetyPanel::onAcknowledgeAlert()
{
    QListWidgetItem* currentItem = m_alertListWidget->currentItem();
    if (currentItem) {
        QString alertText = currentItem->text();
        currentItem->setBackground(QBrush(QColor(240, 240, 240))); // 灰色表示已确认
        
        addSafetyAlert("INFO", "警报已确认");
        emit alertAcknowledged(alertText);
    }
}

void SafetyPanel::updateSafetyMonitoring()
{
    // 定期更新安全监控状态
    // 这里可以添加实际的安全检查逻辑
    
    // 更新最后检查时间
    m_lastCheckLabel->setText(QString("最后检查: %1").arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
}

} // namespace UI