#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <QFileInfo>
#include <iostream>

// 包含相关头文件
#include "../src/Data/STEPModelTree.h"
#include "../src/UI/STEPModelTreeWidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置调试输出
    qDebug() << "=== STEP崩溃调试程序启动 ===";
    
    // 测试文件路径
    QString testFile = "data/model/MPX3500.STEP";
    QFileInfo fileInfo(testFile);
    
    if (!fileInfo.exists()) {
        qCritical() << "测试文件不存在:" << testFile;
        return -1;
    }
    
    qDebug() << "测试文件:" << testFile;
    qDebug() << "文件大小:" << fileInfo.size() << "bytes";
    
    try {
        // 创建STEP模型树组件
        STEPModelTreeWidget* treeWidget = new STEPModelTreeWidget();
        
        // 连接完成信号
        QObject::connect(treeWidget, &STEPModelTreeWidget::loadCompleted,
            [&](bool success, const QString& message) {
                qDebug() << "=== STEP模型树加载完成 ===";
                qDebug() << "成功:" << success;
                qDebug() << "消息:" << message;
                
                if (success) {
                    qDebug() << "等待2秒后退出...";
                    QTimer::singleShot(2000, [&]() {
                        qDebug() << "正常退出";
                        app.quit();
                    });
                } else {
                    qDebug() << "加载失败，立即退出";
                    app.quit();
                }
            });
        
        // 显示窗口
        treeWidget->show();
        treeWidget->resize(400, 600);
        
        // 开始加载
        qDebug() << "开始加载STEP文件...";
        treeWidget->loadSTEPFile(testFile);
        
        // 运行事件循环
        int result = app.exec();
        
        qDebug() << "=== 程序正常结束 ===";
        
        // 手动清理
        delete treeWidget;
        
        return result;
        
    } catch (const std::exception& e) {
        qCritical() << "程序异常:" << e.what();
        return -1;
    } catch (...) {
        qCritical() << "未知异常";
        return -1;
    }
}