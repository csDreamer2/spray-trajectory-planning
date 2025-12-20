#include "VTKWidget.h"
#include "StatusPanel.h"
#include <QDebug>
#include <QMessageBox>
#include <QFileInfo>
#include <QApplication>
#include <QDir>
#include <QProcess>
#include <QTextStream>
#include <QFile>
#include <QElapsedTimer>
#include <QMessageBox>

// VTK includes
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkNamedColors.h>
#include <vtkProperty.h>
#include <vtkLight.h>
#include <vtkLightCollection.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkSTLReader.h>
#include <vtkTransform.h>
#include <QTimer>
#include <cmath>

// OpenCASCADE includes for STEP reading
#include <STEPControl_Reader.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <Poly_ArrayOfNodes.hxx>
#include <Poly_Array1OfTriangle.hxx>
#include <TopLoc_Location.hxx>
#include <gp_Pnt.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <IFSelect_ReturnStatus.hxx>

#ifdef Q_OS_WIN
#include <windows.h>
#include <processthreadsapi.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace UI {

VTKWidget::VTKWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_controlLayout(nullptr)
    , m_vtkWidget(nullptr)
    , m_resetCameraBtn(nullptr)
    , m_fitSceneBtn(nullptr)
    , m_toggleAxesBtn(nullptr)
    , m_toggleWorkpieceBtn(nullptr)
    , m_toggleRobotBtn(nullptr)
    , m_statusLabel(nullptr)
    , m_progressBar(nullptr)
    , m_renderer(nullptr)
    , m_renderWindow(nullptr)
    , m_interactor(nullptr)
    , m_workshopActor(nullptr)
    , m_workpieceActor(nullptr)
    , m_robotActor(nullptr)
    , m_trajectoryActor(nullptr)
    , m_axesActor(nullptr)
    , m_axesWidget(nullptr)
    , m_workshopLoaded(false)
    , m_workpieceLoaded(false)
    , m_robotLoaded(false)
    , m_axesVisible(true)
    , m_robotAnimationTimer(nullptr)
    , m_robotTransform(nullptr)
    , m_animationSteps(0)
    , m_currentAnimationStep(0)
    , m_stepLoaderThread(nullptr)
    , m_stepLoaderWorker(nullptr)
    , m_isLoading(false)
    , m_statusPanel(nullptr)
{
    setupUI();
    setupVTKPipeline();
    setupControls();
    
    // 初始化机械臂动画
    m_robotAnimationTimer = new QTimer(this);
    connect(m_robotAnimationTimer, &QTimer::timeout, this, &VTKWidget::updateRobotAnimation);
    
    // 初始化机械臂变换
    m_robotTransform = vtkSmartPointer<vtkTransform>::New();
    
    // 初始化位姿
    for (int i = 0; i < 6; ++i) {
        m_robotCurrentPose[i] = 0.0;
        m_robotTargetPose[i] = 0.0;
    }
}

VTKWidget::~VTKWidget()
{
    // 清理异步STEP加载线程
    if (m_stepLoaderThread) {
        qDebug() << "=== VTKWidget析构: 正在清理异步加载线程 ===";
        
        // 设置停止标志
        {
            QMutexLocker locker(&m_loadingMutex);
            m_isLoading = false;
        }
        
        // 请求线程退出
        m_stepLoaderThread->requestInterruption();
        m_stepLoaderThread->quit();
        
        // 等待线程完成，最多等待5秒
        if (!m_stepLoaderThread->wait(5000)) {
            qWarning() << "警告: 异步加载线程未能在5秒内正常退出，强制终止";
            m_stepLoaderThread->terminate();
            m_stepLoaderThread->wait(1000); // 再等1秒确保终止
        }
        
        qDebug() << "=== VTKWidget析构: 异步加载线程已清理 ===";
    }
    
    // 停止机械臂动画定时器
    if (m_robotAnimationTimer) {
        m_robotAnimationTimer->stop();
    }
    
    qDebug() << "=== VTKWidget析构完成 ===";
    // VTK智能指针会自动清理资源
}

void VTKWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // 状态标签
    m_statusLabel = new QLabel("VTK 3D仿真视图 - 就绪", this);
    m_statusLabel->setStyleSheet(
        "QLabel {"
        "   background-color: #2a2a2a;"
        "   color: #cccccc;"
        "   padding: 5px;"
        "   border-radius: 3px;"
        "   font-size: 12px;"
        "}"
    );
    m_statusLabel->setMaximumHeight(25);
    
    // 进度条
    m_progressBar = new QProgressBar(this);
    m_progressBar->setStyleSheet(
        "QProgressBar {"
        "   background-color: #2a2a2a;"
        "   border: 1px solid #555555;"
        "   border-radius: 3px;"
        "   text-align: center;"
        "   color: #cccccc;"
        "   font-size: 11px;"
        "}"
        "QProgressBar::chunk {"
        "   background-color: #4CAF50;"
        "   border-radius: 2px;"
        "}"
    );
    m_progressBar->setMaximumHeight(20);
    m_progressBar->setVisible(false); // 初始隐藏
    
    // VTK渲染窗口
    m_vtkWidget = new QVTKOpenGLNativeWidget(this);
    m_vtkWidget->setMinimumSize(800, 600);
    
    // 控制按钮区域
    QWidget* controlWidget = new QWidget(this);
    controlWidget->setMaximumHeight(50);
    m_controlLayout = new QHBoxLayout(controlWidget);
    
    // 布局
    m_mainLayout->addWidget(m_statusLabel);
    m_mainLayout->addWidget(m_progressBar);
    m_mainLayout->addWidget(m_vtkWidget, 1);
    m_mainLayout->addWidget(controlWidget);
    
    setLayout(m_mainLayout);
}

void VTKWidget::setupVTKPipeline()
{
    // 创建渲染器
    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    
    // 设置背景色（深灰色，适合工业应用）
    vtkSmartPointer<vtkNamedColors> colors = vtkSmartPointer<vtkNamedColors>::New();
    m_renderer->SetBackground(colors->GetColor3d("DarkSlateGray").GetData());
    
    // 创建渲染窗口
    m_renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    m_renderWindow->AddRenderer(m_renderer);
    
    // 设置VTK widget的渲染窗口
    m_vtkWidget->setRenderWindow(m_renderWindow);
    
    // 获取交互器
    m_interactor = m_renderWindow->GetInteractor();
    
    // 设置交互样式（轨迹球相机控制）
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = 
        vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    m_interactor->SetInteractorStyle(style);
    
    // 添加坐标轴
    m_axesActor = vtkSmartPointer<vtkAxesActor>::New();
    m_axesActor->SetTotalLength(100, 100, 100);
    m_axesActor->SetShaftType(vtkAxesActor::CYLINDER_SHAFT);
    m_axesActor->SetTipType(vtkAxesActor::CONE_TIP);
    
    m_axesWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    m_axesWidget->SetOrientationMarker(m_axesActor);
    m_axesWidget->SetInteractor(m_interactor);
    m_axesWidget->SetViewport(0.0, 0.0, 0.2, 0.2);
    m_axesWidget->SetEnabled(1);
    m_axesWidget->InteractiveOff();
    
    // 设置光照
    vtkSmartPointer<vtkLight> light1 = vtkSmartPointer<vtkLight>::New();
    light1->SetPosition(1, 1, 1);
    light1->SetFocalPoint(0, 0, 0);
    light1->SetColor(1.0, 1.0, 1.0);
    light1->SetIntensity(0.8);
    m_renderer->AddLight(light1);
    
    vtkSmartPointer<vtkLight> light2 = vtkSmartPointer<vtkLight>::New();
    light2->SetPosition(-1, -1, 1);
    light2->SetFocalPoint(0, 0, 0);
    light2->SetColor(1.0, 1.0, 1.0);
    light2->SetIntensity(0.4);
    m_renderer->AddLight(light2);
    
    qDebug() << "✅ VTK渲染管线初始化完成";
}

