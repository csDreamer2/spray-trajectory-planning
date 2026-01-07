#include "MainWindow.h"
#include "VTKWidget.h"
#include "ParameterPanel.h"
#include "StatusPanel.h"
#include "SafetyPanel.h"
#include "PointCloudLoader.h"
#include "ModelTreeDockWidget.h"
#include "STEPModelTreeWidget.h"
#include "WorkpieceManagerPanel.h"
#include "../Data/PointCloudParser.h"
#include "../Data/STEPModelTree.h"
#include "../Robot/RobotController.h"
#include "../Robot/RobotControlPanel.h"

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
#include <QTabWidget>
#include <QListWidget>
#include <QTreeWidget>
#include <QGroupBox>
#include <QScrollArea>
#include <QSettings>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainSplitter(nullptr)
    , m_vtkView(nullptr)
    , m_parameterPanel(nullptr)
    , m_statusPanel(nullptr)
    , m_safetyPanel(nullptr)
    , m_modelTreePanel(nullptr)
    , m_menuBar(nullptr)
    , m_mainToolBar(nullptr)
    , m_statusBar(nullptr)
    , m_parameterDock(nullptr)
    , m_statusDock(nullptr)
    , m_safetyDock(nullptr)
    , m_modelTreeDock(nullptr)
    , m_statusLabel(nullptr)
    , m_robotStatusLabel(nullptr)
    , m_simulationStatusLabel(nullptr)
    , m_pointCloudLoader(nullptr)
    , m_robotController(nullptr)
    , m_robotControlPanel(nullptr)
    , m_robotControlDock(nullptr)
{
    setWindowTitle("机器人喷涂轨迹规划系统 - 王睿 (浙江大学)");
    setMinimumSize(1400, 900);
    resize(1800, 1000);
    
    // 允许嵌套停靠
    setDockNestingEnabled(true);
    
    // 设置角落归属，允许灵活布局
    setCorner(Qt::BottomLeftCorner, Qt::BottomDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    
    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupDockWidgets();
    
    connectSignals();
    connectPanelSignals();
    connectVTKSignals();
    
    // 强制重置布局（不恢复之前保存的状态）
    // restoreLayout(); // 暂时禁用以确保使用新布局
    
    // 强制应用新布局
    QTimer::singleShot(100, this, [this]() {
        resetLayout();
    });
    
    // 初始化状态
    m_statusLabel->setText("VTK 3D引擎已就绪");
    if (m_statusPanel) {
        m_statusPanel->addLogMessage("SUCCESS", "VTK 3D可视化引擎初始化完成");
        m_statusPanel->addLogMessage("INFO", "系统就绪，可以开始导入点云数据");
    }
    
    // 延迟加载机器人模型（等待VTK完全初始化）
    QTimer::singleShot(500, this, [this]() {
        loadRobotModel();
    });
}

MainWindow::~MainWindow()
{
    // 保存窗口布局
    saveLayout();
}

void MainWindow::setupUI()
{
    // 创建中央部件 - 只包含VTK 3D视图
    m_vtkView = new UI::VTKWidget(this);
    m_vtkView->setMinimumSize(800, 600);
    setCentralWidget(m_vtkView);
}

void MainWindow::setupMenuBar()
{
    // 文件菜单
    QMenu* fileMenu = menuBar()->addMenu("文件(&F)");
    
    QAction* importAction = new QAction("导入点云(&I)", this);
    importAction->setShortcut(QKeySequence("Ctrl+I"));
    connect(importAction, &QAction::triggered, this, &MainWindow::OnImportWorkpiece);
    fileMenu->addAction(importAction);
    
    QAction* importSTEPAction = new QAction("导入STEP模型(&S)", this);
    importSTEPAction->setShortcut(QKeySequence("Ctrl+S"));
    connect(importSTEPAction, &QAction::triggered, this, &MainWindow::OnImportSTEPModel);
    fileMenu->addAction(importSTEPAction);
    
    QAction* importSTEPFastAction = new QAction("快速导入STEP模型(&Q)", this);
    importSTEPFastAction->setShortcut(QKeySequence("Ctrl+Q"));
    importSTEPFastAction->setToolTip("使用缓存快速加载STEP模型（适合大型模型）");
    connect(importSTEPFastAction, &QAction::triggered, this, &MainWindow::OnImportSTEPModelFast);
    fileMenu->addAction(importSTEPFastAction);
    
    QAction* importModelAction = new QAction("导入车间模型(&M)", this);
    connect(importModelAction, &QAction::triggered, this, [this]() {
        QString fileName = QFileDialog::getOpenFileName(this, "选择车间模型", "data/model",
            "CAD文件 (*.step *.stp *.stl);;所有文件 (*.*)");
        if (!fileName.isEmpty() && m_vtkView) {
            m_vtkView->LoadSTEPModel(fileName);
        }
    });
    fileMenu->addAction(importModelAction);
    
    fileMenu->addSeparator();
    
    QAction* exitAction = new QAction("退出(&X)", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAction);

    // 视图菜单
    QMenu* viewMenu = menuBar()->addMenu("视图(&V)");
    
    QAction* resetViewAction = new QAction("重置视图(&R)", this);
    resetViewAction->setShortcut(QKeySequence("R"));
    connect(resetViewAction, &QAction::triggered, m_vtkView, &UI::VTKWidget::ResetCamera);
    viewMenu->addAction(resetViewAction);
    
    QAction* fitSceneAction = new QAction("适应场景(&F)", this);
    fitSceneAction->setShortcut(QKeySequence("F"));
    connect(fitSceneAction, &QAction::triggered, m_vtkView, &UI::VTKWidget::FitToScene);
    viewMenu->addAction(fitSceneAction);
    
    viewMenu->addSeparator();
    
    // 面板显示/隐藏菜单
    m_panelMenu = viewMenu->addMenu("面板(&P)");
    
    viewMenu->addSeparator();
    
    QAction* resetLayoutAction = new QAction("重置布局(&L)", this);
    connect(resetLayoutAction, &QAction::triggered, this, &MainWindow::resetLayout);
    viewMenu->addAction(resetLayoutAction);
    
    // 帮助菜单
    QMenu* helpMenu = menuBar()->addMenu("帮助(&H)");
    
    QAction* aboutAction = new QAction("关于(&A)", this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::OnAbout);
    helpMenu->addAction(aboutAction);
}

void MainWindow::setupToolBar()
{
    m_mainToolBar = addToolBar("主工具栏");
    m_mainToolBar->setMovable(false);
    m_mainToolBar->setIconSize(QSize(24, 24));
    
    // 文件操作
    QAction* importAction = m_mainToolBar->addAction("📂 导入点云");
    connect(importAction, &QAction::triggered, this, &MainWindow::OnImportWorkpiece);
    
    m_mainToolBar->addSeparator();
    
    // 视图操作
    QAction* resetViewAction = m_mainToolBar->addAction("🔄 重置视图");
    connect(resetViewAction, &QAction::triggered, m_vtkView, &UI::VTKWidget::ResetCamera);
    
    QAction* fitSceneAction = m_mainToolBar->addAction("🎯 适应场景");
    connect(fitSceneAction, &QAction::triggered, m_vtkView, &UI::VTKWidget::FitToScene);
    
    m_mainToolBar->addSeparator();
    
    // 测试功能
    QAction* testTrajectoryAction = m_mainToolBar->addAction("📈 测试轨迹");
    connect(testTrajectoryAction, &QAction::triggered, this, [this]() {
        std::vector<std::array<double, 3>> trajectory;
        for (int i = 0; i < 100; ++i) {
            double t = i * 0.1;
            trajectory.push_back({50 * cos(t), 50 * sin(t), t * 5});
        }
        m_vtkView->ShowSprayTrajectory(trajectory);
        m_statusLabel->setText("测试轨迹已显示");
    });
}

void MainWindow::setupStatusBar()
{
    m_statusBar = statusBar();
    
    m_statusLabel = new QLabel("VTK 3D引擎已就绪", this);
    m_robotStatusLabel = new QLabel("🤖 机器人: 未连接", this);
    m_simulationStatusLabel = new QLabel("⏸️ 仿真: 停止", this);
    
    m_statusBar->addWidget(m_statusLabel, 1);
    m_statusBar->addPermanentWidget(m_robotStatusLabel);
    m_statusBar->addPermanentWidget(m_simulationStatusLabel);
}

void MainWindow::setupDockWidgets()
{
    // ========== 所有面板都放在右侧 ==========
    
    // 1. 轨迹规划面板
    m_trajectoryDock = new QDockWidget("轨迹规划", this);
    m_trajectoryDock->setObjectName("trajectoryDock");
    m_trajectoryDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    m_trajectoryDock->setFeatures(QDockWidget::DockWidgetClosable | 
                                   QDockWidget::DockWidgetMovable | 
                                   QDockWidget::DockWidgetFloatable);
    QWidget* trajectoryWidget = createTrajectoryPanel();
    m_trajectoryDock->setWidget(trajectoryWidget);
    addDockWidget(Qt::RightDockWidgetArea, m_trajectoryDock);
    
    // 2. 参数设置面板
    m_parameterDock = new QDockWidget("参数设置", this);
    m_parameterDock->setObjectName("parameterDock");
    m_parameterDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    m_parameterDock->setFeatures(QDockWidget::DockWidgetClosable | 
                                  QDockWidget::DockWidgetMovable | 
                                  QDockWidget::DockWidgetFloatable);
    QWidget* parameterWidget = createParameterPanel();
    m_parameterDock->setWidget(parameterWidget);
    addDockWidget(Qt::RightDockWidgetArea, m_parameterDock);
    
    // 3. 系统日志面板 - 默认显示
    m_statusDock = new QDockWidget("系统日志", this);
    m_statusDock->setObjectName("statusDock");
    m_statusDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    m_statusDock->setFeatures(QDockWidget::DockWidgetClosable | 
                               QDockWidget::DockWidgetMovable | 
                               QDockWidget::DockWidgetFloatable);
    m_statusPanel = new UI::StatusPanel(this);
    m_statusDock->setWidget(m_statusPanel);
    addDockWidget(Qt::RightDockWidgetArea, m_statusDock);
    
    // 4. STEP模型树面板
    m_modelTreeDock = new QDockWidget("STEP模型树", this);
    m_modelTreeDock->setObjectName("modelTreeDock");
    m_modelTreeDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    m_modelTreeDock->setFeatures(QDockWidget::DockWidgetClosable | 
                                  QDockWidget::DockWidgetMovable | 
                                  QDockWidget::DockWidgetFloatable);
    
    // 创建STEP模型树控件（不是停靠窗口）
    STEPModelTreeWidget* modelTreeWidget = new STEPModelTreeWidget(this);
    m_modelTreeDock->setWidget(modelTreeWidget);
    addDockWidget(Qt::RightDockWidgetArea, m_modelTreeDock);
    
    // 保存引用以便后续使用
    m_modelTreePanel = modelTreeWidget;
    
    // 5. 安全监控面板
    m_safetyDock = new QDockWidget("安全监控", this);
    m_safetyDock->setObjectName("safetyDock");
    m_safetyDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    m_safetyDock->setFeatures(QDockWidget::DockWidgetClosable | 
                               QDockWidget::DockWidgetMovable | 
                               QDockWidget::DockWidgetFloatable);
    QWidget* safetyWidget = createSafetyPanel();
    m_safetyDock->setWidget(safetyWidget);
    addDockWidget(Qt::RightDockWidgetArea, m_safetyDock);
    
    // 6. 工件库面板（新增）
    m_workpieceManagerDock = new QDockWidget("工件库", this);
    m_workpieceManagerDock->setObjectName("workpieceManagerDock");
    m_workpieceManagerDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    m_workpieceManagerDock->setFeatures(QDockWidget::DockWidgetClosable | 
                                         QDockWidget::DockWidgetMovable | 
                                         QDockWidget::DockWidgetFloatable);
    m_workpieceManager = new UI::WorkpieceManagerPanel(this);
    m_workpieceManagerDock->setWidget(m_workpieceManager);
    addDockWidget(Qt::RightDockWidgetArea, m_workpieceManagerDock);
    
    // 连接工件管理器信号
    connect(m_workpieceManager, &UI::WorkpieceManagerPanel::workpieceDoubleClicked,
            this, [this](const QString& filePath) {
                // 直接加载点云文件
                if (m_vtkView) {
                    bool success = m_vtkView->LoadPointCloud(filePath);
                    if (success) {
                        m_statusPanel->addLogMessage("INFO", QString("工件加载成功: %1").arg(QFileInfo(filePath).fileName()));
                    } else {
                        m_statusPanel->addLogMessage("ERROR", QString("工件加载失败: %1").arg(QFileInfo(filePath).fileName()));
                    }
                }
            });
    connect(m_workpieceManager, &UI::WorkpieceManagerPanel::workpieceSelected,
            this, [this](const QString& filePath) {
                m_statusPanel->addLogMessage("INFO", QString("选中工件: %1").arg(QFileInfo(filePath).fileName()));
            });
    
    // 7. 机器人控制面板
    m_robotControlDock = new QDockWidget("机器人控制", this);
    m_robotControlDock->setObjectName("robotControlDock");
    m_robotControlDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    m_robotControlDock->setFeatures(QDockWidget::DockWidgetClosable | 
                                     QDockWidget::DockWidgetMovable | 
                                     QDockWidget::DockWidgetFloatable);
    
    // 创建机器人控制器和面板
    m_robotController = new Robot::RobotController(this);
    m_robotControlPanel = new Robot::RobotControlPanel(this);
    m_robotControlPanel->setRobotController(m_robotController);
    m_robotControlDock->setWidget(m_robotControlPanel);
    addDockWidget(Qt::RightDockWidgetArea, m_robotControlDock);
    
    // 连接机器人控制面板信号到VTK视图（用于3D仿真）
    connect(m_robotControlPanel, &Robot::RobotControlPanel::jointAnglesChanged,
            this, [this](const std::array<double, 6>& angles) {
                // 更新3D视图中的机器人模型
                if (m_vtkView) {
                    m_vtkView->UpdateRobotJoints(angles);
                }
                if (m_statusPanel) {
                    m_statusPanel->addLogMessage("INFO", 
                        QString("关节角度: J1=%1 J2=%2 J3=%3 J4=%4 J5=%5 J6=%6")
                        .arg(angles[0], 0, 'f', 1).arg(angles[1], 0, 'f', 1).arg(angles[2], 0, 'f', 1)
                        .arg(angles[3], 0, 'f', 1).arg(angles[4], 0, 'f', 1).arg(angles[5], 0, 'f', 1));
                }
            });
    
    // 将面板堆叠为标签页，系统日志为默认
    tabifyDockWidget(m_statusDock, m_trajectoryDock);
    tabifyDockWidget(m_trajectoryDock, m_parameterDock);
    tabifyDockWidget(m_parameterDock, m_modelTreeDock);
    tabifyDockWidget(m_modelTreeDock, m_safetyDock);
    tabifyDockWidget(m_safetyDock, m_workpieceManagerDock);
    tabifyDockWidget(m_workpieceManagerDock, m_robotControlDock);
    m_statusDock->raise(); // 默认显示系统日志
    
    // ========== 添加面板到视图菜单 ==========
    if (m_panelMenu) {
        m_panelMenu->addAction(m_trajectoryDock->toggleViewAction());
        m_panelMenu->addAction(m_parameterDock->toggleViewAction());
        m_panelMenu->addAction(m_modelTreeDock->toggleViewAction());
        m_panelMenu->addAction(m_statusDock->toggleViewAction());
        m_panelMenu->addAction(m_safetyDock->toggleViewAction());
        m_panelMenu->addAction(m_workpieceManagerDock->toggleViewAction());
        m_panelMenu->addAction(m_robotControlDock->toggleViewAction());
    }
    
    // 设置面板大小限制
    setupDockSizeConstraints();
    
    // 设置右侧面板默认宽度，给足够空间显示内容
    resizeDocks({m_statusDock}, {420}, Qt::Horizontal);
    
    // 设置VTKWidget的StatusPanel引用，用于输出性能统计
    if (m_vtkView && m_statusPanel) {
        m_vtkView->SetStatusPanel(m_statusPanel);
    }
}

QWidget* MainWindow::createWorkpiecePanel()
{
    // 旧的工件管理面板已被工件库面板替代
    // 保留此函数以避免编译错误，但返回空面板
    QWidget* panel = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(8, 8, 8, 8);
    
    QLabel* label = new QLabel("此面板已废弃，请使用工件库面板", panel);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("QLabel { color: #999; font-size: 12px; }");
    layout->addWidget(label);
    
    return panel;
}

QWidget* MainWindow::createTrajectoryPanel()
{
    QWidget* panel = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);
    
    // 轨迹参数
    QGroupBox* paramGroup = new QGroupBox("轨迹参数", panel);
    QVBoxLayout* paramLayout = new QVBoxLayout(paramGroup);
    paramLayout->setContentsMargins(8, 8, 8, 8);
    paramLayout->setSpacing(6);
    
    QLabel* spacingLabel = new QLabel("喷涂间距 (mm):", panel);
    spacingLabel->setStyleSheet("font-size: 12px;");
    paramLayout->addWidget(spacingLabel);
    QSpinBox* spacingSpinBox = new QSpinBox(panel);
    spacingSpinBox->setRange(1, 100);
    spacingSpinBox->setValue(20);
    spacingSpinBox->setMinimumHeight(24);
    paramLayout->addWidget(spacingSpinBox);
    
    QLabel* speedLabel = new QLabel("喷涂速度 (mm/s):", panel);
    speedLabel->setStyleSheet("font-size: 12px;");
    paramLayout->addWidget(speedLabel);
    QSpinBox* speedSpinBox = new QSpinBox(panel);
    speedSpinBox->setRange(10, 500);
    speedSpinBox->setValue(100);
    speedSpinBox->setMinimumHeight(24);
    paramLayout->addWidget(speedSpinBox);
    
    layout->addWidget(paramGroup);

    // 轨迹操作
    QGroupBox* actionGroup = new QGroupBox("轨迹操作", panel);
    QVBoxLayout* actionLayout = new QVBoxLayout(actionGroup);
    actionLayout->setContentsMargins(8, 8, 8, 8);
    actionLayout->setSpacing(6);
    
    QPushButton* generateBtn = new QPushButton("生成轨迹", panel);
    QPushButton* previewBtn = new QPushButton("预览轨迹", panel);
    QPushButton* exportBtn = new QPushButton("导出轨迹", panel);
    
    // 设置按钮样式
    QString btnStyle = "QPushButton { padding: 6px 12px; margin: 2px; font-size: 12px; }";
    generateBtn->setStyleSheet(btnStyle);
    previewBtn->setStyleSheet(btnStyle);
    exportBtn->setStyleSheet(btnStyle);
    
    connect(generateBtn, &QPushButton::clicked, this, [this]() {
        m_statusLabel->setText("轨迹生成功能开发中...");
        if (m_statusPanel) m_statusPanel->addLogMessage("INFO", "轨迹生成功能开发中");
    });
    
    connect(previewBtn, &QPushButton::clicked, this, [this]() {
        // 显示测试轨迹
        std::vector<std::array<double, 3>> trajectory;
        for (int i = 0; i < 100; ++i) {
            double t = i * 0.1;
            trajectory.push_back({50 * cos(t), 50 * sin(t), t * 5});
        }
        m_vtkView->ShowSprayTrajectory(trajectory);
        m_statusLabel->setText("预览轨迹已显示");
    });
    
    actionLayout->addWidget(generateBtn);
    actionLayout->addWidget(previewBtn);
    actionLayout->addWidget(exportBtn);
    
    layout->addWidget(actionGroup);
    layout->addStretch();
    
    return panel;
}

