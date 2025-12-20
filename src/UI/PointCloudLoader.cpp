#include "PointCloudLoader.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QFileInfo>
#include <QDebug>
#include <QApplication>

namespace UI {

PointCloudLoader::PointCloudLoader(QObject* parent)
    : QObject(parent)
    , m_workerThread(nullptr)
    , m_isLoading(false)
    , m_cancelRequested(false)
{
}

PointCloudLoader::~PointCloudLoader()
{
    qDebug() << "🗑️ PointCloudLoader析构开始";
    
    // 确保取消所有正在进行的操作
    m_cancelRequested = true;
    m_isLoading = false;
    
    // 安全清理工作线程（非阻塞方式）
    if (m_workerThread) {
        qDebug() << "🧹 清理工作线程（非阻塞）...";
        
        // 请求中断
        m_workerThread->requestInterruption();
        
        // 断开所有信号连接，避免析构时的信号发送
        m_workerThread->disconnect();
        
        // 不等待线程结束，直接标记为删除
        // Qt会在线程结束后自动清理
        if (m_workerThread->isRunning()) {
            qDebug() << "⚠️ 工作线程仍在运行，将在后台自动清理";
            // 让线程在后台自然结束，不阻塞析构
            m_workerThread->deleteLater();
        } else {
            // 线程已结束，直接删除
            m_workerThread->deleteLater();
        }
        
        m_workerThread = nullptr;
        qDebug() << "✅ 工作线程清理请求已发送";
    }
    
    qDebug() << "✅ PointCloudLoader析构完成（非阻塞）";
}

void PointCloudLoader::loadPointCloudAsync(const QString& filePath)
{
    // 如果有正在运行的线程，先取消它
    if (m_isLoading || (m_workerThread && m_workerThread->isRunning())) {
        qDebug() << "🔄 检测到正在运行的任务，先取消...";
        cancelLoading();
        
        // 等待一小段时间让取消操作生效
        QApplication::processEvents();
    }

    qDebug() << "🚀 开始异步加载点云文件:" << filePath;
    qDebug() << "🧵 主线程ID:" << QThread::currentThreadId();

    m_currentFilePath = filePath;
    m_isLoading = true;
    m_cancelRequested = false;

    // 使用QThread::create创建真正的异步任务
    m_workerThread = QThread::create([this]() {
        qDebug() << "🔄 工作线程开始执行，线程ID:" << QThread::currentThreadId();
        
        // 使用局部变量避免访问成员变量
        QString filePath = m_currentFilePath;
        
        // 检查取消状态和线程中断
        if (m_cancelRequested || QThread::currentThread()->isInterruptionRequested()) {
            qDebug() << "❌ 工作线程：任务已取消或被中断";
            return;
        }

        // 在工作线程中创建解析器
        Data::PointCloudParser parser;
        Data::PointCloudData pointCloudData;
        
        // 连接进度信号（使用队列连接确保线程安全）
        connect(&parser, &Data::PointCloudParser::parseProgress, this, &PointCloudLoader::onParseProgress, Qt::QueuedConnection);
        
        // 设置取消标志的连接
        connect(this, &PointCloudLoader::cancelRequested, &parser, &Data::PointCloudParser::setCancelRequested, Qt::DirectConnection);
        
        qDebug() << "📂 开始解析文件:" << filePath;
        
        // 解析点云文件，定期检查取消状态
        Data::PointCloudParser::ParseResult result = Data::PointCloudParser::ParseError;
        
        try {
            result = parser.parseFile(filePath, pointCloudData);
        } catch (...) {
            qWarning() << "❌ 点云解析过程中发生异常";
            result = Data::PointCloudParser::ParseError;
        }
        
        qDebug() << "📊 解析结果:" << (result == Data::PointCloudParser::Success ? "成功" : "失败");
        
        // 再次检查取消状态和线程中断
        if (m_cancelRequested || QThread::currentThread()->isInterruptionRequested()) {
            qDebug() << "❌ 工作线程：解析后检查，任务已取消或被中断";
            return;
        }

        // 标记加载完成
        m_isLoading = false;

        if (result == Data::PointCloudParser::Success && pointCloudData.isValid()) {
            qDebug() << "✅ 开始创建JSON数据...";
            
            // 解析成功，创建JSON数据
            QJsonObject pointCloudJson = createPointCloudJson(pointCloudData, filePath);
            
            // 最后一次检查取消状态
            if (m_cancelRequested || QThread::currentThread()->isInterruptionRequested()) {
                qDebug() << "❌ 工作线程：JSON创建后检查，任务已取消或被中断";
                return;
            }
            
            qDebug() << "📤 发送加载完成信号";
            emit loadCompleted(true, pointCloudJson, QString());
            
            qDebug() << "异步加载完成 - 文件:" << pointCloudData.fileName 
                     << "点数:" << pointCloudData.pointCount;
        } else {
            // 解析失败
            QString errorMessage = parser.getLastError();
            
            // 检查是否因为取消而失败
            if (m_cancelRequested || QThread::currentThread()->isInterruptionRequested()) {
                qDebug() << "❌ 工作线程：因取消而失败";
                return;
            }
            
            qDebug() << "📤 发送加载失败信号:" << errorMessage;
            emit loadCompleted(false, QJsonObject(), errorMessage);
            
            qDebug() << "异步加载失败:" << errorMessage;
        }

        qDebug() << "🏁 工作线程即将退出";
    });
    
    // 连接线程完成信号（使用队列连接确保线程安全）
    connect(m_workerThread, &QThread::finished, m_workerThread, &QObject::deleteLater, Qt::QueuedConnection);
    connect(m_workerThread, &QThread::finished, this, [this]() {
        qDebug() << "🧹 工作线程已清理";
        m_workerThread = nullptr;
        m_isLoading = false;
    }, Qt::QueuedConnection);

    // 启动线程
    qDebug() << "▶️ 启动工作线程...";
    m_workerThread->start();
}

void PointCloudLoader::cancelLoading()
{
    if (!m_isLoading && !m_workerThread) {
        return;
    }

    qDebug() << "🛑 开始取消点云加载...";
    m_cancelRequested = true;
    m_isLoading = false;
    
    // 发送取消信号给解析器
    emit cancelRequested(true);
    
    if (m_workerThread && m_workerThread->isRunning()) {
        qDebug() << "🚀 请求工作线程中断（非阻塞）...";
        
        // 请求中断，但不等待
        m_workerThread->requestInterruption();
        
        // 不等待线程结束，让它在后台自然退出
        // 线程会在finished信号中自动清理
        qDebug() << "✅ 中断请求已发送，线程将在后台退出";
    }

    // 立即发送取消信号，不等待线程结束
    emit loadCanceled();
    
    qDebug() << "✅ 点云加载已取消（非阻塞）";
}

// doLoadWork方法已移除，逻辑合并到loadPointCloudAsync中

void PointCloudLoader::onParseProgress(int progress)
{
    if (!m_cancelRequested) {
        emit loadProgress(progress);
        qDebug() << "加载进度:" << progress << "%";
    }
}

QJsonObject PointCloudLoader::createPointCloudJson(const Data::PointCloudData& pointCloudData, const QString& filePath)
{
    QJsonObject workpieceJson;
    workpieceJson["type"] = "pointcloud";
    
    // 使用安全的文件名（避免编码问题）
    QString safeFileName = QFileInfo(filePath).baseName();
    workpieceJson["fileName"] = safeFileName;
    workpieceJson["format"] = pointCloudData.format;
    workpieceJson["pointCount"] = pointCloudData.pointCount;
    workpieceJson["fileSize"] = pointCloudData.fileSize;
    
    // 边界框信息
    QJsonArray bboxMin, bboxMax;
    bboxMin.append(pointCloudData.boundingBoxMin.x());
    bboxMin.append(pointCloudData.boundingBoxMin.y());
    bboxMin.append(pointCloudData.boundingBoxMin.z());
    bboxMax.append(pointCloudData.boundingBoxMax.x());
    bboxMax.append(pointCloudData.boundingBoxMax.y());
    bboxMax.append(pointCloudData.boundingBoxMax.z());
    workpieceJson["boundingBoxMin"] = bboxMin;
    workpieceJson["boundingBoxMax"] = bboxMax;
    
    // 智能采样策略：根据工件大小和点云密度优化传输
    QJsonArray pointsArray;
    
    // 计算工件尺寸
    QVector3D size = pointCloudData.boundingBoxMax - pointCloudData.boundingBoxMin;
    float maxDimension = qMax(qMax(size.x(), size.y()), size.z());
    
    // 根据工件大小和点数动态调整传输点数（激进优化）
    int maxPoints;
    if (pointCloudData.pointCount > 500000) {
        maxPoints = 8000;  // 超大型工件：8000个点（82万->8千，大幅降采样）
    } else if (pointCloudData.pointCount > 100000) {
        maxPoints = 12000; // 大型工件：12000个点
    } else if (maxDimension > 1000.0f) {
        maxPoints = 18000; // 大型工件：18000个点
    } else if (maxDimension > 500.0f) {
        maxPoints = 15000; // 中型工件：15000个点
    } else {
        maxPoints = 12000; // 小型工件：12000个点
    }
    
    int sampleStep = qMax(1, pointCloudData.pointCount / maxPoints);
    
    qDebug() << "智能采样策略:";
    qDebug() << "  - 工件尺寸:" << size.x() << "x" << size.y() << "x" << size.z();
    qDebug() << "  - 最大维度:" << maxDimension;
    qDebug() << "  - 总点数:" << pointCloudData.points.size();
    qDebug() << "  - 目标点数:" << maxPoints;
    qDebug() << "  - 采样步长:" << sampleStep;
    
    // 确保有有效的点数据
    if (!pointCloudData.points.isEmpty()) {
        // 计算预期点数（QJsonArray不支持reserve）
        int expectedPointCount = qMin(pointCloudData.points.size() / sampleStep, maxPoints);
        
        int actualPointCount = 0;
        for (int i = 0; i < pointCloudData.points.size() && actualPointCount < maxPoints; i += sampleStep) {
            if (m_cancelRequested || i >= pointCloudData.points.size()) {
                break;
            }
            
            const QVector3D& point = pointCloudData.points[i];
            
            // 验证点的有效性
            if (std::isfinite(point.x()) && std::isfinite(point.y()) && std::isfinite(point.z())) {
                // 添加点到数组格式：[x1,y1,z1,x2,y2,z2,...]
                pointsArray.append(point.x());
                pointsArray.append(point.y());
                pointsArray.append(point.z());
                actualPointCount++;
            }
        }
        
        qDebug() << "采样完成 - 有效点数:" << actualPointCount << "数组大小:" << pointsArray.size();
    }
    
    workpieceJson["points"] = pointsArray;
    workpieceJson["sampleStep"] = sampleStep;
    workpieceJson["actualPointsSent"] = pointsArray.size() / 3;
    
    // 添加调试信息
    QJsonDocument debugDoc(workpieceJson);
    QByteArray jsonData = debugDoc.toJson(QJsonDocument::Compact);
    qDebug() << "JSON数据大小:" << jsonData.size() << "字节";
    
    return workpieceJson;
}

} // namespace UI

// MOC文件会自动生成，不需要手动包含