void VTKWidget::setupControls()
{
    // 重置相机按钮
    m_resetCameraBtn = new QPushButton("重置视角", this);
    m_resetCameraBtn->setMaximumWidth(80);
    connect(m_resetCameraBtn, &QPushButton::clicked, this, &VTKWidget::OnResetCamera);
    
    // 适应场景按钮
    m_fitSceneBtn = new QPushButton("适应场景", this);
    m_fitSceneBtn->setMaximumWidth(80);
    connect(m_fitSceneBtn, &QPushButton::clicked, this, &VTKWidget::OnFitToScene);
    
    // 切换坐标轴按钮
    m_toggleAxesBtn = new QPushButton("坐标轴", this);
    m_toggleAxesBtn->setMaximumWidth(70);
    m_toggleAxesBtn->setCheckable(true);
    m_toggleAxesBtn->setChecked(true);
    connect(m_toggleAxesBtn, &QPushButton::clicked, this, &VTKWidget::OnToggleAxes);
    
    // 切换工件显示按钮
    m_toggleWorkpieceBtn = new QPushButton("工件", this);
    m_toggleWorkpieceBtn->setMaximumWidth(60);
    m_toggleWorkpieceBtn->setCheckable(true);
    m_toggleWorkpieceBtn->setChecked(true);
    m_toggleWorkpieceBtn->setEnabled(false); // 初始禁用，加载工件后启用
    connect(m_toggleWorkpieceBtn, &QPushButton::clicked, this, &VTKWidget::OnToggleWorkpiece);
    
    // 切换机器人显示按钮
    m_toggleRobotBtn = new QPushButton("机器人", this);
    m_toggleRobotBtn->setMaximumWidth(70);
    m_toggleRobotBtn->setCheckable(true);
    m_toggleRobotBtn->setChecked(true);
    m_toggleRobotBtn->setEnabled(false); // 初始禁用，加载机器人后启用
    connect(m_toggleRobotBtn, &QPushButton::clicked, this, &VTKWidget::OnToggleRobot);
    
    // 机械臂控制按钮
    QPushButton* robotControlBtn = new QPushButton("机械臂控制", this);
    robotControlBtn->setMaximumWidth(90);
    robotControlBtn->setEnabled(false); // 初始禁用，加载机器人后启用
    connect(robotControlBtn, &QPushButton::clicked, this, [this]() {
        if (m_robotLoaded) {
            StartRobotAnimation();
        }
    });
    
    // 当机器人加载后启用控制按钮
    connect(this, &VTKWidget::ModelLoaded, this, [robotControlBtn](const QString& modelType, bool success) {
        if (modelType == "Robot" && success) {
            robotControlBtn->setEnabled(true);
        }
    });
    
    // 添加到布局
    m_controlLayout->addStretch();
    m_controlLayout->addWidget(m_resetCameraBtn);
    m_controlLayout->addWidget(m_fitSceneBtn);
    m_controlLayout->addWidget(m_toggleAxesBtn);
    m_controlLayout->addWidget(m_toggleWorkpieceBtn);
    m_controlLayout->addWidget(m_toggleRobotBtn);
    m_controlLayout->addWidget(robotControlBtn);
    m_controlLayout->addStretch();
}

bool VTKWidget::LoadSTEPModel(const QString& filePath, LoadQuality quality)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qWarning() << "STEP文件不存在:" << filePath;
        m_statusLabel->setText("错误: STEP文件不存在");
        return false;
    }
    
    // 检查是否正在加载
    {
        QMutexLocker locker(&m_loadingMutex);
        if (m_isLoading) {
            qWarning() << "已有STEP文件正在加载中，请稍候";
            m_statusLabel->setText("提示: 已有文件正在加载中，请稍候");
            return false;
        }
    } // 锁在这里释放
    
    // 如果没有指定质量，显示质量选择对话框
    LoadQuality selectedQuality = quality;
    if (quality == LoadQuality::Balanced) { // 默认值，显示选择对话框
        QMessageBox qualityDialog(this);
        qualityDialog.setWindowTitle("STEP加载质量选择");
        qualityDialog.setText("请选择STEP文件加载质量：");
        qualityDialog.setInformativeText(
            "快速预览：低精度，约6-7分钟（推荐用于预览）\n"
            "平衡模式：中等精度，约8-10分钟（推荐用于日常使用）\n"
            "高质量：高精度，约15-20分钟（仅用于精确分析）\n\n"
            "注意：高质量模式耗时很长，建议优先使用平衡模式"
        );
        
        QPushButton* fastBtn = qualityDialog.addButton("快速预览", QMessageBox::ActionRole);
        QPushButton* balancedBtn = qualityDialog.addButton("平衡模式", QMessageBox::ActionRole);
        QPushButton* highBtn = qualityDialog.addButton("高质量", QMessageBox::ActionRole);
        qualityDialog.addButton(QMessageBox::Cancel);
        
        qualityDialog.setDefaultButton(balancedBtn);
        
        qualityDialog.exec();
        
        if (qualityDialog.clickedButton() == fastBtn) {
            selectedQuality = LoadQuality::Fast;
        } else if (qualityDialog.clickedButton() == balancedBtn) {
            selectedQuality = LoadQuality::Balanced;
        } else if (qualityDialog.clickedButton() == highBtn) {
            // 高质量模式确认对话框
            QMessageBox confirmDialog(this);
            confirmDialog.setWindowTitle("高质量模式确认");
            confirmDialog.setText("您选择了高质量模式");
            confirmDialog.setInformativeText(
                "高质量模式可能需要15-20分钟的加载时间。\n"
                "对于大多数用途，平衡模式已经提供足够的质量。\n\n"
                "您确定要使用高质量模式吗？"
            );
            confirmDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            confirmDialog.setDefaultButton(QMessageBox::No);
            
            if (confirmDialog.exec() == QMessageBox::Yes) {
                selectedQuality = LoadQuality::High;
            } else {
                selectedQuality = LoadQuality::Balanced; // 默认回到平衡模式
            }
        } else {
            // 用户取消
            m_statusLabel->setText("STEP加载已取消");
            return false;
        }
    }
    
    QString qualityStr;
    switch (selectedQuality) {
        case LoadQuality::Fast: qualityStr = "快速预览"; break;
        case LoadQuality::Balanced: qualityStr = "平衡模式"; break;
        case LoadQuality::High: qualityStr = "高质量"; break;
    }
    
    qDebug() << "=== LoadSTEPModel: Starting async loading ===" << filePath << "质量:" << qualityStr;
    m_statusLabel->setText(QString("开始异步STEP加载 (%1)...").arg(qualityStr));
    
    // 输出开始加载的日志（异步方式，避免阻塞）
    if (m_statusPanel) {
        QFileInfo fileInfo(filePath);
        QString logMessage = QString("开始加载STEP文件: %1 (质量: %2)").arg(fileInfo.fileName()).arg(qualityStr);
        QTimer::singleShot(0, this, [this, logMessage]() {
            if (m_statusPanel) {
                m_statusPanel->addLogMessage("INFO", logMessage);
            }
        });
    }
    
    LoadSTEPModelAsync(filePath, selectedQuality);
    
    qDebug() << "=== LoadSTEPModel: LoadSTEPModelAsync called, returning ===";
    return true;
}

void VTKWidget::LoadSTEPModelAsync(const QString& filePath, LoadQuality quality)
{
    qDebug() << "=== LoadSTEPModelAsync ENTRY ===" << filePath;
    
    QMutexLocker locker(&m_loadingMutex);
    qDebug() << "=== LoadSTEPModelAsync: Mutex locked ===";
    
    if (m_isLoading) {
        qDebug() << "=== LoadSTEPModelAsync: Already loading, returning ===";
        return;
    }
    
    m_isLoading = true;
    qDebug() << "=== LoadSTEPModelAsync: Set m_isLoading = true ===";
    
    m_statusLabel->setText("Starting async STEP file loading...");
    qDebug() << "=== LoadSTEPModelAsync: Status label updated ===";
    
    qDebug() << "=== LoadSTEPModelAsync: About to check thread creation ===";
    
    // 创建工作线程和Worker对象
    if (!m_stepLoaderThread) {
        m_stepLoaderThread = new QThread(this);
        m_stepLoaderWorker = new STEPLoaderWorker(); // 不设置父对象
        
        // 移动Worker到线程
        m_stepLoaderWorker->moveToThread(m_stepLoaderThread);
        
        // 连接信号槽（在moveToThread之后）
        connect(m_stepLoaderWorker, &STEPLoaderWorker::stepLoaded,
                this, &VTKWidget::onSTEPLoaded, Qt::QueuedConnection);
        
        connect(m_stepLoaderWorker, &STEPLoaderWorker::stepLoadFailed,
                this, &VTKWidget::onSTEPLoadFailed, Qt::QueuedConnection);
        
        connect(m_stepLoaderWorker, &STEPLoaderWorker::progressUpdate,
                this, &VTKWidget::onSTEPLoadProgress, Qt::QueuedConnection);
        
        connect(m_stepLoaderWorker, &STEPLoaderWorker::progressPercentage,
                this, &VTKWidget::onSTEPLoadProgressPercentage, Qt::QueuedConnection);
        
        connect(m_stepLoaderWorker, &STEPLoaderWorker::timeStatistics,
                this, &VTKWidget::onTimeStatistics, Qt::QueuedConnection);
        
        // 连接线程清理信号
        connect(m_stepLoaderThread, &QThread::finished,
                m_stepLoaderWorker, &QObject::deleteLater);
        
        // 启动线程并设置高优先级
        m_stepLoaderThread->start(QThread::HighPriority);
        
        qDebug() << "Worker thread started with high priority, sending load request...";
    }
    
    // 发送加载请求到Worker线程，传递质量参数
    QMetaObject::invokeMethod(m_stepLoaderWorker, "loadSTEPFile", 
                              Qt::QueuedConnection, 
                              Q_ARG(QString, filePath),
                              Q_ARG(LoadQuality, quality));
}

