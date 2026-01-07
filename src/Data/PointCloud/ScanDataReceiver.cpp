#include "ScanDataReceiver.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QStandardPaths>
#include <QDateTime>
#include <QUuid>

namespace Data {

// ScanBatchInfo 实现
QJsonObject ScanBatchInfo::toJson() const
{
    QJsonObject json;
    json["batchId"] = batchId;
    json["batchName"] = batchName;
    json["scannerModel"] = scannerModel;
    json["timestamp"] = timestamp;
    json["operator"] = operator_;
    json["totalFiles"] = totalFiles;
    json["processedFiles"] = processedFiles;
    json["totalSize"] = totalSize;
    json["status"] = status;
    
    QJsonArray files;
    for (const QString& file : fileList) {
        files.append(file);
    }
    json["fileList"] = files;
    
    return json;
}

void ScanBatchInfo::fromJson(const QJsonObject& json)
{
    batchId = json["batchId"].toString();
    batchName = json["batchName"].toString();
    scannerModel = json["scannerModel"].toString();
    timestamp = json["timestamp"].toString();
    operator_ = json["operator"].toString();
    totalFiles = json["totalFiles"].toInt();
    processedFiles = json["processedFiles"].toInt();
    totalSize = json["totalSize"].toDouble();
    status = json["status"].toString();
    
    fileList.clear();
    QJsonArray files = json["fileList"].toArray();
    for (const QJsonValue& file : files) {
        fileList.append(file.toString());
    }
}

// SiKanScannerConfig 实现
QJsonObject SiKanScannerConfig::toJson() const
{
    QJsonObject json;
    json["ipAddress"] = ipAddress;
    json["port"] = port;
    json["protocol"] = protocol;
    json["apiVersion"] = apiVersion;
    json["authToken"] = authToken;
    json["dataFormat"] = dataFormat;
    json["outputPath"] = outputPath;
    json["autoReceive"] = autoReceive;
    json["timeoutSeconds"] = timeoutSeconds;
    return json;
}

void SiKanScannerConfig::fromJson(const QJsonObject& json)
{
    ipAddress = json["ipAddress"].toString();
    port = json["port"].toInt();
    protocol = json["protocol"].toString();
    apiVersion = json["apiVersion"].toString();
    authToken = json["authToken"].toString();
    dataFormat = json["dataFormat"].toString();
    outputPath = json["outputPath"].toString();
    autoReceive = json["autoReceive"].toBool();
    timeoutSeconds = json["timeoutSeconds"].toInt();
}

bool SiKanScannerConfig::isValid() const
{
    return !ipAddress.isEmpty() && 
           port > 0 && port < 65536 &&
           !protocol.isEmpty() &&
           timeoutSeconds > 0;
}

// ScanDataReceiver 实现
ScanDataReceiver::ScanDataReceiver(QObject *parent)
    : QObject(parent)
    , m_receiveMode(FileWatcher)
    , m_status(Idle)
    , m_siKanConnected(false)
    , m_pollingInterval(5000) // 5秒
{
    // 初始化组件
    m_parser = std::make_unique<PointCloudParser>(this);
    m_pollingTimer = std::make_unique<QTimer>(this);
    
    // 连接信号
    connect(m_parser.get(), &PointCloudParser::parseCompleted,
            this, &ScanDataReceiver::onFileProcessed);
    
    connect(m_pollingTimer.get(), &QTimer::timeout,
            this, &ScanDataReceiver::onPollingTimer);
    
    // 设置支持的格式
    m_supportedFormats << "ply" << "stl" << "obj" << "pcd";
    
    // 设置批次存储路径
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_batchStoragePath = appDataPath + "/scan_batches";
    QDir().mkpath(m_batchStoragePath);
    
    qDebug() << "ScanDataReceiver initialized, batch storage:" << m_batchStoragePath;
}

void ScanDataReceiver::setSiKanConfig(const SiKanScannerConfig& config)
{
    m_siKanConfig = config;
    
    if (m_siKanConnected) {
        disconnectFromSiKanScanner();
    }
    
    qDebug() << "SiKan配置已更新:" << config.ipAddress << ":" << config.port;
}

void ScanDataReceiver::setWatchDirectory(const QString& directory)
{
    if (m_watchDirectory == directory) {
        return;
    }
    
    m_watchDirectory = directory;
    
    if (m_receiveMode == FileWatcher && isReceiving()) {
        // 重新设置文件监控
        cleanupFileWatcher();
        setupFileWatcher();
    }
    
    qDebug() << "监控目录已设置:" << directory;
}

void ScanDataReceiver::setReceiveMode(ReceiveMode mode)
{
    if (m_receiveMode == mode) {
        return;
    }
    
    bool wasReceiving = isReceiving();
    if (wasReceiving) {
        stopReceiving();
    }
    
    m_receiveMode = mode;
    
    if (wasReceiving) {
        startReceiving();
    }
    
    qDebug() << "接收模式已设置:" << mode;
}

bool ScanDataReceiver::startReceiving()
{
    if (isReceiving()) {
        qWarning() << "已经在接收状态";
        return true;
    }
    
    switch (m_receiveMode) {
    case FileWatcher:
        if (m_watchDirectory.isEmpty()) {
            setError("监控目录未设置");
            return false;
        }
        if (!QDir(m_watchDirectory).exists()) {
            setError("监控目录不存在: " + m_watchDirectory);
            return false;
        }
        setupFileWatcher();
        break;
        
    case ApiPolling:
        if (!m_siKanConfig.isValid()) {
            setError("SiKan配置无效");
            return false;
        }
        if (!connectToSiKanScanner()) {
            return false;
        }
        m_pollingTimer->start(m_pollingInterval);
        break;
        
    case ManualImport:
        // 手动模式不需要启动监控
        break;
    }
    
    setStatus(Monitoring);
    qDebug() << "开始接收扫描数据，模式:" << m_receiveMode;
    return true;
}

void ScanDataReceiver::stopReceiving()
{
    if (!isReceiving()) {
        return;
    }
    
    cleanupFileWatcher();
    m_pollingTimer->stop();
    
    setStatus(Idle);
    qDebug() << "停止接收扫描数据";
}

bool ScanDataReceiver::importScanFiles(const QStringList& filePaths, const QString& batchName)
{
    if (filePaths.isEmpty()) {
        setError("文件列表为空");
        return false;
    }
    
    // 验证文件
    QStringList validFiles;
    for (const QString& filePath : filePaths) {
        if (validateScanFile(filePath)) {
            validFiles.append(filePath);
        } else {
            qWarning() << "跳过无效文件:" << filePath;
        }
    }
    
    if (validFiles.isEmpty()) {
        setError("没有有效的扫描文件");
        return false;
    }
    
    // 创建批次
    QString batchId = createBatchFromFiles(validFiles, batchName);
    if (batchId.isEmpty()) {
        return false;
    }
    
    // 处理批次
    return processBatch(batchId);
}

bool ScanDataReceiver::importScanDirectory(const QString& directory, const QString& batchName)
{
    QDir dir(directory);
    if (!dir.exists()) {
        setError("目录不存在: " + directory);
        return false;
    }
    
    // 查找支持的文件
    QStringList nameFilters;
    for (const QString& format : m_supportedFormats) {
        nameFilters << "*." + format;
    }
    
    QStringList files = dir.entryList(nameFilters, QDir::Files);
    if (files.isEmpty()) {
        setError("目录中没有找到支持的扫描文件");
        return false;
    }
    
    // 转换为完整路径
    QStringList filePaths;
    for (const QString& file : files) {
        filePaths.append(dir.absoluteFilePath(file));
    }
    
    return importScanFiles(filePaths, batchName);
}

QString ScanDataReceiver::createBatchFromFiles(const QStringList& filePaths, const QString& batchName)
{
    ScanBatchInfo batchInfo;
    batchInfo.batchId = generateBatchId();
    batchInfo.batchName = batchName.isEmpty() ? 
        QString("Batch_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")) : 
        batchName;
    batchInfo.fileList = filePaths;
    batchInfo.totalFiles = filePaths.size();
    batchInfo.processedFiles = 0;
    batchInfo.timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    batchInfo.operator_ = qgetenv("USERNAME"); // Windows用户名
    batchInfo.status = "pending";
    
