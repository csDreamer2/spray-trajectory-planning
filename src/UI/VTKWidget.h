#ifndef VTKWIDGET_H
#define VTKWIDGET_H

#include <QVTKOpenGLNativeWidget.h>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QString>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkPLYReader.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkLine.h>
#include <vtkCellArray.h>
#include <vtkSTLReader.h>
#include <vtkTransform.h>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <array>

// Forward declarations for OpenCASCADE
class TopoDS_Shape;

// Forward declarations for UI
namespace UI {
    class StatusPanel;
}

// Forward declaration for STEP Model Tree Widget
class STEPModelTreeWidget;

namespace UI {

/**
 * @brief VTK 3D可视化组件
 * 
 * 使用VTK进行专业的3D可视化，支持：
 * - STEP/IGES CAD模型加载
 * - 点云数据显示
 * - 机器人模型渲染
 * - 喷涂轨迹可视化
 */
class VTKWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VTKWidget(QWidget *parent = nullptr);
    ~VTKWidget();

    // 模型加载
    bool LoadSTEPModel(const QString& filePath, STEPModelTreeWidget* treeWidget = nullptr);
    bool LoadSTLModel(const QString& filePath);
    bool LoadPointCloud(const QString& filePath);
    bool LoadRobotModel(const QString& urdfPath);
    
    // 轨迹显示
    void ShowSprayTrajectory(const std::vector<std::array<double, 3>>& trajectory);
    void ClearTrajectory();
    
    // 机械臂控制（简化版）
    void SetRobotPose(double x, double y, double z, double rx, double ry, double rz);
    void AnimateRobotToPosition(double x, double y, double z, double rx, double ry, double rz, int durationMs = 2000);
    void StartRobotAnimation();
    
    // 机械臂关节控制（6轴）
    void UpdateRobotJoints(const std::array<double, 6>& jointAngles);
    
    // 视图控制
    void ResetCamera();
    void FitToScene();
    void SetViewMode(const QString& mode); // "front", "top", "iso"
    
    // 显示控制
    void SetWorkpieceVisible(bool visible);
    void SetRobotVisible(bool visible);
    void SetTrajectoryVisible(bool visible);
    void RefreshRender();  // 刷新渲染
    
    // 获取渲染器
    vtkRenderer* getRenderer() const { return m_renderer; }
    
    // 系统日志接口
    void SetStatusPanel(StatusPanel* statusPanel);
    
    // 设置STEP模型树引用（用于关节变换）
    void SetSTEPModelTreeWidget(STEPModelTreeWidget* treeWidget) { m_modelTreeWidget = treeWidget; }
    
    // 启用/禁用机器人按钮
    void enableRobotToggleButton(bool enable) { 
        if (m_toggleRobotBtn) {
            m_toggleRobotBtn->setEnabled(enable);
            m_robotLoaded = enable;
        }
    }

signals:
    void ModelLoaded(const QString& modelType, bool success);
    void SceneClicked(double x, double y, double z);
    void CameraChanged();

private slots:
    void OnResetCamera();
    void OnFitToScene();
    void OnToggleAxes();
    void OnToggleWorkpiece();
    void OnToggleRobot();
    void updateRobotAnimation();

private:
    void setupUI();
    void setupVTKPipeline();
    void setupControls();
    void updateScene();
    
    /**
     * @brief 创建备用测试点云（当文件读取失败时）
     */
    bool CreateFallbackPointCloud();

private:
    // UI组件
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_controlLayout;
    QVTKOpenGLNativeWidget* m_vtkWidget;
    
    // 控制按钮
    QPushButton* m_resetCameraBtn;
    QPushButton* m_fitSceneBtn;
    QPushButton* m_toggleAxesBtn;
    QPushButton* m_toggleWorkpieceBtn;
    QPushButton* m_toggleRobotBtn;
    
    // 状态标签和进度条
    QLabel* m_statusLabel;
    QProgressBar* m_progressBar;
    
    // VTK渲染管线
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkRenderWindow> m_renderWindow;
    vtkSmartPointer<vtkRenderWindowInteractor> m_interactor;
    
    // 3D模型actors
    vtkSmartPointer<vtkActor> m_workshopActor;      // 车间模型
    vtkSmartPointer<vtkActor> m_workpieceActor;     // 点云工件
    vtkSmartPointer<vtkActor> m_robotActor;         // 机器人模型
    vtkSmartPointer<vtkActor> m_trajectoryActor;    // 喷涂轨迹
    
    // 坐标轴
    vtkSmartPointer<vtkAxesActor> m_axesActor;
    vtkSmartPointer<vtkOrientationMarkerWidget> m_axesWidget;
    
    // 状态标志
    bool m_workshopLoaded;
    bool m_workpieceLoaded;
    bool m_robotLoaded;
    bool m_axesVisible;
    
    // 机械臂动画控制
    QTimer* m_robotAnimationTimer;
    vtkSmartPointer<vtkTransform> m_robotTransform;
    double m_robotTargetPose[6];  // x,y,z,rx,ry,rz
    double m_robotCurrentPose[6];
    int m_animationSteps;
    int m_currentAnimationStep;
    
    // 系统日志面板
    StatusPanel* m_statusPanel;
    
    // STEP模型树引用（用于关节变换）
    STEPModelTreeWidget* m_modelTreeWidget;
};

} // namespace UI

#endif // VTKWIDGET_H