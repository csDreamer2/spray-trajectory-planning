#include "STEPModelTreeWorker.h"
#include "STEPModelTree.h"

#include <QDebug>
#include <QThread>
#include <QCoreApplication>
#include <QTimer>

STEPModelTreeWorker::STEPModelTreeWorker(QObject* parent)
    : QObject(parent)
    , m_modelTree(nullptr)
    , m_isLoading(false)
{
}

STEPModelTreeWorker::~STEPModelTreeWorker()
{
    qDebug() << "STEPModelTreeWorker: 开始析构...";
    
    // 确保不在加载状态
    m_isLoading = false;
    
    // 清理模型树对象
    if (m_modelTree) {
        qDebug() << "STEPModelTreeWorker: 删除模型树对象...";
        delete m_modelTree;
        m_modelTree = nullptr;
    }
    
    qDebug() << "STEPModelTreeWorker: 析构完成";
}

void STEPModelTreeWorker::loadSTEPFile(const QString& filePath)
{
    if (m_isLoading) {
        qWarning() << "STEPModelTreeWorker: Already loading a file";
        return;
    }

    m_isLoading = true;
    
    qDebug() << "=== WORKER THREAD: Starting STEP model tree loading ===" << filePath;
    qDebug() << "Worker thread ID:" << QThread::currentThreadId();

    try {
        // 检查线程中断
        if (QThread::currentThread()->isInterruptionRequested()) {
            qDebug() << "WORKER: Thread interruption requested, stopping";
            m_isLoading = false;
            return;
        }

        emit progressUpdate(0, "初始化STEP模型树解析器...");

        // 创建新的STEPModelTree实例（在工作线程中）
        if (m_modelTree) {
            delete m_modelTree;
        }
        m_modelTree = new STEPModelTree();

        // 连接进度信号
        connect(m_modelTree, &STEPModelTree::loadProgress,
                this, &STEPModelTreeWorker::progressUpdate, Qt::DirectConnection);

        // 检查中断
        if (QThread::currentThread()->isInterruptionRequested()) {
            qDebug() << "WORKER: Thread interruption requested, stopping";
            m_isLoading = false;
            return;
        }

        emit progressUpdate(10, "开始加载STEP文件...");

        // 执行实际的加载操作
        bool success = m_modelTree->loadFromSTEPFile(filePath);

        // 检查中断
        if (QThread::currentThread()->isInterruptionRequested()) {
            qDebug() << "WORKER: Thread interruption requested after loading, stopping";
            m_isLoading = false;
            return;
        }

        if (success) {
            emit progressUpdate(100, "STEP模型树加载完成");
            
            // 获取根节点
            auto rootNode = m_modelTree->getRootNode();
            
            qDebug() << "=== WORKER THREAD: STEP model tree loading completed successfully ===";
            
            // 关键修复：确保工作线程状态稳定后再发送完成信号
            qDebug() << "STEPModelTreeWorker: 准备发送完成信号，先稳定线程状态...";
            
            // 多重状态稳定措施
            QThread::msleep(10);  // 给OpenCASCADE时间完成内部清理
            QCoreApplication::processEvents(); // 处理待处理的事件
            QThread::msleep(5);   // 再次等待
            
            // 使用QTimer延迟发送信号，确保不在OpenCASCADE调用栈中发送
            QTimer::singleShot(20, this, [this, rootNode]() {
                qDebug() << "STEPModelTreeWorker: 延迟发送完成信号";
                try {
                    emit modelTreeLoaded(true, "STEP模型树加载成功", rootNode);
                    qDebug() << "STEPModelTreeWorker: 完成信号已发送";
                } catch (const std::exception& e) {
                    qCritical() << "STEPModelTreeWorker: 发送完成信号异常:" << e.what();
                    emit loadFailed(QString("发送完成信号异常: %1").arg(e.what()));
                } catch (...) {
                    qCritical() << "STEPModelTreeWorker: 发送完成信号未知异常";
                    emit loadFailed("发送完成信号发生未知异常");
                }
            });
            
        } else {
            qDebug() << "=== WORKER THREAD: STEP model tree loading failed ===";
            
            // 失败情况也延迟发送
            QTimer::singleShot(10, this, [this]() {
                emit modelTreeLoaded(false, "STEP模型树加载失败", nullptr);
            });
        }

    } catch (const std::exception& e) {
        qCritical() << "WORKER: Exception during STEP loading:" << e.what();
        emit loadFailed(QString("STEP加载异常: %1").arg(e.what()));
    } catch (...) {
        qCritical() << "WORKER: Unknown exception during STEP loading";
        emit loadFailed("STEP加载未知错误");
    }

    m_isLoading = false;
    qDebug() << "=== WORKER THREAD: STEP model tree loading finished ===";
}