    // 计算总大小
    double totalSize = 0.0;
    for (const QString& filePath : filePaths) {
        QFileInfo fileInfo(filePath);
        totalSize += fileInfo.size() / (1024.0 * 1024.0); // MB
    }
    batchInfo.totalSize = totalSize;
    
    // 保存批次信息
    m_batches[batchInfo.batchId] = batchInfo;
    saveBatchInfo(batchInfo);
    
    emit batchCreated(batchInfo.batchId, batchInfo.batchName);
    qDebug() << "创建批次:" << batchInfo.batchId << "文件数:" << batchInfo.totalFiles;
    
    return batchInfo.batchId;
}

bool ScanDataReceiver::processBatch(const QString& batchId)
{
    if (!m_batches.contains(batchId)) {
        setError("批次不存在: " + batchId);
        return false;
    }
    
    ScanBatchInfo& batchInfo = m_batches[batchId];
    batchInfo.status = "processing";
    batchInfo.processedFiles = 0;
    
    setStatus(Processing);
    
    // 处理每个文件
    for (const QString& filePath : batchInfo.fileList) {
        PointCloudData data;
        PointCloudParser::ParseResult result = m_parser->parseFile(filePath, data);
        
        bool success = (result == PointCloudParser::Success);
        onFileProcessed(filePath, success);
        
        batchInfo.processedFiles++;
        
        // 发送进度信号
        int percentage = (int)(batchInfo.progress());
        emit batchProgress(batchId, percentage);
        
        // 更新统计
        updateStatistics(filePath, success);
    }
    
    // 完成批次
    batchInfo.status = batchInfo.processedFiles == batchInfo.totalFiles ? "completed" : "failed";
    saveBatchInfo(batchInfo);
    
    bool success = (batchInfo.status == "completed");
    emit batchCompleted(batchId, success);
    
    if (success) {
        m_statistics.completedBatches++;
    } else {
        m_statistics.failedBatches++;
    }
    
    setStatus(Idle);
    qDebug() << "批次处理完成:" << batchId << "成功:" << success;
    
    return success;
}

