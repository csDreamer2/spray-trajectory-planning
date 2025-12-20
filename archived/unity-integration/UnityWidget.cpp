#include "UnityWidget.h"
#include "QtUnityBridge.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QProcess>
#include <QMessageBox>
#include <QApplication>
#include <QDir>
#include <QTimer>
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>
#include <QWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <QWindow>
#include <QTimer>
#endif

namespace UI {

UnityWidget::UnityWidget(QWidget *parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_placeholderLabel(nullptr)
    , m_initButton(nullptr)
    , m_unityProcess(nullptr)
    , m_unityInitialized(false)
    , m_unityWindowId(0)
    , m_bridge(nullptr)
{
    // 设置焦点策略以接收键盘和鼠标事件
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    
    setupUI();
}

UnityWidget::~UnityWidget()
{
    if (m_unityProcess && m_unityProcess->state() != QProcess::NotRunning) {
        m_unityProcess->terminate();
        if (!m_unityProcess->waitForFinished(3000)) {
            m_unityProcess->kill();
        }
    }
}

void UnityWidget::setupUI()
{
    m_layout = new QVBoxLayout(this);
    
    // 创建状态标签
    m_statusLabel = new QLabel("状态: 等待Unity连接", this);
    m_statusLabel->setStyleSheet(
        "QLabel {"
        "   background-color: #3a3a3a;"
        "   color: #cccccc;"
        "   padding: 5px;"
        "   border-radius: 3px;"
        "   font-size: 12px;"
        "}"
    );
    m_statusLabel->setMaximumHeight(25);
    
    // 创建Unity显示区域（占位符，显示连接状态和指导信息）
    m_placeholderLabel = new QLabel(this);
    m_placeholderLabel->setAlignment(Qt::AlignCenter);
    m_placeholderLabel->setStyleSheet(
        "QLabel {"
        "   border: 2px dashed #666666;"
        "   border-radius: 10px;"
        "   background-color: #2a2a2a;"
        "   color: #cccccc;"
        "   font-size: 14px;"
        "   padding: 20px;"
        "}"
    );
    m_placeholderLabel->setMinimumSize(800, 600);
    
    // 创建控制按钮区域
    QWidget* controlWidget = new QWidget(this);
    controlWidget->setMaximumHeight(50);
    QHBoxLayout* controlLayout = new QHBoxLayout(controlWidget);
    
    m_initButton = new QPushButton("启动Unity应用程序", this);
    m_initButton->setMaximumWidth(150);
    connect(m_initButton, &QPushButton::clicked, this, &UnityWidget::InitializeUnity);
    
    QPushButton* openUnityButton = new QPushButton("打开Unity窗口", this);
    openUnityButton->setMaximumWidth(120);
    connect(openUnityButton, &QPushButton::clicked, this, [this]() {
        if (m_unityProcess && m_unityProcess->state() == QProcess::Running) {
            // Unity进程正在运行，尝试将其窗口置于前台
#ifdef Q_OS_WIN
            HWND unityHwnd = FindWindowA(nullptr, "SpraySimulation");
            if (!unityHwnd) {
                unityHwnd = FindWindowA(nullptr, "Unity Player");
            }
            if (unityHwnd) {
                SetForegroundWindow(unityHwnd);
                ShowWindow(unityHwnd, SW_RESTORE);
                qDebug() << "✅ Unity窗口已置于前台";
            } else {
                qDebug() << "❌ 未找到Unity窗口";
            }
#endif
        } else {
            qDebug() << "❌ Unity进程未运行";
        }
    });
    
    QPushButton* helpButton = new QPushButton("帮助", this);
    helpButton->setMaximumWidth(80);
    connect(helpButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "Unity集成帮助", 
            "Unity 3D视图集成说明：\n\n"
            "1. 点击'启动Unity应用程序'按钮\n"
            "2. Unity会在独立窗口中运行\n"
            "3. 点击'打开Unity窗口'将Unity窗口置于前台\n"
            "4. Unity连接成功后即可加载点云数据\n\n"
            "使用提示：\n"
            "• Unity在独立窗口中运行，可自由移动\n"
            "• 通过TCP通信传输点云数据\n"
            "• 可以同时操作Qt程序和Unity窗口");
    });
    
