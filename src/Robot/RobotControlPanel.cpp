#include "RobotControlPanel.h"
#include "RobotController.h"
#include <QDebug>
#include <QMessageBox>
#include <QTimer>
#include <QScrollArea>

namespace Robot {

// ==================== JointControlWidget ====================

JointControlWidget::JointControlWidget(int jointIndex, const QString& name,
                                       double minAngle, double maxAngle,
                                       QWidget* parent)
    : QWidget(parent)
    , m_jointIndex(jointIndex)
    , m_minAngle(minAngle)
    , m_maxAngle(maxAngle)
    , m_updating(false)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 2, 0, 2);
    layout->setSpacing(8);
    
    // 关节名称
    m_nameLabel = new QLabel(name, this);
    m_nameLabel->setFixedWidth(40);
    m_nameLabel->setStyleSheet("QLabel { color: #ffffff; font-weight: bold; }");
    
    // 滑动条
    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setRange(static_cast<int>(minAngle * 10), static_cast<int>(maxAngle * 10));
    m_slider->setValue(0);
    m_slider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "   border: 1px solid #555555;"
        "   height: 8px;"
        "   background: #2b2b2b;"
        "   border-radius: 4px;"
        "}"
        "QSlider::handle:horizontal {"
        "   background: #0d47a1;"
        "   border: 1px solid #0d47a1;"
        "   width: 16px;"
        "   margin: -4px 0;"
        "   border-radius: 8px;"
        "}"
        "QSlider::handle:horizontal:hover {"
        "   background: #1565c0;"
        "}"
        "QSlider::sub-page:horizontal {"
        "   background: #1976d2;"
        "   border-radius: 4px;"
        "}"
    );
    
    // 数值输入框
    m_spinBox = new QDoubleSpinBox(this);
    m_spinBox->setRange(minAngle, maxAngle);
    m_spinBox->setDecimals(1);
    m_spinBox->setSuffix("°");
    m_spinBox->setFixedWidth(80);
    m_spinBox->setStyleSheet(
        "QDoubleSpinBox {"
        "   background-color: #2b2b2b;"
        "   color: #ffffff;"
        "   border: 1px solid #555555;"
        "   border-radius: 3px;"
        "   padding: 2px;"
        "}"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {"
        "   background-color: #3d3d3d;"
        "   border: none;"
        "}"
    );
    
    // 范围标签
    m_rangeLabel = new QLabel(QString("[%1, %2]").arg(minAngle, 0, 'f', 0).arg(maxAngle, 0, 'f', 0), this);
    m_rangeLabel->setFixedWidth(80);
    m_rangeLabel->setStyleSheet("QLabel { color: #888888; font-size: 10px; }");
    
    layout->addWidget(m_nameLabel);
    layout->addWidget(m_slider, 1);
    layout->addWidget(m_spinBox);
    layout->addWidget(m_rangeLabel);
    
    // 连接信号
    connect(m_slider, &QSlider::valueChanged, this, &JointControlWidget::onSliderChanged);
    connect(m_spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &JointControlWidget::onSpinBoxChanged);
}

void JointControlWidget::setAngle(double angle)
{
    m_updating = true;
    m_slider->setValue(static_cast<int>(angle * 10));
    m_spinBox->setValue(angle);
    m_updating = false;
}

double JointControlWidget::getAngle() const
{
    return m_spinBox->value();
}

void JointControlWidget::setEnabled(bool enabled)
{
    m_slider->setEnabled(enabled);
    m_spinBox->setEnabled(enabled);
}

void JointControlWidget::onSliderChanged(int value)
{
    if (m_updating) return;
    
    double angle = value / 10.0;
    m_updating = true;
    m_spinBox->setValue(angle);
    m_updating = false;
    
    emit angleChanged(m_jointIndex, angle);
}

void JointControlWidget::onSpinBoxChanged(double value)
{
    if (m_updating) return;
    
    m_updating = true;
    m_slider->setValue(static_cast<int>(value * 10));
    m_updating = false;
    
    emit angleChanged(m_jointIndex, value);
}

// ==================== RobotControlPanel ====================

RobotControlPanel::RobotControlPanel(QWidget* parent)
    : QWidget(parent)
    , m_controller(nullptr)
    , m_updating(false)
{
    setupUI();
    setupStyles();
    connectSignals();
}

RobotControlPanel::~RobotControlPanel()
{
}

