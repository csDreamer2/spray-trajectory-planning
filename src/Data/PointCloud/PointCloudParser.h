#ifndef POINTCLOUDPARSER_H
#define POINTCLOUDPARSER_H

#include <QObject>
#include <QString>
#include <QVector3D>
#include <QList>
#include <QJsonObject>
#include <memory>

// PCL includes
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/io/ply_io.h>

namespace Data {

/**
 * @brief 点云数据结构
 */
struct PointCloudData
{
    QList<QVector3D> points;        // 点坐标
    QList<QVector3D> normals;       // 法向量（可选）
    QList<QVector3D> colors;        // 颜色信息（可选）
    QString fileName;               // 文件名
    QString format;                 // 文件格式
    int pointCount;                 // 点数量
    QVector3D boundingBoxMin;       // 边界框最小值
    QVector3D boundingBoxMax;       // 边界框最大值
    double fileSize;                // 文件大小（MB）
    
    PointCloudData() : pointCount(0), fileSize(0.0) {}
    
    // 计算边界框
    void calculateBoundingBox();
    
    // 转换为JSON
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);
    
    // 验证数据完整性
    bool isValid() const;
    QStringList validationErrors() const;
};

/**
 * @brief 扫描数据位置信息
 */
struct ScanPositionInfo
{
    QVector3D position;             // 扫描仪位置
    QVector3D rotation;             // 扫描仪旋转角度
    QString timestamp;              // 扫描时间戳
    QString scannerModel;           // 扫描仪型号
    double accuracy;                // 扫描精度
    QString coordinateSystem;       // 坐标系统
    
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);
};

/**
 * @brief 点云文件解析器
 * 
 * 支持多种点云文件格式的解析和处理：
 * - PLY (Stanford Polygon Library)
 * - STL (Stereolithography)
 * - OBJ (Wavefront OBJ)
 * - PCD (Point Cloud Data)
 */
class PointCloudParser : public QObject
{
    Q_OBJECT

public:
    enum FileFormat {
        Unknown = 0,
        PLY,
        STL,
        OBJ,
        PCD
    };
    Q_ENUM(FileFormat)

    enum ParseResult {
        Success = 0,
        FileNotFound,
        UnsupportedFormat,
        CorruptedFile,
        InsufficientMemory,
        InvalidData,
        ParseError
    };
    Q_ENUM(ParseResult)

    explicit PointCloudParser(QObject *parent = nullptr);
    ~PointCloudParser() = default;

    // 文件格式检测
    static FileFormat detectFileFormat(const QString& filePath);
    static QString formatToString(FileFormat format);
    static FileFormat formatFromString(const QString& formatStr);

    // 点云文件解析
    ParseResult parseFile(const QString& filePath, PointCloudData& data);
    ParseResult parsePLY(const QString& filePath, PointCloudData& data);
    ParseResult parseSTL(const QString& filePath, PointCloudData& data);
    ParseResult parseOBJ(const QString& filePath, PointCloudData& data);
    ParseResult parsePCD(const QString& filePath, PointCloudData& data);

    // 位置信息解析
    bool parsePositionInfo(const QString& filePath, ScanPositionInfo& posInfo);
    bool parsePositionFromMetadata(const QString& filePath, ScanPositionInfo& posInfo);
    bool parsePositionFromSidecar(const QString& filePath, ScanPositionInfo& posInfo);

    // 数据验证和完整性检查
    bool validatePointCloud(const PointCloudData& data);
    QStringList getValidationErrors() const { return m_validationErrors; }
    
    // 数据预处理
    bool preprocessPointCloud(PointCloudData& data);
    bool removeOutliers(PointCloudData& data, double stddevMult = 1.0);
    bool downsample(PointCloudData& data, double leafSize = 0.01);
    bool estimateNormals(PointCloudData& data, int kNeighbors = 20);

    // 统计信息
    struct ParseStatistics {
        int totalFiles;
        int successfulFiles;
        int failedFiles;
        double totalProcessingTime;
        double averageFileSize;
        int totalPoints;
        
        ParseStatistics() : totalFiles(0), successfulFiles(0), failedFiles(0),
                          totalProcessingTime(0.0), averageFileSize(0.0), totalPoints(0) {}
    };
    
    const ParseStatistics& getStatistics() const { return m_statistics; }
    void resetStatistics();

    // 错误处理
    QString getLastError() const { return m_lastError; }
    ParseResult getLastResult() const { return m_lastResult; }
    
    // 取消控制
    void setCancelRequested(bool cancel) { m_cancelRequested = cancel; }
    bool isCancelRequested() const { return m_cancelRequested; }

signals:
    void parseProgress(int percentage);
    void parseCompleted(const QString& filePath, bool success);
    void parseError(const QString& filePath, const QString& error);

private:
    // PCL点云类型定义
    using PointT = pcl::PointXYZ;
    using PointCloudT = pcl::PointCloud<PointT>;
    using PointNormalT = pcl::PointXYZRGBNormal;
    using PointCloudNormalT = pcl::PointCloud<PointNormalT>;

    // 内部辅助方法
    bool convertPCLToQVector(const PointCloudT::Ptr& pclCloud, PointCloudData& data);
    bool convertPCLNormalToQVector(const PointCloudNormalT::Ptr& pclCloud, PointCloudData& data);
    
    void updateStatistics(const PointCloudData& data, double processingTime);
    ParseResult setError(ParseResult result, const QString& message);
    
    // 文件大小检查
    bool checkFileSize(const QString& filePath, double maxSizeMB = 500.0);
    
    // 内存使用估算
    double estimateMemoryUsage(int pointCount) const;

private:
    QString m_lastError;
    ParseResult m_lastResult;
    QStringList m_validationErrors;
    ParseStatistics m_statistics;
    
    // 配置参数
    double m_maxFileSizeMB;
    int m_maxPointCount;
    bool m_enablePreprocessing;
    bool m_enableNormalEstimation;
    
    // 取消控制
    bool m_cancelRequested;
};

} // namespace Data

Q_DECLARE_METATYPE(Data::PointCloudParser::FileFormat)
Q_DECLARE_METATYPE(Data::PointCloudParser::ParseResult)

#endif // POINTCLOUDPARSER_H