    controlLayout->addStretch();
    controlLayout->addWidget(m_initButton);
    controlLayout->addWidget(openUnityButton);
    controlLayout->addWidget(helpButton);
    controlLayout->addStretch();
    
    // 布局
    m_layout->addWidget(m_statusLabel);
    m_layout->addWidget(m_placeholderLabel, 1);
    m_layout->addWidget(controlWidget);
    
    setLayout(m_layout);
    
    // 初始化显示内容
    updateConnectionStatus();
}

void UnityWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    // Unity在独立窗口中运行，不需要调整嵌入窗口大小
}

void UnityWidget::SetBridge(QtUnityBridge* bridge)
{
    m_bridge = bridge;
    
    if (m_bridge) {
        // 连接信号
        connect(m_bridge, &QtUnityBridge::UnityConnected, this, &UnityWidget::OnBridgeConnected);
        connect(m_bridge, &QtUnityBridge::UnityDisconnected, this, &UnityWidget::OnBridgeDisconnected);
        connect(m_bridge, &QtUnityBridge::ConnectionError, this, &UnityWidget::OnBridgeError);
        connect(m_bridge, &QtUnityBridge::WorkpieceLoaded, this, &UnityWidget::OnWorkpieceLoaded);
        connect(m_bridge, &QtUnityBridge::TrajectoryDisplayed, this, &UnityWidget::OnTrajectoryDisplayed);
        
        updateConnectionStatus();
    }
}

