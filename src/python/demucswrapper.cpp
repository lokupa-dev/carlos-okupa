#include "demucswrapper.h"
#include "pythonprocess.h"
#include "../utils/logger.h"
#include <QJsonObject>
#include <QJsonArray>

DemucsWrapper::DemucsWrapper(const QString &pythonPath, const QString &wrapperScriptPath)
    : pythonPath_(pythonPath)
    , wrapperScriptPath_(wrapperScriptPath)
{
    process_ = std::make_unique<PythonProcess>(pythonPath, wrapperScriptPath);

    process_->setProgressCallback([this](const QJsonObject &data) {
        onProgress(data);
    });

    process_->setErrorCallback([this](const QString &message) {
        onError(message);
    });

    process_->setCompletedCallback([this]() {
        onCompleted();
    });
}

DemucsWrapper::~DemucsWrapper()
{
    if (isRunning()) {
        cancel();
    }
}

bool DemucsWrapper::separate(const QString &inputFile,
                             const QStringList &stems,
                             const QString &outputFolder)
{
    if (isRunning()) {
        Logger::log(Logger::Warning, "Demucs separation already running");
        return false;
    }

    Logger::log(Logger::Info, QString("Starting Demucs separation: %1").arg(inputFile));
    Logger::log(Logger::Info, QString("Stems: %1").arg(stems.join(", ")));
    Logger::log(Logger::Info, QString("Output: %1").arg(outputFolder));

    // Build command line arguments
    QStringList args;
    args << inputFile;
    args << outputFolder;

    // Add stems as arguments
    for (const auto &stem : stems) {
        args << stem.toLower();
    }

    return process_->start(args);
}

void DemucsWrapper::cancel()
{
    if (isRunning()) {
        Logger::log(Logger::Info, "Canceling Demucs separation...");
        process_->stop();
    }
}

bool DemucsWrapper::isRunning() const
{
    return process_ && process_->isRunning();
}

void DemucsWrapper::onProgress(const QJsonObject &data)
{
    QString type = data["type"].toString();

    if (type == "progress") {
        SeparationProgress progress;
        progress.percent = data["percent"].toInt();
        progress.stage = data["stage"].toString();
        progress.model = data["model"].toString();
        progress.currentChunk = data["current_chunk"].toInt();
        progress.totalChunks = data["total_chunks"].toInt();

        Logger::log(Logger::Debug, QString("Progress: %1% - %2 (%3/%4)").arg(progress.percent).arg(progress.stage).arg(progress.currentChunk).arg(progress.totalChunks));

        if (progressCallback_) {
            progressCallback_(progress);
        }
    } else if (type == "error") {
        QString errorMsg = data["message"].toString();
        Logger::log(Logger::Error, QString("Demucs error: %1").arg(errorMsg));
        onError(errorMsg);
    } else {
        Logger::log(Logger::Debug, QString("Demucs message: %1").arg(type));
    }
}

void DemucsWrapper::onError(const QString &message)
{
    if (errorCallback_) {
        errorCallback_(message);
    }
}

void DemucsWrapper::onCompleted()
{
    Logger::log(Logger::Info, "Demucs separation completed");
    // TODO: Get output folder from Python process result
    if (completedCallback_) {
        completedCallback_(QString());
    }
}
