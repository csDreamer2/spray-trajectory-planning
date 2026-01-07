#ifndef SIMPLE3DWIDGET_H
#define SIMPLE3DWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QVector3D>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <vector>

namespace UI {

/**
 * @brief 简化的3D渲染组件
 * 
 * 使用原生OpenGL实现基本的3D可视化功能，
 * 验证3D渲染概念，为后续VTK集成做准备
 */
class Simple3DWidget : public QWidget
{
    Q_OBJECT

public:
    explicit Simple3DWidget(QWidget *parent = nullptr);
    ~Simple3DWidget();

    // 数据加载
    bool LoadPointCloud(const QString& filePath);
    void ShowTestData();
    void ClearData();
    
    // 视图控制
    void ResetCamera();
    void FitToScene();

signals:
    void DataLoaded(bool success);
    void CameraChanged();

private slots:
    void OnResetCamera();
    void OnFitToScene();
    void OnShowTestData();
    void OnClearData();

private:
    void setupUI();
    void setupControls();

private:
    // UI组件
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_controlLayout;
    QOpenGLWidget* m_glWidget;
    
    // 控制按钮
    QPushButton* m_resetCameraBtn;
    QPushButton* m_fitSceneBtn;
    QPushButton* m_testDataBtn;
    QPushButton* m_clearDataBtn;
    
    // 状态标签
    QLabel* m_statusLabel;
    
    // 3D渲染器
    class Simple3DRenderer;
    Simple3DRenderer* m_renderer;
};

/**
 * @brief 简化的3D渲染器
 * 
 * 使用OpenGL直接渲染点云和基本几何体
 */
class Simple3DWidget::Simple3DRenderer : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit Simple3DRenderer(QWidget *parent = nullptr);
    ~Simple3DRenderer();

    // 数据管理
    void SetPointCloudData(const std::vector<QVector3D>& points);
    void SetTestData();
    void ClearData();
    
    // 相机控制
    void ResetCamera();
    void FitToScene();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void updateCamera();
    void drawAxes();
    void drawPointCloud();
    void drawTestCube();

private:
    // 点云数据
    std::vector<QVector3D> m_pointCloudData;
    bool m_hasPointCloud;
    bool m_showTestData;
    
    // 相机参数
    QMatrix4x4 m_projection;
    QMatrix4x4 m_view;
    QVector3D m_cameraPos;
    QVector3D m_cameraTarget;
    QVector3D m_cameraUp;
    
    // 交互参数
    QPoint m_lastMousePos;
    bool m_mousePressed;
    float m_cameraDistance;
    float m_cameraYaw;
    float m_cameraPitch;
};

} // namespace UI

#endif // SIMPLE3DWIDGET_H