QWidget* MainWindow::createParameterPanel()
{
    QWidget* panel = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);
    
    // 喷涂参数
    QGroupBox* sprayGroup = new QGroupBox("喷涂参数", panel);
    QVBoxLayout* sprayLayout = new QVBoxLayout(sprayGroup);
    
    sprayLayout->addWidget(new QLabel("喷涂压力 (MPa):", panel));
    QDoubleSpinBox* pressureSpinBox = new QDoubleSpinBox(panel);
    pressureSpinBox->setRange(0.1, 1.0);
    pressureSpinBox->setValue(0.4);
    pressureSpinBox->setSingleStep(0.05);
    sprayLayout->addWidget(pressureSpinBox);
    
    sprayLayout->addWidget(new QLabel("喷涂流量 (ml/min):", panel));
    QSpinBox* flowSpinBox = new QSpinBox(panel);
    flowSpinBox->setRange(50, 500);
    flowSpinBox->setValue(200);
    sprayLayout->addWidget(flowSpinBox);
    
    layout->addWidget(sprayGroup);
    
    // 机器人参数
    QGroupBox* robotGroup = new QGroupBox("机器人参数", panel);
    QVBoxLayout* robotLayout = new QVBoxLayout(robotGroup);
    
    robotLayout->addWidget(new QLabel("最大速度 (%):", panel));
    QSlider* speedSlider = new QSlider(Qt::Horizontal, panel);
    speedSlider->setRange(1, 100);
    speedSlider->setValue(50);
    robotLayout->addWidget(speedSlider);
    
    robotLayout->addWidget(new QLabel("加速度 (%):", panel));
    QSlider* accelSlider = new QSlider(Qt::Horizontal, panel);
    accelSlider->setRange(1, 100);
    accelSlider->setValue(30);
    robotLayout->addWidget(accelSlider);
    
    layout->addWidget(robotGroup);
    layout->addStretch();
    
    return panel;
}

