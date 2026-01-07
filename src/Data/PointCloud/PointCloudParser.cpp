#include "PointCloudParser.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTextStream>
#include <QRegularExpression>
#include <QDateTime>
#include <QFile>
#include <cmath>

// PCL includes
#include <pcl/filters/statistical_outlier_removal.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/features/normal_3d.h>
#include <pcl/kdtree/kdtree_flann.h>

namespace Data {

// PointCloudData 实现
void PointCloudData::calculateBoundingBox()
{
    if (points.isEmpty()) {
        boundingBoxMin = boundingBoxMax = QVector3D(0, 0, 0);
        return;
    }
    
    boundingBoxMin = boundingBoxMax = points.first();
    
    for (const QVector3D& point : points) {
        boundingBoxMin.setX(qMin(boundingBoxMin.x(), point.x()));
        boundingBoxMin.setY(qMin(boundingBoxMin.y(), point.y()));
        boundingBoxMin.setZ(qMin(boundingBoxMin.z(), point.z()));
        
        boundingBoxMax.setX(qMax(boundingBoxMax.x(), point.x()));
        boundingBoxMax.setY(qMax(boundingBoxMax.y(), point.y()));
        boundingBoxMax.setZ(qMax(boundingBoxMax.z(), point.z()));
    }
}

QJsonObject PointCloudData::toJson() const
{
    QJsonObject json;
    json["fileName"] = fileName;
    json["format"] = format;
    json["pointCount"] = pointCount;
    json["fileSize"] = fileSize;
    
    QJsonArray minBox;
    minBox.append(boundingBoxMin.x());
    minBox.append(boundingBoxMin.y());
    minBox.append(boundingBoxMin.z());
    json["boundingBoxMin"] = minBox;
    
    QJsonArray maxBox;
    maxBox.append(boundingBoxMax.x());
    maxBox.append(boundingBoxMax.y());
    maxBox.append(boundingBoxMax.z());
    json["boundingBoxMax"] = maxBox;
    
    return json;
}

void PointCloudData::fromJson(const QJsonObject& json)
{
    fileName = json["fileName"].toString();
    format = json["format"].toString();
    pointCount = json["pointCount"].toInt();
    fileSize = json["fileSize"].toDouble();
    
    QJsonArray minBox = json["boundingBoxMin"].toArray();
    if (minBox.size() == 3) {
        boundingBoxMin = QVector3D(minBox[0].toDouble(), minBox[1].toDouble(), minBox[2].toDouble());
    }
    
    QJsonArray maxBox = json["boundingBoxMax"].toArray();
    if (maxBox.size() == 3) {
        boundingBoxMax = QVector3D(maxBox[0].toDouble(), maxBox[1].toDouble(), maxBox[2].toDouble());
    }
}

bool PointCloudData::isValid() const
{
    return !fileName.isEmpty() && 
           !format.isEmpty() && 
           pointCount > 0 && 
           !points.isEmpty() &&
           points.size() == pointCount;
}

QStringList PointCloudData::validationErrors() const
{
    QStringList errors;
    
    if (fileName.isEmpty()) {
        errors << "文件名为空";
    }
    
    if (format.isEmpty()) {
        errors << "文件格式未指定";
    }
    
    if (pointCount <= 0) {
        errors << "点数量无效";
    }
    
    if (points.isEmpty()) {
        errors << "点云数据为空";
    }
    
    if (points.size() != pointCount) {
        errors << QString("点数量不匹配：期望%1，实际%2").arg(pointCount).arg(points.size());
    }
    
    return errors;
}

// ScanPositionInfo 实现
QJsonObject ScanPositionInfo::toJson() const
{
    QJsonObject json;
    
    QJsonArray pos;
    pos.append(position.x());
    pos.append(position.y());
    pos.append(position.z());
    json["position"] = pos;
    
    QJsonArray rot;
    rot.append(rotation.x());
    rot.append(rotation.y());
    rot.append(rotation.z());
    json["rotation"] = rot;
    
    json["timestamp"] = timestamp;
    json["scannerModel"] = scannerModel;
    json["accuracy"] = accuracy;
    json["coordinateSystem"] = coordinateSystem;
    
    return json;
}

void ScanPositionInfo::fromJson(const QJsonObject& json)
{
    QJsonArray pos = json["position"].toArray();
    if (pos.size() == 3) {
        position = QVector3D(pos[0].toDouble(), pos[1].toDouble(), pos[2].toDouble());
    }
    
    QJsonArray rot = json["rotation"].toArray();
    if (rot.size() == 3) {
        rotation = QVector3D(rot[0].toDouble(), rot[1].toDouble(), rot[2].toDouble());
    }
    
    timestamp = json["timestamp"].toString();
    scannerModel = json["scannerModel"].toString();
    accuracy = json["accuracy"].toDouble();
    coordinateSystem = json["coordinateSystem"].toString();
}

// PointCloudParser 实现
PointCloudParser::PointCloudParser(QObject *parent)
    : QObject(parent)
    , m_lastResult(Success)
    , m_maxFileSizeMB(500.0)
    , m_maxPointCount(10000000) // 1000万点
    , m_enablePreprocessing(true)
    , m_enableNormalEstimation(false)
    , m_cancelRequested(false)
{
}

PointCloudParser::FileFormat PointCloudParser::detectFileFormat(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    
    if (suffix == "ply") {
        return PLY;
    } else if (suffix == "stl") {
        return STL;
    } else if (suffix == "obj") {
        return OBJ;
    } else if (suffix == "pcd") {
        return PCD;
    }
    
    return Unknown;
}

QString PointCloudParser::formatToString(FileFormat format)
{
    switch (format) {
    case PLY: return "PLY";
    case STL: return "STL";
    case OBJ: return "OBJ";
    case PCD: return "PCD";
    default: return "Unknown";
    }
}

PointCloudParser::FileFormat PointCloudParser::formatFromString(const QString& formatStr)
{
    QString format = formatStr.toUpper();
    if (format == "PLY") return PLY;
    if (format == "STL") return STL;
    if (format == "OBJ") return OBJ;
    if (format == "PCD") return PCD;
    return Unknown;
}

PointCloudParser::ParseResult PointCloudParser::parseFile(const QString& filePath, PointCloudData& data)
{
    QElapsedTimer timer;
    timer.start();
    
    // 重置错误状态
    m_lastError.clear();
    m_validationErrors.clear();
    m_lastResult = Success;
    m_cancelRequested = false;
    
    // 检查文件是否存在
    if (!QFileInfo::exists(filePath)) {
        return setError(FileNotFound, QString("文件不存在: %1").arg(filePath)), FileNotFound;
    }
    
    // 检查文件大小
    if (!checkFileSize(filePath)) {
        return setError(InsufficientMemory, "文件过大"), InsufficientMemory;
    }
    
    // 检测文件格式
    FileFormat format = detectFileFormat(filePath);
    if (format == Unknown) {
        return setError(UnsupportedFormat, "不支持的文件格式"), UnsupportedFormat;
    }
    
    // 初始化数据
    data = PointCloudData();
    data.fileName = QFileInfo(filePath).fileName();
    data.format = formatToString(format);
    data.fileSize = QFileInfo(filePath).size() / (1024.0 * 1024.0); // MB
    
    // 根据格式解析文件
    ParseResult result = Success;
    try {
        switch (format) {
        case PLY:
            result = parsePLY(filePath, data);
            break;
        case STL:
            result = parseSTL(filePath, data);
            break;
        case OBJ:
            result = parseOBJ(filePath, data);
            break;
        case PCD:
            result = parsePCD(filePath, data);
            break;
        default:
            result = UnsupportedFormat;
            break;
        }
    } catch (const std::exception& e) {
        return setError(ParseError, QString("解析异常: %1").arg(e.what())), ParseError;
    }
    
    if (result == Success) {
        // 计算边界框
        data.calculateBoundingBox();
        
        // 验证数据
        if (!validatePointCloud(data)) {
            result = InvalidData;
        } else {
            // 暂时禁用预处理以避免潜在问题
            // if (m_enablePreprocessing) {
            //     preprocessPointCloud(data);
            // }
            
            // 更新统计信息
            updateStatistics(data, timer.elapsed());
            
            emit parseCompleted(filePath, true);
        }
    }
    
    if (result != Success) {
        emit parseError(filePath, m_lastError);
    }
    
    return result;
}

PointCloudParser::ParseResult PointCloudParser::parsePCD(const QString& filePath, PointCloudData& data)
{
    try {
        PointCloudT::Ptr cloud(new PointCloudT);
        
        if (pcl::io::loadPCDFile<PointT>(filePath.toStdString(), *cloud) == -1) {
            return setError(ParseError, "PCD文件加载失败"), ParseError;
        }
        
        if (!convertPCLToQVector(cloud, data)) {
            return setError(ParseError, "PCD数据转换失败"), ParseError;
        }
        
        qDebug() << "成功解析PCD文件:" << filePath << "点数:" << data.pointCount;
        return Success;
        
    } catch (const std::exception& e) {
        return setError(ParseError, QString("PCD解析异常: %1").arg(e.what())), ParseError;
    }
}

PointCloudParser::ParseResult PointCloudParser::parsePLY(const QString& filePath, PointCloudData& data)
{
    try {
        PointCloudT::Ptr cloud(new PointCloudT);
        
        qDebug() << "尝试加载PLY文件:" << filePath;
        
        // 检查文件名是否包含非ASCII字符
        bool hasNonAscii = false;
        for (QChar c : filePath) {
            if (c.unicode() > 127) {
                hasNonAscii = true;
                break;
            }
        }
        
        QString workingPath = filePath;
        QString tempFilePath;
        
        // 如果包含非ASCII字符（如中文），创建临时副本
        if (hasNonAscii) {
            qDebug() << "检测到非ASCII字符，创建临时文件...";
            emit parseProgress(10); // 10%
            
            // 检查取消状态
            if (m_cancelRequested) {
                qDebug() << "❌ 解析已取消（临时文件创建阶段）";
                return setError(ParseError, "解析已取消"), ParseError;
            }
            
            // 创建临时文件路径
            QString tempDir = QDir::tempPath();
            QString tempFileName = QString("pointcloud_temp_%1.ply").arg(QDateTime::currentMSecsSinceEpoch());
            tempFilePath = QDir(tempDir).filePath(tempFileName);
            
            // 复制文件
            if (!QFile::copy(filePath, tempFilePath)) {
                return setError(ParseError, QString("无法创建临时文件: %1").arg(tempFilePath)), ParseError;
            }
            
            workingPath = tempFilePath;
            qDebug() << "临时文件已创建:" << tempFilePath;
            emit parseProgress(20); // 20%
        }
        
        // 检查取消状态
        if (m_cancelRequested) {
            qDebug() << "❌ 解析已取消（PCL加载前）";
            if (!tempFilePath.isEmpty()) {
                QFile::remove(tempFilePath);
            }
            return setError(ParseError, "解析已取消"), ParseError;
        }
        
        // 使用本地8位编码（Windows ANSI）
        std::string localPath = workingPath.toLocal8Bit().toStdString();
        
        qDebug() << "加载路径:" << workingPath;
        emit parseProgress(30); // 30%
        
        qDebug() << "开始PCL加载，这可能需要一些时间...";
        qDebug() << "⚠️ 注意：PCL加载过程无法中断，请耐心等待";
        
        if (pcl::io::loadPLYFile<PointT>(localPath, *cloud) == -1) {
            // 清理临时文件
            if (!tempFilePath.isEmpty()) {
                QFile::remove(tempFilePath);
            }
            return setError(ParseError, QString("PLY文件加载失败: %1").arg(filePath)), ParseError;
        }
        
        // PCL加载完成后立即检查取消状态
        if (m_cancelRequested) {
            qDebug() << "❌ 解析已取消（PCL加载后）";
            if (!tempFilePath.isEmpty()) {
                QFile::remove(tempFilePath);
            }
            return setError(ParseError, "解析已取消"), ParseError;
        }
        
        emit parseProgress(70); // 70%
        
        // 清理临时文件
        if (!tempFilePath.isEmpty()) {
            QFile::remove(tempFilePath);
            qDebug() << "临时文件已删除";
        }
        
        if (!convertPCLToQVector(cloud, data)) {
            return setError(ParseError, "PLY数据转换失败"), ParseError;
        }
        
        emit parseProgress(90); // 90%
        
        qDebug() << "成功解析PLY文件:" << filePath << "点数:" << data.pointCount;
        emit parseProgress(100); // 100%
        return Success;
        
    } catch (const std::exception& e) {
        return setError(ParseError, QString("PLY解析异常: %1").arg(e.what())), ParseError;
    }
}

PointCloudParser::ParseResult PointCloudParser::parseSTL(const QString& filePath, PointCloudData& data)
{
    // STL文件解析暂时不支持，返回不支持的格式
    return setError(UnsupportedFormat, "STL格式暂不支持，请使用PLY或PCD格式");
}

PointCloudParser::ParseResult PointCloudParser::parseOBJ(const QString& filePath, PointCloudData& data)
{
    try {
        // OBJ文件手动解析（PCL对OBJ支持有限）
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return setError(FileNotFound, "无法打开OBJ文件"), FileNotFound;
        }
        
        QTextStream in(&file);
        QList<QVector3D> vertices;
        
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.startsWith("v ")) {
                QStringList parts = line.split(' ', Qt::SkipEmptyParts);
                if (parts.size() >= 4) {
                    bool ok1, ok2, ok3;
                    float x = parts[1].toFloat(&ok1);
                    float y = parts[2].toFloat(&ok2);
                    float z = parts[3].toFloat(&ok3);
                    
                    if (ok1 && ok2 && ok3) {
                        vertices.append(QVector3D(x, y, z));
                    }
                }
            }
        }
        
        data.points = vertices;
        data.pointCount = vertices.size();
        
        if (data.pointCount == 0) {
            return setError(InvalidData, "OBJ文件中没有找到有效的顶点数据"), InvalidData;
        }
        
        qDebug() << "成功解析OBJ文件:" << filePath << "点数:" << data.pointCount;
        return Success;
        
    } catch (const std::exception& e) {
        return setError(ParseError, QString("OBJ解析异常: %1").arg(e.what())), ParseError;
    }
}

