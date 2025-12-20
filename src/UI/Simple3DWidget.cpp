#include "Simple3DWidget.h"
#include <QDebug>
#include <QMessageBox>
#include <QFileInfo>
#include <QTextStream>
#include <QApplication>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <cmath>

namespace UI {

// Simple3DWidget 实现
Simple3DWidget::Simple3DWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_controlLayout(nullptr)
    , m_glWidget(nullptr)
    , m_resetCameraBtn(nullptr)
    , m_fitSceneBtn(nullptr)
    , m_testDataBtn(nullptr)
    , m_clearDataBtn(nullptr)
    , m_statusLabel(nullptr)
    , m_renderer(nullptr)
{
    setupUI();
    setupControls();
}

Simple3DWidget::~Simple3DWidget()
{
    // Qt会自动清理子组件
}

void Simple3DWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // 状态标签
    m_statusLabel = new QLabel("简化3D渲染器 - 就绪", this);
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
    
    // 3D渲染器
    m_renderer = new Simple3DRenderer(this);
    m_renderer->setMinimumSize(800, 600);
    
    // 控制按钮区域
    QWidget* controlWidget = new QWidget(this);
    controlWidget->setMaximumHeight(50);
    m_controlLayout = new QHBoxLayout(controlWidget);
    
    // 布局
    m_mainLayout->addWidget(m_statusLabel);
    m_mainLayout->addWidget(m_renderer, 1);
    m_mainLayout->addWidget(controlWidget);
    
    setLayout(m_mainLayout);
}

void Simple3DWidget::setupControls()
{
    // 重置相机按钮
    m_resetCameraBtn = new QPushButton("重置视角", this);
    m_resetCameraBtn->setMaximumWidth(80);
    connect(m_resetCameraBtn, &QPushButton::clicked, this, &Simple3DWidget::OnResetCamera);
    
    // 适应场景按钮
    m_fitSceneBtn = new QPushButton("适应场景", this);
    m_fitSceneBtn->setMaximumWidth(80);
    connect(m_fitSceneBtn, &QPushButton::clicked, this, &Simple3DWidget::OnFitToScene);
    
    // 显示测试数据按钮
    m_testDataBtn = new QPushButton("测试数据", this);
    m_testDataBtn->setMaximumWidth(80);
    connect(m_testDataBtn, &QPushButton::clicked, this, &Simple3DWidget::OnShowTestData);
    
    // 清除数据按钮
    m_clearDataBtn = new QPushButton("清除", this);
    m_clearDataBtn->setMaximumWidth(60);
    connect(m_clearDataBtn, &QPushButton::clicked, this, &Simple3DWidget::OnClearData);
    
    // 添加到布局
    m_controlLayout->addStretch();
    m_controlLayout->addWidget(m_resetCameraBtn);
    m_controlLayout->addWidget(m_fitSceneBtn);
    m_controlLayout->addWidget(m_testDataBtn);
    m_controlLayout->addWidget(m_clearDataBtn);
    m_controlLayout->addStretch();
}

bool Simple3DWidget::LoadPointCloud(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qWarning() << "点云文件不存在:" << filePath;
        m_statusLabel->setText("错误: 点云文件不存在");
        return false;
    }
    
    qDebug() << "开始加载点云:" << filePath;
    m_statusLabel->setText("正在加载点云数据...");
    QApplication::processEvents();
    
    try {
        // 简化的PLY文件解析
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw std::runtime_error("无法打开文件");
        }
        
        QTextStream in(&file);
        std::vector<QVector3D> points;
        
        // 跳过PLY头部
        QString line;
        int vertexCount = 0;
        bool inHeader = true;
        
        while (!in.atEnd() && inHeader) {
            line = in.readLine();
            if (line.startsWith("element vertex")) {
                QStringList parts = line.split(' ');
                if (parts.size() >= 3) {
                    vertexCount = parts[2].toInt();
                }
            } else if (line == "end_header") {
                inHeader = false;
            }
        }
        
        // 读取顶点数据
        for (int i = 0; i < vertexCount && !in.atEnd(); ++i) {
            line = in.readLine();
            QStringList coords = line.split(' ');
            if (coords.size() >= 3) {
                float x = coords[0].toFloat();
                float y = coords[1].toFloat();
                float z = coords[2].toFloat();
                points.push_back(QVector3D(x, y, z));
            }
        }
        
        if (points.empty()) {
            throw std::runtime_error("未找到有效的点云数据");
        }
        
        // 设置点云数据到渲染器
        m_renderer->SetPointCloudData(points);
        
        qDebug() << "✅ 点云加载成功，点数:" << points.size();
        m_statusLabel->setText(QString("点云已加载 (%1 个点)").arg(points.size()));
        
        // 自动适应场景
        FitToScene();
        
        emit DataLoaded(true);
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "加载点云失败:" << e.what();
        m_statusLabel->setText("错误: 点云加载失败");
        emit DataLoaded(false);
        return false;
    }
}

