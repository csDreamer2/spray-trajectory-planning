#ifndef DATABASEINITIALIZER_H
#define DATABASEINITIALIZER_H

#include <QObject>
#include <QString>

namespace Data {

/**
 * @brief 数据库初始化器
 * 
 * 负责系统启动时的数据库初始化工作，包括：
 * - 检查数据库连接
 * - 创建默认数据
 * - 数据迁移
 * - 初始配置
 */
class DatabaseInitializer : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseInitializer(QObject *parent = nullptr);

    // 初始化流程
    bool initializeSystem();
    bool createDefaultData();
    bool migrateData();
    bool validateDatabase();

    // 配置管理
    bool loadSystemConfig();
    bool saveSystemConfig();
    
    // 默认数据创建
    bool createDefaultUsers();
    bool createDefaultCategories();
    bool createSystemConfigs();

signals:
    void initializationProgress(int percentage, const QString& message);
    void initializationCompleted(bool success);
    void errorOccurred(const QString& error);

private:
    bool checkDatabaseVersion();
    bool upgradeDatabase(int fromVersion, int toVersion);
    QString getDefaultConfigValue(const QString& key) const;

private:
    static const int CURRENT_DATABASE_VERSION = 1;
};

} // namespace Data

#endif // DATABASEINITIALIZER_H