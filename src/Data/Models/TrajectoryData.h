#ifndef TRAJECTORYDATA_H
#define TRAJECTORYDATA_H

#include "BaseModel.h"
#include <QtGui/QVector3D>
#include <QtGui/QQuaternion>
#include <QList>

namespace Data {

/**
 * @brief 轨迹点数据结构
 */
struct TrajectoryPoint
{
    int index;                  // 点索引
    QVector3D position;         // 位置 (x, y, z)
    QQuaternion orientation;    // 姿态 (四元数)
    double speed;               // 速度 mm/s
    double flowRate;            // 流量 0.0-1.0
    double sprayWidth;          // 喷涂宽度 mm
    double dwellTime;           // 停留时间 秒

    TrajectoryPoint()
        : index(0), speed(0.0), flowRate(0.0), sprayWidth(0.0), dwellTime(0.0) {}

    TrajectoryPoint(int idx, const QVector3D& pos, const QQuaternion& orient = QQuaternion(),
                   double spd = 50.0, double flow = 0.8, double width = 10.0, double dwell = 0.0)
        : index(idx), position(pos), orientation(orient), speed(spd), 
          flowRate(flow), sprayWidth(width), dwellTime(dwell) {}

    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);
};

/**
 * @brief 轨迹数据模型
 * 
 * 存储完整的喷涂轨迹信息，包括：
 * - 轨迹基本信息
 * - 轨迹点列表
 * - 质量评估数据
 * - 执行参数
 */
class TrajectoryData : public BaseModel
{
    Q_OBJECT
    Q_PROPERTY(int workpieceId READ workpieceId WRITE setWorkpieceId NOTIFY workpieceIdChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(TrajectoryType trajectoryType READ trajectoryType WRITE setTrajectoryType NOTIFY trajectoryTypeChanged)
    Q_PROPERTY(int totalPoints READ totalPoints NOTIFY totalPointsChanged)
    Q_PROPERTY(double totalLength READ totalLength WRITE setTotalLength NOTIFY totalLengthChanged)
    Q_PROPERTY(int estimatedTime READ estimatedTime WRITE setEstimatedTime NOTIFY estimatedTimeChanged)
    Q_PROPERTY(double qualityScore READ qualityScore WRITE setQualityScore NOTIFY qualityScoreChanged)
    Q_PROPERTY(double coverageRate READ coverageRate WRITE setCoverageRate NOTIFY coverageRateChanged)
    Q_PROPERTY(int createdBy READ createdBy WRITE setCreatedBy NOTIFY createdByChanged)

public:
    enum TrajectoryType {
        Spray = 0,      // 喷涂轨迹
        Move,           // 移动轨迹
        Approach,       // 接近轨迹
        Retract         // 退出轨迹
    };
    Q_ENUM(TrajectoryType)

    explicit TrajectoryData(QObject *parent = nullptr);
    ~TrajectoryData() override = default;

    // 基本属性
    int workpieceId() const { return m_workpieceId; }
    void setWorkpieceId(int id);

    QString name() const { return m_name; }
    void setName(const QString& name);

    QString description() const { return m_description; }
    void setDescription(const QString& description);

    TrajectoryType trajectoryType() const { return m_trajectoryType; }
    void setTrajectoryType(TrajectoryType type);

    // 轨迹统计信息
    int totalPoints() const { return m_points.size(); }
    
    double totalLength() const { return m_totalLength; }
    void setTotalLength(double length);

    int estimatedTime() const { return m_estimatedTime; }
    void setEstimatedTime(int seconds);

    double qualityScore() const { return m_qualityScore; }
    void setQualityScore(double score);

    double coverageRate() const { return m_coverageRate; }
    void setCoverageRate(double rate);

    int createdBy() const { return m_createdBy; }
    void setCreatedBy(int userId);

    // 轨迹点管理
    const QList<TrajectoryPoint>& points() const { return m_points; }
    void setPoints(const QList<TrajectoryPoint>& points);
    void addPoint(const TrajectoryPoint& point);
    void insertPoint(int index, const TrajectoryPoint& point);
    void removePoint(int index);
    void clearPoints();
    
    TrajectoryPoint getPoint(int index) const;
    void updatePoint(int index, const TrajectoryPoint& point);

    // 参数配置
    QJsonObject parameters() const { return m_parameters; }
    void setParameters(const QJsonObject& params);
    void setParameter(const QString& key, const QVariant& value);
    QVariant getParameter(const QString& key, const QVariant& defaultValue = QVariant()) const;

    // 重写基类方法
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& json) override;
    bool isValid() const override;
    QStringList validationErrors() const override;
    BaseModel* clone() const override;

    // 轨迹特有方法
    double calculateTotalLength() const;
    int calculateEstimatedTime() const;
    QVector3D getBoundingBoxMin() const;
    QVector3D getBoundingBoxMax() const;
    QVector3D getBoundingBoxSize() const;
    QVector3D getCenterPoint() const;
    
    // 轨迹优化
    void optimizePoints();
    void smoothTrajectory(double factor = 0.5);
    void resampleTrajectory(double targetSpacing);

signals:
    void workpieceIdChanged();
    void nameChanged();
    void descriptionChanged();
    void trajectoryTypeChanged();
    void totalPointsChanged();
    void totalLengthChanged();
    void estimatedTimeChanged();
    void qualityScoreChanged();
    void coverageRateChanged();
    void createdByChanged();
    void pointsChanged();
    void parametersChanged();

private:
    void updateStatistics();
    QString trajectoryTypeToString(TrajectoryType type) const;
    TrajectoryType trajectoryTypeFromString(const QString& typeStr) const;

private:
    int m_workpieceId;
    QString m_name;
    QString m_description;
    TrajectoryType m_trajectoryType;
    double m_totalLength;
    int m_estimatedTime;
    double m_qualityScore;
    double m_coverageRate;
    int m_createdBy;
    
    QList<TrajectoryPoint> m_points;
    QJsonObject m_parameters;
};

} // namespace Data

Q_DECLARE_METATYPE(Data::TrajectoryData::TrajectoryType)

#endif // TRAJECTORYDATA_H