#include <QApplication>
#include <QMainWindow>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <QMenuBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

// 包含我们的STEP模型树组件
#include "../src/UI/STEPModelTreeWidget.h"

/**
 * @brief 简单的主程序集成测试
 */
class TestMainWindow : public QMainWindow {
    Q_OBJECT

public:
    TestMainWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("STEP模型树主程序集成测试");
        setMinimumSize(1200, 800);
        
        setupUI();
        setupMenus();
    }

private slots:
    void onImportSTEP() {
        QString fileName = QFileDialog::getOpenFileName(this, "选择STEP文件",
            "data/model", "STEP文件 (*.step *.stp);;所有文件 (*.*)");
        
        if (!fileName.isEmpty()) {
            qDebug() << "加载STEP文件:" << fileName;
            
            // 显示模型树面板
            m_modelTreeDock->show();
            m_modelTreeDock->raise();
            
            // 加载文件
            m_modelTreeWidget->loadSTEPFile(fileName);
        }
    }

private:
    void setupUI() {
        // 创建中央控件
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        QVBoxLayout* layout = new QVBoxLayout(centralWidget);
        layout->addWidget(new QWidget()); // 占位符
        
        // 创建STEP模型树停靠面板
        m_modelTreeDock = new QDockWidget("STEP模型树", this);
        m_modelTreeDock->setObjectName("stepModelTreeDock");
        m_modelTreeDock->setAllowedAreas(Qt::AllDockWidgetAreas);
        
        m_modelTreeWidget = new STEPModelTreeWidget(this);
        m_modelTreeDock->setWidget(m_modelTreeWidget);
        
        addDockWidget(Qt::RightDockWidgetArea, m_modelTreeDock);
    }
    
    void setupMenus() {
        QMenu* fileMenu = menuBar()->addMenu("文件");
        
        QAction* importAction = new QAction("导入STEP模型", this);
        importAction->setShortcut(QKeySequence("Ctrl+O"));
        connect(importAction, &QAction::triggered, this, &TestMainWindow::onImportSTEP);
        fileMenu->addAction(importAction);
        
        fileMenu->addSeparator();
        
        QAction* exitAction = new QAction("退出", this);
        exitAction->setShortcut(QKeySequence::Quit);
        connect(exitAction, &QAction::triggered, this, &QWidget::close);
        fileMenu->addAction(exitAction);
    }

private:
    QDockWidget* m_modelTreeDock;
    STEPModelTreeWidget* m_modelTreeWidget;
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    
    app.setApplicationName("STEP模型树集成测试");
    app.setApplicationVersion("1.0.0");
    
    try {
        TestMainWindow window;
        window.show();
        
        return app.exec();
    }
    catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "错误", 
                            QString("程序运行时发生错误：%1").arg(e.what()));
        return -1;
    }
}

#include "main_integration_test.moc"