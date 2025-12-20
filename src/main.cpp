#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDebug>
#include "UI/MainWindow.h"
#include "Core/Application.h"

#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
#ifdef _WIN32
    // 为Windows GUI程序分配控制台窗口
    if (AllocConsole()) {
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
        freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
        freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
        
        // 设置控制台标题
        SetConsoleTitle(L"Qt Application Debug Console");
        
        qDebug() << "=== Qt Application Debug Console Started ===";
    }
#endif
    
    // 设置应用程序信息
    app.setApplicationName("自动喷涂轨迹规划系统");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("SprayTech");
    app.setOrganizationDomain("spraytech.com");
    
    // 设置应用程序样式
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // 设置深色主题
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    app.setPalette(darkPalette);
    
    try {
        // 初始化核心应用程序
        Core::Application coreApp;
        if (!coreApp.initialize()) {
            QMessageBox::critical(nullptr, "初始化错误", "应用程序初始化失败！");
            return -1;
        }
        
        // 创建主窗口
        UI::MainWindow mainWindow;
        mainWindow.show();
        
        return app.exec();
    }
    catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "运行时错误", 
                            QString("应用程序运行时发生错误：%1").arg(e.what()));
        return -1;
    }
}