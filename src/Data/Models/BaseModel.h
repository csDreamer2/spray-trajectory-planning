#ifndef BASEMODEL_H
#define BASEMODEL_H

#include <QObject>
#include <QDateTime>
#include <QJsonObject>
#include <QVariant>

namespace Data {

/**
 * @brief 数据模型基类
 * 
 * 提供所有数据模型的基础功能，包括：
 * - 基础字段（ID、创建时间、更新时间）
 * - JSON序列化/反序列化
 * - 数据验证
 * - 变更跟踪
 */
class BaseModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int id READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QDateTime createdAt READ createdAt WRITE setCreatedAt NOTIFY createdAtChanged)
    Q_PROPERTY(QDateTime updatedAt READ updatedAt WRITE setUpdatedAt NOTIFY updatedAtChanged)
    Q_PROPERTY(bool isActive READ isActive WRITE setIsActive NOTIFY isActiveChanged)

public:
    explicit BaseModel(QObject *parent = nullptr);
    virtual ~BaseModel() = default;

    // 基础属性
    int id() const { return m_id; }
    void setId(int id);

    QDateTime createdAt() const { return m_createdAt; }
    void setCreatedAt(const QDateTime& dateTime);

    QDateTime updatedAt() const { return m_updatedAt; }
    void setUpdatedAt(const QDateTime& dateTime);

    bool isActive() const { return m_isActive; }
    void setIsActive(bool active);

    // 数据状态
    bool isNew() const { return m_id <= 0; }
    bool isDirty() const { return m_isDirty; }
    void markClean() { m_isDirty = false; }
    void markDirty() { m_isDirty = true; }

    // JSON序列化
    virtual QJsonObject toJson() const;
    virtual void fromJson(const QJsonObject& json);

    // 数据验证
    virtual bool isValid() const;
    virtual QStringList validationErrors() const;

    // 克隆
    virtual BaseModel* clone() const = 0;

signals:
    void idChanged();
    void createdAtChanged();
    void updatedAtChanged();
    void isActiveChanged();
    void dataChanged();

protected:
    // 子类调用此方法来标记数据已修改
    void notifyDataChanged();

    // 验证辅助方法
    bool validateRequired(const QString& fieldName, const QVariant& value, QStringList& errors) const;
    bool validateLength(const QString& fieldName, const QString& value, int maxLength, QStringList& errors) const;
    bool validateRange(const QString& fieldName, double value, double min, double max, QStringList& errors) const;

private:
    int m_id;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
    bool m_isActive;
    bool m_isDirty;
};

} // namespace Data

#endif // BASEMODEL_H