#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QSplitter>
#include <QTextEdit>
#include <QLabel>
#include <QDebug>

#include "../src/Data/STEPModelTree.h"
#include "../src/UI/STEPModelTreeWidget.h"
#include "../src/UI/ModelTreeDockWidget.h"

/**
 * @brief STEP模型树测试主窗口
 */
class STEPModelTreeTestWindow : public QMainWindow {
    Q_OBJECT

public:
    STEPModelTreeTestWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        setupUI();
        connectSignals();
    }

private slots:
    void openSTEPFile() {
        QString filePath = QFileDialog::getOpenFileName(
            this,
            tr("选择STEP文件"),
            "data/model",
            tr("STEP Files (*.step *.stp);;All Files (*)")
        );

        if (!filePath.isEmpty()) {
            m_logText->append(QString("正在加载文件: %1").arg(filePath));
            m_modelTreeDock->loadSTEPFile(filePath);
        }
    }

    void onModelVisibilityChanged(const std::vector<TopoDS_Shape>& visibleShapes) {
        m_logText->append(QString("可见形状数量: %1").arg(visibleShapes.size()));
        
        // 这里应该更新3D渲染器，现在只是记录日志
        for (size_t i = 0; i < visibleShapes.size() && i < 5; ++i) {
            const auto& shape = visibleShapes[i];
            if (!shape.IsNull()) {
                m_logText->append(QString("  形状 %1: 类型 %2")
                    .arg(i + 1)
                    .arg(shape.ShapeType()));
            }
        }
        
        if (visibleShapes.size() > 5) {
            m_logText->append(QString("  ... 还有 %1 个形状").arg(visibleShapes.size() - 5));
        }
    }

    void onModelSelectionChanged(const std::vector<TopoDS_Shape>& selectedShapes) {
        m_logText->append(QString("选中形状数量: %1").arg(selectedShapes.size()));
    }

    void testBasicFunctions() {
        m_logText->append("=== 开始基本功能测试 ===");
        
        auto modelTree = m_modelTreeDock->getModelTreeWidget()->getModelTree();
        if (!modelTree) {
            m_logText->append("错误: 模型树未初始化");
            return;
        }

        // 测试统计信息
        auto stats = modelTree->getModelStats();
        m_logText->append(QString("模型统计:"));
        m_logText->append(QString("  总节点: %1").arg(stats.totalNodes));
        m_logText->append(QString("  装配体: %1").arg(stats.assemblies));
        m_logText->append(QString("  零件: %1").arg(stats.parts));
        m_logText->append(QString("  最大深度: %1").arg(stats.maxDepth));
        m_logText->append(QString("  可见节点: %1").arg(stats.visibleNodes));

        // 测试查找功能
        auto nodes = modelTree->findNodesByName("MPX3500");
        m_logText->append(QString("查找 'MPX3500': 找到 %1 个节点").arg(nodes.size()));

        // 测试可见形状获取
        auto visibleShapes = modelTree->getVisibleShapes();
        m_logText->append(QString("当前可见形状: %1 个").arg(visibleShapes.size()));

        m_logText->append("=== 基本功能测试完成 ===");
    }

    void clearLog() {
        m_logText->clear();
    }

private:
    void setupUI() {
        setWindowTitle("STEP模型树功能测试");
        setMinimumSize(1000, 700);

        // 创建中央窗口部件
        auto centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        // 创建主布局
        auto mainLayout = new QVBoxLayout(centralWidget);

        // 创建工具栏
        auto toolLayout = new QHBoxLayout();
        
        auto openButton = new QPushButton("打开STEP文件", this);
        auto testButton = new QPushButton("测试功能", this);
        auto clearButton = new QPushButton("清除日志", this);
        
        toolLayout->addWidget(openButton);
        toolLayout->addWidget(testButton);
        toolLayout->addWidget(clearButton);
        toolLayout->addStretch();

        // 创建分割器
        auto splitter = new QSplitter(Qt::Horizontal, this);

        // 创建模型树停靠窗口
        m_modelTreeDock = new ModelTreeDockWidget(this);
        m_modelTreeDock->setFloating(false);
        
        // 将停靠窗口的内容添加到分割器
        auto treeWidget = m_modelTreeDock->widget();
        treeWidget->setParent(splitter);
        splitter->addWidget(treeWidget);

        // 创建日志区域
        m_logText = new QTextEdit(this);
        m_logText->setReadOnly(true);
        m_logText->setMaximumHeight(200);
        splitter->addWidget(m_logText);

        // 设置分割器比例
        splitter->setStretchFactor(0, 1);
        splitter->setStretchFactor(1, 1);

        // 添加到主布局
        mainLayout->addLayout(toolLayout);
        mainLayout->addWidget(splitter);

        // 连接按钮信号
        connect(openButton, &QPushButton::clicked, this, &STEPModelTreeTestWindow::openSTEPFile);
        connect(testButton, &QPushButton::clicked, this, &STEPModelTreeTestWindow::testBasicFunctions);
        connect(clearButton, &QPushButton::clicked, this, &STEPModelTreeTestWindow::clearLog);

        // 初始日志
        m_logText->append("STEP模型树测试程序已启动");
        m_logText->append("请点击'打开STEP文件'加载测试文件");
    }

    void connectSignals() {
        connect(m_modelTreeDock, &ModelTreeDockWidget::modelVisibilityChanged,
                this, &STEPModelTreeTestWindow::onModelVisibilityChanged);
        connect(m_modelTreeDock, &ModelTreeDockWidget::modelSelectionChanged,
                this, &STEPModelTreeTestWindow::onModelSelectionChanged);
    }

private:
    ModelTreeDockWidget* m_modelTreeDock;
    QTextEdit* m_logText;
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("STEP Model Tree Test");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Spray Trajectory Planning");

    try {
        STEPModelTreeTestWindow window;
        window.show();

        return app.exec();
    }
    catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "错误", 
            QString("程序启动失败: %1").arg(e.what()));
        return -1;
    }
}

#include "test_step_model_tree.moc"