bool UnityWidget::InitializeUnity()
{
    if (m_unityInitialized) {
        return true;
    }
    
    m_initButton->setText("正在初始化...");
    m_initButton->setEnabled(false);
    
    // 创建Unity进程
    m_unityProcess = new QProcess(this);
    
    // 连接信号
    connect(m_unityProcess, &QProcess::started, 
            this, &UnityWidget::OnUnityProcessStarted);
    connect(m_unityProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &UnityWidget::OnUnityProcessFinished);
    connect(m_unityProcess, &QProcess::errorOccurred,
            this, &UnityWidget::OnUnityProcessError);
    
    // 优先查找构建的Unity应用程序
    QString unityAppPath = QApplication::applicationDirPath() + "/Unity/SpraySimulation.exe";
    QString unityProjectPath = QApplication::applicationDirPath() + "/../Unity/SpraySimulation";
    
    qDebug() << "检查Unity应用程序路径:" << unityAppPath;
    
    if (QFile::exists(unityAppPath)) {
        // 方案A: 启动构建的Unity应用程序（推荐）
        qDebug() << "✅ 找到构建的Unity应用程序，使用独立模式";
        
        QStringList arguments;
        arguments << "-screen-width" << "1280";
        arguments << "-screen-height" << "720";
        arguments << "-screen-fullscreen" << "0"; // 窗口模式
        
        qDebug() << "启动Unity应用程序:" << unityAppPath;
        qDebug() << "启动参数:" << arguments;
        
        m_unityProcess->start(unityAppPath, arguments);
        
        m_placeholderLabel->setText(
            "🎮 Unity 3D 仿真引擎\n\n"
            "✅ 启动独立Unity应用程序\n\n"
            "📋 正在启动Unity 3D引擎...\n"
            "等待应用程序完全加载\n\n"
            "💡 提示：这是优化的独立版本\n"
            "   没有编辑器界面干扰"
        );
        
        return true;
    }
    
    // 方案B: 回退到Unity编辑器模式
    qDebug() << "⚠️ 未找到构建的Unity应用程序，回退到编辑器模式";
    
    // 查找Unity编辑器
    QString unityEditorPath;
    QStringList possibleUnityPaths = {
        "C:/Program Files/Unity/Hub/Editor/2022.3.*/Editor/Unity.exe",
        "C:/Program Files/Unity/Hub/Editor/2023.*/Editor/Unity.exe",
        "C:/Program Files/Unity/Hub/Editor/*/Editor/Unity.exe",
        "C:/Program Files (x86)/Unity/Editor/Unity.exe",
        "C:/Program Files/Unity/Editor/Unity.exe"
    };
    
    // 查找Unity编辑器
    for (const QString& path : possibleUnityPaths) {
        if (path.contains("*")) {
            // 处理通配符路径
            QDir dir(path.section('/', 0, -3));
            if (dir.exists()) {
                QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
                for (const QFileInfo& entry : entries) {
                    QString testPath = entry.absoluteFilePath() + "/Editor/Unity.exe";
                    if (QFile::exists(testPath)) {
                        unityEditorPath = testPath;
                        break;
                    }
                }
            }
        } else {
            if (QFile::exists(path)) {
                unityEditorPath = path;
                break;
            }
        }
        if (!unityEditorPath.isEmpty()) break;
    }
    
    if (unityEditorPath.isEmpty()) {
        m_placeholderLabel->setText(
            "Unity 3D 仿真引擎\n\n"
            "❌ 未找到Unity应用程序或编辑器\n\n"
            "推荐方案：构建Unity独立应用\n"
            "1. 在Unity中打开SpraySimulation项目\n"
            "2. File → Build Settings\n"
            "3. 构建到: build/bin/Debug/Unity/\n"
            "4. 文件名: SpraySimulation.exe\n\n"
            "备用方案：手动启动Unity编辑器\n"
            "然后点击'手动嵌入Unity'按钮"
        );
        m_initButton->setText("需要构建Unity应用");
        m_initButton->setEnabled(true);
        return false;
    }
    
    if (!QDir(unityProjectPath).exists()) {
        m_placeholderLabel->setText(
            "Unity 3D 仿真引擎\n\n"
            "❌ Unity项目路径不存在\n"
            "项目路径: " + unityProjectPath + "\n\n"
            "请检查Unity项目是否正确放置"
        );
        m_initButton->setText("Unity项目未找到");
        emit UnityError("Unity项目路径不存在");
        return false;
    }
    
    // 启动Unity编辑器（备用方案）
    QStringList arguments;
    arguments << "-projectPath" << QDir::toNativeSeparators(unityProjectPath);
    arguments << "-logFile" << "-";
    
    qDebug() << "启动Unity编辑器（备用方案）:" << unityEditorPath;
    qDebug() << "项目路径:" << unityProjectPath;
    
    m_placeholderLabel->setText(
        "🎮 Unity 3D 仿真引擎\n\n"
        "⚠️ 使用Unity编辑器模式\n\n"
        "📋 正在启动Unity编辑器...\n"
        "请在Unity中点击▶️播放按钮\n\n"
        "💡 建议：构建独立Unity应用\n"
        "   获得更好的用户体验"
    );
    
    m_unityProcess->start(unityEditorPath, arguments);
    
    return true;
}

