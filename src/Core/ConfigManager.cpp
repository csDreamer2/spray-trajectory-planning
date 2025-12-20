#include "ConfigManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

namespace Core {

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
    , m_settings(nullptr)
{
}

ConfigManager::~ConfigManager()
{
    delete m_settings;
}

bool ConfigManager::initialize()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(configPath);
    
    QString configFile = configPath + "/config.ini";
    m_settings = new QSettings(configFile, QSettings::IniFormat, this);
    
    loadDefaultSettings();
    
    qDebug() << "配置管理器初始化完成，配置文件:" << configFile;
    return true;
}

QVariant ConfigManager::getValue(const QString& key, const QVariant& defaultValue) const
{
    if (!m_settings) {
        return defaultValue;
    }
    return m_settings->value(key, defaultValue);
}

void ConfigManager::setValue(const QString& key, const QVariant& value)
{
    if (!m_settings) {
        return;
    }
    
    m_settings->setValue(key, value);
    m_settings->sync();
    
    emit configChanged(key, value);
}

QString ConfigManager::getDatabasePath() const
{
    return getValue("Database/Path", "spray_trajectory.db").toString();
}

QString ConfigManager::getLogLevel() const
{
    return getValue("Logging/Level", "Info").toString();
}

void ConfigManager::setDatabasePath(const QString& path)
{
    setValue("Database/Path", path);
}

void ConfigManager::setLogLevel(const QString& level)
{
    setValue("Logging/Level", level);
}

void ConfigManager::loadDefaultSettings()
{
    if (!m_settings) {
        return;
    }
    
    // 设置默认值（如果不存在）
    if (!m_settings->contains("Database/Type")) {
        m_settings->setValue("Database/Type", "SQLite");
        m_settings->setValue("Database/Path", "spray_trajectory.db");
    }
    
    if (!m_settings->contains("Logging/Level")) {
        m_settings->setValue("Logging/Level", "Info");
        m_settings->setValue("Logging/MaxFileSize", "10MB");
    }
    
    m_settings->sync();
}

} // namespace Core