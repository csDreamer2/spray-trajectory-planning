#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <QFileInfo>
#include <QMessageBox>
#include <iostream>

// 只测试STEP模型树，不涉及VTK
#include "../src/UI/STEPModelTreeWidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "=== STEP模型树单独测试程序 ===";
    
    // 测试文件路径
    QString testFile = "data/model/MPX3500.STEP";
    QFileInfo fileInfo(testFile);
    
    if (!fileInfo.exists()) {
        qCritical() << "测试文件不存在:" << testFile;
        QMessageBox::critical(nullptr, "错误", QString("测试文件不存在:\n%1").arg(testFile));
        return -1;
    }
    
    qDebug() << "测试文件:" << testFile;
    qDebug() << "文件大小:" << fileInfo.size() << "bytes";
    
    try {
        // 创建STEP模型树组件
        STEPModelTreeWidget* treeWidget = new STEPModelTreeWidget();
        treeWidget->setWindowTitle("STEP模型树测试");
        
        // 连接完成信号
        QObject::connect(treeWidget, &STEPModelTreeWidget::loadCompleted,
            [&](bool success, const QString& message) {
                qDebug() << "=== STEP模型树加载完成 ===";
                qDebug() << "成功:" << success;
                qDebug() << "消息:" << message;
                
                if (success) {
                    qDebug() << "加载成功！等待5秒后自动退出...";
                    QTimer::singleShot(5000, [&]() {
                        qDebug() << "测试完成，正常退出";
                        app.quit();
                    });
                } else {
                    qDebug() << "加载失败，2秒后退出";
                    QTimer::singleShot(2000, [&]() {
                        app.quit();
                    });
                }
            });
        
        // 显示窗口
        treeWidget->show();
        treeWidget->resize(500, 700);
        
        // 延迟开始加载，让窗口先显示
        QTimer::singleShot(500, [=]() {
            qDebug() << "开始加载STEP文件...";
            treeWidget->loadSTEPFile(testFile);
        });
        
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