#include "WorkpieceData.h"
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

namespace Data {

WorkpieceData::WorkpieceData(QObject *parent)
    : BaseModel(parent)
    , m_modelFileSize(0)
    , m_dimensions(0, 0, 0)
    , m_surfaceArea(0.0)
    , m_complexityScore(0.0)
    , m_createdBy(0)
{
}

void WorkpieceData::setName(const QString& name)
{
    if (m_name != name) {
        m_name = name.trimmed();
        emit nameChanged();
        notifyDataChanged();
    }
}

void WorkpieceData::setDescription(const QString& description)
{
    if (m_description != description) {
        m_description = description.trimmed();
        emit descriptionChanged();
        notifyDataChanged();
    }
}

void WorkpieceData::setCategory(const QString& category)
{
    if (m_category != category) {
        m_category = category.trimmed();
        emit categoryChanged();
        notifyDataChanged();
    }
}

void WorkpieceData::setModelFilePath(const QString& path)
{
    if (m_modelFilePath != path) {
        m_modelFilePath = path;
        emit modelFilePathChanged();
        notifyDataChanged();
    }
}

void WorkpieceData::setModelFileSize(qint64 size)
{
    if (m_modelFileSize != size) {
        m_modelFileSize = size;
        emit modelFileSizeChanged();
        notifyDataChanged();
    }
}

void WorkpieceData::setModelFileHash(const QString& hash)
{
    if (m_modelFileHash != hash) {
        m_modelFileHash = hash;
        emit modelFileHashChanged();
        notifyDataChanged();
    }
}

void WorkpieceData::setDimensions(const QVector3D& dimensions)
{
    if (m_dimensions != dimensions) {
        m_dimensions = dimensions;
        emit dimensionsChanged();
        notifyDataChanged();
    }
}

void WorkpieceData::setMaterial(const QString& material)
{
    if (m_material != material) {
        m_material = material.trimmed();
        emit materialChanged();
        notifyDataChanged();
    }
}

void WorkpieceData::setSurfaceArea(double area)
{
    if (qAbs(m_surfaceArea - area) > 0.001) {
        m_surfaceArea = area;
        emit surfaceAreaChanged();
        notifyDataChanged();
    }
}

void WorkpieceData::setComplexityScore(double score)
{
    if (qAbs(m_complexityScore - score) > 0.001) {
        m_complexityScore = qBound(0.0, score, 1.0);
        emit complexityScoreChanged();
        notifyDataChanged();
    }
}

void WorkpieceData::setCreatedBy(int userId)
{
    if (m_createdBy != userId) {
        m_createdBy = userId;
        emit createdByChanged();
        notifyDataChanged();
    }
}

QJsonObject WorkpieceData::toJson() const
{
    QJsonObject json = BaseModel::toJson();
    
    json["name"] = m_name;
    json["description"] = m_description;
    json["category"] = m_category;
    json["model_file_path"] = m_modelFilePath;
    json["model_file_size"] = m_modelFileSize;
    json["model_file_hash"] = m_modelFileHash;
    
    QJsonObject dimensions;
    dimensions["length"] = m_dimensions.x();
    dimensions["width"] = m_dimensions.y();
    dimensions["height"] = m_dimensions.z();
    json["dimensions"] = dimensions;
    
    json["material"] = m_material;
    json["surface_area"] = m_surfaceArea;
    json["complexity_score"] = m_complexityScore;
    json["created_by"] = m_createdBy;
    
    return json;
}

void WorkpieceData::fromJson(const QJsonObject& json)
{
    BaseModel::fromJson(json);
    
    if (json.contains("name")) {
        setName(json["name"].toString());
    }
    
    if (json.contains("description")) {
        setDescription(json["description"].toString());
    }
    
    if (json.contains("category")) {
        setCategory(json["category"].toString());
    }
    
    if (json.contains("model_file_path")) {
        setModelFilePath(json["model_file_path"].toString());
    }
    
    if (json.contains("model_file_size")) {
        setModelFileSize(json["model_file_size"].toVariant().toLongLong());
    }
    
    if (json.contains("model_file_hash")) {
        setModelFileHash(json["model_file_hash"].toString());
    }
    
    if (json.contains("dimensions")) {
        QJsonObject dims = json["dimensions"].toObject();
        setDimensions(QVector3D(
            dims["length"].toDouble(),
            dims["width"].toDouble(),
            dims["height"].toDouble()
        ));
    }
    
    if (json.contains("material")) {
        setMaterial(json["material"].toString());
    }
    
    if (json.contains("surface_area")) {
        setSurfaceArea(json["surface_area"].toDouble());
    }
    
    if (json.contains("complexity_score")) {
        setComplexityScore(json["complexity_score"].toDouble());
    }
    
    if (json.contains("created_by")) {
        setCreatedBy(json["created_by"].toInt());
    }
}

bool WorkpieceData::isValid() const
{
    return BaseModel::isValid() && validationErrors().isEmpty();
}

QStringList WorkpieceData::validationErrors() const
{
    QStringList errors = BaseModel::validationErrors();
    
    // 验证必填字段
    validateRequired("工件名称", m_name, errors);
    validateRequired("工件类别", m_category, errors);
    
    // 验证字段长度
    validateLength("工件名称", m_name, 100, errors);
    validateLength("工件类别", m_category, 50, errors);
    validateLength("材质", m_material, 50, errors);
    
    // 验证数值范围
    validateRange("复杂度评分", m_complexityScore, 0.0, 1.0, errors);
    
    // 验证几何尺寸
    if (m_dimensions.x() <= 0 || m_dimensions.y() <= 0 || m_dimensions.z() <= 0) {
        errors << "工件尺寸必须大于0";
    }
    
    if (m_surfaceArea < 0) {
        errors << "表面积不能为负数";
    }
    
    // 验证模型文件
    if (!m_modelFilePath.isEmpty()) {
        QFileInfo fileInfo(m_modelFilePath);
        if (!fileInfo.exists()) {
            errors << "模型文件不存在";
        }
        
        QStringList supportedFormats = {"stl", "obj", "ply", "pcd"};
        if (!supportedFormats.contains(fileInfo.suffix().toLower())) {
            errors << "不支持的模型文件格式";
        }
    }
    
    return errors;
}

BaseModel* WorkpieceData::clone() const
{
    WorkpieceData* cloned = new WorkpieceData();
    cloned->fromJson(this->toJson());
    cloned->setId(0); // 克隆对象为新对象
    return cloned;
}

double WorkpieceData::getVolume() const
{
    return m_dimensions.x() * m_dimensions.y() * m_dimensions.z();
}

QString WorkpieceData::getDisplayName() const
{
    if (m_name.isEmpty()) {
        return QString("工件_%1").arg(id());
    }
    return m_name;
}

bool WorkpieceData::hasModelFile() const
{
    return !m_modelFilePath.isEmpty() && QFileInfo(m_modelFilePath).exists();
}

QString WorkpieceData::getModelFileExtension() const
{
    if (m_modelFilePath.isEmpty()) {
        return QString();
    }
    return QFileInfo(m_modelFilePath).suffix().toLower();
}

} // namespace Data