#ifndef CORE_LOGGER_H
#define CORE_LOGGER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>

namespace Core {

enum class LogLevel {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3,
    Critical = 4
};

/**
 * @brief 日志记录器
 * 
 * 提供线程安全的日志记录功能
 */
class Logger : public QObject
{
    Q_OBJECT

public:
    explicit Logger(QObject *parent = nullptr);
    ~Logger();

    bool initialize();

    // 日志记录方法
    void debug(const QString& message);
    void info(const QString& message);
    void warning(const QString& message);
    void error(const QString& message);
    void critical(const QString& message);

    // 设置日志级别
    void setLogLevel(LogLevel level);
    LogLevel logLevel() const { return m_logLevel; }

signals:
    void logMessage(LogLevel level, const QString& message);

private:
    void writeLog(LogLevel level, const QString& message);
    QString formatLogMessage(LogLevel level, const QString& message) const;
    QString logLevelToString(LogLevel level) const;

private:
    QFile* m_logFile;
    QTextStream* m_logStream;
    LogLevel m_logLevel;
    QMutex m_mutex;
};

} // namespace Core

#endif // CORE_LOGGER_H