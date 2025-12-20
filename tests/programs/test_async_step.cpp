#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QDebug>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>

// OpenCASCADE includes
#include <STEPControl_Reader.hxx>
#include <TopoDS_Shape.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <IFSelect_ReturnStatus.hxx>

// 简化的异步STEP加载测试
class SimpleSTEPWorker : public QObject
{
    Q_OBJECT
    
public:
    explicit SimpleSTEPWorker(QObject* parent = nullptr) : QObject(parent) {}
    
public slots:
    void loadSTEPFile(const QString& filePath) {
        qDebug() << "=== WORKER THREAD: Starting STEP loading ===" << filePath;
        
        try {
            emit progressUpdate("Reading STEP file...");
            
            STEPControl_Reader reader;
            std::string pathStr = filePath.toStdString();
            
            qDebug() << "WORKER: Calling ReadFile...";
            IFSelect_ReturnStatus status = reader.ReadFile(pathStr.c_str());
            
            if (status != IFSelect_RetDone) {
                qDebug() << "WORKER: ReadFile failed with status:" << status;
                emit loadFailed("Failed to read STEP file");
                return;
            }
            
            qDebug() << "WORKER: ReadFile successful, transferring roots...";
            emit progressUpdate("Parsing geometry...");
            
            reader.TransferRoots();
            TopoDS_Shape shape = reader.OneShape();
            
            if (shape.IsNull()) {
                qDebug() << "WORKER: Shape is null";
                emit loadFailed("No geometry found");
                return;
            }
            
            qDebug() << "WORKER: Shape extracted, generating mesh...";
            emit progressUpdate("Generating mesh...");
            
            BRepMesh_IncrementalMesh mesher(shape, 1.0);
            
            qDebug() << "WORKER: Mesh generation completed";
            emit loadCompleted("STEP file loaded successfully");
            
        } catch (const std::exception& e) {
            qDebug() << "WORKER: Exception:" << e.what();
            emit loadFailed(QString("Exception: %1").arg(e.what()));
        }
    }
    
signals:
    void progressUpdate(const QString& message);
    void loadCompleted(const QString& message);
    void loadFailed(const QString& error);
};

class SimpleTestWidget : public QWidget
{
    Q_OBJECT
    
public:
    SimpleTestWidget(QWidget* parent = nullptr) : QWidget(parent), m_isLoading(false) {
        setupUI();
        setupWorker();
    }
    
private slots:
    void onLoadClicked() {
        if (m_isLoading) {
            m_statusLabel->setText("Already loading...");
            return;
        }
        
        QString filePath = QFileDialog::getOpenFileName(this, "Select STEP File", "", "STEP Files (*.step *.stp)");
        if (filePath.isEmpty()) return;
        
        QMutexLocker locker(&m_mutex);
        m_isLoading = true;
        
        qDebug() << "=== MAIN THREAD: Starting async load ===" << filePath;
        m_statusLabel->setText("Starting async loading...");
        
        // 发送加载请求到工作线程
        QMetaObject::invokeMethod(m_worker, "loadSTEPFile", 
                                  Qt::QueuedConnection, Q_ARG(QString, filePath));
    }
    
    void onProgressUpdate(const QString& message) {
        qDebug() << "=== PROGRESS UPDATE ===" << message;
        m_statusLabel->setText(message);
    }
    
    void onLoadCompleted(const QString& message) {
        QMutexLocker locker(&m_mutex);
        m_isLoading = false;
        
        qDebug() << "=== LOAD COMPLETED ===" << message;
        m_statusLabel->setText(message);
    }
    
    void onLoadFailed(const QString& error) {
        QMutexLocker locker(&m_mutex);
        m_isLoading = false;
        
        qDebug() << "=== LOAD FAILED ===" << error;
        m_statusLabel->setText(QString("ERROR: %1").arg(error));
    }
    
private:
    void setupUI() {
        setWindowTitle("Simple Async STEP Loading Test");
        resize(400, 200);
        
        QVBoxLayout* layout = new QVBoxLayout(this);
        
        m_loadButton = new QPushButton("Load STEP File", this);
        m_statusLabel = new QLabel("Ready to load STEP file", this);
        
        layout->addWidget(m_loadButton);
        layout->addWidget(m_statusLabel);
        
        connect(m_loadButton, &QPushButton::clicked, this, &SimpleTestWidget::onLoadClicked);
    }
    
    void setupWorker() {
        m_thread = new QThread(this);
        m_worker = new SimpleSTEPWorker();
        
        m_worker->moveToThread(m_thread);
        
        connect(m_worker, &SimpleSTEPWorker::progressUpdate,
                this, &SimpleTestWidget::onProgressUpdate, Qt::QueuedConnection);
        
        connect(m_worker, &SimpleSTEPWorker::loadCompleted,
                this, &SimpleTestWidget::onLoadCompleted, Qt::QueuedConnection);
        
        connect(m_worker, &SimpleSTEPWorker::loadFailed,
                this, &SimpleTestWidget::onLoadFailed, Qt::QueuedConnection);
        
        connect(m_thread, &QThread::finished,
                m_worker, &QObject::deleteLater);
        
        m_thread->start();
        qDebug() << "Worker thread started";
    }
    
private:
    QPushButton* m_loadButton;
    QLabel* m_statusLabel;
    QThread* m_thread;
    SimpleSTEPWorker* m_worker;
    QMutex m_mutex;
    bool m_isLoading;
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    
    SimpleTestWidget widget;
    widget.show();
    
    return app.exec();
}

#include "test_async_step.moc"