QWidget* MainWindow::createSafetyPanel()
{
    QWidget* panel = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(8, 8, 8, 8);
    
    // 安全状态指示
    QHBoxLayout* statusLayout = new QHBoxLayout();
    
    QLabel* safetyIcon = new QLabel("🟢", panel);
    safetyIcon->setStyleSheet("font-size: 24px;");
    QLabel* safetyText = new QLabel("系统安全状态: 正常", panel);
    safetyText->setStyleSheet("font-weight: bold; color: green;");
    
    statusLayout->addWidget(safetyIcon);
    statusLayout->addWidget(safetyText);
    statusLayout->addStretch();
    
    layout->addLayout(statusLayout);
    
    // 安全检查列表
    QGroupBox* checkGroup = new QGroupBox("安全检查", panel);
    QVBoxLayout* checkLayout = new QVBoxLayout(checkGroup);
    
    checkLayout->addWidget(new QLabel("✅ 碰撞检测: 无碰撞", panel));
    checkLayout->addWidget(new QLabel("✅ 关节限位: 正常", panel));
    checkLayout->addWidget(new QLabel("✅ 速度限制: 正常", panel));
    checkLayout->addWidget(new QLabel("✅ 急停状态: 未触发", panel));
    
    layout->addWidget(checkGroup);
    layout->addStretch();
    
    return panel;
}