bool VTKWidget::LoadSTEPModelSync(const QString& filePath)
{
    // 原来的同步实现，重命名避免递归
    QFileInfo fileInfo(filePath);
    
    qDebug() << "开始同步加载STEP模型:" << filePath;
    m_statusLabel->setText("正在读取STEP文件（同步模式）...");
    QApplication::processEvents();
    
    try {
        // 1️⃣ 使用OpenCASCADE读取STEP文件
        STEPControl_Reader reader;
        std::string pathStr = filePath.toStdString();
        
        IFSelect_ReturnStatus status = reader.ReadFile(pathStr.c_str());
        if (status != IFSelect_RetDone) {
            qCritical() << "❌ 无法读取STEP文件:" << filePath;
            m_statusLabel->setText("错误: STEP文件格式不正确");
            return false;
        }
        
        qDebug() << "✅ STEP文件读取成功";
        m_statusLabel->setText("正在解析STEP几何...");
        QApplication::processEvents();
        
        // 传输根对象
        reader.TransferRoots();
        TopoDS_Shape shape = reader.OneShape();
        
        if (shape.IsNull()) {
            qCritical() << "❌ STEP文件中没有有效的几何体";
            m_statusLabel->setText("错误: STEP文件中没有几何体");
            return false;
        }
        
        qDebug() << "✅ 几何体解析成功";
        m_statusLabel->setText("正在生成网格...");
        QApplication::processEvents();
        
        // 2️⃣ 对Shape做三角化（关键步骤）
        double meshDeflection = 1.0; // 同步模式使用较低精度，提高速度
        BRepMesh_IncrementalMesh mesher(shape, meshDeflection);
        
        if (!mesher.IsDone()) {
            qWarning() << "⚠️ 网格生成可能不完整";
        }
        
        qDebug() << "✅ 网格生成完成";
        m_statusLabel->setText("正在转换为VTK格式...");
        QApplication::processEvents();
        
        // 3️⃣ 转换OCCT Mesh到vtkPolyData
        vtkSmartPointer<vtkPolyData> polyData = ConvertOCCTToVTK(shape);
        
        if (!polyData || polyData->GetNumberOfPoints() == 0) {
            qCritical() << "❌ 转换为VTK格式失败";
            m_statusLabel->setText("错误: 无法转换几何体");
            return false;
        }
        
        qDebug() << "✅ VTK转换成功，点数:" << polyData->GetNumberOfPoints() 
                 << "面数:" << polyData->GetNumberOfCells();
        
        // 4️⃣ 创建VTK显示
        return CreateVTKActorFromPolyData(polyData, fileInfo.baseName());
        
    } catch (const std::exception& e) {
        qCritical() << "❌ STEP加载异常:" << e.what();
        m_statusLabel->setText("错误: STEP文件加载异常");
        return false;
    } catch (...) {
        qCritical() << "❌ STEP加载未知错误";
        m_statusLabel->setText("错误: 未知错误");
        return false;
    }
}

bool VTKWidget::LoadSTLModel(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qWarning() << "STL文件不存在:" << filePath;
        m_statusLabel->setText("错误: STL文件不存在");
        return false;
    }
    
    qDebug() << "开始加载STL模型:" << filePath;
    m_statusLabel->setText("正在加载STL模型...");
    QApplication::processEvents();
    
    try {
        // 使用VTK STL读取器
        vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
        
        // 转换路径格式
        QString absolutePath = fileInfo.absoluteFilePath();
        absolutePath = absolutePath.replace("\\", "/");
        std::string pathStr = absolutePath.toStdString();
        
        qDebug() << "VTK读取STL路径:" << QString::fromStdString(pathStr);
        reader->SetFileName(pathStr.c_str());
        
        try {
            reader->Update();
        } catch (...) {
            qCritical() << "VTK STL读取器内部错误";
            m_statusLabel->setText("错误: STL文件格式不兼容");
            return false;
        }
        
        // 检查读取是否成功
        if (reader->GetOutput() == nullptr || reader->GetOutput()->GetNumberOfPoints() == 0) {
            qCritical() << "VTK STL读取失败：输出为空";
            m_statusLabel->setText("错误: STL文件读取失败");
            return false;
        }
        
        // 创建mapper
        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(reader->GetOutputPort());
        
        // 判断是哪个模型（根据文件名）
        QString fileName = fileInfo.baseName().toLower();
        bool isWorkshop = fileName.contains("总装") || fileName.contains("workshop");
        bool isRobot = fileName.contains("mpx") || fileName.contains("robot");
        
        vtkSmartPointer<vtkActor>* targetActor = nullptr;
        QPushButton* targetButton = nullptr;
        bool* loadedFlag = nullptr;
        QString modelType;
        
        if (isWorkshop) {
            // 移除旧的车间模型
            if (m_workshopActor) {
                m_renderer->RemoveActor(m_workshopActor);
            }
            targetActor = &m_workshopActor;
            loadedFlag = &m_workshopLoaded;
            modelType = "Workshop";
        } else if (isRobot) {
            // 移除旧的机器人模型
            if (m_robotActor) {
                m_renderer->RemoveActor(m_robotActor);
            }
            targetActor = &m_robotActor;
            targetButton = m_toggleRobotBtn;
            loadedFlag = &m_robotLoaded;
            modelType = "Robot";
        } else {
            // 默认作为工件处理
            if (m_workpieceActor) {
                m_renderer->RemoveActor(m_workpieceActor);
            }
            targetActor = &m_workpieceActor;
            targetButton = m_toggleWorkpieceBtn;
            loadedFlag = &m_workpieceLoaded;
            modelType = "Workpiece";
        }
        
        // 创建新的actor
        *targetActor = vtkSmartPointer<vtkActor>::New();
        (*targetActor)->SetMapper(mapper);
        
        // 设置模型显示属性
        if (isWorkshop) {
            (*targetActor)->GetProperty()->SetColor(0.7, 0.7, 0.7); // 灰色车间
            (*targetActor)->GetProperty()->SetOpacity(0.3); // 半透明
        } else if (isRobot) {
            (*targetActor)->GetProperty()->SetColor(0.2, 0.6, 0.8); // 蓝色机器人
        } else {
            (*targetActor)->GetProperty()->SetColor(0.8, 0.6, 0.2); // 橙色工件
        }
        
        (*targetActor)->GetProperty()->SetSpecular(0.3);
        (*targetActor)->GetProperty()->SetSpecularPower(20);
        
        // 添加到渲染器
        m_renderer->AddActor(*targetActor);
        qDebug() << modelType << "Actor已添加到渲染器";
        
        // 获取模型信息
        vtkPolyData* polyData = reader->GetOutput();
        int numPoints = polyData->GetNumberOfPoints();
        int numCells = polyData->GetNumberOfCells();
        
        // 获取模型边界信息
        double bounds[6];
        polyData->GetBounds(bounds);
        qDebug() << modelType << "边界:";
        qDebug() << "  X: [" << bounds[0] << "," << bounds[1] << "]";
        qDebug() << "  Y: [" << bounds[2] << "," << bounds[3] << "]";
        qDebug() << "  Z: [" << bounds[4] << "," << bounds[5] << "]";
        
        double sizeX = bounds[1] - bounds[0];
        double sizeY = bounds[3] - bounds[2];
        double sizeZ = bounds[5] - bounds[4];
        
        *loadedFlag = true;
        if (targetButton) {
            targetButton->setEnabled(true);
        }
        
        qDebug() << "✅" << modelType << "加载成功，点数:" << numPoints << "面数:" << numCells;
        m_statusLabel->setText(QString("%1已加载 (%2 点, %3 面, 尺寸: %4x%5x%6)")
            .arg(modelType).arg(numPoints).arg(numCells)
            .arg(sizeX, 0, 'f', 0).arg(sizeY, 0, 'f', 0).arg(sizeZ, 0, 'f', 0));
        
        // 自动适应场景（如果是第一个加载的模型）
        if (!m_workshopLoaded && !m_workpieceLoaded && !m_robotLoaded) {
            FitToScene();
        } else {
            // 重置相机裁剪平面以包含新模型
            m_renderer->ResetCameraClippingRange();
        }
        
        // 刷新渲染
        m_renderWindow->Render();
        m_vtkWidget->update();
        QApplication::processEvents();
        
        qDebug() << "✅" << modelType << "渲染完成";
        
        emit ModelLoaded(modelType, true);
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "加载STL模型异常:" << e.what();
        m_statusLabel->setText("错误: STL模型加载异常");
        emit ModelLoaded("STL", false);
        return false;
    } catch (...) {
        qCritical() << "加载STL模型未知错误";
        m_statusLabel->setText("错误: 未知错误");
        emit ModelLoaded("STL", false);
        return false;
    }
}

