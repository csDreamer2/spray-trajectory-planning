#include "MainWindow.h"
#include "VTKWidget.h"
#include "ParameterPanel.h"
#include "StatusPanel.h"
#include "SafetyPanel.h"
#include "PointCloudLoader.h"
#include "../Data/PointCloudParser.h"

#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QAction>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressBar>
#include <QProgressDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QThread>
#include <QTimer>

namespace UI {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainSplitter(nullptr)
    , m_vtkView(nullptr)
    , m_parameterPanel(nullptr)
    , m_statusPanel(nullptr)
    , m_safetyPanel(nullptr)
    , m_menuBar(nullptr)
    , m_mainToolBar(nullptr)
    , m_statusBar(nullptr)
    , m_parameterDock(nullptr)
    , m_statusDock(nullptr)
    , m_safetyDock(nullptr)
    , m_statusLabel(nullptr)
    , m_robotStatusLabel(nullptr)
    , m_simulationStatusLabel(nullptr)
    , m_pointCloudLoader(nullptr)
{
    setWindowTitle("机器人喷涂轨迹规划系统 - VTK 3D引擎");
    setMinimumSize(1200, 800);
    resize(1600, 1000);
    
    // 设置应用程序图标
    setWindowIcon(QIcon(":/icons/app_icon.png"));
    
    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupDockWidgets();
    
    // 创建点云加载器
    m_pointCloudLoader = new PointCloudLoader(this);
    
    connectSignals();
    connectPanelSignals();
    connectVTKSignals();
    
    // 初始化状态
    m_statusLabel->setText("VTK 3D引擎已就绪");
    if (m_statusPanel) {
        m_statusPanel->addLogMessage("SUCCESS", "VTK 3D可视化引擎初始化完成");
        m_statusPanel->addLogMessage("INFO", "系统就绪，可以开始导入点云数据");
    }
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
    
    // 创建右侧控制面板容器
    QWidget* controlPanel = new QWidget(this);
    controlPanel->setMaximumWidth(300);
    controlPanel->setMinimumWidth(250);
    
    QVBoxLayout* controlLayout = new QVBoxLayout(controlPanel);
    
    // 添加VTK控制按钮
    QPushButton* loadPointCloudBtn = new QPushButton("导入点云", this);
    QPushButton* loadWorkshopBtn = new QPushButton("加载车间模型", this);
    QPushButton* resetViewBtn = new QPushButton("重置视图", this);
    QPushButton* fitSceneBtn = new QPushButton("适应场景", this);
    QPushButton* showTestDataBtn = new QPushButton("显示测试数据", this);
    
    // 连接VTK控制信号
    connect(loadPointCloudBtn, &QPushButton::clicked, this, &MainWindow::OnImportWorkpiece);
    connect(loadWorkshopBtn, &QPushButton::clicked, this, [this]() {
        QString fileName = QFileDialog::getOpenFileName(
            this,
            "选择车间模型文件",
            "data/model",
            "CAD文件 (*.step *.stp *.stl);;所有文件 (*.*)"
        );
        
        if (!fileName.isEmpty()) {
            bool success = m_vtkView->LoadSTEPModel(fileName);
            if (success) {
                m_statusLabel->setText("车间模型加载成功");
            } else {
                QMessageBox::information(this, "提示", 
                    "STEP文件需要额外支持。\n建议将STEP文件转换为STL格式。");
            }
        }
    });
    
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
        m_statusLabel->setText("测试轨迹已显示");
    });
    
    controlLayout->addWidget(new QLabel("VTK 3D控制", this));
    controlLayout->addWidget(loadPointCloudBtn);
    controlLayout->addWidget(loadWorkshopBtn);
    controlLayout->addWidget(resetViewBtn);
    controlLayout->addWidget(fitSceneBtn);
    controlLayout->addWidget(showTestDataBtn);
    controlLayout->addStretch();
    
    // 添加到分割器
    m_mainSplitter->addWidget(m_vtkView);
    m_mainSplitter->addWidget(controlPanel);
    m_mainSplitter->setStretchFactor(0, 3); // VTK视图占3/4
    m_mainSplitter->setStretchFactor(1, 1); // 控制面板占1/4
    
    centralLayout->addWidget(m_mainSplitter);
}

