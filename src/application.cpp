#include "application.h"
#include "mainwindow.h"
#include "python/pythonruntime.h"
#include "processing/separationengine.h"
#include "audio/audioengine.h"
#include "utils/logger.h"

CarlosOkupaApplication::CarlosOkupaApplication()
    : mainWindow(nullptr)
    , pythonRuntime(nullptr)
    , separationEngine(nullptr)
    , audioEngine(nullptr)
{
}

CarlosOkupaApplication::~CarlosOkupaApplication() = default;

void CarlosOkupaApplication::initialize()
{
    // Initialize logger
    Logger::instance().initialize();
    Logger::log(Logger::Info, "Initializing Carlos Okupa...");

    try {
        // Initialize Python runtime
        pythonRuntime = std::make_unique<PythonRuntime>();
        if (!pythonRuntime->initialize()) {
            Logger::log(Logger::Error, "Failed to initialize Python runtime");
            return;
        }

        // Initialize audio engine
        audioEngine = std::make_unique<AudioEngine>();
        if (!audioEngine->initialize()) {
            Logger::log(Logger::Error, "Failed to initialize audio engine");
            return;
        }

        // Initialize separation engine
        separationEngine = std::make_unique<SeparationEngine>();
        if (!separationEngine->initialize()) {
            Logger::log(Logger::Error, "Failed to initialize separation engine");
            return;
        }

        // Create main window
        mainWindow = std::make_unique<MainWindow>();
        mainWindow->setSeparationEngine(separationEngine.get());
        mainWindow->setAudioEngine(audioEngine.get());

        Logger::log(Logger::Info, "Carlos Okupa initialized successfully");
    }
    catch (const std::exception &e) {
        Logger::log(Logger::Error, QString("Initialization failed: %1").arg(e.what()));
    }
}

void CarlosOkupaApplication::show()
{
    if (mainWindow) {
        mainWindow->showMaximized();
    }
}