vtkSmartPointer<vtkPolyData> VTKWidget::ConvertOCCTToVTK(const TopoDS_Shape& shape)
{
    // 3️⃣ 核心代码：OCCT Mesh → vtkPolyData
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();
    
    int totalPoints = 0;
    int totalTriangles = 0;
    
    // 遍历所有面
    for (TopExp_Explorer exp(shape, TopAbs_FACE); exp.More(); exp.Next()) {
        TopoDS_Face face = TopoDS::Face(exp.Current());
        TopLoc_Location loc;
        Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(face, loc);
        
        if (tri.IsNull()) {
            qWarning() << "⚠️ 面没有三角化数据，跳过";
            continue;
        }
        
        // 兼容的OpenCASCADE API调用
        vtkIdType offset = points->GetNumberOfPoints();
        
        // 获取节点数量和三角形数量
        int nbNodes = tri->NbNodes();
        int nbTriangles = tri->NbTriangles();
        
        // 添加顶点
        for (int i = 1; i <= nbNodes; ++i) {
            gp_Pnt p = tri->Node(i).Transformed(loc.Transformation());
            points->InsertNextPoint(p.X(), p.Y(), p.Z());
            totalPoints++;
        }
        
        // 添加三角形
        for (int i = 1; i <= nbTriangles; ++i) {
            int n1, n2, n3;
            tri->Triangle(i).Get(n1, n2, n3);
            
            vtkIdType ids[3] = {
                offset + n1 - 1,
                offset + n2 - 1,
                offset + n3 - 1
            };
            triangles->InsertNextCell(3, ids);
            totalTriangles++;
        }
    }
    
    polyData->SetPoints(points);
    polyData->SetPolys(triangles);
    
    qDebug() << "✅ OCCT→VTK转换完成，顶点:" << totalPoints << "三角形:" << totalTriangles;
    
    return polyData;
}

bool VTKWidget::CreateVTKActorFromPolyData(vtkSmartPointer<vtkPolyData> polyData, const QString& modelName)
{
    // 4️⃣ VTK显示（工业级实现）
    qDebug() << "Creating VTK mapper for" << polyData->GetNumberOfPoints() << "points," 
             << polyData->GetNumberOfCells() << "cells";
    
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    
    // 对于大型模型，优化渲染设置
    int numCells = polyData->GetNumberOfCells();
    if (numCells > 100000) {
        qDebug() << "Large model detected (" << numCells << " cells), optimizing rendering";
        // VTK 9.2中使用其他优化方法
        mapper->SetStatic(1); // 标记为静态数据，优化GPU缓存
    }
    
    mapper->SetInputData(polyData);
    
    // 判断模型类型（基于文件名）
    QString lowerName = modelName.toLower();
    bool isWorkshop = lowerName.contains("总装") || lowerName.contains("workshop");
    bool isRobot = lowerName.contains("mpx") || lowerName.contains("robot") || lowerName.contains("arm");
    
    vtkSmartPointer<vtkActor>* targetActor = nullptr;
    QPushButton* targetButton = nullptr;
    bool* loadedFlag = nullptr;
    QString modelType;
    
    if (isWorkshop) {
        // 车间总装模型
        if (m_workshopActor) {
            m_renderer->RemoveActor(m_workshopActor);
        }
        targetActor = &m_workshopActor;
        loadedFlag = &m_workshopLoaded;
        modelType = "Workshop";
    } else if (isRobot) {
        // 机械臂模型
        if (m_robotActor) {
            m_renderer->RemoveActor(m_robotActor);
        }
        targetActor = &m_robotActor;
        targetButton = m_toggleRobotBtn;
        loadedFlag = &m_robotLoaded;
        modelType = "Robot";
    } else {
        // 默认作为工件处理
        if (m_workpieceActor) {
            m_renderer->RemoveActor(m_workpieceActor);
        }
        targetActor = &m_workpieceActor;
        targetButton = m_toggleWorkpieceBtn;
        loadedFlag = &m_workpieceLoaded;
        modelType = "Workpiece";
    }
    
    // 创建新的actor
    *targetActor = vtkSmartPointer<vtkActor>::New();
    (*targetActor)->SetMapper(mapper);
    
    // 设置材质属性（工业级外观）
    if (isWorkshop) {
        (*targetActor)->GetProperty()->SetColor(0.7, 0.7, 0.7); // 灰色车间
        (*targetActor)->GetProperty()->SetOpacity(0.3); // 半透明
    } else if (isRobot) {
        (*targetActor)->GetProperty()->SetColor(0.2, 0.6, 0.8); // 蓝色机械臂
        (*targetActor)->GetProperty()->SetMetallic(0.3);
        (*targetActor)->GetProperty()->SetRoughness(0.2);
    } else {
        (*targetActor)->GetProperty()->SetColor(0.8, 0.6, 0.2); // 橙色工件
    }
    
    (*targetActor)->GetProperty()->SetSpecular(0.3);
    (*targetActor)->GetProperty()->SetSpecularPower(20);
    
    // 添加到渲染器
    m_renderer->AddActor(*targetActor);
    qDebug() << "✅" << modelType << "Actor已添加到渲染器";
    
    // 获取模型边界信息
    double bounds[6];
    polyData->GetBounds(bounds);
    double sizeX = bounds[1] - bounds[0];
    double sizeY = bounds[3] - bounds[2];
    double sizeZ = bounds[5] - bounds[4];
    
    *loadedFlag = true;
    if (targetButton) {
        targetButton->setEnabled(true);
    }
    
    qDebug() << "✅" << modelType << "加载成功，点数:" << polyData->GetNumberOfPoints() 
             << "面数:" << polyData->GetNumberOfCells();
    m_statusLabel->setText(QString("%1已加载 (%2 点, %3 面, 尺寸: %4x%5x%6)")
        .arg(modelType).arg(polyData->GetNumberOfPoints()).arg(polyData->GetNumberOfCells())
        .arg(sizeX, 0, 'f', 0).arg(sizeY, 0, 'f', 0).arg(sizeZ, 0, 'f', 0));
    
    // 自动适应场景
    if (!m_workshopLoaded && !m_workpieceLoaded && !m_robotLoaded) {
        FitToScene();
    } else {
        m_renderer->ResetCameraClippingRange();
    }
    
    // 立即渲染，但添加异常保护
    qDebug() << "Performing immediate rendering for" << modelType;
    try {
        m_renderWindow->Render();
        m_vtkWidget->update();
        QApplication::processEvents(); // 确保事件处理
        qDebug() << "✅" << modelType << "渲染完成";
    } catch (const std::exception& e) {
        qWarning() << "渲染异常:" << e.what();
    } catch (...) {
        qWarning() << "渲染未知异常";
    }
    
    emit ModelLoaded(modelType, true);
    return true;
}

