#pragma once

#include <QObject>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QDateTime>
#include "../Models/WorkpieceData.h"

namespace Data {

/**
 * @brief 工件批次信息
 */
struct BatchInfo
{
    QString batchId;           // 批次ID
    QString batchName;         // 批次名称
    QDateTime createTime;      // 创建时间
    QString description;       // 批次描述
    QList<QString> workpieceIds; // 工件ID列表
    QString status;            // 批次状态：pending, processing, completed, failed
    
    // 统计信息
    int totalWorkpieces = 0;   // 总工件数
    int completedWorkpieces = 0; // 已完成工件数
    double totalVolume = 0.0;  // 总体积
    double totalSurfaceArea = 0.0; // 总表面积
    double estimatedTime = 0.0; // 预估时间（分钟）
    
    bool isValid() const {
        return !batchId.isEmpty() && !batchName.isEmpty();
    }
};

/**
 * @brief 工件类别信息
 */
struct WorkpieceCategory
{
    QString categoryId;        // 类别ID
    QString categoryName;      // 类别名称
    QString description;       // 类别描述
    QJsonObject defaultParams; // 默认喷涂参数
    
    // 几何特征
    double typicalVolume = 0.0;     // 典型体积
    double typicalSurfaceArea = 0.0; // 典型表面积
    QString geometryType;           // 几何类型：box, cylinder, complex等
    
    bool isValid() const {
        return !categoryId.isEmpty() && !categoryName.isEmpty();
    }
};

/**
 * @brief 批次数据管理器
 * 负责多工件批次的数据接收、管理和处理
 */
class BatchManager : public QObject
{
    Q_OBJECT

public:
    explicit BatchManager(QObject* parent = nullptr);
    ~BatchManager();

    // 批次管理
    QString createBatch(const QString& batchName, const QString& description = "");
    bool deleteBatch(const QString& batchId);
    bool updateBatchInfo(const QString& batchId, const BatchInfo& info);
    BatchInfo getBatchInfo(const QString& batchId) const;
    QList<BatchInfo> getAllBatches() const;
    
    // 工件管理
    bool addWorkpieceToBatch(const QString& batchId, const QString& workpieceId);
    bool removeWorkpieceFromBatch(const QString& batchId, const QString& workpieceId);
    QList<QString> getWorkpiecesInBatch(const QString& batchId) const;
    
    // 工件类别管理
    QString createCategory(const QString& categoryName, const QString& description = "");
    bool deleteCategory(const QString& categoryId);
    bool updateCategory(const QString& categoryId, const WorkpieceCategory& category);
    WorkpieceCategory getCategory(const QString& categoryId) const;
    QList<WorkpieceCategory> getAllCategories() const;
    
    // 批次分析
    bool analyzeBatchLayout(const QString& batchId);
    double calculateSpaceUtilization(const QString& batchId) const;
    double estimateBatchProcessingTime(const QString& batchId) const;
    QJsonObject generateBatchReport(const QString& batchId) const;
    
    // 数据导入导出
    bool importBatchFromFile(const QString& filePath);
    bool exportBatchToFile(const QString& batchId, const QString& filePath) const;
    
    // 批次验证
    bool validateBatchData(const QString& batchId, QStringList& errors) const;
    bool checkBatchConstraints(const QString& batchId, QStringList& warnings) const;

signals:
    /**
     * @brief 批次创建信号
     */
    void batchCreated(const QString& batchId);
    
    /**
     * @brief 批次更新信号
     */
    void batchUpdated(const QString& batchId);
    
    /**
     * @brief 批次删除信号
     */
    void batchDeleted(const QString& batchId);
    
    /**
     * @brief 工件添加到批次信号
     */
    void workpieceAddedToBatch(const QString& batchId, const QString& workpieceId);
    
    /**
     * @brief 工件从批次移除信号
     */
    void workpieceRemovedFromBatch(const QString& batchId, const QString& workpieceId);
    
    /**
     * @brief 批次分析完成信号
     */
    void batchAnalysisCompleted(const QString& batchId, const QJsonObject& results);

private:
    /**
     * @brief 生成唯一ID
     */
    QString generateUniqueId() const;
    
    /**
     * @brief 计算工件几何特征
     */
    void calculateWorkpieceFeatures(const QString& workpieceId);
    
    /**
     * @brief 更新批次统计信息
     */
    void updateBatchStatistics(const QString& batchId);
    
    /**
     * @brief 保存批次数据到数据库
     */
    bool saveBatchToDatabase(const BatchInfo& batch);
    
    /**
     * @brief 从数据库加载批次数据
     */
    bool loadBatchFromDatabase(const QString& batchId, BatchInfo& batch);

private:
    QList<BatchInfo> m_batches;           // 批次列表
    QList<WorkpieceCategory> m_categories; // 工件类别列表
    QString m_currentBatchId;             // 当前活动批次ID
};

} // namespace Data