bool ScanDataReceiver::validateScanFile(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    
    // 检查文件是否存在
    if (!fileInfo.exists()) {
        m_validationErrors << "文件不存在: " + filePath;
        return false;
    }
    
    // 检查文件格式
    QString suffix = fileInfo.suffix().toLower();
    if (!m_supportedFormats.contains(suffix)) {
        m_validationErrors << "不支持的文件格式: " + suffix;
        return false;
    }
    
    // 检查文件大小
    if (fileInfo.size() == 0) {
        m_validationErrors << "文件为空: " + filePath;
        return false;
    }
    
    // 检查文件大小限制（500MB）
    double fileSizeMB = fileInfo.size() / (1024.0 * 1024.0);
    if (fileSizeMB > 500.0) {
        m_validationErrors << QString("文件过大: %1MB").arg(fileSizeMB, 0, 'f', 1);
        return false;
    }
    
    return true;
}

void ScanDataReceiver::setupFileWatcher()
{
    if (m_watchDirectory.isEmpty()) {
        return;
    }
    
    m_fileWatcher = std::make_unique<QFileSystemWatcher>(this);
    
    // 监控目录
    m_fileWatcher->addPath(m_watchDirectory);
    
    // 连接信号
    connect(m_fileWatcher.get(), &QFileSystemWatcher::directoryChanged,
            this, &ScanDataReceiver::onDirectoryChanged);
    connect(m_fileWatcher.get(), &QFileSystemWatcher::fileChanged,
            this, &ScanDataReceiver::onFileChanged);
    
    qDebug() << "文件监控已设置:" << m_watchDirectory;
}

void ScanDataReceiver::cleanupFileWatcher()
{
    if (m_fileWatcher) {
        m_fileWatcher.reset();
    }
}

void ScanDataReceiver::onDirectoryChanged(const QString& path)
{
    qDebug() << "目录变化:" << path;
    
    // 扫描新文件
    QDir dir(path);
    QStringList nameFilters;
    for (const QString& format : m_supportedFormats) {
        nameFilters << "*." + format;
    }
    
    QStringList files = dir.entryList(nameFilters, QDir::Files);
    for (const QString& file : files) {
        QString filePath = dir.absoluteFilePath(file);
        emit newFileDetected(filePath);
        processNewFile(filePath);
    }
}

void ScanDataReceiver::onFileChanged(const QString& path)
{
    qDebug() << "文件变化:" << path;
    processNewFile(path);
}

bool ScanDataReceiver::processNewFile(const QString& filePath)
{
    if (!validateScanFile(filePath)) {
        qWarning() << "文件验证失败:" << filePath;
        return false;
    }
    
    // 创建单文件批次
    QStringList files;
    files << filePath;
    
    QString batchId = createBatchFromFiles(files, QString());
    return processBatch(batchId);
}