void RobotControlPanel::setRobotController(RobotController* controller)
{
    if (m_controller) {
        disconnect(m_controller, nullptr, this, nullptr);
    }
    
    m_controller = controller;
    
    if (m_controller) {
        // 连接控制器信号
        connect(m_controller, &RobotController::jointAnglesChanged,
                this, &RobotControlPanel::onControllerJointAnglesChanged);
        connect(m_controller, &RobotController::endEffectorPoseChanged,
                this, &RobotControlPanel::onControllerPoseChanged);
        connect(m_controller, &RobotController::connectionStateChanged,
                this, [this](ConnectionState state) {
                    onControllerConnectionStateChanged(static_cast<int>(state));
                });
        connect(m_controller, &RobotController::errorOccurred,
                this, &RobotControlPanel::onControllerError);
        
        // 初始化关节限位
        for (int i = 0; i < 6; ++i) {
            JointLimit limit = m_controller->getJointLimit(i);
            // 更新滑动条范围（如果需要）
        }
        
        // 更新当前角度
        auto angles = m_controller->getJointAngles();
        onControllerJointAnglesChanged(angles);
        
        // 更新末端位姿
        EndEffectorPose pose = m_controller->getEndEffectorPose();
        onControllerPoseChanged(pose);
    }
}

RobotController* RobotControlPanel::getRobotController() const
{
    return m_controller;
}

void RobotControlPanel::setupUI()
{
    // 创建主布局
    QVBoxLayout* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);
    
    // 创建滚动区域
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet(
        "QScrollArea { background-color: transparent; border: none; }"
        "QScrollBar:vertical {"
        "   background-color: #2b2b2b;"
        "   width: 10px;"
        "   border-radius: 5px;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background-color: #555555;"
        "   border-radius: 5px;"
        "   min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "   background-color: #777777;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "   height: 0px;"
        "}"
    );
    
    // 创建内容容器
    QWidget* contentWidget = new QWidget();
    m_mainLayout = new QVBoxLayout(contentWidget);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(10);
    
    setupConnectionPanel();
    setupJointControls();
    setupPoseDisplay();
    setupControlButtons();
    
    m_mainLayout->addStretch();
    
    scrollArea->setWidget(contentWidget);
    outerLayout->addWidget(scrollArea);
}

void RobotControlPanel::setupJointControls()
{
    m_jointGroup = new QGroupBox("关节控制", this);
    QVBoxLayout* layout = new QVBoxLayout(m_jointGroup);
    layout->setSpacing(4);
    
    // 关节名称和默认限位
    struct JointInfo {
        QString name;
        double min;
        double max;
    };
    
    std::array<JointInfo, 6> jointInfos = {{
        {"J1", -180, 180},
        {"J2", -90, 155},
        {"J3", -175, 90},
        {"J4", -200, 200},
        {"J5", -150, 150},
        {"J6", -455, 455}
    }};
    
    for (int i = 0; i < 6; ++i) {
        m_jointWidgets[i] = new JointControlWidget(
            i, jointInfos[i].name, jointInfos[i].min, jointInfos[i].max, this);
        
        connect(m_jointWidgets[i], &JointControlWidget::angleChanged,
                this, &RobotControlPanel::onJointAngleChanged);
        
        layout->addWidget(m_jointWidgets[i]);
    }
    
    m_mainLayout->addWidget(m_jointGroup);
}

void RobotControlPanel::setupPoseDisplay()
{
    m_poseGroup = new QGroupBox("末端位姿", this);
    QGridLayout* layout = new QGridLayout(m_poseGroup);
    layout->setSpacing(8);
    
    QString labelStyle = "QLabel { color: #ffffff; }";
    QString valueStyle = "QLabel { color: #4fc3f7; font-family: monospace; }";
    
    // 位置
    QLabel* posLabel = new QLabel("位置 (mm):", this);
    posLabel->setStyleSheet(labelStyle);
    layout->addWidget(posLabel, 0, 0);
    
    layout->addWidget(new QLabel("X:", this), 0, 1);
    m_posXLabel = new QLabel("0.00", this);
    m_posXLabel->setStyleSheet(valueStyle);
    layout->addWidget(m_posXLabel, 0, 2);
    
    layout->addWidget(new QLabel("Y:", this), 0, 3);
    m_posYLabel = new QLabel("0.00", this);
    m_posYLabel->setStyleSheet(valueStyle);
    layout->addWidget(m_posYLabel, 0, 4);
    
    layout->addWidget(new QLabel("Z:", this), 0, 5);
    m_posZLabel = new QLabel("0.00", this);
    m_posZLabel->setStyleSheet(valueStyle);
    layout->addWidget(m_posZLabel, 0, 6);
    
    // 姿态
    QLabel* rotLabel = new QLabel("姿态 (°):", this);
    rotLabel->setStyleSheet(labelStyle);
    layout->addWidget(rotLabel, 1, 0);
    
    layout->addWidget(new QLabel("R:", this), 1, 1);
    m_rotRLabel = new QLabel("0.00", this);
    m_rotRLabel->setStyleSheet(valueStyle);
    layout->addWidget(m_rotRLabel, 1, 2);
    
    layout->addWidget(new QLabel("P:", this), 1, 3);
    m_rotPLabel = new QLabel("0.00", this);
    m_rotPLabel->setStyleSheet(valueStyle);
    layout->addWidget(m_rotPLabel, 1, 4);
    
    layout->addWidget(new QLabel("Y:", this), 1, 5);
    m_rotYLabel = new QLabel("0.00", this);
    m_rotYLabel->setStyleSheet(valueStyle);
    layout->addWidget(m_rotYLabel, 1, 6);
    
    m_mainLayout->addWidget(m_poseGroup);
}

