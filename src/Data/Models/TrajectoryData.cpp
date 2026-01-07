#include "TrajectoryData.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QtMath>

namespace Data {

// TrajectoryPoint 实现
QJsonObject TrajectoryPoint::toJson() const
{
    QJsonObject json;
    json["index"] = index;
    
    QJsonObject pos;
    pos["x"] = position.x();
    pos["y"] = position.y();
    pos["z"] = position.z();
    json["position"] = pos;
    
    QJsonObject orient;
    orient["x"] = orientation.x();
    orient["y"] = orientation.y();
    orient["z"] = orientation.z();
    orient["w"] = orientation.scalar();
    json["orientation"] = orient;
    
    json["speed"] = speed;
    json["flow_rate"] = flowRate;
    json["spray_width"] = sprayWidth;
    json["dwell_time"] = dwellTime;
    
    return json;
}

void TrajectoryPoint::fromJson(const QJsonObject& json)
{
    index = json["index"].toInt();
    
    if (json.contains("position")) {
        QJsonObject pos = json["position"].toObject();
        position = QVector3D(pos["x"].toDouble(), pos["y"].toDouble(), pos["z"].toDouble());
    }
    
    if (json.contains("orientation")) {
        QJsonObject orient = json["orientation"].toObject();
        orientation = QQuaternion(orient["w"].toDouble(), orient["x"].toDouble(), 
                                 orient["y"].toDouble(), orient["z"].toDouble());
    }
    
    speed = json["speed"].toDouble();
    flowRate = json["flow_rate"].toDouble();
    sprayWidth = json["spray_width"].toDouble();
    dwellTime = json["dwell_time"].toDouble();
}

// TrajectoryData 实现
TrajectoryData::TrajectoryData(QObject *parent)
    : BaseModel(parent)
    , m_workpieceId(0)
    , m_trajectoryType(Spray)
    , m_totalLength(0.0)
    , m_estimatedTime(0)
    , m_qualityScore(0.0)
    , m_coverageRate(0.0)
    , m_createdBy(0)
{
}

void TrajectoryData::setWorkpieceId(int id)
{
    if (m_workpieceId != id) {
        m_workpieceId = id;
        emit workpieceIdChanged();
        notifyDataChanged();
    }
}

void TrajectoryData::setName(const QString& name)
{
    if (m_name != name) {
        m_name = name.trimmed();
        emit nameChanged();
        notifyDataChanged();
    }
}

void TrajectoryData::setDescription(const QString& description)
{
    if (m_description != description) {
        m_description = description.trimmed();
        emit descriptionChanged();
        notifyDataChanged();
    }
}

void TrajectoryData::setTrajectoryType(TrajectoryType type)
{
    if (m_trajectoryType != type) {
        m_trajectoryType = type;
        emit trajectoryTypeChanged();
        notifyDataChanged();
    }
}

void TrajectoryData::setTotalLength(double length)
{
    if (qAbs(m_totalLength - length) > 0.001) {
        m_totalLength = length;
        emit totalLengthChanged();
        notifyDataChanged();
    }
}

void TrajectoryData::setEstimatedTime(int seconds)
{
    if (m_estimatedTime != seconds) {
        m_estimatedTime = seconds;
        emit estimatedTimeChanged();
        notifyDataChanged();
    }
}

void TrajectoryData::setQualityScore(double score)
{
    if (qAbs(m_qualityScore - score) > 0.001) {
        m_qualityScore = qBound(0.0, score, 1.0);
        emit qualityScoreChanged();
        notifyDataChanged();
    }
}

void TrajectoryData::setCoverageRate(double rate)
{
    if (qAbs(m_coverageRate - rate) > 0.001) {
        m_coverageRate = qBound(0.0, rate, 100.0);
        emit coverageRateChanged();
        notifyDataChanged();
    }
}

void TrajectoryData::setCreatedBy(int userId)
{
    if (m_createdBy != userId) {
        m_createdBy = userId;
        emit createdByChanged();
        notifyDataChanged();
    }
}

void TrajectoryData::setPoints(const QList<TrajectoryPoint>& points)
{
    m_points = points;
    updateStatistics();
    emit pointsChanged();
    emit totalPointsChanged();
    notifyDataChanged();
}

void TrajectoryData::addPoint(const TrajectoryPoint& point)
{
    m_points.append(point);
    updateStatistics();
    emit pointsChanged();
    emit totalPointsChanged();
    notifyDataChanged();
}

void TrajectoryData::insertPoint(int index, const TrajectoryPoint& point)
{
    if (index >= 0 && index <= m_points.size()) {
        m_points.insert(index, point);
        updateStatistics();
        emit pointsChanged();
        emit totalPointsChanged();
        notifyDataChanged();
    }
}

void TrajectoryData::removePoint(int index)
{
    if (index >= 0 && index < m_points.size()) {
        m_points.removeAt(index);
        updateStatistics();
        emit pointsChanged();
        emit totalPointsChanged();
        notifyDataChanged();
    }
}

void TrajectoryData::clearPoints()
{
    m_points.clear();
    updateStatistics();
    emit pointsChanged();
    emit totalPointsChanged();
    notifyDataChanged();
}

TrajectoryPoint TrajectoryData::getPoint(int index) const
{
    if (index >= 0 && index < m_points.size()) {
        return m_points[index];
    }
    return TrajectoryPoint();
}

void TrajectoryData::updatePoint(int index, const TrajectoryPoint& point)
{
    if (index >= 0 && index < m_points.size()) {
        m_points[index] = point;
        updateStatistics();
        emit pointsChanged();
        notifyDataChanged();
    }
}

void TrajectoryData::setParameters(const QJsonObject& params)
{
    m_parameters = params;
    emit parametersChanged();
    notifyDataChanged();
}

void TrajectoryData::setParameter(const QString& key, const QVariant& value)
{
    m_parameters[key] = QJsonValue::fromVariant(value);
    emit parametersChanged();
    notifyDataChanged();
}

QVariant TrajectoryData::getParameter(const QString& key, const QVariant& defaultValue) const
{
    if (m_parameters.contains(key)) {
        return m_parameters[key].toVariant();
    }
    return defaultValue;
}

QJsonObject TrajectoryData::toJson() const
{
    QJsonObject json = BaseModel::toJson();
    
    json["workpiece_id"] = m_workpieceId;
    json["name"] = m_name;
    json["description"] = m_description;
    json["trajectory_type"] = trajectoryTypeToString(m_trajectoryType);
    json["total_points"] = m_points.size();
    json["total_length"] = m_totalLength;
    json["estimated_time"] = m_estimatedTime;
    json["quality_score"] = m_qualityScore;
    json["coverage_rate"] = m_coverageRate;
    json["created_by"] = m_createdBy;
    json["parameters"] = m_parameters;
    
    // 轨迹点数据
    QJsonArray pointsArray;
    for (const auto& point : m_points) {
        pointsArray.append(point.toJson());
    }
    json["points"] = pointsArray;
    
    return json;
}

void TrajectoryData::fromJson(const QJsonObject& json)
{
    BaseModel::fromJson(json);
    
    if (json.contains("workpiece_id")) {
        setWorkpieceId(json["workpiece_id"].toInt());
    }
    
    if (json.contains("name")) {
        setName(json["name"].toString());
    }
    
    if (json.contains("description")) {
        setDescription(json["description"].toString());
    }
    
    if (json.contains("trajectory_type")) {
        setTrajectoryType(trajectoryTypeFromString(json["trajectory_type"].toString()));
    }
    
    if (json.contains("total_length")) {
        setTotalLength(json["total_length"].toDouble());
    }
    
    if (json.contains("estimated_time")) {
        setEstimatedTime(json["estimated_time"].toInt());
    }
    
    if (json.contains("quality_score")) {
        setQualityScore(json["quality_score"].toDouble());
    }
    
    if (json.contains("coverage_rate")) {
        setCoverageRate(json["coverage_rate"].toDouble());
    }
    
    if (json.contains("created_by")) {
        setCreatedBy(json["created_by"].toInt());
    }
    
    if (json.contains("parameters")) {
        setParameters(json["parameters"].toObject());
    }
    
    // 加载轨迹点
    if (json.contains("points")) {
        QJsonArray pointsArray = json["points"].toArray();
        QList<TrajectoryPoint> points;
        
        for (const auto& value : pointsArray) {
            TrajectoryPoint point;
            point.fromJson(value.toObject());
            points.append(point);
        }
        
        setPoints(points);
    }
}

bool TrajectoryData::isValid() const
{
    return BaseModel::isValid() && validationErrors().isEmpty();
}

QStringList TrajectoryData::validationErrors() const
{
    QStringList errors = BaseModel::validationErrors();
    
    // 验证必填字段
    validateRequired("轨迹名称", m_name, errors);
    
    // 验证字段长度
    validateLength("轨迹名称", m_name, 100, errors);
    
    // 验证数值范围
    validateRange("质量评分", m_qualityScore, 0.0, 1.0, errors);
    validateRange("覆盖率", m_coverageRate, 0.0, 100.0, errors);
    
    // 验证工件ID
    if (m_workpieceId <= 0) {
        errors << "必须关联有效的工件";
    }
    
    // 验证轨迹点
    if (m_points.isEmpty()) {
        errors << "轨迹必须包含至少一个点";
    }
    
    // 验证轨迹点的连续性
    for (int i = 0; i < m_points.size(); ++i) {
        if (m_points[i].index != i) {
            errors << QString("轨迹点索引不连续：期望 %1，实际 %2").arg(i).arg(m_points[i].index);
        }
        
        if (m_points[i].speed <= 0) {
            errors << QString("轨迹点 %1 的速度必须大于0").arg(i);
        }
        
        if (m_points[i].flowRate < 0 || m_points[i].flowRate > 1.0) {
            errors << QString("轨迹点 %1 的流量必须在0-1之间").arg(i);
        }
    }
    
    return errors;
}

BaseModel* TrajectoryData::clone() const
{
    TrajectoryData* cloned = new TrajectoryData();
    cloned->fromJson(this->toJson());
    cloned->setId(0); // 克隆对象为新对象
    return cloned;
}

double TrajectoryData::calculateTotalLength() const
{
    if (m_points.size() < 2) {
        return 0.0;
    }
    
    double totalLength = 0.0;
    for (int i = 1; i < m_points.size(); ++i) {
        QVector3D diff = m_points[i].position - m_points[i-1].position;
        totalLength += diff.length();
    }
    
    return totalLength;
}

int TrajectoryData::calculateEstimatedTime() const
{
    if (m_points.isEmpty()) {
        return 0;
    }
    
    double totalTime = 0.0;
    
    for (int i = 1; i < m_points.size(); ++i) {
        QVector3D diff = m_points[i].position - m_points[i-1].position;
        double distance = diff.length();
        double avgSpeed = (m_points[i-1].speed + m_points[i].speed) / 2.0;
        
        if (avgSpeed > 0) {
            totalTime += distance / avgSpeed; // 时间 = 距离 / 速度
        }
        
        // 添加停留时间
        totalTime += m_points[i].dwellTime;
    }
    
    return static_cast<int>(totalTime);
}

QVector3D TrajectoryData::getBoundingBoxMin() const
{
    if (m_points.isEmpty()) {
        return QVector3D();
    }
    
    QVector3D minPoint = m_points[0].position;
    for (const auto& point : m_points) {
        minPoint.setX(qMin(minPoint.x(), point.position.x()));
        minPoint.setY(qMin(minPoint.y(), point.position.y()));
        minPoint.setZ(qMin(minPoint.z(), point.position.z()));
    }
    
    return minPoint;
}

QVector3D TrajectoryData::getBoundingBoxMax() const
{
    if (m_points.isEmpty()) {
        return QVector3D();
    }
    
    QVector3D maxPoint = m_points[0].position;
    for (const auto& point : m_points) {
        maxPoint.setX(qMax(maxPoint.x(), point.position.x()));
        maxPoint.setY(qMax(maxPoint.y(), point.position.y()));
        maxPoint.setZ(qMax(maxPoint.z(), point.position.z()));
    }
    
    return maxPoint;
}

QVector3D TrajectoryData::getBoundingBoxSize() const
{
    return getBoundingBoxMax() - getBoundingBoxMin();
}

QVector3D TrajectoryData::getCenterPoint() const
{
    return (getBoundingBoxMin() + getBoundingBoxMax()) / 2.0f;
}

void TrajectoryData::optimizePoints()
{
    // 简单的轨迹优化：移除冗余点
    if (m_points.size() < 3) {
        return;
    }
    
    QList<TrajectoryPoint> optimizedPoints;
    optimizedPoints.append(m_points[0]); // 保留第一个点
    
    const double threshold = 0.1; // 共线阈值
    
    for (int i = 1; i < m_points.size() - 1; ++i) {
        QVector3D v1 = m_points[i].position - m_points[i-1].position;
        QVector3D v2 = m_points[i+1].position - m_points[i].position;
        
        // 计算角度
        double dot = QVector3D::dotProduct(v1.normalized(), v2.normalized());
        double angle = qAcos(qBound(-1.0, dot, 1.0));
        
        // 如果角度变化较大，保留该点
        if (angle > threshold) {
            optimizedPoints.append(m_points[i]);
        }
    }
    
    optimizedPoints.append(m_points.last()); // 保留最后一个点
    
    // 重新索引
    for (int i = 0; i < optimizedPoints.size(); ++i) {
        optimizedPoints[i].index = i;
    }
    
    setPoints(optimizedPoints);
}

void TrajectoryData::smoothTrajectory(double factor)
{
    if (m_points.size() < 3) {
        return;
    }
    
    QList<TrajectoryPoint> smoothedPoints = m_points;
    
    // 简单的平滑算法：移动平均
    for (int i = 1; i < smoothedPoints.size() - 1; ++i) {
        QVector3D avgPos = (m_points[i-1].position + m_points[i].position + m_points[i+1].position) / 3.0f;
        smoothedPoints[i].position = m_points[i].position * (1.0f - factor) + avgPos * factor;
    }
    
    setPoints(smoothedPoints);
}

void TrajectoryData::resampleTrajectory(double targetSpacing)
{
    if (m_points.size() < 2 || targetSpacing <= 0) {
        return;
    }
    
    QList<TrajectoryPoint> resampledPoints;
    resampledPoints.append(m_points[0]); // 保留第一个点
    
    double accumulatedDistance = 0.0;
    int currentIndex = 0;
    
    for (int i = 1; i < m_points.size(); ++i) {
        QVector3D diff = m_points[i].position - m_points[i-1].position;
        double segmentLength = diff.length();
        
        accumulatedDistance += segmentLength;
        
        // 如果累积距离达到目标间距，添加新点
        while (accumulatedDistance >= targetSpacing) {
            double ratio = (accumulatedDistance - targetSpacing) / segmentLength;
            
            TrajectoryPoint newPoint;
            newPoint.index = ++currentIndex;
            newPoint.position = m_points[i].position - diff * ratio;
            
            // 插值其他属性
            double t = 1.0 - ratio;
            newPoint.speed = m_points[i-1].speed * (1.0 - t) + m_points[i].speed * t;
            newPoint.flowRate = m_points[i-1].flowRate * (1.0 - t) + m_points[i].flowRate * t;
            newPoint.sprayWidth = m_points[i-1].sprayWidth * (1.0 - t) + m_points[i].sprayWidth * t;
            newPoint.dwellTime = m_points[i-1].dwellTime * (1.0 - t) + m_points[i].dwellTime * t;
            
            resampledPoints.append(newPoint);
            accumulatedDistance -= targetSpacing;
        }
    }
    
    // 确保保留最后一个点
    if (resampledPoints.last().position != m_points.last().position) {
        TrajectoryPoint lastPoint = m_points.last();
        lastPoint.index = resampledPoints.size();
        resampledPoints.append(lastPoint);
    }
    
    setPoints(resampledPoints);
}

void TrajectoryData::updateStatistics()
{
    m_totalLength = calculateTotalLength();
    m_estimatedTime = calculateEstimatedTime();
}

QString TrajectoryData::trajectoryTypeToString(TrajectoryType type) const
{
    switch (type) {
    case Spray: return "spray";
    case Move: return "move";
    case Approach: return "approach";
    case Retract: return "retract";
    default: return "spray";
    }
}

TrajectoryData::TrajectoryType TrajectoryData::trajectoryTypeFromString(const QString& typeStr) const
{
    if (typeStr == "move") return Move;
    if (typeStr == "approach") return Approach;
    if (typeStr == "retract") return Retract;
    return Spray; // 默认为喷涂轨迹
}

} // namespace Data