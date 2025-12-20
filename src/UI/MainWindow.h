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

namespace UI {

class VTKWidget;
class ParameterPanel;
class StatusPanel;
class SafetyPanel;
class PointCloudLoader;

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

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupDockWidgets();
    void connectSignals();
    void connectPanelSignals();
    void connectVTKSignals();
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
    VTKWidget* m_vtkView;
    
    // 面板组件
    ParameterPanel* m_parameterPanel;
    StatusPanel* m_statusPanel;
    SafetyPanel* m_safetyPanel;
    
    // 菜单和工具栏
    QMenuBar* m_menuBar;
    QToolBar* m_mainToolBar;
    QStatusBar* m_statusBar;
    QMenu* m_panelMenu;
    
    // 停靠窗口
    QDockWidget* m_workpieceDock;
    QDockWidget* m_trajectoryDock;
    QDockWidget* m_parameterDock;
    QDockWidget* m_statusDock;
    QDockWidget* m_safetyDock;
    
    // 工件面板组件
    QListWidget* m_workpieceList;
    QLabel* m_workpieceInfo;
    
    // 状态标签
    QLabel* m_statusLabel;
    QLabel* m_robotStatusLabel;
    QLabel* m_simulationStatusLabel;
    
    // 点云加载器（暂时保留以兼容现有代码）
    PointCloudLoader* m_pointCloudLoader;
};

} // namespace UI

#endif // MAINWINDOW_H