void UnityWidget::ShowWorkpiece(const QString& workpieceData)
{
    if (!m_bridge || !m_bridge->IsConnected()) {
        qWarning() << "Unity未连接，无法显示工件";
        return;
    }
    
    // 构建工件数据JSON
    QJsonObject workpiece;
    workpiece["file_path"] = workpieceData;
    workpiece["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    m_bridge->SendWorkpieceData(workpiece);
}

void UnityWidget::ShowTrajectory(const QString& trajectoryData)
{
    if (!m_bridge || !m_bridge->IsConnected()) {
        qWarning() << "Unity未连接，无法显示轨迹";
        return;
    }
    
    // 构建轨迹数据JSON
    QJsonObject trajectory;
    trajectory["data"] = trajectoryData;
    trajectory["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    m_bridge->SendTrajectoryData(trajectory);
}

void UnityWidget::StartSimulation()
{
    if (!m_bridge || !m_bridge->IsConnected()) {
        qWarning() << "Unity未连接，无法启动仿真";
        return;
    }
    
    m_bridge->StartSimulation();
    updateConnectionStatus();
}

void UnityWidget::StopSimulation()
{
    if (!m_bridge || !m_bridge->IsConnected()) {
        qWarning() << "Unity未连接，无法停止仿真";
        return;
    }
    
    m_bridge->StopSimulation();
    updateConnectionStatus();
}

void UnityWidget::ResetView()
{
    if (!m_bridge || !m_bridge->IsConnected()) {
        qWarning() << "Unity未连接，无法重置视图";
        return;
    }
    
    m_bridge->ResetCamera();
    updateConnectionStatus();
}

void UnityWidget::OnUnityProcessStarted()
{
    m_placeholderLabel->setText(
        "🎮 Unity 3D 仿真引擎\n\n"
        "✅ Unity应用程序启动成功\n\n"
        "📋 Unity正在独立窗口中运行\n"
        "等待Unity连接到Qt程序...\n\n"
        "💡 提示：Unity窗口是独立的\n"
        "   可以自由移动和调整大小\n\n"
        "🔗 连接状态：等待Unity连接..."
    );
    
    m_initButton->setText("Unity应用已启动");
    m_initButton->setEnabled(false);
    
    // 不再需要嵌入定时器
    m_unityInitialized = true;
    
    qDebug() << "Unity进程已启动，等待TCP连接";
}

void UnityWidget::OnUnityProcessFinished(int exitCode)
{
    m_unityInitialized = false;
    m_initButton->setText("重新启动Unity");
    m_initButton->setEnabled(true);
    
    if (exitCode == 0) {
        m_placeholderLabel->setText(
            "🎮 Unity 3D 仿真引擎\n\n"
            "Unity应用程序已正常退出\n\n"
            "点击'重新启动Unity'按钮\n"
            "可以重新启动Unity应用程序"
        );
    } else {
        m_placeholderLabel->setText(
            "🎮 Unity 3D 仿真引擎\n\n"
            "❌ Unity应用程序异常退出\n"
            "退出代码: " + QString::number(exitCode) + "\n\n"
            "点击'重新启动Unity'按钮重试"
        );
        emit UnityError(QString("Unity进程异常退出，代码: %1").arg(exitCode));
    }
}

void UnityWidget::OnUnityProcessError(QProcess::ProcessError error)
{
    m_unityInitialized = false;
    m_initButton->setText("重新启动Unity");
    m_initButton->setEnabled(true);
    
    QString errorMsg;
    switch (error) {
    case QProcess::FailedToStart:
        errorMsg = "Unity应用程序启动失败";
        break;
    case QProcess::Crashed:
        errorMsg = "Unity应用程序崩溃";
        break;
    case QProcess::Timedout:
        errorMsg = "Unity应用程序超时";
        break;
    case QProcess::WriteError:
        errorMsg = "Unity应用程序写入错误";
        break;
    case QProcess::ReadError:
        errorMsg = "Unity应用程序读取错误";
        break;
    default:
        errorMsg = "Unity应用程序未知错误";
        break;
    }
    
    m_placeholderLabel->setText(
        "🎮 Unity 3D 仿真引擎\n\n"
        "❌ 错误: " + errorMsg + "\n\n"
        "请检查Unity应用程序是否正确构建\n"
        "点击'重新启动Unity'按钮重试"
    );
    
    emit UnityError(errorMsg);
}

// 窗口嵌入功能已移除，改为独立窗口模式

// 窗口嵌入功能已移除，Unity现在在独立窗口中运行

void UnityWidget::OnBridgeConnected()
{
    qDebug() << "Unity Bridge: 连接已建立";
    updateConnectionStatus();
}

void UnityWidget::OnBridgeDisconnected()
{
    qDebug() << "Unity Bridge: 连接已断开";
    updateConnectionStatus();
}

void UnityWidget::OnBridgeError(const QString& error)
{
    qWarning() << "Unity Bridge错误:" << error;
    m_statusLabel->setText("状态: 连接错误 - " + error);
    m_statusLabel->setStyleSheet(
        "QLabel {"
        "   background-color: #aa3333;"
        "   color: white;"
        "   padding: 5px;"
        "   border-radius: 3px;"
        "}"
    );
}

void UnityWidget::OnWorkpieceLoaded(bool success, const QString& message)
{
    if (success) {
        qDebug() << "工件加载成功:" << message;
        m_placeholderLabel->setText(
            "Unity 3D 仿真视图\n\n"
            "工件已成功加载\n" + message
        );
    } else {
        qWarning() << "工件加载失败:" << message;
        m_placeholderLabel->setText(
            "Unity 3D 仿真视图\n\n"
            "工件加载失败\n" + message
        );
    }
}

void UnityWidget::OnTrajectoryDisplayed(bool success, const QString& message)
{
    if (success) {
        qDebug() << "轨迹显示成功:" << message;
        m_placeholderLabel->setText(
            "Unity 3D 仿真视图\n\n"
            "轨迹已成功显示\n" + message
        );
    } else {
        qWarning() << "轨迹显示失败:" << message;
        m_placeholderLabel->setText(
            "Unity 3D 仿真视图\n\n"
            "轨迹显示失败\n" + message
        );
    }
}

void UnityWidget::updateConnectionStatus()
{
    if (m_bridge && m_bridge->IsConnected()) {
        m_statusLabel->setText("状态: Unity已连接 ✓");
        m_statusLabel->setStyleSheet(
            "QLabel {"
            "   background-color: #33aa33;"
            "   color: white;"
            "   padding: 5px;"
            "   border-radius: 3px;"
            "   font-size: 12px;"
            "}"
        );
        
        m_placeholderLabel->setText(
            "🎮 Unity 3D 仿真引擎\n\n"
            "✅ Unity应用程序连接成功！\n\n"
            "📋 系统状态：\n"
            "• TCP通信：已建立\n"
            "• 点云传输：就绪\n"
            "• 3D渲染：正常\n\n"
            "🖥️ Unity独立窗口：\n"
            "• 可自由移动和调整大小\n"
            "• 支持完整的3D交互\n"
            "• 点击'打开Unity窗口'置于前台\n\n"
            "🚀 现在可以导入点云文件进行3D可视化"
        );
        
        m_initButton->setText("Unity已连接");
        m_initButton->setEnabled(false);
    } else {
        m_statusLabel->setText("状态: 等待Unity连接...");
        m_statusLabel->setStyleSheet(
            "QLabel {"
            "   background-color: #aa6633;"
            "   color: white;"
            "   padding: 5px;"
            "   border-radius: 3px;"
            "   font-size: 12px;"
            "}"
        );
        
        if (m_unityInitialized) {
            m_placeholderLabel->setText(
                "🎮 Unity 3D 仿真引擎\n\n"
                "⏳ Unity应用程序已启动\n"
                "等待TCP连接建立...\n\n"
                "📋 连接信息：\n"
                "• 服务器地址：localhost:12346\n"
                "• 通信协议：TCP Socket\n"
                "• 数据格式：JSON\n\n"
                "🖥️ Unity窗口：\n"
                "• 在独立窗口中运行\n"
                "• 点击'打开Unity窗口'查看\n\n"
                "💡 如果连接失败，请重启Unity应用程序"
            );
        } else {
            m_placeholderLabel->setText(
                "🎮 Unity 3D 仿真引擎\n\n"
                "⏳ 准备启动Unity应用程序...\n\n"
                "📋 启动步骤：\n"
                "1. 点击'启动Unity应用程序'按钮\n"
                "2. Unity将在独立窗口中启动\n"
                "3. 自动建立TCP通信连接\n"
                "4. 开始3D点云可视化\n\n"
                "🔧 系统要求：\n"
                "• Unity应用程序已构建\n"
                "• 端口12346可用\n\n"
                "❓ 如需帮助，请点击'帮助'按钮"
            );
            
            m_initButton->setText("启动Unity应用程序");
            m_initButton->setEnabled(true);
        }
    }
}

// 事件处理已简化，Unity在独立窗口中运行，不需要事件转发

} // namespace UI