bool PointCloudParser::convertPCLToQVector(const PointCloudT::Ptr& pclCloud, PointCloudData& data)
{
    if (!pclCloud || pclCloud->empty()) {
        return false;
    }
    
    data.points.clear();
    data.points.reserve(pclCloud->size());
    
    for (const auto& point : *pclCloud) {
        if (std::isfinite(point.x) && std::isfinite(point.y) && std::isfinite(point.z)) {
            data.points.append(QVector3D(point.x, point.y, point.z));
        }
    }
    
    data.pointCount = data.points.size();
    return data.pointCount > 0;
}

bool PointCloudParser::validatePointCloud(const PointCloudData& data)
{
    m_validationErrors = data.validationErrors();
    
    // 额外验证
    if (data.pointCount > m_maxPointCount) {
        m_validationErrors << QString("点数量超过限制：%1 > %2").arg(data.pointCount).arg(m_maxPointCount);
    }
    
    // 检查点的有效性
    int invalidPoints = 0;
    for (const QVector3D& point : data.points) {
        if (!std::isfinite(point.x()) || !std::isfinite(point.y()) || !std::isfinite(point.z())) {
            invalidPoints++;
        }
    }
    
    if (invalidPoints > 0) {
        m_validationErrors << QString("发现%1个无效点（NaN或Inf）").arg(invalidPoints);
    }
    
    return m_validationErrors.isEmpty();
}

