#include <QApplication>
#include <QMainWindow>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QDebug>
#include <QTimer>
#include <QFileInfo>
#include <QMessageBox>
#include <QProgressDialog>
#include <QThread>
#include <QMutex>
#include <iostream>

// 关键修复：使用STEPModelTreeWidget而不是直接使用STEPModelTree
#include "../src/UI/STEPModelTreeWidget.h"

class SafeTreeFixedWindow : public QMainWindow
{
    Q_OBJECT

public:
    SafeTreeFixedWindow(QWidget* parent = nullptr) : QMainWindow(parent)
    {
        setupUI();
        setupSTEPModelTreeWidget();
        
        setWindowTitle("修复版安全STEP树状界面测试");
        resize(800, 600);
    }
    
    ~SafeTreeFixedWindow() {
        qDebug() << "SafeTreeFixedWindow: 开始析构";
        
        if (m_stepWidget) {
            qDebug() << "SafeTreeFixedWindow: 删除STEPModelTreeWidget";
            delete m_stepWidget;
            m_stepWidget = nullptr;
        }
        
        qDebug() << "SafeTreeFixedWindow: 析构完成";
    }
    
    void loadTestFile() {
        QString testFile = "data/model/MPX3500.STEP";
        QFileInfo fileInfo(testFile);
        
        if (!fileInfo.exists()) {
            QMessageBox::critical(this, "错误", QString("测试文件不存在:\n%1").arg(testFile));
            return;
        }
        
        qDebug() << "测试文件:" << testFile;
        qDebug() << "文件大小:" << fileInfo.size() << "bytes";
        
        // 对于大文件给出警告
        if (fileInfo.size() > 100 * 1024 * 1024) { // 100MB
            QMessageBox::StandardButton reply = QMessageBox::question(this, "大文件警告", 
                QString("文件大小为 %1 MB，解析可能需要较长时间且有崩溃风险。\n是否继续？")
                .arg(fileInfo.size() / (1024.0 * 1024.0), 0, 'f', 1),
                QMessageBox::Yes | QMessageBox::No);
            
            if (reply != QMessageBox::Yes) {
                return;
            }
        }
        
        // 显示加载状态
        m_statusLabel->setText("正在加载STEP文件...");
        m_progressBar->setVisible(true);
        m_progressBar->setValue(0);
        m_loadButton->setEnabled(false);
        m_treeWidget->clear();
        
        qDebug() << "开始异步加载STEP文件（使用工作线程）...";
        
        // 关键修复：使用STEPModelTreeWidget的异步加载
        m_stepWidget->loadSTEPFile(testFile);
    }

private slots:
    void onLoadCompleted(bool success, const QString& message) {
        qDebug() << "=== 修复版STEP加载完成 ===";
        qDebug() << "成功:" << success << "消息:" << message;
        
        m_progressBar->setVisible(false);
        m_loadButton->setEnabled(true);
        
        if (success) {
            m_statusLabel->setText("STEP文件加载成功");
            buildTreeFromSTEPWidget();
            
            QMessageBox::information(this, "成功", 
                QString("STEP文件加载成功！\n%1\n\n现在可以查看树状结构了。").arg(message));
        } else {
            m_statusLabel->setText("STEP文件加载失败");
            QMessageBox::critical(this, "失败", 
                QString("STEP文件加载失败：\n%1").arg(message));
        }
    }

private:
    void setupUI() {
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
        
        // 控制按钮
        QHBoxLayout* buttonLayout = new QHBoxLayout();
        m_loadButton = new QPushButton("加载STEP文件", this);
        connect(m_loadButton, &QPushButton::clicked, this, &SafeTreeFixedWindow::loadTestFile);
        buttonLayout->addWidget(m_loadButton);
        buttonLayout->addStretch();
        
        mainLayout->addLayout(buttonLayout);
        
        // 状态标签
        m_statusLabel = new QLabel("准备加载STEP模型...", this);
        m_statusLabel->setStyleSheet("QLabel { color: #666; font-size: 12px; padding: 4px; }");
        mainLayout->addWidget(m_statusLabel);
        
        // 进度条
        m_progressBar = new QProgressBar(this);
        m_progressBar->setVisible(false);
        m_progressBar->setTextVisible(true);
        mainLayout->addWidget(m_progressBar);
        
        // 树状列表
        m_treeWidget = new QTreeWidget(this);
        m_treeWidget->setHeaderLabels({"组件名称", "类型", "可见性", "标签"});
        m_treeWidget->setAlternatingRowColors(true);
        m_treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
        
        mainLayout->addWidget(m_treeWidget);
        
        // 信息标签
        QLabel* infoLabel = new QLabel(
            "🔧 修复版安全STEP树状界面测试程序\n"
            "✅ 使用STEPModelTreeWidget进行异步处理（与safe_step_test.exe相同）\n"
            "✅ 避免了直接调用STEPModelTree导致的崩溃问题\n"
            "✅ 完整的工作线程保护和信号处理", this);
        infoLabel->setStyleSheet("QLabel { color: #0a5c2b; font-size: 10px; padding: 8px; background-color: #e8f5e8; border-radius: 4px; }");
        infoLabel->setWordWrap(true);
        mainLayout->addWidget(infoLabel);
    }
    
