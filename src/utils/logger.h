#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <memory>

class Logger
{
public:
    enum LogLevel {
        Debug,
        Info,
        Warning,
        Error
    };

    static Logger &instance();

    void initialize();
    static void log(LogLevel level, const QString &message);

private:
    Logger();
    ~Logger();

    void logInternal(LogLevel level, const QString &message);
    QString levelToString(LogLevel level) const;
};

#endif // LOGGER_H