void VTKWidget::SetRobotPose(double x, double y, double z, double rx, double ry, double rz)
{
    if (!m_robotActor) {
        qWarning() << "机械臂模型未加载";
        return;
    }
    
    // 更新当前位姿
    m_robotCurrentPose[0] = x;
    m_robotCurrentPose[1] = y;
    m_robotCurrentPose[2] = z;
    m_robotCurrentPose[3] = rx;
    m_robotCurrentPose[4] = ry;
    m_robotCurrentPose[5] = rz;
    
    // 应用变换
    m_robotTransform->Identity();
    m_robotTransform->Translate(x, y, z);
    m_robotTransform->RotateX(rx);
    m_robotTransform->RotateY(ry);
    m_robotTransform->RotateZ(rz);
    
    m_robotActor->SetUserTransform(m_robotTransform);
    
    // 刷新渲染
    m_renderWindow->Render();
    
    qDebug() << "机械臂位姿已更新:" << x << y << z << rx << ry << rz;
}

void VTKWidget::AnimateRobotToPosition(double x, double y, double z, double rx, double ry, double rz, int durationMs)
{
    if (!m_robotActor) {
        qWarning() << "机械臂模型未加载";
        return;
    }
    
    // 设置目标位姿
    m_robotTargetPose[0] = x;
    m_robotTargetPose[1] = y;
    m_robotTargetPose[2] = z;
    m_robotTargetPose[3] = rx;
    m_robotTargetPose[4] = ry;
    m_robotTargetPose[5] = rz;
    
    // 计算动画参数
    m_animationSteps = durationMs / 50; // 50ms间隔，约20fps
    m_currentAnimationStep = 0;
    
    qDebug() << "开始机械臂动画，目标位置:" << x << y << z << rx << ry << rz;
    qDebug() << "动画步数:" << m_animationSteps << "持续时间:" << durationMs << "ms";
    
    // 启动动画定时器
    m_robotAnimationTimer->start(50);
}

void VTKWidget::StartRobotAnimation()
{
    if (!m_robotActor) {
        qWarning() << "机械臂模型未加载";
        return;
    }
    
    // 演示动画：机械臂做圆周运动
    static double angle = 0.0;
    
    double radius = 200.0;
    double x = radius * cos(angle);
    double y = radius * sin(angle);
    double z = 100.0;
    double rx = 0.0;
    double ry = 0.0;
    double rz = angle * 180.0 / M_PI; // 跟随圆周方向
    
    AnimateRobotToPosition(x, y, z, rx, ry, rz, 1000);
    
    angle += M_PI / 6; // 每次转30度
    if (angle >= 2 * M_PI) {
        angle = 0.0;
    }
}

void VTKWidget::updateRobotAnimation()
{
    if (m_currentAnimationStep >= m_animationSteps) {
        // 动画完成
        m_robotAnimationTimer->stop();
        qDebug() << "机械臂动画完成";
        return;
    }
    
    // 计算插值比例
    double t = static_cast<double>(m_currentAnimationStep) / m_animationSteps;
    
    // 线性插值计算当前位姿
    double currentPose[6];
    for (int i = 0; i < 6; ++i) {
        currentPose[i] = m_robotCurrentPose[i] + t * (m_robotTargetPose[i] - m_robotCurrentPose[i]);
    }
    
    // 应用变换
    m_robotTransform->Identity();
    m_robotTransform->Translate(currentPose[0], currentPose[1], currentPose[2]);
    m_robotTransform->RotateX(currentPose[3]);
    m_robotTransform->RotateY(currentPose[4]);
    m_robotTransform->RotateZ(currentPose[5]);
    
    m_robotActor->SetUserTransform(m_robotTransform);
    
    // 刷新渲染
    m_renderWindow->Render();
    
    m_currentAnimationStep++;
    
    // 动画完成时更新当前位姿
    if (m_currentAnimationStep >= m_animationSteps) {
        for (int i = 0; i < 6; ++i) {
            m_robotCurrentPose[i] = m_robotTargetPose[i];
        }
    }
}

bool VTKWidget::LoadPointCloud(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qWarning() << "点云文件不存在:" << filePath;
        m_statusLabel->setText("错误: 点云文件不存在");
        return false;
    }
    
    qDebug() << "开始加载点云:" << filePath;
    qDebug() << "文件绝对路径:" << fileInfo.absoluteFilePath();
    m_statusLabel->setText("正在加载点云数据...");
    QApplication::processEvents();
    
    try {
        // 使用VTK PLY读取器
        vtkSmartPointer<vtkPLYReader> reader = vtkSmartPointer<vtkPLYReader>::New();
        
        // 🔧 关键修复：使用绝对路径并转换为标准路径格式
        QString absolutePath = fileInfo.absoluteFilePath();
        // 将Windows路径的反斜杠转换为正斜杠（VTK更兼容）
        absolutePath = absolutePath.replace("\\", "/");
        
        // 使用UTF-8编码（VTK推荐）
        std::string pathStr = absolutePath.toStdString();
        qDebug() << "VTK读取路径:" << QString::fromStdString(pathStr);
        
        reader->SetFileName(pathStr.c_str());
        
        // 🔧 添加错误处理：捕获VTK内部错误
        try {
            reader->Update();
        } catch (...) {
            qCritical() << "VTK PLY读取器内部错误";
            m_statusLabel->setText("错误: PLY文件格式不兼容");
            
            // 尝试手动创建简单点云作为备用方案
            qDebug() << "尝试创建备用测试点云...";
            return CreateFallbackPointCloud();
        }
        
        // 检查读取是否成功
        if (reader->GetOutput() == nullptr || reader->GetOutput()->GetNumberOfPoints() == 0) {
            qCritical() << "VTK读取失败：输出为空";
            m_statusLabel->setText("错误: PLY文件读取失败");
            
            // 尝试备用方案
            qDebug() << "尝试创建备用测试点云...";
            return CreateFallbackPointCloud();
        }
        
        // 创建点云可视化
        vtkSmartPointer<vtkVertexGlyphFilter> vertexFilter = 
            vtkSmartPointer<vtkVertexGlyphFilter>::New();
        vertexFilter->SetInputConnection(reader->GetOutputPort());
        vertexFilter->Update();
        
        // 创建mapper
        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(vertexFilter->GetOutputPort());
        
        // 🔧 关键修复：先移除旧的actor，再创建新的
        if (m_workpieceActor) {
            m_renderer->RemoveActor(m_workpieceActor);
            m_workpieceActor = nullptr;
        }
        
        // 创建新的actor
        m_workpieceActor = vtkSmartPointer<vtkActor>::New();
        m_workpieceActor->SetMapper(mapper);
        
        // 设置点云显示属性
        m_workpieceActor->GetProperty()->SetColor(0.8, 0.2, 0.2); // 红色点云
        m_workpieceActor->GetProperty()->SetPointSize(3.0);  // 增大点大小
        m_workpieceActor->GetProperty()->SetRenderPointsAsSpheres(false); // 关闭球体渲染，提高性能
        
        // 添加到渲染器
        m_renderer->AddActor(m_workpieceActor);
        qDebug() << "Actor已添加到渲染器";
        
        // 获取点云信息
        vtkPolyData* polyData = reader->GetOutput();
        int numPoints = polyData->GetNumberOfPoints();
        
        qDebug() << "✅ 点云读取成功，点数:" << numPoints;
        
        if (numPoints == 0) {
            qWarning() << "警告: 点云文件为空";
            m_statusLabel->setText("警告: 点云文件为空");
            emit ModelLoaded("PointCloud", false);
            return false;
        }
        
        // 🔧 获取点云边界信息用于调试
        double bounds[6];
        polyData->GetBounds(bounds);
        qDebug() << "点云边界:";
        qDebug() << "  X: [" << bounds[0] << "," << bounds[1] << "]";
        qDebug() << "  Y: [" << bounds[2] << "," << bounds[3] << "]";
        qDebug() << "  Z: [" << bounds[4] << "," << bounds[5] << "]";
        
        // 计算点云尺度
        double sizeX = bounds[1] - bounds[0];
        double sizeY = bounds[3] - bounds[2];
        double sizeZ = bounds[5] - bounds[4];
        double maxSize = std::max({sizeX, sizeY, sizeZ});
        qDebug() << "点云尺寸: " << sizeX << " x " << sizeY << " x " << sizeZ;
        qDebug() << "最大尺寸: " << maxSize;
        
        // 🔧 根据点云尺度动态调整点大小
        double pointSize = 2.0;
        if (maxSize > 10000) {
            pointSize = 5.0;  // 大型工件用更大的点
        } else if (maxSize > 1000) {
            pointSize = 3.0;
        } else if (maxSize < 10) {
            pointSize = 1.0;  // 小型工件用小点
        }
        
        m_workpieceActor->GetProperty()->SetPointSize(pointSize);
        qDebug() << "点大小设置为:" << pointSize;
        
        m_workpieceLoaded = true;
        m_toggleWorkpieceBtn->setEnabled(true);
        
        qDebug() << "✅ 点云加载成功，点数:" << numPoints;
        m_statusLabel->setText(QString("点云已加载 (%1 个点, 尺寸: %2x%3x%4)")
            .arg(numPoints)
            .arg(sizeX, 0, 'f', 0).arg(sizeY, 0, 'f', 0).arg(sizeZ, 0, 'f', 0));
        
        // 🔧 关键修复：确保相机正确对准点云
        m_renderer->ResetCamera();
        
        // 获取相机并调整
        vtkCamera* camera = m_renderer->GetActiveCamera();
        if (camera) {
            // 计算点云中心
            double centerX = (bounds[0] + bounds[1]) / 2.0;
            double centerY = (bounds[2] + bounds[3]) / 2.0;
            double centerZ = (bounds[4] + bounds[5]) / 2.0;
            
            qDebug() << "点云中心: (" << centerX << "," << centerY << "," << centerZ << ")";
            
            // 设置相机焦点为点云中心
            camera->SetFocalPoint(centerX, centerY, centerZ);
            
            // 设置相机位置（从斜上方观察）
            double distance = maxSize * 2.0;
            camera->SetPosition(
                centerX + distance,
                centerY - distance,
                centerZ + distance
            );
            
            // 设置向上方向
            camera->SetViewUp(0, 0, 1);
            
            // 重置裁剪平面
            m_renderer->ResetCameraClippingRange();
            
            qDebug() << "相机位置已调整";
        }
        
        // 🔧 关键修复：强制刷新渲染
        m_renderWindow->Render();
        
        // 强制Qt widget更新
        m_vtkWidget->update();
        QApplication::processEvents();
        
        qDebug() << "✅ 渲染完成";
        
        emit ModelLoaded("PointCloud", true);
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "加载点云异常:" << e.what();
        m_statusLabel->setText("错误: 点云加载异常");
        emit ModelLoaded("PointCloud", false);
        return false;
    } catch (...) {
        qCritical() << "加载点云未知错误";
        m_statusLabel->setText("错误: 未知错误");
        emit ModelLoaded("PointCloud", false);
        return false;
    }
}

