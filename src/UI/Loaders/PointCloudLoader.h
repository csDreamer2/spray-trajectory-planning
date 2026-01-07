#pragma once

#include <QObject>
#include <QThread>
#include <QJsonObject>
#include <QString>
#include "../../Data/PointCloud/PointCloudParser.h"

namespace UI {

/**
 * @brief 异步点云加载器
 * 在后台线程中加载和处理点云文件，避免UI卡死
 */
class PointCloudLoader : public QObject
{
    Q_OBJECT

public:
    explicit PointCloudLoader(QObject* parent = nullptr);
    ~PointCloudLoader();

    /**
     * @brief 开始异步加载点云文件
     * @param filePath 文件路径
     */
    void loadPointCloudAsync(const QString& filePath);

    /**
     * @brief 取消当前加载操作
     */
    void cancelLoading();

    /**
     * @brief 检查是否正在加载
     */
    bool isLoading() const { return m_isLoading; }

signals:
    /**
     * @brief 加载进度信号
     * @param progress 进度百分比 (0-100)
     */
    void loadProgress(int progress);

    /**
     * @brief 加载完成信号
     * @param success 是否成功
     * @param pointCloudJson 点云JSON数据
     * @param errorMessage 错误信息（如果失败）
     */
    void loadCompleted(bool success, const QJsonObject& pointCloudJson, const QString& errorMessage);

    /**
     * @brief 加载被取消信号
     */
    void loadCanceled();
    
    /**
     * @brief 内部取消请求信号
     */
    void cancelRequested(bool cancel);

private slots:
    /**
     * @brief 处理解析进度
     */
    void onParseProgress(int progress);

private:
    /**
     * @brief 创建点云JSON数据
     */
    QJsonObject createPointCloudJson(const Data::PointCloudData& pointCloudData, const QString& filePath);

private:
    QThread* m_workerThread;
    QString m_currentFilePath;
    bool m_isLoading;
    bool m_cancelRequested;
};

} // namespace UI