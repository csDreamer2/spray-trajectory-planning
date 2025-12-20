#ifndef CORE_CONFIGMANAGER_H
#define CORE_CONFIGMANAGER_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <QVariant>

namespace Core {

/**
 * @brief 配置管理器
 * 
 * 负责应用程序配置的读取、保存和管理
 */
class ConfigManager : public QObject
{
    Q_OBJECT

public:
    explicit ConfigManager(QObject *parent = nullptr);
    ~ConfigManager();

    bool initialize();

    // 配置读写
    QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const;
    void setValue(const QString& key, const QVariant& value);

    // 专用配置访问
    QString getDatabasePath() const;
    QString getLogLevel() const;
    
    void setDatabasePath(const QString& path);
    void setLogLevel(const QString& level);

signals:
    void configChanged(const QString& key, const QVariant& value);

private:
    void loadDefaultSettings();

private:
    QSettings* m_settings;
};

} // namespace Core

#endif // CORE_CONFIGMANAGER_H