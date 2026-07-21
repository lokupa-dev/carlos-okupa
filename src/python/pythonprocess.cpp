#include "pythonprocess.h"
#include "../utils/logger.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QProcess>
#include <signal.h>
#include <thread>
#include <chrono>

PythonProcess::PythonProcess(const QString &pythonPath, const QString &scriptPath)
    : pythonPath_(pythonPath)
    , scriptPath_(scriptPath)
{
    process_ = std::make_unique<QProcess>();
    connect(process_.get(), &QProcess::readyReadStandardOutput, this, &PythonProcess::onReadyReadStandardOutput);
    connect(process_.get(), &QProcess::readyReadStandardError, this, &PythonProcess::onReadyReadStandardError);
    connect(process_.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &PythonProcess::onProcessFinished);
    connect(process_.get(), QOverload<QProcess::ProcessError>::of(&QProcess::error),
            this, &PythonProcess::onProcessError);
}

PythonProcess::~PythonProcess()
{
    if (process_ && process_->state() == QProcess::Running) {
        stop();
    }
}

bool PythonProcess::start(const QStringList &arguments)
{
    if (isRunning()) {
        Logger::log(Logger::Warning, "Python process already running");
        return false;
    }

    Logger::log(Logger::Info, QString("Starting Python: %1 %2 %3").arg(pythonPath_, scriptPath_).arg(arguments.join(" ")));

    QStringList args;
    args << scriptPath_ << arguments;

    process_->start(pythonPath_, args);

    if (!process_->waitForStarted()) {
        Logger::log(Logger::Error, QString("Failed to start Python process: %1").arg(process_->errorString()));
        return false;
    }

    Logger::log(Logger::Info, "Python process started successfully");
    return true;
}

void PythonProcess::stop()
{
    if (!isRunning()) {
        return;
    }

    Logger::log(Logger::Info, "Stopping Python process...");

    // Try graceful termination first
#ifdef Q_OS_WIN
    process_->terminate();
#else
    // Send SIGTERM
    if (process_->processId() > 0) {
        ::kill(process_->processId(), SIGTERM);
    }
#endif

    // Wait up to 5 seconds
    if (!process_->waitForFinished(5000)) {
        Logger::log(Logger::Warning, "Python process did not terminate gracefully, killing...");
        process_->kill();
        process_->waitForFinished(2000);
    }

    Logger::log(Logger::Info, "Python process stopped");
}

bool PythonProcess::isRunning() const
{
    return process_ && process_->state() == QProcess::Running;
}

void PythonProcess::onReadyReadStandardOutput()
{
    QByteArray data = process_->readAllStandardOutput();
    outputBuffer_ += QString::fromUtf8(data);

    // Process complete lines
    int newlineIndex;
    while ((newlineIndex = outputBuffer_.indexOf('\n')) != -1) {
        QString line = outputBuffer_.left(newlineIndex).trimmed();
        outputBuffer_.remove(0, newlineIndex + 1);

        if (!line.isEmpty()) {
            processOutputLine(line);
        }
    }
}

void PythonProcess::onReadyReadStandardError()
{
    QByteArray data = process_->readAllStandardError();
    QString errorMsg = QString::fromUtf8(data).trimmed();

    if (!errorMsg.isEmpty()) {
        Logger::log(Logger::Warning, QString("Python stderr: %1").arg(errorMsg));
    }
}

void PythonProcess::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Logger::log(Logger::Info, QString("Python process finished with exit code %1").arg(exitCode));

    // Process any remaining output
    QString remaining = outputBuffer_.trimmed();
    if (!remaining.isEmpty()) {
        processOutputLine(remaining);
    }

    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        emitCompleted();
    } else {
        emitError(QString("Python process exited with code %1").arg(exitCode));
    }
}

void PythonProcess::onProcessError(QProcess::ProcessError error)
{
    QString errorMsg;
    switch (error) {
    case QProcess::FailedToStart:
        errorMsg = "Failed to start Python process";
        break;
    case QProcess::Crashed:
        errorMsg = "Python process crashed";
        break;
    case QProcess::Timedout:
        errorMsg = "Python process timed out";
        break;
    case QProcess::WriteError:
        errorMsg = "Failed to write to Python process";
        break;
    case QProcess::ReadError:
        errorMsg = "Failed to read from Python process";
        break;
    default:
        errorMsg = "Unknown error";
    }

    Logger::log(Logger::Error, QString("Python process error: %1").arg(errorMsg));
    emitError(errorMsg);
}

void PythonProcess::processOutputLine(const QString &line)
{
    // Try to parse as JSON
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &parseError);

    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        Logger::log(Logger::Debug, QString("Python output: %1").arg(obj["type"].toString()));
        emitProgress(obj);
    } else {
        // Not JSON, log as regular output
        Logger::log(Logger::Debug, QString("Python: %1").arg(line));
    }
}

void PythonProcess::emitProgress(const QJsonObject &data)
{
    if (progressCallback_) {
        try {
            progressCallback_(data);
        } catch (const std::exception &e) {
            Logger::log(Logger::Error, QString("Exception in progress callback: %1").arg(e.what()));
        }
    }
}

void PythonProcess::emitError(const QString &message)
{
    if (errorCallback_) {
        try {
            errorCallback_(message);
        } catch (const std::exception &e) {
            Logger::log(Logger::Error, QString("Exception in error callback: %1").arg(e.what()));
        }
    }
}

void PythonProcess::emitCompleted()
{
    if (completedCallback_) {
        try {
            completedCallback_();
        } catch (const std::exception &e) {
            Logger::log(Logger::Error, QString("Exception in completed callback: %1").arg(e.what()));
        }
    }
}
