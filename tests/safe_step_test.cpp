#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <QFileInfo>
#include <QMessageBox>
#include <QProgressDialog>
#include <iostream>

// 只测试STEP模型树，增加更多保护措施
#include "../src/UI/STEPModelTreeWidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "=== 安全STEP模型树测试程序 ===";
    
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
    
    // 对于大文件给出警告
    if (fileInfo.size() > 100 * 1024 * 1024) { // 100MB
        QMessageBox::StandardButton reply = QMessageBox::question(nullptr, "大文件警告", 
            QString("文件大小为 %1 MB，解析可能需要较长时间且有崩溃风险。\n是否继续？")
            .arg(fileInfo.size() / (1024.0 * 1024.0), 0, 'f', 1),
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply != QMessageBox::Yes) {
            return 0;
        }
    }
    
    try {
        // 创建进度对话框
        QProgressDialog* progressDialog = new QProgressDialog("正在加载STEP文件...", "取消", 0, 100);
        progressDialog->setWindowModality(Qt::WindowModal);
        progressDialog->show();
        
        // 创建STEP模型树组件
        STEPModelTreeWidget* treeWidget = new STEPModelTreeWidget();
        treeWidget->setWindowTitle("安全STEP模型树测试");
        
        // 连接进度信号
        QObject::connect(treeWidget, &STEPModelTreeWidget::loadCompleted,
            [&](bool success, const QString& message) {
                progressDialog->hide();
                
                qDebug() << "=== STEP模型树加载完成 ===";
                qDebug() << "成功:" << success;
                qDebug() << "消息:" << message;
                
                if (success) {
                    qDebug() << "加载成功！显示结果窗口，10秒后自动退出...";
                    treeWidget->show();
                    treeWidget->resize(600, 800);
                    
                    QTimer::singleShot(10000, [&]() {
                        qDebug() << "测试完成，正常退出";
                        app.quit();
                    });
                } else {
                    qDebug() << "加载失败，显示错误信息";
                    QMessageBox::critical(nullptr, "加载失败", message);
                    QTimer::singleShot(2000, [&]() {
                        app.quit();
                    });
                }
            });
        
        // 连接取消信号
        QObject::connect(progressDialog, &QProgressDialog::canceled, [&]() {
            qDebug() << "用户取消加载";
            // 这里应该取消加载，但目前的实现可能不支持
            app.quit();
        });
        
        // 延迟开始加载
        QTimer::singleShot(1000, [=]() {
            qDebug() << "开始加载STEP文件...";
            qDebug() << "注意：大文件可能需要几分钟时间，请耐心等待";
            treeWidget->loadSTEPFile(testFile);
        });
        
        // 设置超时保护（10分钟）
        QTimer::singleShot(600000, [&]() {
            qWarning() << "加载超时（10分钟），强制退出";
            QMessageBox::warning(nullptr, "超时", "加载超时，程序将退出");
            app.quit();
        });
        
        // 运行事件循环
        int result = app.exec();
        
        qDebug() << "=== 程序正常结束 ===";
        
        // 手动清理
        delete treeWidget;
        delete progressDialog;
        
        return result;
        
    } catch (const std::exception& e) {
        qCritical() << "程序异常:" << e.what();
        return -1;
    } catch (...) {
        qCritical() << "未知异常";
        return -1;
    }
}