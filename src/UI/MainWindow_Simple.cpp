#include "MainWindow.h"
#include "VTKWidget.h"
#include "ParameterPanel.h"
#include "StatusPanel.h"
#include "SafetyPanel.h"
#include "PointCloudLoader.h"

#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QAction>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonObject>

namespace UI {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainSplitter(nullptr)
    , m_vtkView(nullptr)
    , m_parameterPanel(nullptr)
    , m_statusPanel(nullptr)
    , m_safetyPanel(nullptr)
    , m_pointCloudLoader(nullptr)
{
    setWindowTitle("机器人喷涂轨迹规划系统 - VTK 3D引擎");
    setMinimumSize(1200, 800);
    resize(1600, 1000);
    
    setupUI();
    setupMenuBar();
    setupStatusBar();
    
    // 创建点云加载器
    m_pointCloudLoader = new PointCloudLoader(this);
    
    connectVTKSignals();
    
    // 初始化状态
    statusBar()->showMessage("VTK 3D引擎已就绪");
}

MainWindow::~MainWindow()
{
    // Qt会自动清理子对象
}

void MainWindow::setupUI()
{
    // 创建中央部件
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    QVBoxLayout* centralLayout = new QVBoxLayout(m_centralWidget);
    centralLayout->setContentsMargins(5, 5, 5, 5);
    
    // 创建主分割器
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // 创建VTK 3D视图
    m_vtkView = new VTKWidget(this);
    m_vtkView->setMinimumSize(800, 600);
    
    // 创建右侧控制面板
    QWidget* controlPanel = new QWidget(this);
    controlPanel->setMaximumWidth(300);
    controlPanel->setMinimumWidth(250);
    
    QVBoxLayout* controlLayout = new QVBoxLayout(controlPanel);
    
    // 添加VTK控制按钮
    QPushButton* loadPointCloudBtn = new QPushButton("导入点云", this);
    QPushButton* resetViewBtn = new QPushButton("重置视图", this);
    QPushButton* fitSceneBtn = new QPushButton("适应场景", this);
    QPushButton* showTestDataBtn = new QPushButton("显示测试数据", this);
    
    // 连接信号
    connect(loadPointCloudBtn, &QPushButton::clicked, this, &MainWindow::OnImportWorkpiece);
    connect(resetViewBtn, &QPushButton::clicked, m_vtkView, &VTKWidget::ResetCamera);
    connect(fitSceneBtn, &QPushButton::clicked, m_vtkView, &VTKWidget::FitToScene);
    connect(showTestDataBtn, &QPushButton::clicked, this, [this]() {
        // 显示测试轨迹
        std::vector<std::array<double, 3>> trajectory;
        for (int i = 0; i < 100; ++i) {
            double t = i * 0.1;
            double x = 50 * cos(t);
            double y = 50 * sin(t);
            double z = t * 5;
            trajectory.push_back({x, y, z});
        }
        m_vtkView->ShowSprayTrajectory(trajectory);
        statusBar()->showMessage("测试轨迹已显示");
    });
    
    controlLayout->addWidget(new QLabel("VTK 3D控制", this));
    controlLayout->addWidget(loadPointCloudBtn);
    controlLayout->addWidget(resetViewBtn);
    controlLayout->addWidget(fitSceneBtn);
    controlLayout->addWidget(showTestDataBtn);
    controlLayout->addStretch();
    
    // 添加到分割器
    m_mainSplitter->addWidget(m_vtkView);
    m_mainSplitter->addWidget(controlPanel);
    m_mainSplitter->setStretchFactor(0, 3);
    m_mainSplitter->setStretchFactor(1, 1);
    
    centralLayout->addWidget(m_mainSplitter);
}

void MainWindow::setupMenuBar()
{
    // 文件菜单
    QMenu* fileMenu = menuBar()->addMenu("文件(&F)");
    
    QAction* importAction = new QAction("导入点云(&I)", this);
    importAction->setShortcut(QKeySequence::Open);
    connect(importAction, &QAction::triggered, this, &MainWindow::OnImportWorkpiece);
    fileMenu->addAction(importAction);
    
    fileMenu->addSeparator();
    
    QAction* exitAction = new QAction("退出(&X)", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAction);
    
    // 视图菜单
    QMenu* viewMenu = menuBar()->addMenu("视图(&V)");
    
    QAction* resetViewAction = new QAction("重置视图(&R)", this);
    connect(resetViewAction, &QAction::triggered, m_vtkView, &VTKWidget::ResetCamera);
    viewMenu->addAction(resetViewAction);
    
    QAction* fitSceneAction = new QAction("适应场景(&F)", this);
    connect(fitSceneAction, &QAction::triggered, m_vtkView, &VTKWidget::FitToScene);
    viewMenu->addAction(fitSceneAction);
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("VTK 3D引擎已就绪");
}

void MainWindow::connectVTKSignals()
{
    if (m_vtkView) {
        connect(m_vtkView, &VTKWidget::ModelLoaded, this, [this](const QString& modelType, bool success) {
            if (success) {
                statusBar()->showMessage(QString("VTK: %1 加载成功").arg(modelType));
            } else {
                statusBar()->showMessage(QString("VTK: %1 加载失败").arg(modelType));
            }
        });
        
        connect(m_vtkView, &VTKWidget::CameraChanged, this, [this]() {
            statusBar()->showMessage("3D视图已更新", 2000);
        });
        
        connect(m_vtkView, &VTKWidget::SceneClicked, this, [this](double x, double y, double z) {
            statusBar()->showMessage(QString("点击位置: (%.2f, %.2f, %.2f)").arg(x).arg(y).arg(z), 3000);
        });
    }
}

// 实现必要的槽函数
void MainWindow::OnImportWorkpiece()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "选择点云文件",
        "test_data",
        "点云文件 (*.ply *.pcd);;所有文件 (*.*)"
    );
    
    if (!fileName.isEmpty()) {
        LoadWorkpiece(fileName);
    }
}

