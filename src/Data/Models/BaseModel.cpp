#include "BaseModel.h"
#include <QJsonObject>
#include <QDebug>

namespace Data {

BaseModel::BaseModel(QObject *parent)
    : QObject(parent)
    , m_id(0)
    , m_createdAt(QDateTime::currentDateTime())
    , m_updatedAt(QDateTime::currentDateTime())
    , m_isActive(true)
    , m_isDirty(false)
{
}

void BaseModel::setId(int id)
{
    if (m_id != id) {
        m_id = id;
        emit idChanged();
        notifyDataChanged();
    }
}

void BaseModel::setCreatedAt(const QDateTime& dateTime)
{
    if (m_createdAt != dateTime) {
        m_createdAt = dateTime;
        emit createdAtChanged();
        notifyDataChanged();
    }
}

void BaseModel::setUpdatedAt(const QDateTime& dateTime)
{
    if (m_updatedAt != dateTime) {
        m_updatedAt = dateTime;
        emit updatedAtChanged();
        notifyDataChanged();
    }
}

void BaseModel::setIsActive(bool active)
{
    if (m_isActive != active) {
        m_isActive = active;
        emit isActiveChanged();
        notifyDataChanged();
    }
}

QJsonObject BaseModel::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["created_at"] = m_createdAt.toString(Qt::ISODate);
    json["updated_at"] = m_updatedAt.toString(Qt::ISODate);
    json["is_active"] = m_isActive;
    return json;
}

void BaseModel::fromJson(const QJsonObject& json)
{
    if (json.contains("id")) {
        setId(json["id"].toInt());
    }
    
    if (json.contains("created_at")) {
        setCreatedAt(QDateTime::fromString(json["created_at"].toString(), Qt::ISODate));
    }
    
    if (json.contains("updated_at")) {
        setUpdatedAt(QDateTime::fromString(json["updated_at"].toString(), Qt::ISODate));
    }
    
    if (json.contains("is_active")) {
        setIsActive(json["is_active"].toBool());
    }
}

bool BaseModel::isValid() const
{
    return validationErrors().isEmpty();
}

QStringList BaseModel::validationErrors() const
{
    QStringList errors;
    
    // 基础验证
    if (m_createdAt > QDateTime::currentDateTime()) {
        errors << "创建时间不能是未来时间";
    }
    
    if (m_updatedAt > QDateTime::currentDateTime()) {
        errors << "更新时间不能是未来时间";
    }
    
    if (m_createdAt > m_updatedAt) {
        errors << "创建时间不能晚于更新时间";
    }
    
    return errors;
}

void BaseModel::notifyDataChanged()
{
    m_isDirty = true;
    m_updatedAt = QDateTime::currentDateTime();
    emit dataChanged();
}

bool BaseModel::validateRequired(const QString& fieldName, const QVariant& value, QStringList& errors) const
{
    if (value.isNull() || value.toString().trimmed().isEmpty()) {
        errors << QString("%1 是必填字段").arg(fieldName);
        return false;
    }
    return true;
}

bool BaseModel::validateLength(const QString& fieldName, const QString& value, int maxLength, QStringList& errors) const
{
    if (value.length() > maxLength) {
        errors << QString("%1 长度不能超过 %2 个字符").arg(fieldName).arg(maxLength);
        return false;
    }
    return true;
}

bool BaseModel::validateRange(const QString& fieldName, double value, double min, double max, QStringList& errors) const
{
    if (value < min || value > max) {
        errors << QString("%1 必须在 %2 到 %3 之间").arg(fieldName).arg(min).arg(max);
        return false;
    }
    return true;
}

} // namespace Data