void MainWindow::connectVTKSignals()
{
    if (m_vtkView) {
        connect(m_vtkView, &VTKWidget::ModelLoaded, this, [this](const QString& modelType, bool success) {
            if (success) {
                m_statusLabel->setText(QString("VTK: %1 加载成功").arg(modelType));
                if (m_statusPanel) {
                    m_statusPanel->addLogMessage("SUCCESS", QString("%1模型加载完成").arg(modelType));
                }
            } else {
                m_statusLabel->setText(QString("VTK: %1 加载失败").arg(modelType));
                if (m_statusPanel) {
                    m_statusPanel->addLogMessage("ERROR", QString("%1模型加载失败").arg(modelType));
                }
            }
        });
        
        connect(m_vtkView, &VTKWidget::CameraChanged, this, [this]() {
            if (m_statusPanel) {
                m_statusPanel->addLogMessage("INFO", "3D视图已更新");
            }
        });
        
        connect(m_vtkView, &VTKWidget::SceneClicked, this, [this](double x, double y, double z) {
            if (m_statusPanel) {
                m_statusPanel->addLogMessage("INFO", 
                    QString("点击位置: (%.2f, %.2f, %.2f)").arg(x).arg(y).arg(z));
            }
        });
    }
}

void MainWindow::OnPointCloudLoadCompleted(bool success, const QJsonObject& pointCloudJson, const QString& errorMessage)
{
    if (m_pointCloudLoader) {
        m_pointCloudLoader->setEnabled(true);
    }
    
    if (success && m_vtkView) {
        // 直接从文件路径加载到VTK
        QString filePath = pointCloudJson["file_path"].toString();
        bool vtkSuccess = m_vtkView->LoadPointCloud(filePath);
        
        if (vtkSuccess) {
            m_statusLabel->setText("点云加载成功");
            if (m_statusPanel) {
                int pointCount = pointCloudJson["point_count"].toInt();
                m_statusPanel->addLogMessage("SUCCESS", 
                    QString("点云加载完成，共 %1 个点").arg(pointCount));
            }
        } else {
            m_statusLabel->setText("VTK点云加载失败");
            if (m_statusPanel) {
                m_statusPanel->addLogMessage("ERROR", "VTK点云渲染失败");
            }
        }
    } else {
        m_statusLabel->setText("点云加载失败: " + errorMessage);
        if (m_statusPanel) {
            m_statusPanel->addLogMessage("ERROR", "点云加载失败: " + errorMessage);
        }
        
        QMessageBox::warning(this, "加载失败", 
            QString("点云文件加载失败:\n%1").arg(errorMessage));
    }
}

void MainWindow::LoadWorkpiece(const QString& filePath)
{
    if (m_vtkView) {
        bool success = m_vtkView->LoadPointCloud(filePath);
        if (success) {
            m_statusLabel->setText("工件加载成功");
        } else {
            m_statusLabel->setText("工件加载失败");
        }
    }
}

void MainWindow::DisplayTrajectory(const QString& trajectoryData)
{
    // 解析轨迹数据并显示
    if (m_vtkView) {
        // 这里需要解析trajectoryData为轨迹点
        // 暂时显示测试轨迹
        std::vector<std::array<double, 3>> trajectory;
        // TODO: 解析实际轨迹数据
        m_vtkView->ShowSprayTrajectory(trajectory);
    }
}

void MainWindow::ShowSimulation(const QString& simulationResult)
{
    if (m_statusPanel) {
        m_statusPanel->addLogMessage("INFO", "仿真结果: " + simulationResult);
    }
}

void MainWindow::OnVTKSceneReady()
{
    m_statusLabel->setText("VTK 3D场景已就绪");
    if (m_statusPanel) {
        m_statusPanel->addLogMessage("SUCCESS", "VTK 3D场景初始化完成");
    }
}

// 其他函数保持不变...
// setupMenuBar(), setupToolBar(), setupStatusBar(), setupDockWidgets()
// connectSignals(), connectPanelSignals()
// OnNewProject(), OnOpenProject(), OnSaveProject(), OnImportWorkpiece()
// 等函数可以从原来的MainWindow.cpp复制过来

} // namespace UI