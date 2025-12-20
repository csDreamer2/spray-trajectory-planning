#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <QFileInfo>
#include <QMessageBox>
#include <iostream>

// 测试修复后的STEP模型树
#include "../src/UI/STEPModelTreeWidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "=== 最终STEP修复测试程序 ===";
    
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
        treeWidget->setWindowTitle("最终STEP修复测试");
        
        // 连接完成信号
        QObject::connect(treeWidget, &STEPModelTreeWidget::loadCompleted,
            [&](bool success, const QString& message) {
                qDebug() << "=== STEP模型树加载完成 ===";
                qDebug() << "成功:" << success;
                qDebug() << "消息:" << message;
                
                if (success) {
                    qDebug() << "✅ 加载成功！等待3秒后退出...";
                    treeWidget->show();
                    treeWidget->resize(500, 700);
                    
                    QTimer::singleShot(3000, [&]() {
                        qDebug() << "✅ 测试完成，正常退出";
                        app.quit();
                    });
                } else {
                    qDebug() << "❌ 加载失败:" << message;
                    QTimer::singleShot(1000, [&]() {
                        app.quit();
                    });
                }
            });
        
        // 延迟开始加载
        QTimer::singleShot(500, [=]() {
            qDebug() << "开始加载STEP文件...";
            treeWidget->loadSTEPFile(testFile);
        });
        
        // 运行事件循环
        int result = app.exec();
        
        qDebug() << "=== 程序结束 ===";
        
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