void MainWindow::setupDockSizeConstraints()
{
    // 完全移除尺寸限制，让面板内容自由显示
    QList<QDockWidget*> docks = {m_trajectoryDock, m_parameterDock, m_statusDock, m_safetyDock, m_workpieceManagerDock};
    
    for (auto* dock : docks) {
        if (dock && dock->widget()) {
            // 移除所有尺寸限制
            dock->widget()->setMinimumSize(0, 0);
            dock->widget()->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
            dock->setMinimumSize(0, 0);
            dock->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        }
    }
}

void MainWindow::saveLayout()
{
    QSettings settings("SpraySystem", "MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
}

void MainWindow::restoreLayout()
{
    QSettings settings("SpraySystem", "MainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}

void MainWindow::resetLayout()
{
    // 重置所有面板到右侧
    QList<QDockWidget*> docks = {m_trajectoryDock, m_parameterDock, m_statusDock, m_safetyDock, m_workpieceManagerDock};
    
    for (auto* dock : docks) {
        if (dock) {
            dock->setFloating(false);
            dock->show();
            addDockWidget(Qt::RightDockWidgetArea, dock);
        }
    }
    
    // 重新堆叠为标签页，系统日志为默认
    tabifyDockWidget(m_statusDock, m_trajectoryDock);
    tabifyDockWidget(m_trajectoryDock, m_parameterDock);
    tabifyDockWidget(m_parameterDock, m_safetyDock);
    tabifyDockWidget(m_safetyDock, m_workpieceManagerDock);
    
    m_statusDock->raise(); // 显示系统日志
    
    // 重新应用大小约束
    setupDockSizeConstraints();
    
    // 重置宽度
    resizeDocks({m_statusDock}, {420}, Qt::Horizontal);
    
    m_statusLabel->setText("布局已重置");
}

void MainWindow::connectSignals()
{
    // VTK直接加载，不需要PointCloudLoader信号连接
}

void MainWindow::connectPanelSignals()
{
    // 旧的工件列表信号已移除，现在使用工件库面板
    /*
    // 连接工件列表选择信号
    if (m_workpieceList) {
        connect(m_workpieceList, &QListWidget::currentItemChanged, this, 
            [this](QListWidgetItem* current, QListWidgetItem*) {
                if (current && m_workpieceInfo) {
                    m_workpieceInfo->setText(QString("文件: %1").arg(current->text()));
                }
            });
    }
    */
}

void MainWindow::connectVTKSignals()
{
    if (m_vtkView) {
        connect(m_vtkView, &UI::VTKWidget::ModelLoaded, this, 
            [this](const QString& modelType, bool success) {
                if (success) {
                    m_statusLabel->setText(QString("VTK: %1 加载成功").arg(modelType));
                    if (m_statusPanel) {
                        m_statusPanel->addLogMessage("SUCCESS", QString("%1模型加载完成").arg(modelType));
                    }
                    // 旧的工件列表已移除
                    // if (m_workpieceList && modelType == "PointCloud") {
                    //     m_workpieceList->addItem("点云工件");
                    // }
                } else {
                    m_statusLabel->setText(QString("VTK: %1 加载失败").arg(modelType));
                    if (m_statusPanel) {
                        m_statusPanel->addLogMessage("ERROR", QString("%1模型加载失败").arg(modelType));
                    }
                }
            });
        
        connect(m_vtkView, &UI::VTKWidget::CameraChanged, this, [this]() {
            if (m_statusPanel) {
                m_statusPanel->addLogMessage("INFO", "3D视图已更新");
            }
        });
        
        connect(m_vtkView, &UI::VTKWidget::SceneClicked, this, 
            [this](double x, double y, double z) {
                if (m_statusPanel) {
                    m_statusPanel->addLogMessage("INFO", 
                        QString("点击位置: (%.2f, %.2f, %.2f)").arg(x).arg(y).arg(z));
                }
            });
    }
}

// 注意：connectModelTreeToVTK方法已不再需要，可见性连接已在OnImportSTEPModel中完成
// 保留此方法以防其他地方调用，但功能已简化
void MainWindow::connectModelTreeToVTK()
{
    if (!m_modelTreePanel || !m_vtkView) {
        qWarning() << "MainWindow: 无法连接模型树到VTK，组件为空";
        return;
    }
    
    qDebug() << "MainWindow: connectModelTreeToVTK被调用（已废弃，连接在OnImportSTEPModel中完成）";
    
    if (m_statusPanel) {
        m_statusPanel->addLogMessage("INFO", "模型树已连接到3D视图");
    }
}

void MainWindow::OnImportWorkpiece()
{
    QString fileName = QFileDialog::getOpenFileName(this, "选择点云文件",
        "test_data/pointclouds", "点云文件 (*.ply *.pcd);;所有文件 (*.*)");
    
    if (!fileName.isEmpty()) {
        QFileInfo fileInfo(fileName);
        if (!fileInfo.exists()) {
            QMessageBox::warning(this, "文件错误", QString("文件不存在:\n%1").arg(fileName));
            return;
        }
        
        qDebug() << "开始加载点云:" << fileName;
        
        // 直接调用VTK加载，简单直接
        m_statusLabel->setText("正在加载点云文件...");
        QApplication::processEvents();
        
        bool success = m_vtkView->LoadPointCloud(fileName);
        
        if (success) {
            m_statusLabel->setText("点云加载成功");
            if (m_statusPanel) {
                m_statusPanel->addLogMessage("SUCCESS", "点云加载完成");
            }
            // 旧的工件列表已移除
            // if (m_workpieceList) {
            //     QFileInfo fi(fileName);
            //     m_workpieceList->addItem(fi.fileName());
            // }
        } else {
            m_statusLabel->setText("点云加载失败");
            if (m_statusPanel) {
                m_statusPanel->addLogMessage("ERROR", "点云加载失败");
            }
        }
    }
}

void MainWindow::OnImportSTEPModel()
{
    QString fileName = QFileDialog::getOpenFileName(this, "选择STEP模型文件",
        "data/model", "STEP文件 (*.step *.stp);;所有文件 (*.*)");
    
    if (!fileName.isEmpty()) {
        QFileInfo fileInfo(fileName);
        if (!fileInfo.exists()) {
            QMessageBox::warning(this, "文件错误", QString("文件不存在:\n%1").arg(fileName));
            return;
        }
        
        qDebug() << "MainWindow: 开始加载STEP模型:" << fileName;
        
        if (m_statusPanel) {
            m_statusPanel->addLogMessage("INFO", QString("开始加载STEP文件: %1").arg(fileInfo.fileName()));
        }
        
        // 显示STEP模型树面板
        if (m_modelTreeDock) {
            m_modelTreeDock->show();
            m_modelTreeDock->raise();
        }
        
        // 同步加载STEP文件
        if (m_modelTreePanel && m_vtkView) {
            m_statusLabel->setText("正在加载STEP模型...");
            QApplication::processEvents();
            
            // 同步加载STEP文件
            bool success = m_modelTreePanel->loadSTEPFile(fileName);
            
            if (success) {
                qDebug() << "MainWindow: STEP模型树加载成功";
                
                // 将所有Actor添加到VTK渲染器
                m_modelTreePanel->addActorsToRenderer(m_vtkView->getRenderer());
                qDebug() << "MainWindow: Actor已添加到VTK渲染器";
                
                // 设置STEP模型树引用到VTKWidget（用于关节变换）
                m_vtkView->SetSTEPModelTreeWidget(m_modelTreePanel);
                
                // 连接可见性变化信号（移除UniqueConnection，因为lambda不支持）
                // 注意：每次加载新文件时会创建新的连接，但这是可以接受的
                connect(m_modelTreePanel, &STEPModelTreeWidget::partVisibilityChanged,
                        this, [this](const QString& partName, bool visible) {
                            qDebug() << "MainWindow: 部件可见性变化:" << partName << visible;
                            
                            // 刷新VTK渲染
                            if (m_vtkView) {
                                m_vtkView->RefreshRender();
                            }
                            
                            if (m_statusPanel) {
                                m_statusPanel->addLogMessage("INFO", 
                                    QString("组件 %1: %2").arg(partName).arg(visible ? "显示" : "隐藏"));
                            }
                        }, Qt::DirectConnection);
                
                // 重置相机以显示完整模型
                m_vtkView->ResetCamera();
                qDebug() << "MainWindow: 相机已重置";
                
                m_statusLabel->setText("STEP模型加载成功");
                if (m_statusPanel) {
                    m_statusPanel->addLogMessage("SUCCESS", "STEP模型加载完成");
                    m_statusPanel->addLogMessage("INFO", "可以在模型树中选择显示/隐藏零件");
                }
                
            } else {
                m_statusLabel->setText("STEP模型加载失败");
                if (m_statusPanel) {
                    m_statusPanel->addLogMessage("ERROR", "STEP模型加载失败");
                }
                QMessageBox::critical(this, "加载失败", "无法加载STEP文件，请检查文件格式");
            }
        } else {
            qWarning() << "MainWindow: 缺少必要组件 - modelTreePanel:" << (m_modelTreePanel ? "有效" : "空") 
                      << "vtkView:" << (m_vtkView ? "有效" : "空");
            QMessageBox::critical(this, "错误", "缺少必要组件，无法加载STEP模型");
        }
    }
}

void MainWindow::OnImportSTEPModelFast()
{
    QString fileName = QFileDialog::getOpenFileName(this, "选择STEP模型文件（快速加载）",
        "data/model", "STEP文件 (*.step *.stp);;所有文件 (*.*)");
    
    if (!fileName.isEmpty()) {
        QFileInfo fileInfo(fileName);
        if (!fileInfo.exists()) {
            QMessageBox::warning(this, "文件错误", QString("文件不存在:\n%1").arg(fileName));
            return;
        }
        
        qDebug() << "MainWindow: 开始快速加载STEP模型:" << fileName;
        
        if (m_statusPanel) {
            m_statusPanel->addLogMessage("INFO", QString("开始快速加载STEP文件: %1").arg(fileInfo.fileName()));
            m_statusPanel->addLogMessage("INFO", "使用缓存机制加速加载（首次加载会创建缓存）");
        }
        
        // 显示STEP模型树面板
        if (m_modelTreeDock) {
            m_modelTreeDock->show();
            m_modelTreeDock->raise();
        }
        
        // 快速加载STEP文件（使用缓存）
        if (m_modelTreePanel && m_vtkView) {
            m_statusLabel->setText("正在快速加载STEP模型...");
            QApplication::processEvents();
            
            // 快速加载STEP文件
            bool success = m_modelTreePanel->loadSTEPFileFast(fileName);
            
            if (success) {
                qDebug() << "MainWindow: STEP模型快速加载成功";
                
                // 将所有Actor添加到VTK渲染器
                m_modelTreePanel->addActorsToRenderer(m_vtkView->getRenderer());
                qDebug() << "MainWindow: Actor已添加到VTK渲染器";
                
                // 连接可见性变化信号
                connect(m_modelTreePanel, &STEPModelTreeWidget::partVisibilityChanged,
                        this, [this](const QString& partName, bool visible) {
                            qDebug() << "MainWindow: 部件可见性变化:" << partName << visible;
                            
                            // 刷新VTK渲染
                            if (m_vtkView) {
                                m_vtkView->RefreshRender();
                            }
                            
                            if (m_statusPanel) {
                                m_statusPanel->addLogMessage("INFO", 
                                    QString("组件 %1: %2").arg(partName).arg(visible ? "显示" : "隐藏"));
                            }
                        }, Qt::DirectConnection);
                
                // 重置相机以显示完整模型
                m_vtkView->ResetCamera();
                qDebug() << "MainWindow: 相机已重置";
                
                m_statusLabel->setText("STEP模型快速加载成功");
                if (m_statusPanel) {
                    m_statusPanel->addLogMessage("SUCCESS", "STEP模型快速加载完成");
                    m_statusPanel->addLogMessage("INFO", "缓存已保存，下次加载将更快");
                }
                
            } else {
                m_statusLabel->setText("STEP模型快速加载失败");
                if (m_statusPanel) {
                    m_statusPanel->addLogMessage("ERROR", "STEP模型快速加载失败");
                }
                QMessageBox::critical(this, "加载失败", "无法加载STEP文件，请检查文件格式");
            }
        } else {
            qWarning() << "MainWindow: 缺少必要组件 - modelTreePanel:" << (m_modelTreePanel ? "有效" : "空") 
                      << "vtkView:" << (m_vtkView ? "有效" : "空");
            QMessageBox::critical(this, "错误", "缺少必要组件，无法加载STEP模型");
        }
    }
}

void MainWindow::LoadWorkpiece(const QString& filePath)
{
    if (m_vtkView) {
        bool success = m_vtkView->LoadPointCloud(filePath);
        if (success) {
            m_statusLabel->setText("工件加载成功");
            // 旧的工件列表已移除
            // if (m_workpieceList) {
            //     QFileInfo fi(filePath);
            //     m_workpieceList->addItem(fi.fileName());
            // }
        } else {
            m_statusLabel->setText("工件加载失败");
            QMessageBox::warning(this, "加载失败", "点云文件加载失败，请检查文件格式。");
        }
    }
}



void MainWindow::DisplayTrajectory(const QString& trajectoryData)
{
    if (m_vtkView) {
        std::vector<std::array<double, 3>> trajectory;
        m_vtkView->ShowSprayTrajectory(trajectory);
    }
}

void MainWindow::ShowSimulation(const QString& simulationResult)
{
    if (m_statusPanel) {
        m_statusPanel->addLogMessage("INFO", "仿真结果: " + simulationResult);
    }
}

void MainWindow::UpdateRobotStatus(const QString& statusData)
{
    m_statusLabel->setText("机器人状态: " + statusData);
    if (m_robotStatusLabel) {
        m_robotStatusLabel->setText("🤖 " + statusData);
    }
}

void MainWindow::ShowNotification(const QString& type, const QString& message)
{
    m_statusLabel->setText(type + ": " + message);
    if (m_statusPanel) {
        m_statusPanel->addLogMessage(type, message);
    }
}

void MainWindow::ShowSafetyAlert(const QString& alertData)
{
    QMessageBox::warning(this, "安全警告", alertData);
}



void MainWindow::OnVTKSceneReady()
{
    m_statusLabel->setText("VTK 3D场景已就绪");
}

void MainWindow::OnAbout()
{
    QMessageBox::about(this, "关于",
        "机器人喷涂轨迹规划系统\n\n"
        "版本: 1.0.0\n"
        "作者: 王睿 (浙江大学)\n"
        "3D引擎: VTK 9.2\n"
        "UI框架: Qt 6\n"
        "CAD内核: OpenCASCADE 7.8\n\n"
        "功能:\n"
        "• STEP/STL模型导入和可视化\n"
        "• 点云数据处理\n"
        "• 喷涂轨迹规划\n"
        "• 机器人仿真\n"
        "• 安全监控\n\n"
        "© 2025 浙江大学");
}

// 空实现的槽函数
void MainWindow::OnNewProject() {}
void MainWindow::OnOpenProject() {}
void MainWindow::OnSaveProject() {}
void MainWindow::OnExportTrajectory() {}
void MainWindow::OnStartSimulation() {}
void MainWindow::OnStopSimulation() {}
void MainWindow::OnConnectRobot() {}
void MainWindow::OnDisconnectRobot() {}
void MainWindow::OnCollisionDetected(const QJsonObject&) {}
void MainWindow::OnSafetyWarning(const QString&) {}
void MainWindow::OnTrajectoryChanged() {}
void MainWindow::OnPointCloudLoadProgress(int) {}
void MainWindow::OnPointCloudLoadCanceled() {}
void MainWindow::updateAllStatus() {}

void MainWindow::loadRobotModel()
{
    // 从应用程序路径向上查找项目根目录
    QString appDir = QApplication::applicationDirPath();
    
    // 尝试多种路径查找机器人模型（build/bin/Debug -> 项目根目录）
    QStringList possiblePaths = {
        appDir + "/../../../data/model/MPX3500.STEP",  // build/bin/Debug -> root
        appDir + "/../../data/model/MPX3500.STEP",     // build/bin -> root
        appDir + "/../data/model/MPX3500.STEP",        // build -> root
        appDir + "/data/model/MPX3500.STEP",
        "data/model/MPX3500.STEP",
        "../data/model/MPX3500.STEP",
        "../../data/model/MPX3500.STEP",
        "../../../data/model/MPX3500.STEP"
    };
    
    QString robotModelPath;
    for (const QString& path : possiblePaths) {
        QFileInfo fi(path);
        if (fi.exists()) {
            robotModelPath = fi.absoluteFilePath();
            qDebug() << "MainWindow: 找到机器人模型:" << robotModelPath;
            break;
        }
    }
    
    if (robotModelPath.isEmpty()) {
        qDebug() << "MainWindow: 未找到机器人模型文件";
        qDebug() << "MainWindow: 应用程序路径:" << appDir;
        if (m_statusPanel) {
            m_statusPanel->addLogMessage("WARNING", "未找到机器人模型文件，仿真模式将使用简化模型");
        }
        return;
    }
    
    qDebug() << "MainWindow: 快速加载机器人模型:" << robotModelPath;
    
    if (m_statusPanel) {
        m_statusPanel->addLogMessage("INFO", "正在快速加载机器人模型...");
    }
    
    // 使用快速加载方法（带缓存）
    if (m_modelTreePanel && m_vtkView) {
        bool success = m_modelTreePanel->loadSTEPFileFast(robotModelPath);
        
        if (success) {
            // 将Actor添加到VTK渲染器
            m_modelTreePanel->addActorsToRenderer(m_vtkView->getRenderer());
            
    // 设置STEP模型树引用到VTKWidget（用于关节变换）
            m_vtkView->SetSTEPModelTreeWidget(m_modelTreePanel);
            
            // 启用机器人显示/隐藏按钮
            m_vtkView->enableRobotToggleButton(true);
            
            m_vtkView->ResetCamera();
            
            // 连接可见性变化信号
            connect(m_modelTreePanel, &STEPModelTreeWidget::partVisibilityChanged,
                    this, [this](const QString& partName, bool visible) {
                        qDebug() << "MainWindow: 部件可见性变化:" << partName << visible;
                        
                        // 刷新VTK渲染
                        if (m_vtkView) {
                            m_vtkView->RefreshRender();
                        }
                        
                        if (m_statusPanel) {
                            m_statusPanel->addLogMessage("INFO", 
                                QString("组件 %1: %2").arg(partName).arg(visible ? "显示" : "隐藏"));
                        }
                    }, Qt::DirectConnection);
            
            if (m_statusPanel) {
                m_statusPanel->addLogMessage("SUCCESS", "机器人模型快速加载完成");
            }
            
            // 显示模型树面板
            if (m_modelTreeDock) {
                m_modelTreeDock->show();
            }
        } else {
            if (m_statusPanel) {
                m_statusPanel->addLogMessage("WARNING", "机器人模型加载失败，仿真模式将使用简化模型");
            }
        }
    }
}