void MainWindow::LoadWorkpiece(const QString& filePath)
{
    if (m_vtkView) {
        bool success = m_vtkView->LoadPointCloud(filePath);
        if (success) {
            statusBar()->showMessage("工件加载成功");
        } else {
            statusBar()->showMessage("工件加载失败");
            QMessageBox::warning(this, "加载失败", "点云文件加载失败，请检查文件格式。");
        }
    }
}

void MainWindow::DisplayTrajectory(const QString& trajectoryData)
{
    // 简化实现
    if (m_vtkView) {
        std::vector<std::array<double, 3>> trajectory;
        // TODO: 解析实际轨迹数据
        m_vtkView->ShowSprayTrajectory(trajectory);
    }
}

void MainWindow::ShowSimulation(const QString& simulationResult)
{
    statusBar()->showMessage("仿真结果: " + simulationResult);
}

void MainWindow::UpdateRobotStatus(const QString& statusData)
{
    statusBar()->showMessage("机器人状态: " + statusData);
}

void MainWindow::ShowNotification(const QString& type, const QString& message)
{
    statusBar()->showMessage(type + ": " + message, 5000);
}

void MainWindow::ShowSafetyAlert(const QString& alertData)
{
    QMessageBox::warning(this, "安全警告", alertData);
}

// 空实现的槽函数
void MainWindow::OnNewProject() {}
void MainWindow::OnOpenProject() {}
void MainWindow::OnSaveProject() {}
void MainWindow::OnCollisionDetected(const QJsonObject& collisionData) {}
void MainWindow::OnSafetyWarning(const QString& message) {}
void MainWindow::OnTrajectoryChanged() {}
void MainWindow::OnVTKSceneReady() {}
void MainWindow::OnPointCloudLoadProgress(int progress) {}
void MainWindow::OnPointCloudLoadCompleted(bool success, const QJsonObject& pointCloudJson, const QString& errorMessage) {}
void MainWindow::OnPointCloudLoadCanceled() {}

// 空实现的私有函数
void MainWindow::setupToolBar() {}
void MainWindow::setupDockWidgets() {}
void MainWindow::connectSignals() {}
void MainWindow::connectPanelSignals() {}
void MainWindow::updateAllStatus() {}
void MainWindow::resetLayout() {}

} // namespace UI