#ifndef SCANDATARECEIVER_H
#define SCANDATARECEIVER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QJsonObject>
#include <QJsonArray>
#include <memory>

#include "PointCloudParser.h"

namespace Data {

/**
 * @brief 扫描数据批次信息
 */
struct ScanBatchInfo
{
    QString batchId;                    // 批次ID
    QString batchName;                  // 批次名称
    QStringList fileList;               // 文件列表
    QString scannerModel;               // 扫描仪型号
    QString timestamp;                  // 扫描时间
    QString operator_;                  // 操作员
    int totalFiles;                     // 总文件数
    int processedFiles;                 // 已处理文件数
    double totalSize;                   // 总大小（MB）
    QString status;                     // 状态：pending, processing, completed, failed
    
    ScanBatchInfo() : totalFiles(0), processedFiles(0), totalSize(0.0), status("pending") {}
    
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);
    bool isComplete() const { return processedFiles >= totalFiles; }
    double progress() const { return totalFiles > 0 ? (double)processedFiles / totalFiles * 100.0 : 0.0; }
};

/**
 * @brief 思看扫描系统接口配置
 */
struct SiKanScannerConfig
{
    QString ipAddress;                  // 扫描仪IP地址
    int port;                          // 通信端口
    QString protocol;                  // 通信协议（TCP/UDP/HTTP）
    QString apiVersion;                // API版本
    QString authToken;                 // 认证令牌
    QString dataFormat;                // 数据格式偏好
    QString outputPath;                // 输出路径
    bool autoReceive;                  // 自动接收
    int timeoutSeconds;                // 超时时间
    
    SiKanScannerConfig() : port(8080), protocol("HTTP"), apiVersion("v1.0"), 
                          dataFormat("PLY"), autoReceive(true), timeoutSeconds(30) {}
    
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);
    bool isValid() const;
};

/**
 * @brief 扫描数据接收器
 * 
 * 负责接收和管理来自思看扫描系统的点云数据：
 * - 监控指定目录的新文件
 * - 与思看扫描系统的API通信
 * - 批量处理扫描数据
 * - 数据验证和完整性检查
 */
class ScanDataReceiver : public QObject
{
    Q_OBJECT

public:
    enum ReceiveMode {
        FileWatcher = 0,    // 文件监控模式
        ApiPolling,         // API轮询模式
        ManualImport        // 手动导入模式
    };
    Q_ENUM(ReceiveMode)

    enum ReceiveStatus {
        Idle = 0,
        Monitoring,
        Receiving,
        Processing,
        Error
    };
    Q_ENUM(ReceiveStatus)

    explicit ScanDataReceiver(QObject *parent = nullptr);
    ~ScanDataReceiver() = default;

    // 配置管理
    void setSiKanConfig(const SiKanScannerConfig& config);
    const SiKanScannerConfig& getSiKanConfig() const { return m_siKanConfig; }
    
    void setWatchDirectory(const QString& directory);
    QString getWatchDirectory() const { return m_watchDirectory; }
    
    void setReceiveMode(ReceiveMode mode);
    ReceiveMode getReceiveMode() const { return m_receiveMode; }

    // 接收控制
    bool startReceiving();
    void stopReceiving();
    bool isReceiving() const { return m_status == Monitoring || m_status == Receiving; }
    ReceiveStatus getStatus() const { return m_status; }

    // 手动导入
    bool importScanFiles(const QStringList& filePaths, const QString& batchName = QString());
    bool importScanDirectory(const QString& directory, const QString& batchName = QString());

    // 批次管理
    QStringList getBatchList() const;
    ScanBatchInfo getBatchInfo(const QString& batchId) const;
    bool processBatch(const QString& batchId);
    bool deleteBatch(const QString& batchId);

    // 思看扫描系统接口
    bool connectToSiKanScanner();
    void disconnectFromSiKanScanner();
    bool isConnectedToSiKan() const { return m_siKanConnected; }
    
    bool requestScanData(const QString& projectId);
    bool getScanStatus(const QString& scanId, QString& status);
    QStringList getAvailableScans();

    // 数据验证
    bool validateScanFile(const QString& filePath);
    bool validateBatch(const QString& batchId);
    QStringList getValidationErrors() const { return m_validationErrors; }

    // 统计信息
    struct ReceiveStatistics {
        int totalBatches;
        int completedBatches;
        int failedBatches;
        int totalFiles;
        int successfulFiles;
        int failedFiles;
        double totalDataSize;
        double averageFileSize;
        
        ReceiveStatistics() : totalBatches(0), completedBatches(0), failedBatches(0),
                            totalFiles(0), successfulFiles(0), failedFiles(0),
                            totalDataSize(0.0), averageFileSize(0.0) {}
    };
    
    const ReceiveStatistics& getStatistics() const { return m_statistics; }
    void resetStatistics();

    // 错误处理
    QString getLastError() const { return m_lastError; }

signals:
    void statusChanged(ReceiveStatus status);
    void newFileDetected(const QString& filePath);
    void batchCreated(const QString& batchId, const QString& batchName);
    void batchProgress(const QString& batchId, int percentage);
    void batchCompleted(const QString& batchId, bool success);
    void fileProcessed(const QString& filePath, bool success);
    void siKanConnectionChanged(bool connected);
    void receiveError(const QString& error);

private slots:
    void onDirectoryChanged(const QString& path);
    void onFileChanged(const QString& path);
    void onPollingTimer();
    void onFileProcessed(const QString& filePath, bool success);

private:
    // 内部方法
    void setStatus(ReceiveStatus status);
    void setError(const QString& error);
    
    QString generateBatchId() const;
    QString createBatchFromFiles(const QStringList& filePaths, const QString& batchName);
    
    bool processNewFile(const QString& filePath);
    bool addFileToBatch(const QString& filePath, const QString& batchId);
    
    void updateStatistics(const QString& filePath, bool success);
    
    // 思看API相关
    bool sendSiKanRequest(const QString& endpoint, const QJsonObject& data, QJsonObject& response);
    QString buildSiKanUrl(const QString& endpoint) const;
    
    // 文件系统监控
    void setupFileWatcher();
    void cleanupFileWatcher();
    
    // 数据存储
    void saveBatchInfo(const ScanBatchInfo& batchInfo);
    ScanBatchInfo loadBatchInfo(const QString& batchId) const;
    void deleteBatchInfo(const QString& batchId);

private:
    // 配置
    SiKanScannerConfig m_siKanConfig;
    QString m_watchDirectory;
    ReceiveMode m_receiveMode;
    
    // 状态
    ReceiveStatus m_status;
    bool m_siKanConnected;
    QString m_lastError;
    QStringList m_validationErrors;
    
    // 组件
    std::unique_ptr<QFileSystemWatcher> m_fileWatcher;
    std::unique_ptr<QTimer> m_pollingTimer;
    std::unique_ptr<PointCloudParser> m_parser;
    
    // 数据
    QMap<QString, ScanBatchInfo> m_batches;
    ReceiveStatistics m_statistics;
    
    // 配置参数
    QStringList m_supportedFormats;
    int m_pollingInterval;
    QString m_batchStoragePath;
};

} // namespace Data

Q_DECLARE_METATYPE(Data::ScanDataReceiver::ReceiveMode)
Q_DECLARE_METATYPE(Data::ScanDataReceiver::ReceiveStatus)

#endif // SCANDATARECEIVER_H