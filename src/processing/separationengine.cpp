#include "separationengine.h"
#include "../utils/logger.h"

SeparationEngine::SeparationEngine() = default;

SeparationEngine::~SeparationEngine() = default;

bool SeparationEngine::initialize()
{
    Logger::log(Logger::Info, "Initializing separation engine...");

    if (!verifyProvenance()) {
        Logger::log(Logger::Error, "Model provenance verification failed");
        return false;
    }

    if (!loadModel("demucs-htdemucs-v4")) {
        Logger::log(Logger::Error, "Failed to load Demucs model");
        return false;
    }

    isInitialized_ = true;
    return true;
}

void SeparationEngine::shutdown()
{
    Logger::log(Logger::Info, "Shutting down separation engine...");
    isInitialized_ = false;
}

bool SeparationEngine::startSeparation(const QString &inputFile,
                                       const QStringList &stems,
                                       const QString &outputFolder)
{
    if (!isInitialized_) {
        Logger::log(Logger::Error, "Separation engine not initialized");
        return false;
    }

    Logger::log(Logger::Info, QString("Starting separation: %1 -> %2").arg(inputFile, outputFolder));
    // TODO: Start Python subprocess for separation
    return true;
}

void SeparationEngine::cancelSeparation()
{
    Logger::log(Logger::Info, "Canceling separation...");
    // TODO: Send SIGTERM to Python subprocess
}

bool SeparationEngine::loadModel(const QString &modelId)
{
    Logger::log(Logger::Info, QString("Loading model: %1").arg(modelId));
    // TODO: Load PROVENANCE.json and verify model
    return true;
}

bool SeparationEngine::verifyProvenance()
{
    Logger::log(Logger::Info, "Verifying model provenance...");
    // TODO: Check PROVENANCE.json for licensing restrictions
    return true;
}
