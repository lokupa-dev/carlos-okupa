#include "logger.h"
#include <QDebug>
#include <QDateTime>
#include <iostream>

Logger::Logger() = default;

Logger::~Logger() = default;

Logger &Logger::instance()
{
    static Logger logger;
    return logger;
}

void Logger::initialize()
{
    logInternal(Info, "Logger initialized");
}

void Logger::log(LogLevel level, const QString &message)
{
    Logger::instance().logInternal(level, message);
}

void Logger::logInternal(LogLevel level, const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString levelStr = levelToString(level);
    QString logMessage = QString("[%1] %2: %3").arg(timestamp, levelStr, message);

    qDebug() << logMessage;
    std::cout << logMessage.toStdString() << std::endl;
}

QString Logger::levelToString(LogLevel level) const
{
    switch (level) {
    case Debug:
        return "DEBUG";
    case Info:
        return "INFO";
    case Warning:
        return "WARN";
    case Error:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}
