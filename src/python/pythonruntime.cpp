#include "pythonruntime.h"
#include "../utils/logger.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>

PythonRuntime::PythonRuntime()
    : initialized_(false)
{
}

PythonRuntime::~PythonRuntime() = default;

bool PythonRuntime::initialize()
{
    Logger::log(Logger::Info, "Initializing Python runtime...");

    if (!locatePython()) {
        Logger::log(Logger::Error, "Failed to locate Python executable");
        return false;
    }

    if (!verifyPython()) {
        Logger::log(Logger::Error, "Failed to verify Python installation");
        return false;
    }

    initialized_ = true;
    Logger::log(Logger::Info, QString("Python runtime initialized: %1").arg(pythonExecutable_));
    Logger::log(Logger::Info, QString("Python home: %1").arg(pythonHome_));
    return true;
}

void PythonRuntime::shutdown()
{
    initialized_ = false;
    Logger::log(Logger::Info, "Python runtime shutdown");
}

bool PythonRuntime::locatePython()
{
    // First try to find embedded Python
    QString embedded = findEmbeddedPython();
    if (!embedded.isEmpty() && QFile::exists(embedded)) {
        pythonExecutable_ = embedded;
        pythonHome_ = QFileInfo(embedded).absolutePath();
        modulePath_ = pythonHome_ + "/lib";
        Logger::log(Logger::Info, "Found embedded Python: " + embedded);
        return true;
    }

    // Fallback to system Python
#ifdef Q_OS_WIN
    pythonExecutable_ = "python.exe";
#else
    pythonExecutable_ = "python3";
#endif

    Logger::log(Logger::Info, QString("Using system Python: %1").arg(pythonExecutable_));
    return true;
}

bool PythonRuntime::verifyPython()
{
    Logger::log(Logger::Info, QString("Verifying Python: %1").arg(pythonExecutable_));

    QProcess process;
    process.start(pythonExecutable_, QStringList() << "--version");

    if (!process.waitForFinished(5000)) {
        Logger::log(Logger::Error, "Python verification timed out");
        return false;
    }

    if (process.exitCode() != 0) {
        Logger::log(Logger::Error, QString("Python verification failed: %1").arg(QString::fromUtf8(process.readAllStandardError())));
        return false;
    }

    QString version = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
    Logger::log(Logger::Info, QString("Python version: %1").arg(version));
    return true;
}

QString PythonRuntime::findEmbeddedPython()
{
    // Check app bundle for embedded Python
    QString appPath = QCoreApplication::applicationDirPath();

#ifdef Q_OS_MAC
    // macOS: Look in Contents/Frameworks/Python.framework
    QString pythonPath = appPath + "/../Frameworks/Python.framework/Versions/Current/bin/python3";
    if (QFile::exists(pythonPath)) {
        return QFileInfo(pythonPath).absoluteFilePath();
    }
#elif defined(Q_OS_WIN)
    // Windows: Look in python folder
    QString pythonPath = appPath + "/python/python.exe";
    if (QFile::exists(pythonPath)) {
        return QFileInfo(pythonPath).absoluteFilePath();
    }
#endif

    return QString();
}
