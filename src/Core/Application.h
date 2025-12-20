#ifndef CORE_APPLICATION_H
#define CORE_APPLICATION_H

#include <QObject>
#include <QString>
#include <memory>

namespace Core {

class ConfigManager;
class Logger;

/**
 * @brief 核心应用程序类
 * 
 * 负责应用程序的初始化、配置管理、日志记录等核心功能
 */
class Application : public QObject
{
    Q_OBJECT

public:
    explicit Application(QObject *parent = nullptr);
    ~Application();

    // 初始化和清理
    bool initialize();
    void cleanup();

    // 获取单例实例
    static Application* instance();

    // 获取核心组件
    ConfigManager* configManager() const { return m_configManager.get(); }
    Logger* logger() const { return m_logger.get(); }

    // 应用程序信息
    QString applicationVersion() const { return "1.0.0"; }
    QString applicationName() const { return "自动喷涂轨迹规划系统"; }

signals:
    void initialized();
    void error(const QString& message);

private:
    bool initializeDirectories();
    bool initializeDatabase();
    bool initializeLogging();

private:
    static Application* s_instance;
    
    std::unique_ptr<ConfigManager> m_configManager;
    std::unique_ptr<Logger> m_logger;
    
    bool m_initialized;
};

} // namespace Core

#endif // CORE_APPLICATION_H