void Simple3DWidget::ShowTestData()
{
    m_renderer->SetTestData();
    m_statusLabel->setText("测试数据已显示");
    FitToScene();
}

void Simple3DWidget::ClearData()
{
    m_renderer->ClearData();
    m_statusLabel->setText("数据已清除");
}

void Simple3DWidget::ResetCamera()
{
    m_renderer->ResetCamera();
    emit CameraChanged();
}

void Simple3DWidget::FitToScene()
{
    m_renderer->FitToScene();
    emit CameraChanged();
}

// 槽函数实现
void Simple3DWidget::OnResetCamera()
{
    ResetCamera();
}

void Simple3DWidget::OnFitToScene()
{
    FitToScene();
}

void Simple3DWidget::OnShowTestData()
{
    ShowTestData();
}

void Simple3DWidget::OnClearData()
{
    ClearData();
}

// Simple3DRenderer 实现
Simple3DWidget::Simple3DRenderer::Simple3DRenderer(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_hasPointCloud(false)
    , m_showTestData(false)
    , m_cameraPos(0, 0, 5)
    , m_cameraTarget(0, 0, 0)
    , m_cameraUp(0, 1, 0)
    , m_mousePressed(false)
    , m_cameraDistance(5.0f)
    , m_cameraYaw(0.0f)
    , m_cameraPitch(0.0f)
{
    setFocusPolicy(Qt::StrongFocus);
}

Simple3DWidget::Simple3DRenderer::~Simple3DRenderer()
{
    // OpenGL资源会自动清理
}

void Simple3DWidget::Simple3DRenderer::SetPointCloudData(const std::vector<QVector3D>& points)
{
    m_pointCloudData = points;
    m_hasPointCloud = true;
    m_showTestData = false;
    update();
}

void Simple3DWidget::Simple3DRenderer::SetTestData()
{
    m_showTestData = true;
    m_hasPointCloud = false;
    update();
}

void Simple3DWidget::Simple3DRenderer::ClearData()
{
    m_pointCloudData.clear();
    m_hasPointCloud = false;
    m_showTestData = false;
    update();
}

void Simple3DWidget::Simple3DRenderer::ResetCamera()
{
    m_cameraDistance = 5.0f;
    m_cameraYaw = 0.0f;
    m_cameraPitch = 0.0f;
    updateCamera();
    update();
}

void Simple3DWidget::Simple3DRenderer::FitToScene()
{
    if (m_hasPointCloud && !m_pointCloudData.empty()) {
        // 计算点云边界
        QVector3D minBound = m_pointCloudData[0];
        QVector3D maxBound = m_pointCloudData[0];
        
        for (const auto& point : m_pointCloudData) {
            minBound.setX(std::min(minBound.x(), point.x()));
            minBound.setY(std::min(minBound.y(), point.y()));
            minBound.setZ(std::min(minBound.z(), point.z()));
            maxBound.setX(std::max(maxBound.x(), point.x()));
            maxBound.setY(std::max(maxBound.y(), point.y()));
            maxBound.setZ(std::max(maxBound.z(), point.z()));
        }
        
        QVector3D center = (minBound + maxBound) * 0.5f;
        float size = (maxBound - minBound).length();
        
        m_cameraTarget = center;
        m_cameraDistance = size * 1.5f;
    } else {
        m_cameraDistance = 5.0f;
    }
    
    updateCamera();
    update();
}

void Simple3DWidget::Simple3DRenderer::initializeGL()
{
    initializeOpenGLFunctions();
    
    glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    
    qDebug() << "✅ OpenGL初始化完成";
}

void Simple3DWidget::Simple3DRenderer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // 设置变换矩阵
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m_projection.constData());
    
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(m_view.constData());
    
    // 绘制坐标轴
    drawAxes();
    
    // 绘制数据
    if (m_hasPointCloud) {
        drawPointCloud();
    } else if (m_showTestData) {
        drawTestCube();
    }
}