void RobotControlPanel::setupConnectionPanel()
{
    m_connectionGroup = new QGroupBox("连接设置", this);
    QGridLayout* layout = new QGridLayout(m_connectionGroup);
    layout->setSpacing(8);
    
    // 模式选择
    layout->addWidget(new QLabel("模式:", this), 0, 0);
    m_modeCombo = new QComboBox(this);
    m_modeCombo->addItem("仿真模式", static_cast<int>(OperationMode::Simulation));
    m_modeCombo->addItem("远程控制", static_cast<int>(OperationMode::Remote));
    m_modeCombo->addItem("示教模式", static_cast<int>(OperationMode::Teach));
    layout->addWidget(m_modeCombo, 0, 1, 1, 3);
    
    // IP地址
    layout->addWidget(new QLabel("IP:", this), 1, 0);
    m_ipEdit = new QLineEdit("192.168.1.100", this);
    m_ipEdit->setPlaceholderText("机器人IP地址");
    layout->addWidget(m_ipEdit, 1, 1, 1, 2);
    
    // 端口
    layout->addWidget(new QLabel("端口:", this), 1, 3);
    m_portEdit = new QLineEdit("10040", this);
    m_portEdit->setFixedWidth(60);
    layout->addWidget(m_portEdit, 1, 4);
    
    // 连接按钮
    m_connectBtn = new QPushButton("连接", this);
    m_disconnectBtn = new QPushButton("断开", this);
    m_disconnectBtn->setEnabled(false);
    
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addWidget(m_connectBtn);
    btnLayout->addWidget(m_disconnectBtn);
    layout->addLayout(btnLayout, 2, 0, 1, 5);
    
    // 状态标签
    m_statusLabel = new QLabel("未连接", this);
    m_statusLabel->setStyleSheet("QLabel { color: #ff9800; font-weight: bold; }");
    layout->addWidget(m_statusLabel, 3, 0, 1, 5);
    
    m_mainLayout->addWidget(m_connectionGroup);
}

void RobotControlPanel::setupControlButtons()
{
    m_controlGroup = new QGroupBox("运动控制", this);
    QGridLayout* layout = new QGridLayout(m_controlGroup);
    layout->setSpacing(8);
    
    m_homeBtn = new QPushButton("回零位", this);
    m_stopBtn = new QPushButton("停止", this);
    m_servoOnBtn = new QPushButton("伺服ON", this);
    m_servoOffBtn = new QPushButton("伺服OFF", this);
    m_loadRobotModelBtn = new QPushButton("加载机器人模型", this);
    
    // 停止按钮特殊样式
    m_stopBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #c62828;"
        "   color: white;"
        "   font-weight: bold;"
        "   padding: 8px 16px;"
        "   border: none;"
        "   border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #e53935;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #b71c1c;"
        "}"
    );
    
    // 加载模型按钮特殊样式
    m_loadRobotModelBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #1976d2;"
        "   color: white;"
        "   font-weight: bold;"
        "   padding: 8px 16px;"
        "   border: none;"
        "   border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #1565c0;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #0d47a1;"
        "}"
    );
    
    layout->addWidget(m_homeBtn, 0, 0);
    layout->addWidget(m_stopBtn, 0, 1);
    layout->addWidget(m_servoOnBtn, 1, 0);
    layout->addWidget(m_servoOffBtn, 1, 1);
    layout->addWidget(m_loadRobotModelBtn, 2, 0, 1, 2);
    
    m_mainLayout->addWidget(m_controlGroup);
}

