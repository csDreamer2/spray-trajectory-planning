#ifndef WORKPIECEDATA_H
#define WORKPIECEDATA_H

#include "BaseModel.h"
#include <QJsonObject>
#include <QVector3D>

namespace Data {

/**
 * @brief 工件数据模型
 * 
 * 存储工件的基本信息，包括：
 * - 基本属性（名称、描述、类别）
 * - 模型文件信息
 * - 几何尺寸
 * - 材质和表面积
 * - 复杂度评分
 */
class WorkpieceData : public BaseModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY categoryChanged)
    Q_PROPERTY(QString modelFilePath READ modelFilePath WRITE setModelFilePath NOTIFY modelFilePathChanged)
    Q_PROPERTY(qint64 modelFileSize READ modelFileSize WRITE setModelFileSize NOTIFY modelFileSizeChanged)
    Q_PROPERTY(QString modelFileHash READ modelFileHash WRITE setModelFileHash NOTIFY modelFileHashChanged)
    Q_PROPERTY(QVector3D dimensions READ dimensions WRITE setDimensions NOTIFY dimensionsChanged)
    Q_PROPERTY(QString material READ material WRITE setMaterial NOTIFY materialChanged)
    Q_PROPERTY(double surfaceArea READ surfaceArea WRITE setSurfaceArea NOTIFY surfaceAreaChanged)
    Q_PROPERTY(double complexityScore READ complexityScore WRITE setComplexityScore NOTIFY complexityScoreChanged)
    Q_PROPERTY(int createdBy READ createdBy WRITE setCreatedBy NOTIFY createdByChanged)

public:
    explicit WorkpieceData(QObject *parent = nullptr);
    ~WorkpieceData() override = default;

    // 基本属性
    QString name() const { return m_name; }
    void setName(const QString& name);

    QString description() const { return m_description; }
    void setDescription(const QString& description);

    QString category() const { return m_category; }
    void setCategory(const QString& category);

    // 模型文件信息
    QString modelFilePath() const { return m_modelFilePath; }
    void setModelFilePath(const QString& path);

    qint64 modelFileSize() const { return m_modelFileSize; }
    void setModelFileSize(qint64 size);

    QString modelFileHash() const { return m_modelFileHash; }
    void setModelFileHash(const QString& hash);

    // 几何信息
    QVector3D dimensions() const { return m_dimensions; }
    void setDimensions(const QVector3D& dimensions);

    QString material() const { return m_material; }
    void setMaterial(const QString& material);

    double surfaceArea() const { return m_surfaceArea; }
    void setSurfaceArea(double area);

    double complexityScore() const { return m_complexityScore; }
    void setComplexityScore(double score);

    // 创建者
    int createdBy() const { return m_createdBy; }
    void setCreatedBy(int userId);

    // 重写基类方法
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& json) override;
    bool isValid() const override;
    QStringList validationErrors() const override;
    BaseModel* clone() const override;

    // 工件特有方法
    double getVolume() const;
    QString getDisplayName() const;
    bool hasModelFile() const;
    QString getModelFileExtension() const;

signals:
    void nameChanged();
    void descriptionChanged();
    void categoryChanged();
    void modelFilePathChanged();
    void modelFileSizeChanged();
    void modelFileHashChanged();
    void dimensionsChanged();
    void materialChanged();
    void surfaceAreaChanged();
    void complexityScoreChanged();
    void createdByChanged();

private:
    QString m_name;
    QString m_description;
    QString m_category;
    QString m_modelFilePath;
    qint64 m_modelFileSize;
    QString m_modelFileHash;
    QVector3D m_dimensions;
    QString m_material;
    double m_surfaceArea;
    double m_complexityScore;
    int m_createdBy;
};

} // namespace Data

#endif // WORKPIECEDATA_H