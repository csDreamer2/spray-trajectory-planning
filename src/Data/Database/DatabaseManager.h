#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMutex>
#include <QTimer>
#include <memory>

namespace Data {

class BaseModel;
class WorkpieceData;
class TrajectoryData;

/**
 * @brief 数据库管理器
 * 
 * 负责管理MySQL主数据库和SQLite本地缓存的连接和操作
 * 提供统一的数据访问接口和数据同步功能
 */
class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    enum DatabaseType {
        MySQL = 0,
        SQLite
    };

    enum ConnectionStatus {
        Disconnected = 0,
        Connected,
        Connecting,
        Error
    };

    static DatabaseManager* instance();
    ~DatabaseManager();

    // 数据库连接管理
    bool initializeDatabase();
    bool connectToMySQL(const QString& host, int port, const QString& database,
                       const QString& username, const QString& password);
    bool connectToSQLite(const QString& filePath);
    
    void disconnectAll();
    bool isConnected(DatabaseType type = MySQL) const;
    ConnectionStatus connectionStatus(DatabaseType type = MySQL) const;
    
    // 数据库操作
    bool executeQuery(const QString& sql, DatabaseType type = MySQL);
    QSqlQuery prepareQuery(const QString& sql, DatabaseType type = MySQL);
    bool beginTransaction(DatabaseType type = MySQL);
    bool commitTransaction(DatabaseType type = MySQL);
    bool rollbackTransaction(DatabaseType type = MySQL);
    
    // 数据模型操作
    bool saveModel(BaseModel* model, DatabaseType type = MySQL);
    bool deleteModel(BaseModel* model, DatabaseType type = MySQL);
    BaseModel* loadModel(int id, const QString& tableName, DatabaseType type = MySQL);
    QList<BaseModel*> loadModels(const QString& tableName, const QString& whereClause = QString(),
                                 DatabaseType type = MySQL);
    
    // 工件数据操作
    bool saveWorkpiece(WorkpieceData* workpiece, DatabaseType type = MySQL);
    WorkpieceData* loadWorkpiece(int id, DatabaseType type = MySQL);
    QList<WorkpieceData*> loadWorkpieces(const QString& category = QString(), DatabaseType type = MySQL);
    bool deleteWorkpiece(int id, DatabaseType type = MySQL);
    
    // 轨迹数据操作
    bool saveTrajectory(TrajectoryData* trajectory, DatabaseType type = MySQL);
    TrajectoryData* loadTrajectory(int id, DatabaseType type = MySQL);
    QList<TrajectoryData*> loadTrajectories(int workpieceId = 0, DatabaseType type = MySQL);
    bool deleteTrajectory(int id, DatabaseType type = MySQL);
    
    // 数据同步
    void startAutoSync();
    void stopAutoSync();
    bool syncToLocal();
    bool syncToRemote();
    bool syncModel(BaseModel* model, DatabaseType fromType, DatabaseType toType);
    
    // 数据库维护
    bool createTables(DatabaseType type = MySQL);
    bool upgradeTables(DatabaseType type = MySQL);
    bool optimizeDatabase(DatabaseType type = MySQL);
    bool backupDatabase(const QString& backupPath, DatabaseType type = MySQL);
    
    // 错误处理
    QString lastError() const { return m_lastError; }
    QSqlError lastSqlError(DatabaseType type = MySQL) const;

signals:
    void connectionStatusChanged(DatabaseType type, ConnectionStatus status);
    void syncProgress(int percentage);
    void syncCompleted(bool success);
    void errorOccurred(const QString& error);

private slots:
    void onAutoSyncTimer();
    void checkConnections();

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    
    // 数据库连接
    QSqlDatabase getDatabase(DatabaseType type);
    QString getDatabaseConnectionName(DatabaseType type) const;
    bool testConnection(DatabaseType type);
    
    // SQL生成
    QString generateInsertSQL(BaseModel* model, const QString& tableName);
    QString generateUpdateSQL(BaseModel* model, const QString& tableName);
    QString generateSelectSQL(const QString& tableName, const QString& whereClause = QString());
    
    // 数据转换
    QVariantMap modelToVariantMap(BaseModel* model);
    void variantMapToModel(const QVariantMap& data, BaseModel* model);
    
    // 表结构管理
    bool createMySQLTables();
    bool createSQLiteTables();
    QStringList getMySQLTableDefinitions();
    QStringList getSQLiteTableDefinitions();

private:
    static std::unique_ptr<DatabaseManager> s_instance;
    static QMutex s_mutex;
    
    // 连接状态
    ConnectionStatus m_mysqlStatus;
    ConnectionStatus m_sqliteStatus;
    QString m_lastError;
    
    // 自动同步
    QTimer* m_autoSyncTimer;
    bool m_autoSyncEnabled;
    int m_syncInterval; // 秒
    
    // 数据库配置
    QString m_mysqlHost;
    int m_mysqlPort;
    QString m_mysqlDatabase;
    QString m_mysqlUsername;
    QString m_mysqlPassword;
    QString m_sqlitePath;
    
    // 线程安全
    mutable QMutex m_mutex;
};

} // namespace Data

#endif // DATABASEMANAGER_H