void Simple3DWidget::Simple3DRenderer::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
    
    float aspect = float(width) / float(height ? height : 1);
    m_projection.setToIdentity();
    m_projection.perspective(45.0f, aspect, 0.1f, 1000.0f);
    
    updateCamera();
}

void Simple3DWidget::Simple3DRenderer::mousePressEvent(QMouseEvent *event)
{
    m_lastMousePos = event->pos();
    m_mousePressed = true;
}

void Simple3DWidget::Simple3DRenderer::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_mousePressed) return;
    
    QPoint delta = event->pos() - m_lastMousePos;
    m_lastMousePos = event->pos();
    
    // 旋转相机
    m_cameraYaw += delta.x() * 0.5f;
    m_cameraPitch += delta.y() * 0.5f;
    
    // 限制俯仰角
    m_cameraPitch = std::max(-89.0f, std::min(89.0f, m_cameraPitch));
    
    updateCamera();
    update();
}

void Simple3DWidget::Simple3DRenderer::wheelEvent(QWheelEvent *event)
{
    float delta = event->angleDelta().y() / 120.0f;
    m_cameraDistance *= (1.0f - delta * 0.1f);
    m_cameraDistance = std::max(0.1f, std::min(100.0f, m_cameraDistance));
    
    updateCamera();
    update();
}

void Simple3DWidget::Simple3DRenderer::updateCamera()
{
    // 球坐标转换为笛卡尔坐标
    float yawRad = m_cameraYaw * M_PI / 180.0f;
    float pitchRad = m_cameraPitch * M_PI / 180.0f;
    
    m_cameraPos.setX(m_cameraTarget.x() + m_cameraDistance * cos(pitchRad) * cos(yawRad));
    m_cameraPos.setY(m_cameraTarget.y() + m_cameraDistance * sin(pitchRad));
    m_cameraPos.setZ(m_cameraTarget.z() + m_cameraDistance * cos(pitchRad) * sin(yawRad));
    
    m_view.setToIdentity();
    m_view.lookAt(m_cameraPos, m_cameraTarget, m_cameraUp);
}

void Simple3DWidget::Simple3DRenderer::drawAxes()
{
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    
    // X轴 - 红色
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(1.0f, 0.0f, 0.0f);
    
    // Y轴 - 绿色
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);
    
    // Z轴 - 蓝色
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 1.0f);
    
    glEnd();
}

void Simple3DWidget::Simple3DRenderer::drawPointCloud()
{
    glPointSize(2.0f);
    glColor3f(0.8f, 0.2f, 0.2f); // 红色点云
    
    glBegin(GL_POINTS);
    for (const auto& point : m_pointCloudData) {
        glVertex3f(point.x(), point.y(), point.z());
    }
    glEnd();
}

void Simple3DWidget::Simple3DRenderer::drawTestCube()
{
    glColor3f(0.2f, 0.8f, 0.2f); // 绿色立方体
    
    // 绘制立方体的线框
    glBegin(GL_LINES);
    
    // 底面
    glVertex3f(-1, -1, -1); glVertex3f( 1, -1, -1);
    glVertex3f( 1, -1, -1); glVertex3f( 1,  1, -1);
    glVertex3f( 1,  1, -1); glVertex3f(-1,  1, -1);
    glVertex3f(-1,  1, -1); glVertex3f(-1, -1, -1);
    
    // 顶面
    glVertex3f(-1, -1,  1); glVertex3f( 1, -1,  1);
    glVertex3f( 1, -1,  1); glVertex3f( 1,  1,  1);
    glVertex3f( 1,  1,  1); glVertex3f(-1,  1,  1);
    glVertex3f(-1,  1,  1); glVertex3f(-1, -1,  1);
    
    // 垂直边
    glVertex3f(-1, -1, -1); glVertex3f(-1, -1,  1);
    glVertex3f( 1, -1, -1); glVertex3f( 1, -1,  1);
    glVertex3f( 1,  1, -1); glVertex3f( 1,  1,  1);
    glVertex3f(-1,  1, -1); glVertex3f(-1,  1,  1);
    
    glEnd();
}

} // namespace UI