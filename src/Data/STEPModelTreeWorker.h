#pragma once

#include <QObject>
#include <QThread>
#include <QString>
#include <memory>

class STEPModelTree;
struct STEPTreeNode;

/**
 * @brief STEP模型树异步加载工作线程
 * 
 * 在后台线程中加载和解析STEP文件，避免UI卡顿
 */
class STEPModelTreeWorker : public QObject
{
    Q_OBJECT
    
public:
    explicit STEPModelTreeWorker(QObject* parent = nullptr);
    ~STEPModelTreeWorker();

public slots:
    /**
     * @brief 异步加载STEP文件
     * @param filePath STEP文件路径
     */
    void loadSTEPFile(const QString& filePath);

signals:
    /**
     * @brief 加载进度更新信号
     * @param progress 进度百分比 (0-100)
     * @param message 当前操作描述
     */
    void progressUpdate(int progress, const QString& message);

    /**
     * @brief 模型树加载完成信号
     * @param success 是否成功
     * @param message 消息
     * @param rootNode 根节点（成功时）
     */
    void modelTreeLoaded(bool success, const QString& message, std::shared_ptr<STEPTreeNode> rootNode);

    /**
     * @brief 加载失败信号
     * @param error 错误信息
     */
    void loadFailed(const QString& error);

private:
    STEPModelTree* m_modelTree;
    bool m_isLoading;
};