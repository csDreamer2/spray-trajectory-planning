#ifndef ROBOTCONTROLPANEL_H
#define ROBOTCONTROLPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <array>

namespace Robot {

class RobotController;
struct EndEffectorPose;

/**
 * @brief 单个关节控制组件
 */
class JointControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit JointControlWidget(int jointIndex, const QString& name, 
                                double minAngle, double maxAngle,
                                QWidget* parent = nullptr);

    void setAngle(double angle);
    double getAngle() const;
    void setEnabled(bool enabled);

signals:
    void angleChanged(int jointIndex, double angle);

private slots:
    void onSliderChanged(int value);
    void onSpinBoxChanged(double value);

private:
    int m_jointIndex;
    double m_minAngle;
    double m_maxAngle;
    
    QLabel* m_nameLabel;
    QSlider* m_slider;
    QDoubleSpinBox* m_spinBox;
    QLabel* m_rangeLabel;
    
    bool m_updating;
};

/**
 * @brief 机器人控制面板
 * 
 * 提供6轴关节角度控制界面
 * - 滑动条实时调整关节角度
 * - 显示末端位姿
 * - 连接真实机器人
 * - 模式切换
 */
class RobotControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit RobotControlPanel(QWidget* parent = nullptr);
    ~RobotControlPanel();

    /**
     * @brief 设置机器人控制器
     */
    void setRobotController(RobotController* controller);

    /**
     * @brief 获取机器人控制器
     */
    RobotController* getRobotController() const;

signals:
    /**
     * @brief 关节角度变化（用于3D仿真更新）
     */
    void jointAnglesChanged(const std::array<double, 6>& angles);

    /**
     * @brief 请求连接机器人
     */
    void connectRequested(const QString& ip, int port);

    /**
     * @brief 请求断开连接
     */
    void disconnectRequested();

    /**
     * @brief 请求加载机器人模型
     */
    void loadRobotModelRequested();

private slots:
    void onJointAngleChanged(int jointIndex, double angle);
    void onConnectClicked();
    void onDisconnectClicked();
    void onModeChanged(int index);
    void onHomeClicked();
    void onStopClicked();
    void onServoOnClicked();
    void onServoOffClicked();
    
    // 来自控制器的信号
    void onControllerJointAnglesChanged(const std::array<double, 6>& angles);
    void onControllerPoseChanged(const EndEffectorPose& pose);
    void onControllerConnectionStateChanged(int state);
    void onControllerError(const QString& error);

private:
    void setupUI();
    void setupJointControls();
    void setupPoseDisplay();
    void setupConnectionPanel();
    void setupControlButtons();
    void setupStyles();
    void connectSignals();
    void updatePoseDisplay(const EndEffectorPose& pose);
    void updateConnectionStatus(bool connected);

private:
    RobotController* m_controller;
    
    QVBoxLayout* m_mainLayout;
    
    // 关节控制
    QGroupBox* m_jointGroup;
    std::array<JointControlWidget*, 6> m_jointWidgets;
    
    // 末端位姿显示
    QGroupBox* m_poseGroup;
    QLabel* m_posXLabel;
    QLabel* m_posYLabel;
    QLabel* m_posZLabel;
    QLabel* m_rotRLabel;
    QLabel* m_rotPLabel;
    QLabel* m_rotYLabel;
    
    // 连接面板
    QGroupBox* m_connectionGroup;
    QLineEdit* m_ipEdit;
    QLineEdit* m_portEdit;
    QPushButton* m_connectBtn;
    QPushButton* m_disconnectBtn;
    QLabel* m_statusLabel;
    QComboBox* m_modeCombo;
    
    // 控制按钮
    QGroupBox* m_controlGroup;
    QPushButton* m_homeBtn;
    QPushButton* m_stopBtn;
    QPushButton* m_servoOnBtn;
    QPushButton* m_servoOffBtn;
    
    // 模型加载按钮
    QPushButton* m_loadRobotModelBtn;
    
    bool m_updating;
};

} // namespace Robot

#endif // ROBOTCONTROLPANEL_H
