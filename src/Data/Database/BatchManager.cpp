#include "BatchManager.h"
#include <QUuid>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QStandardPaths>
#include "../Models/WorkpieceData.h"

namespace Data {

BatchManager::BatchManager(QObject* parent)
    : QObject(parent)
{
    qDebug() << "BatchManager: 初始化批次数据管理器";
    
    // 创建默认工件类别
    createCategory("通用工件", "通用类型工件，适用于标准喷涂参数");
    createCategory("大型工件", "大型复杂工件，需要特殊处理");
    createCategory("精密工件", "精密工件，要求高质量喷涂");
}

BatchManager::~BatchManager()
{
    qDebug() << "BatchManager: 销毁批次数据管理器";
}

QString BatchManager::createBatch(const QString& batchName, const QString& description)
{
    BatchInfo batch;
    batch.batchId = generateUniqueId();
    batch.batchName = batchName;
    batch.createTime = QDateTime::currentDateTime();
    batch.description = description;
    batch.status = "pending";
    
    m_batches.append(batch);
    
    qDebug() << "BatchManager: 创建批次" << batch.batchId << "名称:" << batchName;
    
    // 保存到数据库
    saveBatchToDatabase(batch);
    
    emit batchCreated(batch.batchId);
    return batch.batchId;
}

bool BatchManager::deleteBatch(const QString& batchId)
{
    for (int i = 0; i < m_batches.size(); ++i) {
        if (m_batches[i].batchId == batchId) {
            m_batches.removeAt(i);
            qDebug() << "BatchManager: 删除批次" << batchId;
            emit batchDeleted(batchId);
            return true;
        }
    }
    
    qWarning() << "BatchManager: 未找到批次" << batchId;
    return false;
}

bool BatchManager::updateBatchInfo(const QString& batchId, const BatchInfo& info)
{
    for (int i = 0; i < m_batches.size(); ++i) {
        if (m_batches[i].batchId == batchId) {
            m_batches[i] = info;
            m_batches[i].batchId = batchId; // 确保ID不变
            
            qDebug() << "BatchManager: 更新批次" << batchId;
            
            // 保存到数据库
            saveBatchToDatabase(m_batches[i]);
            
            emit batchUpdated(batchId);
            return true;
        }
    }
    
    qWarning() << "BatchManager: 未找到批次" << batchId;
    return false;
}

BatchInfo BatchManager::getBatchInfo(const QString& batchId) const
{
    for (const auto& batch : m_batches) {
        if (batch.batchId == batchId) {
            return batch;
        }
    }
    
    qWarning() << "BatchManager: 未找到批次" << batchId;
    return BatchInfo();
}

QList<BatchInfo> BatchManager::getAllBatches() const
{
    return m_batches;
}

bool BatchManager::addWorkpieceToBatch(const QString& batchId, const QString& workpieceId)
{
    for (auto& batch : m_batches) {
        if (batch.batchId == batchId) {
            if (!batch.workpieceIds.contains(workpieceId)) {
                batch.workpieceIds.append(workpieceId);
                updateBatchStatistics(batchId);
                
                qDebug() << "BatchManager: 添加工件" << workpieceId << "到批次" << batchId;
                
                emit workpieceAddedToBatch(batchId, workpieceId);
                return true;
            } else {
                qWarning() << "BatchManager: 工件" << workpieceId << "已存在于批次" << batchId;
                return false;
            }
        }
    }
    
    qWarning() << "BatchManager: 未找到批次" << batchId;
    return false;
}

bool BatchManager::removeWorkpieceFromBatch(const QString& batchId, const QString& workpieceId)
{
    for (auto& batch : m_batches) {
        if (batch.batchId == batchId) {
            if (batch.workpieceIds.removeOne(workpieceId)) {
                updateBatchStatistics(batchId);
                
                qDebug() << "BatchManager: 从批次" << batchId << "移除工件" << workpieceId;
                
                emit workpieceRemovedFromBatch(batchId, workpieceId);
                return true;
            } else {
                qWarning() << "BatchManager: 工件" << workpieceId << "不存在于批次" << batchId;
                return false;
            }
        }
    }
    
    qWarning() << "BatchManager: 未找到批次" << batchId;
    return false;
}

QList<QString> BatchManager::getWorkpiecesInBatch(const QString& batchId) const
{
    for (const auto& batch : m_batches) {
        if (batch.batchId == batchId) {
            return batch.workpieceIds;
        }
    }
    
    qWarning() << "BatchManager: 未找到批次" << batchId;
    return QList<QString>();
}

QString BatchManager::createCategory(const QString& categoryName, const QString& description)
{
    WorkpieceCategory category;
    category.categoryId = generateUniqueId();
    category.categoryName = categoryName;
    category.description = description;
    category.geometryType = "complex";
    
    // 设置默认喷涂参数
    QJsonObject defaultParams;
    defaultParams["pressure"] = 2.5;        // 压力 (bar)
    defaultParams["flowRate"] = 300.0;      // 流量 (ml/min)
    defaultParams["sprayHeight"] = 200.0;   // 喷涂高度 (mm)
    defaultParams["spraySpeed"] = 500.0;    // 喷涂速度 (mm/min)
    defaultParams["overlapRate"] = 0.3;     // 重叠率
    defaultParams["passCount"] = 2;         // 喷涂遍数
    category.defaultParams = defaultParams;
    
    m_categories.append(category);
    
    qDebug() << "BatchManager: 创建工件类别" << category.categoryId << "名称:" << categoryName;
    
    return category.categoryId;
}

bool BatchManager::deleteCategory(const QString& categoryId)
{
    for (int i = 0; i < m_categories.size(); ++i) {
        if (m_categories[i].categoryId == categoryId) {
            m_categories.removeAt(i);
            qDebug() << "BatchManager: 删除工件类别" << categoryId;
            return true;
        }
    }
    
    qWarning() << "BatchManager: 未找到工件类别" << categoryId;
    return false;
}

bool BatchManager::updateCategory(const QString& categoryId, const WorkpieceCategory& category)
{
    for (int i = 0; i < m_categories.size(); ++i) {
        if (m_categories[i].categoryId == categoryId) {
            m_categories[i] = category;
            m_categories[i].categoryId = categoryId; // 确保ID不变
            
            qDebug() << "BatchManager: 更新工件类别" << categoryId;
            return true;
        }
    }
    
    qWarning() << "BatchManager: 未找到工件类别" << categoryId;
    return false;
}

WorkpieceCategory BatchManager::getCategory(const QString& categoryId) const
{
    for (const auto& category : m_categories) {
        if (category.categoryId == categoryId) {
            return category;
        }
    }
    
    qWarning() << "BatchManager: 未找到工件类别" << categoryId;
    return WorkpieceCategory();
}

QList<WorkpieceCategory> BatchManager::getAllCategories() const
{
    return m_categories;
}

bool BatchManager::analyzeBatchLayout(const QString& batchId)
{
    BatchInfo batch = getBatchInfo(batchId);
    if (!batch.isValid()) {
        return false;
    }
    
    qDebug() << "BatchManager: 开始分析批次布局" << batchId;
    
    // 模拟布局分析过程
    QJsonObject results;
    results["batchId"] = batchId;
    results["totalWorkpieces"] = batch.workpieceIds.size();
    results["spaceUtilization"] = calculateSpaceUtilization(batchId);
    results["estimatedTime"] = estimateBatchProcessingTime(batchId);
    results["analysisTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // 布局建议
    QJsonArray suggestions;
    if (batch.workpieceIds.size() > 10) {
        suggestions.append("建议分批处理，单批次不超过10个工件");
    }
    if (calculateSpaceUtilization(batchId) < 0.6) {
        suggestions.append("空间利用率较低，建议优化工件布局");
    }
    results["suggestions"] = suggestions;
    
    emit batchAnalysisCompleted(batchId, results);
    
    qDebug() << "BatchManager: 批次布局分析完成" << batchId;
    return true;
}

double BatchManager::calculateSpaceUtilization(const QString& batchId) const
{
    BatchInfo batch = getBatchInfo(batchId);
    if (!batch.isValid() || batch.workpieceIds.isEmpty()) {
        return 0.0;
    }
    
    // 简化的空间利用率计算
    // 实际实现中需要考虑工件的实际几何形状和布局
    double totalVolume = batch.totalVolume;
    double availableSpace = 10000000.0; // 假设可用空间 10m³
    
    double utilization = totalVolume / availableSpace;
    return qMin(utilization, 1.0); // 最大100%
}

double BatchManager::estimateBatchProcessingTime(const QString& batchId) const
{
    BatchInfo batch = getBatchInfo(batchId);
    if (!batch.isValid() || batch.workpieceIds.isEmpty()) {
        return 0.0;
    }
    
    // 简化的时间估算
    // 实际实现中需要考虑工件复杂度、喷涂参数等
    double baseTimePerWorkpiece = 30.0; // 基础时间30分钟/工件
    double setupTime = 15.0; // 设置时间15分钟
    double cleanupTime = 10.0; // 清理时间10分钟
    
    double totalTime = setupTime + (batch.workpieceIds.size() * baseTimePerWorkpiece) + cleanupTime;
    
    return totalTime;
}

QJsonObject BatchManager::generateBatchReport(const QString& batchId) const
{
    BatchInfo batch = getBatchInfo(batchId);
    if (!batch.isValid()) {
        return QJsonObject();
    }
    
    QJsonObject report;
    report["batchId"] = batch.batchId;
    report["batchName"] = batch.batchName;
    report["createTime"] = batch.createTime.toString(Qt::ISODate);
    report["status"] = batch.status;
    report["description"] = batch.description;
    
    // 统计信息
    QJsonObject statistics;
    statistics["totalWorkpieces"] = batch.totalWorkpieces;
    statistics["completedWorkpieces"] = batch.completedWorkpieces;
    statistics["totalVolume"] = batch.totalVolume;
    statistics["totalSurfaceArea"] = batch.totalSurfaceArea;
    statistics["estimatedTime"] = batch.estimatedTime;
    statistics["spaceUtilization"] = calculateSpaceUtilization(batchId);
    report["statistics"] = statistics;
    
    // 工件列表
    QJsonArray workpieces;
    for (const QString& workpieceId : batch.workpieceIds) {
        workpieces.append(workpieceId);
    }
    report["workpieces"] = workpieces;
    
    report["generateTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    return report;
}

bool BatchManager::validateBatchData(const QString& batchId, QStringList& errors) const
{
    BatchInfo batch = getBatchInfo(batchId);
    if (!batch.isValid()) {
        errors.append("批次信息无效");
        return false;
    }
    
    bool isValid = true;
    
    // 检查批次名称
    if (batch.batchName.isEmpty()) {
        errors.append("批次名称不能为空");
        isValid = false;
    }
    
    // 检查工件数量
    if (batch.workpieceIds.isEmpty()) {
        errors.append("批次中没有工件");
        isValid = false;
    }
    
    // 检查工件数量限制
    if (batch.workpieceIds.size() > 20) {
        errors.append("批次工件数量超过限制（最大20个）");
        isValid = false;
    }
    
    return isValid;
}

bool BatchManager::checkBatchConstraints(const QString& batchId, QStringList& warnings) const
{
    BatchInfo batch = getBatchInfo(batchId);
    if (!batch.isValid()) {
        return false;
    }
    
    // 检查空间利用率
    double utilization = calculateSpaceUtilization(batchId);
    if (utilization < 0.5) {
        warnings.append("空间利用率较低，建议增加工件或优化布局");
    } else if (utilization > 0.9) {
        warnings.append("空间利用率过高，可能影响操作安全性");
    }
    
    // 检查处理时间
    double estimatedTime = estimateBatchProcessingTime(batchId);
    if (estimatedTime > 480) { // 8小时
        warnings.append("预估处理时间超过8小时，建议分批处理");
    }
    
    return warnings.isEmpty();
}

QString BatchManager::generateUniqueId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void BatchManager::updateBatchStatistics(const QString& batchId)
{
    for (auto& batch : m_batches) {
        if (batch.batchId == batchId) {
            batch.totalWorkpieces = batch.workpieceIds.size();
            
            // 这里应该从实际的工件数据中计算统计信息
            // 简化实现，使用估算值
            batch.totalVolume = batch.totalWorkpieces * 0.1; // 假设每个工件0.1m³
            batch.totalSurfaceArea = batch.totalWorkpieces * 2.0; // 假设每个工件2m²
            batch.estimatedTime = estimateBatchProcessingTime(batchId);
            
            break;
        }
    }
}

bool BatchManager::saveBatchToDatabase(const BatchInfo& batch)
{
    // 简化实现：保存到JSON文件
    // 实际实现中应该保存到MySQL数据库
    
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    
    QString filePath = dataDir + "/batch_" + batch.batchId + ".json";
    
    QJsonObject batchJson;
    batchJson["batchId"] = batch.batchId;
    batchJson["batchName"] = batch.batchName;
    batchJson["createTime"] = batch.createTime.toString(Qt::ISODate);
    batchJson["description"] = batch.description;
    batchJson["status"] = batch.status;
    batchJson["totalWorkpieces"] = batch.totalWorkpieces;
    batchJson["completedWorkpieces"] = batch.completedWorkpieces;
    batchJson["totalVolume"] = batch.totalVolume;
    batchJson["totalSurfaceArea"] = batch.totalSurfaceArea;
    batchJson["estimatedTime"] = batch.estimatedTime;
    
    QJsonArray workpiecesArray;
    for (const QString& workpieceId : batch.workpieceIds) {
        workpiecesArray.append(workpieceId);
    }
    batchJson["workpieces"] = workpiecesArray;
    
    QJsonDocument doc(batchJson);
    
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        return true;
    }
    
    qWarning() << "BatchManager: 无法保存批次数据到文件" << filePath;
    return false;
}

bool BatchManager::loadBatchFromDatabase(const QString& batchId, BatchInfo& batch)
{
    // 简化实现：从JSON文件加载
    // 实际实现中应该从MySQL数据库加载
    
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString filePath = dataDir + "/batch_" + batchId + ".json";
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "BatchManager: 无法打开批次数据文件" << filePath;
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "BatchManager: 批次数据文件格式错误" << filePath;
        return false;
    }
    
    QJsonObject batchJson = doc.object();
    
    batch.batchId = batchJson["batchId"].toString();
    batch.batchName = batchJson["batchName"].toString();
    batch.createTime = QDateTime::fromString(batchJson["createTime"].toString(), Qt::ISODate);
    batch.description = batchJson["description"].toString();
    batch.status = batchJson["status"].toString();
    batch.totalWorkpieces = batchJson["totalWorkpieces"].toInt();
    batch.completedWorkpieces = batchJson["completedWorkpieces"].toInt();
    batch.totalVolume = batchJson["totalVolume"].toDouble();
    batch.totalSurfaceArea = batchJson["totalSurfaceArea"].toDouble();
    batch.estimatedTime = batchJson["estimatedTime"].toDouble();
    
    QJsonArray workpiecesArray = batchJson["workpieces"].toArray();
    batch.workpieceIds.clear();
    for (const QJsonValue& value : workpiecesArray) {
        batch.workpieceIds.append(value.toString());
    }
    
    return true;
}

} // namespace Data