void ScanDataReceiver::onFileProcessed(const QString& filePath, bool success)
{
    emit fileProcessed(filePath, success);
    
    if (success) {
        m_statistics.successfulFiles++;
        qDebug() << "文件处理成功:" << filePath;
    } else {
        m_statistics.failedFiles++;
        qWarning() << "文件处理失败:" << filePath;
    }
}

QString ScanDataReceiver::generateBatchId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void ScanDataReceiver::setStatus(ReceiveStatus status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged(status);
    }
}

void ScanDataReceiver::setError(const QString& error)
{
    m_lastError = error;
    setStatus(Error);
    emit receiveError(error);
    qWarning() << "ScanDataReceiver Error:" << error;
}

void ScanDataReceiver::updateStatistics(const QString& filePath, bool success)
{
    m_statistics.totalFiles++;
    
    QFileInfo fileInfo(filePath);
    double fileSizeMB = fileInfo.size() / (1024.0 * 1024.0);
    m_statistics.totalDataSize += fileSizeMB;
    m_statistics.averageFileSize = m_statistics.totalDataSize / m_statistics.totalFiles;
}

void ScanDataReceiver::saveBatchInfo(const ScanBatchInfo& batchInfo)
{
    QString filePath = m_batchStoragePath + "/" + batchInfo.batchId + ".json";
    
    QJsonDocument doc(batchInfo.toJson());
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
    }
}

ScanBatchInfo ScanDataReceiver::loadBatchInfo(const QString& batchId) const
{
    QString filePath = m_batchStoragePath + "/" + batchId + ".json";
    
    ScanBatchInfo batchInfo;
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        batchInfo.fromJson(doc.object());
    }
    
    return batchInfo;
}

QStringList ScanDataReceiver::getBatchList() const
{
    return m_batches.keys();
}

ScanBatchInfo ScanDataReceiver::getBatchInfo(const QString& batchId) const
{
    if (m_batches.contains(batchId)) {
        return m_batches[batchId];
    }
    return loadBatchInfo(batchId);
}

void ScanDataReceiver::resetStatistics()
{
    m_statistics = ReceiveStatistics();
}

// 思看扫描系统接口的占位实现
bool ScanDataReceiver::connectToSiKanScanner()
{
    // TODO: 实现与思看扫描系统的实际连接
    qDebug() << "连接到思看扫描系统:" << m_siKanConfig.ipAddress;
    m_siKanConnected = true;
    emit siKanConnectionChanged(true);
    return true;
}

void ScanDataReceiver::disconnectFromSiKanScanner()
{
    m_siKanConnected = false;
    emit siKanConnectionChanged(false);
    qDebug() << "断开思看扫描系统连接";
}

bool ScanDataReceiver::requestScanData(const QString& projectId)
{
    // TODO: 实现扫描数据请求
    Q_UNUSED(projectId)
    return false;
}

bool ScanDataReceiver::getScanStatus(const QString& scanId, QString& status)
{
    // TODO: 实现扫描状态查询
    Q_UNUSED(scanId)
    Q_UNUSED(status)
    return false;
}

QStringList ScanDataReceiver::getAvailableScans()
{
    // TODO: 实现可用扫描列表获取
    return QStringList();
}

void ScanDataReceiver::onPollingTimer()
{
    // TODO: 实现API轮询逻辑
    if (m_siKanConnected) {
        // 轮询新的扫描数据
    }
}

bool ScanDataReceiver::deleteBatch(const QString& batchId)
{
    if (m_batches.contains(batchId)) {
        m_batches.remove(batchId);
        deleteBatchInfo(batchId);
        return true;
    }
    return false;
}

void ScanDataReceiver::deleteBatchInfo(const QString& batchId)
{
    QString filePath = m_batchStoragePath + "/" + batchId + ".json";
    QFile::remove(filePath);
}

bool ScanDataReceiver::validateBatch(const QString& batchId)
{
    if (!m_batches.contains(batchId)) {
        return false;
    }
    
    const ScanBatchInfo& batchInfo = m_batches[batchId];
    for (const QString& filePath : batchInfo.fileList) {
        if (!validateScanFile(filePath)) {
            return false;
        }
    }
    
    return true;
}

} // namespace Data