bool VTKWidget::LoadRobotModel(const QString& urdfPath)
{
    // 机器人模型加载（URDF支持）
    // 这里先预留接口，后续实现
    qDebug() << "机器人模型加载功能待实现:" << urdfPath;
    m_statusLabel->setText("机器人模型加载功能开发中...");
    return false;
}

void VTKWidget::ShowSprayTrajectory(const std::vector<std::array<double, 3>>& trajectory)
{
    if (trajectory.empty()) {
        qWarning() << "轨迹数据为空";
        return;
    }
    
    qDebug() << "显示喷涂轨迹，点数:" << trajectory.size();
    
    // 创建轨迹点
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    for (const auto& point : trajectory) {
        points->InsertNextPoint(point[0], point[1], point[2]);
    }
    
    // 创建轨迹线
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    
    // 创建线段连接
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    for (size_t i = 0; i < trajectory.size() - 1; ++i) {
        vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
        line->GetPointIds()->SetId(0, i);
        line->GetPointIds()->SetId(1, i + 1);
        lines->InsertNextCell(line);
    }
    polyData->SetLines(lines);
    
    // 创建mapper
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);
    
    // 创建或更新actor
    if (!m_trajectoryActor) {
        m_trajectoryActor = vtkSmartPointer<vtkActor>::New();
        m_renderer->AddActor(m_trajectoryActor);
    }
    
    m_trajectoryActor->SetMapper(mapper);
    
    // 设置轨迹显示属性
    m_trajectoryActor->GetProperty()->SetColor(0.0, 1.0, 0.0); // 绿色轨迹
    m_trajectoryActor->GetProperty()->SetLineWidth(3.0);
    
    // 刷新渲染
    m_renderWindow->Render();
    
    m_statusLabel->setText(QString("轨迹已显示 (%1 个点)").arg(trajectory.size()));
}

void VTKWidget::ClearTrajectory()
{
    if (m_trajectoryActor) {
        m_renderer->RemoveActor(m_trajectoryActor);
        m_trajectoryActor = nullptr;
        m_renderWindow->Render();
        qDebug() << "轨迹已清除";
    }
}

void VTKWidget::ResetCamera()
{
    m_renderer->ResetCamera();
    m_renderWindow->Render();
    qDebug() << "相机已重置";
}

void VTKWidget::FitToScene()
{
    m_renderer->ResetCamera();
    m_renderer->GetActiveCamera()->Zoom(0.8); // 稍微缩小以留出边距
    m_renderWindow->Render();
    qDebug() << "场景已适应";
}

void VTKWidget::SetViewMode(const QString& mode)
{
    vtkCamera* camera = m_renderer->GetActiveCamera();
    
    if (mode == "front") {
        camera->SetPosition(0, -1000, 0);
        camera->SetViewUp(0, 0, 1);
    } else if (mode == "top") {
        camera->SetPosition(0, 0, 1000);
        camera->SetViewUp(0, 1, 0);
    } else if (mode == "iso") {
        camera->SetPosition(1000, -1000, 1000);
        camera->SetViewUp(0, 0, 1);
    }
    
    camera->SetFocalPoint(0, 0, 0);
    m_renderer->ResetCamera();
    m_renderWindow->Render();
    
    qDebug() << "视图模式设置为:" << mode;
}

void VTKWidget::SetWorkpieceVisible(bool visible)
{
    if (m_workpieceActor) {
        m_workpieceActor->SetVisibility(visible);
        m_renderWindow->Render();
    }
}

void VTKWidget::SetRobotVisible(bool visible)
{
    if (m_robotActor) {
        m_robotActor->SetVisibility(visible);
        m_renderWindow->Render();
    }
}

void VTKWidget::SetTrajectoryVisible(bool visible)
{
    if (m_trajectoryActor) {
        m_trajectoryActor->SetVisibility(visible);
        m_renderWindow->Render();
    }
}

// 槽函数实现
void VTKWidget::OnResetCamera()
{
    ResetCamera();
    emit CameraChanged();
}

void VTKWidget::OnFitToScene()
{
    FitToScene();
    emit CameraChanged();
}

void VTKWidget::OnToggleAxes()
{
    m_axesVisible = !m_axesVisible;
    m_axesWidget->SetEnabled(m_axesVisible);
    m_renderWindow->Render();
    
    m_toggleAxesBtn->setText(m_axesVisible ? "坐标轴" : "坐标轴");
    qDebug() << "坐标轴显示:" << (m_axesVisible ? "开启" : "关闭");
}

void VTKWidget::OnToggleWorkpiece()
{
    bool visible = m_toggleWorkpieceBtn->isChecked();
    SetWorkpieceVisible(visible);
    qDebug() << "工件显示:" << (visible ? "开启" : "关闭");
}

void VTKWidget::OnToggleRobot()
{
    bool visible = m_toggleRobotBtn->isChecked();
    SetRobotVisible(visible);
    qDebug() << "机器人显示:" << (visible ? "开启" : "关闭");
}

void VTKWidget::updateScene()
{
    m_renderWindow->Render();
}

bool VTKWidget::CreateFallbackPointCloud()
{
    qDebug() << "创建备用测试点云...";
    
    try {
        // 手动创建点云数据
        vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
        
        // 创建一个简单的立方体点云
        for (int x = -5; x <= 5; x += 2) {
            for (int y = -5; y <= 5; y += 2) {
                for (int z = -5; z <= 5; z += 2) {
                    points->InsertNextPoint(x, y, z);
                }
            }
        }
        
        // 创建PolyData
        vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
        polyData->SetPoints(points);
        
        // 创建顶点过滤器
        vtkSmartPointer<vtkVertexGlyphFilter> vertexFilter = 
            vtkSmartPointer<vtkVertexGlyphFilter>::New();
        vertexFilter->SetInputData(polyData);
        vertexFilter->Update();
        
        // 创建mapper
        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(vertexFilter->GetOutputPort());
        
        // 创建或更新actor
        if (!m_workpieceActor) {
            m_workpieceActor = vtkSmartPointer<vtkActor>::New();
            m_renderer->AddActor(m_workpieceActor);
        }
        
        m_workpieceActor->SetMapper(mapper);
        
        // 设置点云显示属性
        m_workpieceActor->GetProperty()->SetColor(0.2, 0.8, 0.2); // 绿色点云（备用）
        m_workpieceActor->GetProperty()->SetPointSize(4.0);
        m_workpieceActor->GetProperty()->SetRenderPointsAsSpheres(true);
        
        int numPoints = points->GetNumberOfPoints();
        
        m_workpieceLoaded = true;
        m_toggleWorkpieceBtn->setEnabled(true);
        
        qDebug() << "✅ 备用点云创建成功，点数:" << numPoints;
        m_statusLabel->setText(QString("备用点云已加载 (%1 个点)").arg(numPoints));
        
        // 自动适应场景
        FitToScene();
        
        // 刷新渲染
        m_renderWindow->Render();
        
        emit ModelLoaded("PointCloud", true);
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "创建备用点云失败:" << e.what();
        m_statusLabel->setText("错误: 备用点云创建失败");
        emit ModelLoaded("PointCloud", false);
        return false;
    }
}

} // namespace UI