    void setupSTEPModelTreeWidget() {
        // 关键修复：使用STEPModelTreeWidget而不是直接使用STEPModelTree
        // 这样可以获得完整的异步处理和线程保护
        m_stepWidget = new STEPModelTreeWidget(this);
        
        // 连接完成信号
        connect(m_stepWidget, &STEPModelTreeWidget::loadCompleted,
                this, &SafeTreeFixedWindow::onLoadCompleted);
        
        // 隐藏STEPModelTreeWidget，我们只使用它的逻辑，不显示其UI
        m_stepWidget->setVisible(false);
        
        qDebug() << "SafeTreeFixedWindow: 使用STEPModelTreeWidget进行异步处理";
    }
    
    void buildTreeFromSTEPWidget() {
        if (!m_stepWidget) {
            qWarning() << "SafeTreeFixedWindow: STEPModelTreeWidget为空，无法构建树";
            return;
        }
        
        // 从STEPModelTreeWidget获取Qt模型
        auto qtModel = m_stepWidget->getQtModel();
        if (!qtModel) {
            qWarning() << "SafeTreeFixedWindow: Qt模型为空，无法构建树";
            return;
        }
        
        qDebug() << "SafeTreeFixedWindow: 开始从Qt模型构建树状显示";
        
        m_treeWidget->clear();
        
        try {
            // 从Qt模型复制数据到我们的QTreeWidget
            QStandardItem* rootItem = qtModel->invisibleRootItem();
            
            for (int i = 0; i < rootItem->rowCount(); ++i) {
                QStandardItem* sourceItem = rootItem->child(i, 0);
                if (sourceItem) {
                    QTreeWidgetItem* targetItem = createTreeItemFromQtModel(sourceItem, qtModel);
                    if (targetItem) {
                        m_treeWidget->addTopLevelItem(targetItem);
                    }
                }
            }
            
            // 展开第一层
            m_treeWidget->expandToDepth(1);
            
            qDebug() << "SafeTreeFixedWindow: 树状显示构建完成，共" << m_treeWidget->topLevelItemCount() << "个顶级项";
            
        } catch (const std::exception& e) {
            qWarning() << "SafeTreeFixedWindow: 构建树状显示异常:" << e.what();
        } catch (...) {
            qWarning() << "SafeTreeFixedWindow: 构建树状显示未知异常";
        }
    }
    
    QTreeWidgetItem* createTreeItemFromQtModel(QStandardItem* sourceItem, QStandardItemModel* model) {
        if (!sourceItem || !model) return nullptr;
        
        try {
            QTreeWidgetItem* item = new QTreeWidgetItem();
            
            // 复制第一列数据（组件名称）
            item->setText(0, sourceItem->text());
            
            // 获取同行的其他列数据
            int row = sourceItem->row();
            QStandardItem* parentItem = sourceItem->parent();
            
            // 类型列
            QStandardItem* typeItem = parentItem ? parentItem->child(row, 1) : model->item(row, 1);
            if (typeItem) {
                item->setText(1, typeItem->text());
            }
            
            // 可见性列
            QStandardItem* visItem = parentItem ? parentItem->child(row, 2) : model->item(row, 2);
            if (visItem) {
                item->setText(2, visItem->text());
            }
            
            // 标签列
            QStandardItem* labelItem = parentItem ? parentItem->child(row, 3) : model->item(row, 3);
            if (labelItem) {
                item->setText(3, labelItem->text());
            }
            
            // 设置图标
            QString typeText = item->text(1);
            if (typeText.contains("装配体") || sourceItem->hasChildren()) {
                item->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
            } else {
                item->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
            }
            
            // 递归添加子项
            for (int i = 0; i < sourceItem->rowCount(); ++i) {
                QStandardItem* childSource = sourceItem->child(i, 0);
                if (childSource) {
                    QTreeWidgetItem* childItem = createTreeItemFromQtModel(childSource, model);
                    if (childItem) {
                        item->addChild(childItem);
                    }
                }
            }
            
            return item;
            
        } catch (const std::exception& e) {
            qWarning() << "SafeTreeFixedWindow: 创建树项异常:" << e.what();
            return nullptr;
        } catch (...) {
            qWarning() << "SafeTreeFixedWindow: 创建树项未知异常";
            return nullptr;
        }
    }

private:
    QTreeWidget* m_treeWidget;
    QPushButton* m_loadButton;
    QLabel* m_statusLabel;
    QProgressBar* m_progressBar;
    STEPModelTreeWidget* m_stepWidget;  // 使用STEPModelTreeWidget而不是STEPModelTree
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "=== 修复版安全STEP树状界面测试程序 ===";
    qDebug() << "使用STEPModelTreeWidget进行异步处理，避免崩溃问题";
    
    try {
        SafeTreeFixedWindow window;
        window.show();
        
        // 设置超时保护（10分钟）
        QTimer::singleShot(600000, [&]() {
            qWarning() << "程序运行超时（10分钟），自动退出";
            app.quit();
        });
        
        int result = app.exec();
        
        qDebug() << "=== 程序正常结束 ===";
        return result;
        
    } catch (const std::exception& e) {
        qCritical() << "程序异常:" << e.what();
        return -1;
    } catch (...) {
        qCritical() << "未知异常";
        return -1;
    }
}

#include "safe_tree_gui_fixed.moc"