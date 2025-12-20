#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QDebug>

#include "UI/Simple3DWidget.h"

/**
 * @brief 简化3D渲染测试程序
 * 
 * 这个简单的测试程序让你可以：
 * 1. 立即测试OpenGL 3D渲染功能
 * 2. 加载你的点云文件
 * 3. 验证3D可视化概念
 * 4. 为后续的VTK集成做准备
 */
class Simple3DTestWindow : public QMainWindow
{
    Q_OBJECT

public:
    Simple3DTestWindow(QWidget *parent = nullptr)
        : QMainWindow(parent)
        , m_3dWidget(nullptr)
    {
        setupUI();
        setWindowTitle("简化3D渲染测试 - 机器人喷涂系统");
        resize(1200, 800);
    }

private slots:
    void loadPointCloud()
    {
        QString fileName = QFileDialog::getOpenFileName(
            this,
            "选择点云文件",
            "test_data",
            "点云文件 (*.ply *.pcd);;所有文件 (*.*)"
        );
        
        if (!fileName.isEmpty()) {
            qDebug() << "加载点云文件:" << fileName;
            bool success = m_3dWidget->LoadPointCloud(fileName);
            
            if (success) {
                QMessageBox::information(this, "成功", "点云加载成功！");
            } else {
                QMessageBox::warning(this, "失败", "点云加载失败，请检查文件格式。");
            }
        }
    }
    
    void showTestData()
    {
        m_3dWidget->ShowTestData();
        QMessageBox::information(this, "成功", "测试数据已显示！");
    }
    
    void clearData()
    {
        m_3dWidget->ClearData();
    }
    
    void onDataLoaded(bool success)
    {
        qDebug() << "数据加载结果:" << (success ? "成功" : "失败");
    }

private:
    void setupUI()
    {
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
        
        // 标题
        QLabel* titleLabel = new QLabel("简化3D渲染引擎测试", this);
        titleLabel->setStyleSheet(
            "QLabel {"
            "   font-size: 18px;"
            "   font-weight: bold;"
            "   color: #2c3e50;"
            "   padding: 10px;"
            "   background-color: #ecf0f1;"
            "   border-radius: 5px;"
            "}"
        );
        titleLabel->setAlignment(Qt::AlignCenter);
        
        // 3D渲染组件
        m_3dWidget = new UI::Simple3DWidget(this);
        connect(m_3dWidget, &UI::Simple3DWidget::DataLoaded, 
                this, &Simple3DTestWindow::onDataLoaded);
        
        // 控制按钮
        QWidget* buttonWidget = new QWidget(this);
        QHBoxLayout* buttonLayout = new QHBoxLayout(buttonWidget);
        
        QPushButton* loadPointCloudBtn = new QPushButton("加载点云", this);
        QPushButton* showTestDataBtn = new QPushButton("显示测试数据", this);
        QPushButton* clearDataBtn = new QPushButton("清除数据", this);
        QPushButton* exitBtn = new QPushButton("退出", this);
        
        connect(loadPointCloudBtn, &QPushButton::clicked, this, &Simple3DTestWindow::loadPointCloud);
        connect(showTestDataBtn, &QPushButton::clicked, this, &Simple3DTestWindow::showTestData);
        connect(clearDataBtn, &QPushButton::clicked, this, &Simple3DTestWindow::clearData);
        connect(exitBtn, &QPushButton::clicked, this, &QWidget::close);
        
        buttonLayout->addWidget(loadPointCloudBtn);
        buttonLayout->addWidget(showTestDataBtn);
        buttonLayout->addWidget(clearDataBtn);
        buttonLayout->addStretch();
        buttonLayout->addWidget(exitBtn);
        
        // 布局
        mainLayout->addWidget(titleLabel);
        mainLayout->addWidget(m_3dWidget, 1);
        mainLayout->addWidget(buttonWidget);
    }

private:
    UI::Simple3DWidget* m_3dWidget;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("简化3D渲染测试");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("机器人喷涂系统");
    
    qDebug() << "启动简化3D渲染测试程序...";
    
    Simple3DTestWindow window;
    window.show();
    
    return app.exec();
}

#include "vtk_test_main.moc"