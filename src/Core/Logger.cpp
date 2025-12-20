#include "Logger.h"
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QMutexLocker>
#include <QDebug>

namespace Core {

Logger::Logger(QObject *parent)
    : QObject(parent)
    , m_logFile(nullptr)
    , m_logStream(nullptr)
    , m_logLevel(LogLevel::Info)
{
}

Logger::~Logger()
{
    if (m_logStream) {
        m_logStream->flush();
        delete m_logStream;
    }
    
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
    }
}

bool Logger::initialize()
{
    QString logPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs";
    QDir().mkpath(logPath);
    
    QString logFileName = QString("%1/spray_trajectory_%2.log")
                         .arg(logPath)
                         .arg(QDateTime::currentDateTime().toString("yyyyMMdd"));
    
    m_logFile = new QFile(logFileName, this);
    if (!m_logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
        qCritical() << "无法打开日志文件:" << logFileName;
        return false;
    }
    
    m_logStream = new QTextStream(m_logFile);
    m_logStream->setEncoding(QStringConverter::Utf8);
    
    info("日志系统初始化完成");
    return true;
}

void Logger::debug(const QString& message)
{
    writeLog(LogLevel::Debug, message);
}

void Logger::info(const QString& message)
{
    writeLog(LogLevel::Info, message);
}

void Logger::warning(const QString& message)
{
    writeLog(LogLevel::Warning, message);
}

void Logger::error(const QString& message)
{
    writeLog(LogLevel::Error, message);
}

void Logger::critical(const QString& message)
{
    writeLog(LogLevel::Critical, message);
}

void Logger::setLogLevel(LogLevel level)
{
    m_logLevel = level;
}

void Logger::writeLog(LogLevel level, const QString& message)
{
    if (level < m_logLevel) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    QString formattedMessage = formatLogMessage(level, message);
    
    // 写入文件
    if (m_logStream) {
        *m_logStream << formattedMessage << Qt::endl;
        m_logStream->flush();
    }
    
    // 输出到控制台
    qDebug().noquote() << formattedMessage;
    
    // 发送信号
    emit logMessage(level, message);
}

QString Logger::formatLogMessage(LogLevel level, const QString& message) const
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString levelStr = logLevelToString(level);
    
    return QString("[%1] [%2] %3").arg(timestamp, levelStr, message);
}

QString Logger::logLevelToString(LogLevel level) const
{
    switch (level) {
    case LogLevel::Debug:    return "DEBUG";
    case LogLevel::Info:     return "INFO ";
    case LogLevel::Warning:  return "WARN ";
    case LogLevel::Error:    return "ERROR";
    case LogLevel::Critical: return "CRIT ";
    default:                 return "UNKN ";
    }
}

} // namespace Core