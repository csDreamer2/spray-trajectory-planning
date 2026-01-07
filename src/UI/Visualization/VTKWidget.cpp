#include "VTKWidget.h"
#include "../Panels/StatusPanel.h"
#include "../ModelTree/STEPModelTreeWidget.h"
#include "../../Data/STEP/STEPModelTree.h"  // 添加STEP模型树头文件
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
#include <array>
#include <cmath>

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
#include <TopoDS_Iterator.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <Poly_ArrayOfNodes.hxx>
#include <Poly_Array1OfTriangle.hxx>
#include <TopLoc_Location.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_XYZ.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <Standard_Failure.hxx>  // 添加异常处理头文件
#include <vtkMatrix4x4.h>

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
    , m_statusPanel(nullptr)
    , m_modelTreeWidget(nullptr)
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

bool VTKWidget::LoadSTEPModel(const QString& filePath, STEPModelTreeWidget* treeWidget)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qWarning() << "STEP文件不存在:" << filePath;
        m_statusLabel->setText("错误: STEP文件不存在");
        return false;
    }
    
    qDebug() << "VTKWidget: LoadSTEPModel被调用，但STEP加载现在由MainWindow处理";
    qDebug() << "VTKWidget: 文件路径:" << filePath;
    
    // STEP加载现在完全由MainWindow和STEPModelTreeWidget处理
    // 这个函数保留是为了向后兼容，但实际工作在MainWindow::OnImportSTEPModel()中完成
    
    m_statusLabel->setText("STEP模型加载由主窗口处理");
    return true;
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

void VTKWidget::UpdateRobotJoints(const std::array<double, 6>& jointAngles)
{
    // TODO: 实现完整的6轴机器人关节变换
    // 当前使用简化的末端位姿变换作为演示
    // 实际应用中需要根据机器人DH参数计算各关节的变换矩阵
    
    // 简化演示：将关节角度映射到末端位姿变换
    // J1: 基座旋转 -> 绕Z轴旋转
    // J2: 肩部 -> 影响Z高度
    // J3: 肘部 -> 影响前伸距离
    // J4-J6: 手腕 -> 末端姿态
    
    double baseRotation = jointAngles[0];  // J1 基座旋转
    double shoulderAngle = jointAngles[1]; // J2 肩部
    double elbowAngle = jointAngles[2];    // J3 肘部
    double wristRoll = jointAngles[3];     // J4 手腕旋转
    double wristPitch = jointAngles[4];    // J5 手腕俯仰
    double wristYaw = jointAngles[5];      // J6 末端旋转
    
    // 简化的运动学计算（仅用于演示）
    // 实际应用需要使用完整的DH参数计算
    double armLength1 = 680.0;  // 大臂长度 mm
    double armLength2 = 680.0;  // 小臂长度 mm
    
    // 计算末端位置（简化）
    double rad = M_PI / 180.0;
    double reach = armLength1 * cos(shoulderAngle * rad) + armLength2 * cos((shoulderAngle + elbowAngle) * rad);
    double height = 330.0 + armLength1 * sin(shoulderAngle * rad) + armLength2 * sin((shoulderAngle + elbowAngle) * rad);
    
    double x = reach * cos(baseRotation * rad);
    double y = reach * sin(baseRotation * rad);
    double z = height;
    
    // 如果有专用机器人Actor，应用变换
    if (m_robotActor) {
        m_robotTransform->Identity();
        m_robotTransform->Translate(x, y, z);
        m_robotTransform->RotateZ(baseRotation);
        m_robotTransform->RotateY(wristPitch);
        m_robotTransform->RotateX(wristRoll);
        m_robotTransform->RotateZ(wristYaw);
        
        m_robotActor->SetUserTransform(m_robotTransform);
        m_renderWindow->Render();
    }
    // 如果有STEP模型树，应用变换到各个关节
    else if (m_modelTreeWidget) {
        // 对各个关节应用单独的变换
        // NAUO1: 基座 - 绕Z轴旋转
        vtkSmartPointer<vtkTransform> baseTransform = vtkSmartPointer<vtkTransform>::New();
        baseTransform->Identity();
        baseTransform->RotateZ(baseRotation);
        m_modelTreeWidget->applyTransformToActor("NAUO1", baseTransform);
        
        // NAUO2-NAUO7: 其他关节 - 暂时不变换（需要完整的DH参数）
        // 这里可以后续添加更复杂的关节变换
        
        qDebug() << "VTKWidget: 应用关节变换到STEP模型 -"
                 << "J1(基座旋转):" << baseRotation << "°"
                 << "J2(肩部):" << shoulderAngle << "°"
                 << "J3(肘部):" << elbowAngle << "°";
        
        m_renderWindow->Render();
    }
    else {
        // 仿真模式下，关节角度变化仍然有效，只是没有3D可视化
        qDebug() << "VTKWidget: 关节角度更新 (无3D模型) -"
                 << "J1:" << jointAngles[0]
                 << "J2:" << jointAngles[1]
                 << "J3:" << jointAngles[2]
                 << "J4:" << jointAngles[3]
                 << "J5:" << jointAngles[4]
                 << "J6:" << jointAngles[5];
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
    // 如果有专用机器人Actor
    if (m_robotActor) {
        m_robotActor->SetVisibility(visible);
    }
    
    // 如果有STEP模型树中的机器人模型
    if (m_modelTreeWidget) {
        // 设置所有机器人部件的可见性
        for (int i = 1; i <= 8; ++i) {
            QString partName = QString("NAUO%1").arg(i);
            m_modelTreeWidget->setPartVisibility(partName, visible);
        }
    }
    
    m_renderWindow->Render();
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

void VTKWidget::SetStatusPanel(StatusPanel* statusPanel)
{
    m_statusPanel = statusPanel;
}

void VTKWidget::RefreshRender()
{
    // 直接同步渲染，参考123/StepViewerWidget.cpp
    if (m_renderWindow) {
        m_renderWindow->Render();
    }
    
    if (m_vtkWidget) {
        m_vtkWidget->update();
    }
}

} // namespace UI