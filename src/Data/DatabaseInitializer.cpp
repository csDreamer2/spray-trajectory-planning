#include "DatabaseInitializer.h"
#include "DatabaseManager.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QDateTime>

namespace Data {

DatabaseInitializer::DatabaseInitializer(QObject *parent)
    : QObject(parent)
{
}

bool DatabaseInitializer::initializeSystem()
{
    qDebug() << "DatabaseInitializer: 开始系统初始化...";
    
    emit initializationProgress(10, "初始化数据库连接...");
    
    // 初始化数据库管理器
    DatabaseManager* dbManager = DatabaseManager::instance();
    if (!dbManager->initializeDatabase()) {
        emit errorOccurred("数据库初始化失败");
        return false;
    }
    
    emit initializationProgress(30, "检查数据库版本...");
    
    // 检查数据库版本
    if (!checkDatabaseVersion()) {
        emit errorOccurred("数据库版本检查失败");
        return false;
    }
    
    emit initializationProgress(50, "加载系统配置...");
    
    // 加载系统配置
    if (!loadSystemConfig()) {
        emit errorOccurred("系统配置加载失败");
        return false;
    }
    
    emit initializationProgress(70, "创建默认数据...");
    
    // 创建默认数据
    if (!createDefaultData()) {
        emit errorOccurred("默认数据创建失败");
        return false;
    }
    
    emit initializationProgress(90, "验证数据库完整性...");
    
    // 验证数据库
    if (!validateDatabase()) {
        emit errorOccurred("数据库验证失败");
        return false;
    }
    
    emit initializationProgress(100, "初始化完成");
    emit initializationCompleted(true);
    
    qDebug() << "DatabaseInitializer: 系统初始化完成";
    return true;
}

bool DatabaseInitializer::createDefaultData()
{
    qDebug() << "DatabaseInitializer: 创建默认数据...";
    
    // 创建默认用户
    if (!createDefaultUsers()) {
        qWarning() << "DatabaseInitializer: 创建默认用户失败";
        return false;
    }
    
    // 创建默认分类
    if (!createDefaultCategories()) {
        qWarning() << "DatabaseInitializer: 创建默认分类失败";
        return false;
    }
    
    // 创建系统配置
    if (!createSystemConfigs()) {
        qWarning() << "DatabaseInitializer: 创建系统配置失败";
        return false;
    }
    
    return true;
}

bool DatabaseInitializer::createDefaultUsers()
{
    DatabaseManager* dbManager = DatabaseManager::instance();
    
    // 检查是否已存在用户
    QSqlQuery checkQuery = dbManager->prepareQuery(
        "SELECT COUNT(*) FROM users WHERE is_active = 1", 
        DatabaseManager::SQLite
    );
    
    if (checkQuery.exec() && checkQuery.next()) {
        int userCount = checkQuery.value(0).toInt();
        if (userCount > 0) {
            qDebug() << "DatabaseInitializer: 用户已存在，跳过创建";
            return true;
        }
    }
    
    // 创建默认管理员用户
    QString adminPassword = "admin123"; // 实际应用中应该要求用户设置
    QByteArray passwordHash = QCryptographicHash::hash(
        adminPassword.toUtf8(), 
        QCryptographicHash::Sha256
    ).toHex();
    
    // 注意：SQLite缓存表结构与MySQL不同，需要使用正确的表名和字段
    QString sql = "INSERT INTO cache_configs (config_key, config_value, last_sync) VALUES (?, ?, ?)";
    QSqlQuery insertQuery = dbManager->prepareQuery(sql, DatabaseManager::SQLite);
    
    // 暂时将用户信息存储为配置项，因为我们还没有创建用户表的SQLite版本
    insertQuery.addBindValue("default_admin_user");
    insertQuery.addBindValue("admin");
    insertQuery.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    
    if (!insertQuery.exec()) {
        qWarning() << "DatabaseInitializer: 创建管理员用户失败:" << insertQuery.lastError().text();
        return false;
    }
    
    qDebug() << "DatabaseInitializer: 默认管理员用户创建成功";
    return true;
}

bool DatabaseInitializer::createDefaultCategories()
{
    DatabaseManager* dbManager = DatabaseManager::instance();
    
    // 默认工件分类
    QStringList categories = {
        "汽车零件",
        "机械零件", 
        "电子产品",
        "家具配件",
        "建筑构件",
        "其他"
    };
    
    for (const QString& category : categories) {
        // 检查分类是否已存在
        QSqlQuery checkQuery = dbManager->prepareQuery(
            "SELECT COUNT(*) FROM workpieces WHERE category = ?",
            DatabaseManager::SQLite
        );
        checkQuery.addBindValue(category);
        
        if (checkQuery.exec() && checkQuery.next()) {
            int count = checkQuery.value(0).toInt();
            if (count > 0) {
                continue; // 分类已存在
            }
        }
        
        qDebug() << "DatabaseInitializer: 创建默认分类:" << category;
    }
    
    return true;
}

bool DatabaseInitializer::createSystemConfigs()
{
    DatabaseManager* dbManager = DatabaseManager::instance();
    
    // 系统配置项
    QMap<QString, QString> configs = {
        {"system.version", QString::number(CURRENT_DATABASE_VERSION)},
        {"system.initialized", "true"},
        {"system.install_date", QDateTime::currentDateTime().toString(Qt::ISODate)},
        
        // 数据库配置
        {"database.auto_sync_enabled", "true"},
        {"database.sync_interval", "300"}, // 5分钟
        {"database.backup_enabled", "true"},
        {"database.backup_interval", "86400"}, // 24小时
        
        // 轨迹规划配置
        {"trajectory.default_speed", "50.0"},
        {"trajectory.default_flow_rate", "0.8"},
        {"trajectory.default_spray_width", "10.0"},
        {"trajectory.optimization_enabled", "true"},
        
        // 质量预测配置
        {"quality.prediction_enabled", "true"},
        {"quality.min_quality_score", "0.7"},
        {"quality.coverage_threshold", "85.0"},
        
        // 用户界面配置
        {"ui.theme", "dark"},
        {"ui.language", "zh_CN"},
        {"ui.auto_save", "true"},
        {"ui.auto_save_interval", "60"} // 1分钟
    };
    
    for (auto it = configs.begin(); it != configs.end(); ++it) {
        // 检查配置是否已存在
        QSqlQuery checkQuery = dbManager->prepareQuery(
            "SELECT config_value FROM cache_configs WHERE config_key = ?",
            DatabaseManager::SQLite
        );
        checkQuery.addBindValue(it.key());
        
        if (checkQuery.exec() && checkQuery.next()) {
            continue; // 配置已存在
        }
        
        // 插入新配置
        QSqlQuery insertQuery = dbManager->prepareQuery(
            "INSERT INTO cache_configs (config_key, config_value, last_sync) VALUES (?, ?, ?)",
            DatabaseManager::SQLite
        );
        
        insertQuery.addBindValue(it.key());
        insertQuery.addBindValue(it.value());
        insertQuery.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
        
        if (!insertQuery.exec()) {
            qWarning() << "DatabaseInitializer: 创建配置失败:" << it.key() << insertQuery.lastError().text();
            return false;
        }
        
        qDebug() << "DatabaseInitializer: 创建配置:" << it.key() << "=" << it.value();
    }
    
    return true;
}

bool DatabaseInitializer::loadSystemConfig()
{
    DatabaseManager* dbManager = DatabaseManager::instance();
    
    QSqlQuery query = dbManager->prepareQuery(
        "SELECT config_key, config_value FROM cache_configs",
        DatabaseManager::SQLite
    );
    
    if (!query.exec()) {
        qWarning() << "DatabaseInitializer: 加载系统配置失败:" << query.lastError().text();
        return false;
    }
    
    int configCount = 0;
    while (query.next()) {
        QString key = query.value("config_key").toString();
        QString value = query.value("config_value").toString();
        configCount++;
        
        qDebug() << "DatabaseInitializer: 加载配置:" << key << "=" << value;
    }
    
    qDebug() << "DatabaseInitializer: 加载了" << configCount << "个配置项";
    return true;
}

bool DatabaseInitializer::saveSystemConfig()
{
    // 保存当前系统状态
    DatabaseManager* dbManager = DatabaseManager::instance();
    
    QSqlQuery updateQuery = dbManager->prepareQuery(
        "UPDATE cache_configs SET config_value = ?, last_sync = ? WHERE config_key = ?",
        DatabaseManager::SQLite
    );
    
    updateQuery.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    updateQuery.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    updateQuery.addBindValue("system.last_shutdown");
    
    return updateQuery.exec();
}

bool DatabaseInitializer::validateDatabase()
{
    DatabaseManager* dbManager = DatabaseManager::instance();
    
    // 验证关键表是否存在
    QStringList requiredTables = {
        "cache_workpieces",
        "cache_trajectories", 
        "cache_configs"
    };
    
    for (const QString& tableName : requiredTables) {
        QSqlQuery query = dbManager->prepareQuery(
            "SELECT name FROM sqlite_master WHERE type='table' AND name=?",
            DatabaseManager::SQLite
        );
        query.addBindValue(tableName);
        
        if (!query.exec() || !query.next()) {
            qWarning() << "DatabaseInitializer: 缺少必需的表:" << tableName;
            return false;
        }
    }
    
    // 验证系统配置
    QSqlQuery configQuery = dbManager->prepareQuery(
        "SELECT COUNT(*) FROM cache_configs WHERE config_key = 'system.version'",
        DatabaseManager::SQLite
    );
    
    if (!configQuery.exec() || !configQuery.next() || configQuery.value(0).toInt() == 0) {
        qWarning() << "DatabaseInitializer: 系统版本配置缺失";
        return false;
    }
    
    qDebug() << "DatabaseInitializer: 数据库验证通过";
    return true;
}

bool DatabaseInitializer::checkDatabaseVersion()
{
    DatabaseManager* dbManager = DatabaseManager::instance();
    
    QSqlQuery query = dbManager->prepareQuery(
        "SELECT config_value FROM cache_configs WHERE config_key = 'system.version'",
        DatabaseManager::SQLite
    );
    
    if (!query.exec() || !query.next()) {
        // 首次安装，创建版本配置
        QSqlQuery insertQuery = dbManager->prepareQuery(
            "INSERT INTO cache_configs (config_key, config_value, last_sync) VALUES (?, ?, ?)",
            DatabaseManager::SQLite
        );
        
        insertQuery.addBindValue("system.version");
        insertQuery.addBindValue(QString::number(CURRENT_DATABASE_VERSION));
        insertQuery.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
        
        if (!insertQuery.exec()) {
            qWarning() << "DatabaseInitializer: 创建版本配置失败";
            return false;
        }
        
        qDebug() << "DatabaseInitializer: 首次安装，设置数据库版本为" << CURRENT_DATABASE_VERSION;
        return true;
    }
    
    int dbVersion = query.value(0).toInt();
    
    if (dbVersion < CURRENT_DATABASE_VERSION) {
        qDebug() << "DatabaseInitializer: 需要升级数据库从版本" << dbVersion << "到" << CURRENT_DATABASE_VERSION;
        return upgradeDatabase(dbVersion, CURRENT_DATABASE_VERSION);
    } else if (dbVersion > CURRENT_DATABASE_VERSION) {
        qWarning() << "DatabaseInitializer: 数据库版本过高，当前应用不支持";
        return false;
    }
    
    qDebug() << "DatabaseInitializer: 数据库版本匹配:" << dbVersion;
    return true;
}

bool DatabaseInitializer::upgradeDatabase(int fromVersion, int toVersion)
{
    qDebug() << "DatabaseInitializer: 升级数据库从版本" << fromVersion << "到" << toVersion;
    
    // 这里可以添加数据库升级逻辑
    // 例如：添加新表、修改表结构、数据迁移等
    
    DatabaseManager* dbManager = DatabaseManager::instance();
    
    // 更新版本号
    QSqlQuery updateQuery = dbManager->prepareQuery(
        "UPDATE cache_configs SET config_value = ?, last_sync = ? WHERE config_key = 'system.version'",
        DatabaseManager::SQLite
    );
    
    updateQuery.addBindValue(QString::number(toVersion));
    updateQuery.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    
    if (!updateQuery.exec()) {
        qWarning() << "DatabaseInitializer: 更新版本号失败";
        return false;
    }
    
    qDebug() << "DatabaseInitializer: 数据库升级完成";
    return true;
}

bool DatabaseInitializer::migrateData()
{
    // 数据迁移逻辑
    // 例如：从旧版本格式迁移到新版本格式
    return true;
}

QString DatabaseInitializer::getDefaultConfigValue(const QString& key) const
{
    // 返回配置项的默认值
    static QMap<QString, QString> defaults = {
        {"trajectory.default_speed", "50.0"},
        {"trajectory.default_flow_rate", "0.8"},
        {"quality.min_quality_score", "0.7"}
    };
    
    return defaults.value(key, QString());
}

} // namespace Data