bool PointCloudParser::preprocessPointCloud(PointCloudData& data)
{
    try {
        // 确保pointCount与实际点数一致
        int actualPointCount = qMin(data.pointCount, data.points.size());
        if (actualPointCount != data.pointCount) {
            qWarning() << "点数量不一致，已修正: " << data.pointCount << " -> " << actualPointCount;
            data.pointCount = actualPointCount;
        }
        
        // 转换为PCL格式进行预处理
        PointCloudT::Ptr cloud(new PointCloudT);
        cloud->width = actualPointCount;
        cloud->height = 1;
        cloud->is_dense = false;
        cloud->points.resize(actualPointCount);
        
        for (int i = 0; i < actualPointCount; ++i) {
            cloud->points[i].x = data.points[i].x();
            cloud->points[i].y = data.points[i].y();
            cloud->points[i].z = data.points[i].z();
        }
        
        // 移除离群点
        removeOutliers(data);
        
        // 估算法向量（如果启用）
        if (m_enableNormalEstimation) {
            estimateNormals(data);
        }
        
        return true;
        
    } catch (const std::exception& e) {
        qWarning() << "点云预处理失败:" << e.what();
        return false;
    }
}

bool PointCloudParser::removeOutliers(PointCloudData& data, double stddevMult)
{
    try {
        // 确保pointCount与实际点数一致
        int actualPointCount = qMin(data.pointCount, data.points.size());
        
        PointCloudT::Ptr cloud(new PointCloudT);
        cloud->width = actualPointCount;
        cloud->height = 1;
        cloud->is_dense = false;
        cloud->points.resize(actualPointCount);
        
        for (int i = 0; i < actualPointCount; ++i) {
            cloud->points[i].x = data.points[i].x();
            cloud->points[i].y = data.points[i].y();
            cloud->points[i].z = data.points[i].z();
        }
        
        // 统计离群点移除
        pcl::StatisticalOutlierRemoval<PointT> sor;
        sor.setInputCloud(cloud);
        sor.setMeanK(50);
        sor.setStddevMulThresh(stddevMult);
        
        PointCloudT::Ptr filteredCloud(new PointCloudT);
        sor.filter(*filteredCloud);
        
        // 转换回Qt格式
        return convertPCLToQVector(filteredCloud, data);
        
    } catch (const std::exception& e) {
        qWarning() << "离群点移除失败:" << e.what();
        return false;
    }
}