// STEPLoaderWorker实现（在namespace外面）
void STEPLoaderWorker::loadSTEPFile(const QString& filePath, LoadQuality quality)
{
    qDebug() << "=== WORKER THREAD: Starting STEP file loading ===" << filePath;
    
    // 检查线程中断请求
    if (QThread::currentThread()->isInterruptionRequested()) {
        qDebug() << "WORKER: 线程中断请求，停止加载";
        return;
    }
    
    // 根据质量设置网格精度
    double meshDeflection;
    QString qualityStr;
    switch (quality) {
        case LoadQuality::Fast:
            meshDeflection = 5.0;
            qualityStr = "快速预览";
            break;
        case LoadQuality::Balanced:
            meshDeflection = 2.0;
            qualityStr = "平衡模式";
            break;
        case LoadQuality::High:
            meshDeflection = 0.3;
            qualityStr = "高质量";
            break;
    }
    
    qDebug() << "WORKER: 加载质量:" << qualityStr << "网格精度:" << meshDeflection;
    
    QElapsedTimer totalTimer;
    totalTimer.start();
    
    try {
        // 1️⃣ 使用OpenCASCADE读取STEP文件
        QElapsedTimer stepTimer;
        stepTimer.start();
        
        emit progressPercentage(0);
        qDebug() << "WORKER: Emitting progress update - Reading STEP file...";
        emit progressUpdate(QString("正在读取STEP文件 (%1)...").arg(qualityStr));
        
        // 检查中断
        if (QThread::currentThread()->isInterruptionRequested()) {
            qDebug() << "WORKER: 线程中断请求，停止加载";
            return;
        }
        
        STEPControl_Reader reader;
        std::string pathStr = filePath.toStdString();
        

        
        qDebug() << "WORKER: Starting STEP file reading...";
        IFSelect_ReturnStatus status = reader.ReadFile(pathStr.c_str());
        if (status != IFSelect_RetDone) {
            emit stepLoadFailed("无法读取STEP文件，格式可能不正确");
            return;
        }
        
        int stepReadTime = stepTimer.elapsed();
        emit timeStatistics("STEP文件读取", stepReadTime);
        qDebug() << "WORKER: STEP文件读取完成，耗时:" << stepReadTime << "ms";
        
        emit progressPercentage(15);
        
        // 2️⃣ 几何解析阶段
        stepTimer.restart();
        qDebug() << "WORKER: STEP file read successfully, parsing geometry...";
        emit progressUpdate("正在解析STEP几何体...");
        
        // 检查中断
        if (QThread::currentThread()->isInterruptionRequested()) {
            qDebug() << "WORKER: 线程中断请求，停止加载";
            return;
        }
        
        // 传输根对象（这是最耗时的操作）
        qDebug() << "WORKER: Starting TransferRoots() - this may take several minutes for complex models...";
        
        QString timeEstimate;
        switch (quality) {
            case LoadQuality::Fast:
                timeEstimate = "约2-3分钟";
                break;
            case LoadQuality::Balanced:
                timeEstimate = "约4-6分钟";
                break;
            case LoadQuality::High:
                timeEstimate = "约10-15分钟";
                break;
        }
        
        emit progressUpdate(QString("正在提取几何体 (%1)...").arg(timeEstimate));
        emit progressPercentage(25);
        
        // 开始几何体解析
        qDebug() << "WORKER: 开始几何体解析，预计耗时" << timeEstimate;
        
        // 由于TransferRoots是阻塞调用，我们无法在其内部提供进度更新
        // 但我们可以在开始前给用户明确的预期
        qDebug() << "WORKER: 即将调用TransferRoots()...";
        
        try {
            reader.TransferRoots();
            qDebug() << "WORKER: TransferRoots() 调用完成，开始后续处理...";
        } catch (const std::exception& e) {
            qWarning() << "WORKER: TransferRoots() 异常:" << e.what();
            emit stepLoadFailed(QString("几何体解析异常: %1").arg(e.what()));
            return;
        } catch (...) {
            qWarning() << "WORKER: TransferRoots() 未知异常";
            emit stepLoadFailed("几何体解析发生未知异常");
            return;
        }
        
        // 关键修复：强制刷新线程状态，防止TransferRoots完成后卡住
        QThread::msleep(10); // 短暂休眠让线程调度器有机会处理
        QCoreApplication::processEvents(); // 处理待处理的事件
        
        // 立即继续执行，延迟发送进度信号
        qDebug() << "WORKER: TransferRoots() completed, extracting shape...";
        
        // 先发送文本更新
        emit progressUpdate("正在完成几何体提取...");
        
        // 使用定时器延迟发送进度百分比，避免阻塞
        qDebug() << "WORKER: 安排延迟发送进度更新信号 60%...";
        QTimer::singleShot(10, this, [this]() {
            qDebug() << "WORKER: 延迟发送进度信号 60%";
            emit progressPercentage(60);
        });
        
        qDebug() << "WORKER: 调用OneShape()获取几何体...";
        TopoDS_Shape shape = reader.OneShape();
        qDebug() << "WORKER: OneShape()调用完成";
        
        if (shape.IsNull()) {
            qDebug() << "WORKER: ERROR - No valid geometry in STEP file";
            emit stepLoadFailed("STEP文件中没有找到有效的几何体");
            return;
        }
        
        qDebug() << "WORKER: 几何体有效，计算解析时间...";
        int geometryParseTime = stepTimer.elapsed();
        qDebug() << "WORKER: 发送几何体解析时间统计...";
        emit timeStatistics("几何体解析", geometryParseTime);
        qDebug() << "WORKER: 几何体解析完成，耗时:" << geometryParseTime << "ms";
        
        emit progressPercentage(70);
        
        // 3️⃣ 网格生成阶段
        stepTimer.restart();
        qDebug() << "WORKER: Geometry extracted successfully, generating mesh...";
        emit progressUpdate(QString("正在生成网格 (%1, 精度: %2)...").arg(qualityStr).arg(meshDeflection));
        
        // 检查中断
        if (QThread::currentThread()->isInterruptionRequested()) {
            qDebug() << "WORKER: 线程中断请求，停止加载";
            return;
        }
        
        qDebug() << "WORKER: Using mesh deflection:" << meshDeflection << "for" << qualityStr;
        
        BRepMesh_IncrementalMesh mesher(shape, meshDeflection);
        
        if (!mesher.IsDone()) {
            qWarning() << "WORKER: 网格生成可能不完整";
        }
        
        int meshGenerationTime = stepTimer.elapsed();
        emit timeStatistics("网格生成", meshGenerationTime);
        qDebug() << "WORKER: 网格生成完成，耗时:" << meshGenerationTime << "ms";
        
        emit progressPercentage(85);
        
        // 4️⃣ VTK转换阶段
        stepTimer.restart();
        qDebug() << "WORKER: Mesh generated, converting to VTK format...";
        emit progressUpdate("正在转换为VTK格式...");
        
        // 检查中断
        if (QThread::currentThread()->isInterruptionRequested()) {
            qDebug() << "WORKER: 线程中断请求，停止加载";
            return;
        }
        
        // 转换OCCT Mesh到vtkPolyData
        vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
        vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
        vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();
        
        int totalPoints = 0;
        int totalTriangles = 0;
        
        // 遍历所有面
        for (TopExp_Explorer exp(shape, TopAbs_FACE); exp.More(); exp.Next()) {
            // 定期检查中断请求
            if (totalPoints % 10000 == 0 && QThread::currentThread()->isInterruptionRequested()) {
                qDebug() << "WORKER: 线程中断请求，停止VTK转换";
                return;
            }
            
            TopoDS_Face face = TopoDS::Face(exp.Current());
            TopLoc_Location loc;
            Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(face, loc);
            
            if (tri.IsNull()) {
                continue;
            }
            
            vtkIdType offset = points->GetNumberOfPoints();
            
            // 获取节点数量和三角形数量
            int nbNodes = tri->NbNodes();
            int nbTriangles = tri->NbTriangles();
            
            // 添加顶点
            for (int i = 1; i <= nbNodes; ++i) {
                gp_Pnt p = tri->Node(i).Transformed(loc.Transformation());
                points->InsertNextPoint(p.X(), p.Y(), p.Z());
                totalPoints++;
            }
            
            // 添加三角形
            for (int i = 1; i <= nbTriangles; ++i) {
                int n1, n2, n3;
                tri->Triangle(i).Get(n1, n2, n3);
                
                vtkIdType ids[3] = {
                    offset + n1 - 1,
                    offset + n2 - 1,
                    offset + n3 - 1
                };
                triangles->InsertNextCell(3, ids);
                totalTriangles++;
            }
        }
        
        // 最后检查中断
        if (QThread::currentThread()->isInterruptionRequested()) {
            qDebug() << "WORKER: 线程中断请求，停止最终处理";
            return;
        }
        
        polyData->SetPoints(points);
        polyData->SetPolys(triangles);
        
        if (polyData->GetNumberOfPoints() == 0) {
            emit stepLoadFailed("转换为VTK格式失败，没有生成有效的几何体");
            return;
        }
        
        int vtkConversionTime = stepTimer.elapsed();
        emit timeStatistics("VTK转换", vtkConversionTime);
        qDebug() << "WORKER: VTK转换完成，耗时:" << vtkConversionTime << "ms";
        
        emit progressPercentage(95);
        
        // 总时间统计
        int totalTime = totalTimer.elapsed();
        emit timeStatistics("总计", totalTime);
        
        qDebug() << "✅ 异步STEP加载成功，点数:" << totalPoints << "三角形:" << totalTriangles;
        qDebug() << "总耗时:" << totalTime << "ms (" << (totalTime/1000.0) << "秒)";
        
        QFileInfo fileInfo(filePath);
        emit stepLoaded(polyData, fileInfo.baseName());
        
        emit progressPercentage(100);
        
    } catch (const std::exception& e) {
        if (!QThread::currentThread()->isInterruptionRequested()) {
            emit stepLoadFailed(QString("STEP加载异常: %1").arg(e.what()));
        }
    } catch (...) {
        if (!QThread::currentThread()->isInterruptionRequested()) {
            emit stepLoadFailed("STEP加载未知错误");
        }
    }
}