void RobotControlPanel::setupStyles()
{
    // 整体深色主题
    setStyleSheet(
        "RobotControlPanel {"
        "   background-color: #1e1e1e;"
        "}"
        "QGroupBox {"
        "   color: #ffffff;"
        "   font-weight: bold;"
        "   border: 1px solid #555555;"
        "   border-radius: 4px;"
        "   margin-top: 8px;"
        "   padding-top: 8px;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   left: 10px;"
        "   padding: 0 5px;"
        "}"
        "QLabel {"
        "   color: #cccccc;"
        "}"
        "QLineEdit {"
        "   background-color: #2b2b2b;"
        "   color: #ffffff;"
        "   border: 1px solid #555555;"
        "   border-radius: 3px;"
        "   padding: 4px;"
        "}"
        "QComboBox {"
        "   background-color: #2b2b2b;"
        "   color: #ffffff;"
        "   border: 1px solid #555555;"
        "   border-radius: 3px;"
        "   padding: 4px;"
        "}"
        "QComboBox::drop-down {"
        "   border: none;"
        "}"
        "QComboBox QAbstractItemView {"
        "   background-color: #2b2b2b;"
        "   color: #ffffff;"
        "   selection-background-color: #0d47a1;"
        "}"
        "QPushButton {"
        "   background-color: #2b2b2b;"
        "   color: #ffffff;"
        "   border: 1px solid #555555;"
        "   border-radius: 4px;"
        "   padding: 6px 12px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #3d3d3d;"
        "   border-color: #777777;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #1e1e1e;"
        "}"
        "QPushButton:disabled {"
        "   background-color: #1a1a1a;"
        "   color: #666666;"
        "}"
    );
}

void RobotControlPanel::connectSignals()
{
    connect(m_connectBtn, &QPushButton::clicked, this, &RobotControlPanel::onConnectClicked);
    connect(m_disconnectBtn, &QPushButton::clicked, this, &RobotControlPanel::onDisconnectClicked);
    connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RobotControlPanel::onModeChanged);
    connect(m_homeBtn, &QPushButton::clicked, this, &RobotControlPanel::onHomeClicked);
    connect(m_stopBtn, &QPushButton::clicked, this, &RobotControlPanel::onStopClicked);
    connect(m_servoOnBtn, &QPushButton::clicked, this, &RobotControlPanel::onServoOnClicked);
    connect(m_servoOffBtn, &QPushButton::clicked, this, &RobotControlPanel::onServoOffClicked);
    connect(m_loadRobotModelBtn, &QPushButton::clicked, this, [this]() {
        emit loadRobotModelRequested();
    });
    
    // 初始化时设置为仿真模式
    QTimer::singleShot(0, this, [this]() {
        onModeChanged(0);  // 触发仿真模式初始化
    });
}

void RobotControlPanel::onJointAngleChanged(int jointIndex, double angle)
{
    if (m_updating) return;
    
    if (m_controller) {
        m_controller->setJointAngle(jointIndex, angle);
    }
    
    // 发送信号用于3D仿真更新
    std::array<double, 6> angles;
    for (int i = 0; i < 6; ++i) {
        angles[i] = m_jointWidgets[i]->getAngle();
    }
    emit jointAnglesChanged(angles);
}

void RobotControlPanel::onConnectClicked()
{
    // 检查是否为仿真模式
    OperationMode mode = static_cast<OperationMode>(m_modeCombo->currentData().toInt());
    if (mode == OperationMode::Simulation) {
        QMessageBox::information(this, "提示", "当前为仿真模式，无需连接真实机器人。\n如需连接真实机器人，请切换到远程控制或示教模式。");
        return;
    }
    
    QString ip = m_ipEdit->text().trimmed();
    int port = m_portEdit->text().toInt();
    
    if (ip.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入机器人IP地址");
        return;
    }
    
    if (m_controller) {
        m_controller->connectToRobot(ip, port);
    }
    
    emit connectRequested(ip, port);
}

void RobotControlPanel::onDisconnectClicked()
{
    if (m_controller) {
        m_controller->disconnectFromRobot();
    }
    
    emit disconnectRequested();
}

