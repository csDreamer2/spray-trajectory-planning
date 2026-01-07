#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QSplitter>
#include <QListWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <memory>

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QProgressDialog;
class QMenu;
QT_END_NAMESPACE

namespace Robot {
class RobotController;
class RobotControlPanel;
}

namespace UI {

class VTKWidget;
class ParameterPanel;
class StatusPanel;
class SafetyPanel;
class PointCloudLoader;
class ModelTreeDockWidget;
class WorkpieceManagerPanel;
}

class STEPModelTree;
class STEPModelTreeWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void LoadWorkpiece(const QString& filePath);
    void DisplayTrajectory(const QString& trajectoryData);
    void ShowSimulation(const QString& simulationResult);
    void UpdateRobotStatus(const QString& statusData);
    void ShowNotification(const QString& type, const QString& message);
    void ShowSafetyAlert(const QString& alertData);

private slots:
    void OnNewProject();
    void OnOpenProject();
    void OnSaveProject();
    void OnImportWorkpiece();
    void OnImportSTEPModel();  // 新增：导入STEP模型
    void OnImportSTEPModelFast();  // 新增：快速导入STEP模型（使用缓存）
    void OnExportTrajectory();
    void OnStartSimulation();
    void OnStopSimulation();
    void OnConnectRobot();
    void OnDisconnectRobot();
    void OnAbout();
    
    void OnCollisionDetected(const QJsonObject& collisionData);
    void OnSafetyWarning(const QString& message);
    void OnTrajectoryChanged();
    void OnVTKSceneReady();
    
    void OnPointCloudLoadProgress(int progress);
    void OnPointCloudLoadCanceled();
    
    void loadRobotModel();  // 加载机器人模型

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupDockWidgets();
    void connectSignals();
    void connectPanelSignals();
    void connectVTKSignals();
    void connectModelTreeToVTK();  // 新增：连接模型树到VTK视图
    void updateAllStatus();
    void resetLayout();
    void saveLayout();
    void restoreLayout();
    void setupDockSizeConstraints();
    
    // 面板创建函数
    QWidget* createWorkpiecePanel();
    QWidget* createTrajectoryPanel();
    QWidget* createParameterPanel();
    QWidget* createSafetyPanel();

private:
    // UI组件
    QWidget* m_centralWidget;
    QSplitter* m_mainSplitter;
    
    // VTK 3D视图
    UI::VTKWidget* m_vtkView;
    
    // 面板组件
    UI::ParameterPanel* m_parameterPanel;
    UI::StatusPanel* m_statusPanel;
    UI::SafetyPanel* m_safetyPanel;
    STEPModelTreeWidget* m_modelTreePanel;
    
    // 菜单和工具栏
    QMenuBar* m_menuBar;
    QToolBar* m_mainToolBar;
    QStatusBar* m_statusBar;
    QMenu* m_panelMenu;
    
    // 停靠窗口
    QDockWidget* m_trajectoryDock;
    QDockWidget* m_parameterDock;
    QDockWidget* m_statusDock;
    QDockWidget* m_safetyDock;
    QDockWidget* m_modelTreeDock;
    QDockWidget* m_workpieceManagerDock;
    
    // 工件管理器
    UI::WorkpieceManagerPanel* m_workpieceManager;
    
    // 机器人控制
    Robot::RobotController* m_robotController;
    Robot::RobotControlPanel* m_robotControlPanel;
    QDockWidget* m_robotControlDock;
    
    // 状态标签
    QLabel* m_statusLabel;
    QLabel* m_robotStatusLabel;
    QLabel* m_simulationStatusLabel;
    
    // 点云加载器（暂时保留以兼容现有代码）
    UI::PointCloudLoader* m_pointCloudLoader;
};

#endif // MAINWINDOW_H