bool PointCloudParser::estimateNormals(PointCloudData& data, int kNeighbors)
{
    try {
        // 确保pointCount与实际点数一致
        int actualPointCount = qMin(data.pointCount, data.points.size());
        
        PointCloudT::Ptr cloud(new PointCloudT);
        cloud->width = actualPointCount;
        cloud->height = 1;
        cloud->is_dense = false;
        cloud->points.resize(actualPointCount);
        
        for (int i = 0; i < actualPointCount; ++i) {
            cloud->points[i].x = data.points[i].x();
            cloud->points[i].y = data.points[i].y();
            cloud->points[i].z = data.points[i].z();
        }
        
        // 法向量估算
        pcl::NormalEstimation<PointT, pcl::Normal> ne;
        ne.setInputCloud(cloud);
        
        pcl::search::KdTree<PointT>::Ptr tree(new pcl::search::KdTree<PointT>());
        ne.setSearchMethod(tree);
        ne.setKSearch(kNeighbors);
        
        pcl::PointCloud<pcl::Normal>::Ptr normals(new pcl::PointCloud<pcl::Normal>);
        ne.compute(*normals);
        
        // 转换法向量
        data.normals.clear();
        data.normals.reserve(normals->size());
        
        for (const auto& normal : *normals) {
            if (std::isfinite(normal.normal_x) && std::isfinite(normal.normal_y) && std::isfinite(normal.normal_z)) {
                data.normals.append(QVector3D(normal.normal_x, normal.normal_y, normal.normal_z));
            } else {
                data.normals.append(QVector3D(0, 0, 1)); // 默认法向量
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        qWarning() << "法向量估算失败:" << e.what();
        return false;
    }
}

bool PointCloudParser::checkFileSize(const QString& filePath, double maxSizeMB)
{
    QFileInfo fileInfo(filePath);
    double fileSizeMB = fileInfo.size() / (1024.0 * 1024.0);
    
    if (fileSizeMB > maxSizeMB) {
        qWarning() << "文件过大:" << fileSizeMB << "MB, 限制:" << maxSizeMB << "MB";
        return false;
    }
    
    return true;
}

double PointCloudParser::estimateMemoryUsage(int pointCount) const
{
    // 估算内存使用量（MB）
    // 每个点：3个float（位置）+ 3个float（法向量）+ 3个float（颜色）= 36字节
    return (pointCount * 36.0) / (1024.0 * 1024.0);
}

void PointCloudParser::updateStatistics(const PointCloudData& data, double processingTime)
{
    m_statistics.totalFiles++;
    m_statistics.successfulFiles++;
    m_statistics.totalProcessingTime += processingTime;
    m_statistics.averageFileSize = (m_statistics.averageFileSize * (m_statistics.totalFiles - 1) + data.fileSize) / m_statistics.totalFiles;
    m_statistics.totalPoints += data.pointCount;
}

void PointCloudParser::resetStatistics()
{
    m_statistics = ParseStatistics();
}

PointCloudParser::ParseResult PointCloudParser::setError(ParseResult result, const QString& message)
{
    m_lastResult = result;
    m_lastError = message;
    qWarning() << "PointCloudParser Error:" << message;
    return result;
}

} // namespace Data