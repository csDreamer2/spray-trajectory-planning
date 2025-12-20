#include "Application.h"
#include "ConfigManager.h"
#include "Logger.h"

#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDebug>

namespace Core {

Application* Application::s_instance = nullptr;

Application::Application(QObject *parent)
    : QObject(parent)
    , m_configManager(nullptr)
    , m_logger(nullptr)
    , m_initialized(false)
{
    s_instance = this;
}

Application::~Application()
{
    cleanup();
    s_instance = nullptr;
}

bool Application::initialize()
{
    if (m_initialized) {
        return true;
    }

    qDebug() << "正在初始化核心应用程序...";

    // 1. 初始化目录结构
    if (!initializeDirectories()) {
        emit error("初始化目录结构失败");
        return false;
    }

    // 2. 初始化日志系统
    if (!initializeLogging()) {
        emit error("初始化日志系统失败");
        return false;
    }

    // 3. 初始化配置管理器
    m_configManager = std::make_unique<ConfigManager>(this);
    if (!m_configManager->initialize()) {
        emit error("初始化配置管理器失败");
        return false;
    }

    // 4. 初始化数据库
    if (!initializeDatabase()) {
        emit error("初始化数据库失败");
        return false;
    }

    m_initialized = true;
    
    if (m_logger) {
        m_logger->info("核心应用程序初始化完成");
    }
    
    emit initialized();
    return true;
}

void Application::cleanup()
{
    if (!m_initialized) {
        return;
    }

    if (m_logger) {
        m_logger->info("正在清理核心应用程序...");
    }

    m_configManager.reset();
    m_logger.reset();

    m_initialized = false;
}

Application* Application::instance()
{
    return s_instance;
}

bool Application::initializeDirectories()
{
    // 获取应用程序数据目录
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    
    // 创建必要的目录
    QStringList directories = {
        appDataPath,
        appDataPath + "/logs",
        appDataPath + "/config",
        appDataPath + "/projects",
        appDataPath + "/temp",
        appDataPath + "/cache"
    };

    for (const QString& dir : directories) {
        QDir directory;
        if (!directory.mkpath(dir)) {
            qCritical() << "无法创建目录:" << dir;
            return false;
        }
    }

    qDebug() << "应用程序数据目录:" << appDataPath;
    return true;
}

bool Application::initializeDatabase()
{
    qDebug() << "Application: 初始化数据库系统...";
    
    // 这里暂时返回true，实际的数据库初始化将在后续集成
    // 避免循环依赖问题，数据库初始化可以在UI层或者单独的初始化流程中进行
    qDebug() << "Application: 数据库系统初始化完成（占位实现）";
    return true;
}

bool Application::initializeLogging()
{
    try {
        m_logger = std::make_unique<Logger>(this);
        if (!m_logger->initialize()) {
            qCritical() << "日志系统初始化失败";
            return false;
        }
        
        m_logger->info("日志系统初始化完成");
        return true;
    }
    catch (const std::exception& e) {
        qCritical() << "日志系统初始化异常:" << e.what();
        return false;
    }
}

} // namespace Core