void RobotControlPanel::onModeChanged(int index)
{
    OperationMode mode = static_cast<OperationMode>(m_modeCombo->itemData(index).toInt());
    
    if (m_controller) {
        m_controller->setOperationMode(mode);
    }
    
    // 仿真模式下禁用连接控件
    bool isSimulation = (mode == OperationMode::Simulation);
    m_ipEdit->setEnabled(!isSimulation);
    m_portEdit->setEnabled(!isSimulation);
    m_connectBtn->setEnabled(!isSimulation);
    m_disconnectBtn->setEnabled(false);
    
    if (isSimulation) {
        m_statusLabel->setText("仿真模式 - 已就绪");
        m_statusLabel->setStyleSheet("QLabel { color: #4caf50; font-weight: bold; }");
        // 仿真模式下启用所有控制按钮
        m_servoOnBtn->setEnabled(true);
        m_servoOffBtn->setEnabled(true);
        m_homeBtn->setEnabled(true);
        m_stopBtn->setEnabled(true);
    } else {
        m_statusLabel->setText("未连接");
        m_statusLabel->setStyleSheet("QLabel { color: #ff9800; font-weight: bold; }");
        // 非仿真模式下需要连接后才能控制
        m_servoOnBtn->setEnabled(false);
        m_servoOffBtn->setEnabled(false);
    }
}

void RobotControlPanel::onHomeClicked()
{
    if (m_controller) {
        m_controller->moveToHome();
    }
    
    // 更新UI
    for (int i = 0; i < 6; ++i) {
        m_jointWidgets[i]->setAngle(0);
    }
}

void RobotControlPanel::onStopClicked()
{
    if (m_controller) {
        m_controller->stopMotion();
    }
}

void RobotControlPanel::onServoOnClicked()
{
    if (m_controller) {
        m_controller->servoOn();
    }
}

void RobotControlPanel::onServoOffClicked()
{
    if (m_controller) {
        m_controller->servoOff();
    }
}

void RobotControlPanel::onControllerJointAnglesChanged(const std::array<double, 6>& angles)
{
    m_updating = true;
    for (int i = 0; i < 6; ++i) {
        m_jointWidgets[i]->setAngle(angles[i]);
    }
    m_updating = false;
}

void RobotControlPanel::onControllerPoseChanged(const EndEffectorPose& pose)
{
    updatePoseDisplay(pose);
}

void RobotControlPanel::onControllerConnectionStateChanged(int state)
{
    ConnectionState connState = static_cast<ConnectionState>(state);
    
    switch (connState) {
        case ConnectionState::Disconnected:
            m_statusLabel->setText("未连接");
            m_statusLabel->setStyleSheet("QLabel { color: #ff9800; font-weight: bold; }");
            updateConnectionStatus(false);
            break;
        case ConnectionState::Connecting:
            m_statusLabel->setText("正在连接...");
            m_statusLabel->setStyleSheet("QLabel { color: #2196f3; font-weight: bold; }");
            break;
        case ConnectionState::Connected:
            m_statusLabel->setText("已连接");
            m_statusLabel->setStyleSheet("QLabel { color: #4caf50; font-weight: bold; }");
            updateConnectionStatus(true);
            break;
        case ConnectionState::Error:
            m_statusLabel->setText("连接错误");
            m_statusLabel->setStyleSheet("QLabel { color: #f44336; font-weight: bold; }");
            updateConnectionStatus(false);
            break;
    }
}

void RobotControlPanel::onControllerError(const QString& error)
{
    QMessageBox::warning(this, "机器人错误", error);
}

void RobotControlPanel::updatePoseDisplay(const EndEffectorPose& pose)
{
    m_posXLabel->setText(QString::number(pose.position.x(), 'f', 2));
    m_posYLabel->setText(QString::number(pose.position.y(), 'f', 2));
    m_posZLabel->setText(QString::number(pose.position.z(), 'f', 2));
    
    m_rotRLabel->setText(QString::number(pose.orientation.x(), 'f', 2));
    m_rotPLabel->setText(QString::number(pose.orientation.y(), 'f', 2));
    m_rotYLabel->setText(QString::number(pose.orientation.z(), 'f', 2));
}

void RobotControlPanel::updateConnectionStatus(bool connected)
{
    m_connectBtn->setEnabled(!connected);
    m_disconnectBtn->setEnabled(connected);
    m_ipEdit->setEnabled(!connected);
    m_portEdit->setEnabled(!connected);
    m_servoOnBtn->setEnabled(connected);
    m_servoOffBtn->setEnabled(connected);
}

} // namespace Robot