// VTKWidget槽函数实现（在namespace内）
namespace UI {

void VTKWidget::onSTEPLoaded(vtkSmartPointer<vtkPolyData> polyData, const QString& modelName)
{
    QMutexLocker locker(&m_loadingMutex);
    m_isLoading = false;
    
    qDebug() << "=== MAIN THREAD: Async STEP loading completed, creating VTK Actor ===";
    qDebug() << "PolyData info - Points:" << polyData->GetNumberOfPoints() 
             << "Cells:" << polyData->GetNumberOfCells();
    
    m_statusLabel->setText("正在创建3D可视化...");
    
    // 使用QTimer延迟执行VTK Actor创建，让界面先响应
    QTimer::singleShot(100, this, [this, polyData, modelName]() {
        qDebug() << "Creating VTK Actor in deferred call...";
        
        try {
            // 在主线程中创建VTK Actor
            bool success = CreateVTKActorFromPolyData(polyData, modelName);
            
            if (success) {
                m_statusLabel->setText("STEP模型加载成功!");
                qDebug() << "SUCCESS: Async STEP loading and display completed";
                
                // 确保渲染窗口正确更新
                QTimer::singleShot(200, this, [this]() {
                    try {
                        if (m_renderWindow) {
                            m_renderWindow->Render();
                        }
                        if (m_vtkWidget) {
                            m_vtkWidget->update();
                        }
                        QApplication::processEvents();
                        qDebug() << "Final render update completed";
                    } catch (const std::exception& e) {
                        qWarning() << "渲染更新异常:" << e.what();
                    } catch (...) {
                        qWarning() << "渲染更新未知异常";
                    }
                });
                
            } else {
                m_statusLabel->setText("错误: 无法创建3D模型");
                qWarning() << "ERROR: VTK Actor creation failed";
            }
        } catch (const std::exception& e) {
            m_statusLabel->setText("错误: 3D可视化创建异常");
            qWarning() << "VTK Actor创建异常:" << e.what();
        } catch (...) {
            m_statusLabel->setText("错误: 未知异常");
            qWarning() << "VTK Actor创建未知异常";
        }
    });
}

void VTKWidget::onSTEPLoadFailed(const QString& error)
{
    QMutexLocker locker(&m_loadingMutex);
    m_isLoading = false;
    
    // 隐藏进度条
    m_progressBar->setVisible(false);
    
    qCritical() << "ERROR: Async STEP loading failed:" << error;
    m_statusLabel->setText(QString("错误: %1").arg(error));
    
    QMessageBox::warning(this, "STEP加载失败", 
                        QString("STEP文件加载失败:\n%1").arg(error));
}

void VTKWidget::onSTEPLoadProgress(const QString& message)
{
    m_statusLabel->setText(message);
    qDebug() << "=== PROGRESS UPDATE ===" << message;
    
    // 使用异步方式输出进度到系统日志，避免阻塞事件循环
    if (m_statusPanel) {
        QTimer::singleShot(0, this, [this, message]() {
            if (m_statusPanel) {
                m_statusPanel->addLogMessage("INFO", message);
            }
        });
    }
}

void VTKWidget::onSTEPLoadProgressPercentage(int percentage)
{
    if (percentage >= 0 && percentage <= 100) {
        m_progressBar->setVisible(true);
        m_progressBar->setValue(percentage);
        
        if (percentage == 0) {
            m_progressBar->setFormat("准备中...");
        } else if (percentage == 100) {
            m_progressBar->setFormat("完成");
            // 3秒后隐藏进度条
            QTimer::singleShot(3000, this, [this]() {
                m_progressBar->setVisible(false);
            });
        } else {
            m_progressBar->setFormat(QString("加载中... %1%").arg(percentage));
        }
    }
}

void VTKWidget::SetStatusPanel(StatusPanel* statusPanel)
{
    m_statusPanel = statusPanel;
}

void VTKWidget::onTimeStatistics(const QString& stage, int elapsedMs)
{
    // 输出到系统日志进行性能比较
    double elapsedSec = elapsedMs / 1000.0;
    QString logMessage = QString("[性能统计] %1: %2ms (%3秒)").arg(stage).arg(elapsedMs).arg(elapsedSec, 0, 'f', 1);
    
    qDebug() << logMessage;
    
    // 使用异步方式输出到系统日志面板，避免阻塞事件循环
    if (m_statusPanel) {
        QString systemLogMessage = QString("%1: %2秒").arg(stage).arg(elapsedSec, 0, 'f', 1);
        QTimer::singleShot(0, this, [this, systemLogMessage]() {
            if (m_statusPanel) {
                m_statusPanel->addLogMessage("PERF", systemLogMessage);
            }
        });
    }
    
    // 如果是总计时间，显示完整的性能报告
    if (stage == "总计") {
        QString performanceReport = QString("STEP加载完成 - 总耗时: %1ms (%2秒)").arg(elapsedMs).arg(elapsedSec, 0, 'f', 1);
        qDebug() << "=== 性能报告 ===" << performanceReport;
        
        // 异步输出总结到系统日志
        if (m_statusPanel) {
            QString successMessage = QString("STEP模型加载完成，总耗时: %1秒").arg(elapsedSec, 0, 'f', 1);
            QTimer::singleShot(0, this, [this, successMessage]() {
                if (m_statusPanel) {
                    m_statusPanel->addLogMessage("SUCCESS", successMessage);
                }
            });
        }
        
        // 更新状态标签显示总时间
        m_statusLabel->setText(QString("STEP模型加载完成 (耗时: %1秒)").arg(elapsedSec, 0, 'f', 1));
    }
}

} // namespace UI