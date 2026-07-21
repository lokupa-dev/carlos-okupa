#include "application.h"
#include "mainwindow.h"
#include "python/pythonruntime.h"
#include "processing/separationengine.h"
#include "audio/audioengine.h"
#include "utils/logger.h"
#include <QApplication>
#include <QMessageBox>

CarlosOkupaApplication::CarlosOkupaApplication()
    : mainWindow(nullptr)
    , pythonRuntime(nullptr)
    , separationEngine(nullptr)
    , audioEngine(nullptr)
{
}

CarlosOkupaApplication::~CarlosOkupaApplication() = default;

bool CarlosOkupaApplication::initialize()
{
    Logger::log(Logger::Info, "===========================================");
    Logger::log(Logger::Info, "  Carlos Okupa v0.1.0 - Starting");
    Logger::log(Logger::Info, "===========================================");

    try {
        // Initialize logger
        Logger::instance().initialize();

        // Initialize Python runtime
        Logger::log(Logger::Info, "Step 1/4: Initializing Python runtime...");
        pythonRuntime = std::make_unique<PythonRuntime>();
        if (!pythonRuntime->initialize()) {
            Logger::log(Logger::Error, "Failed to initialize Python runtime");
            showError("Initialization Error", "Failed to initialize Python runtime.");
            return false;
        }
        Logger::log(Logger::Info, "✓ Python runtime ready");

        // Initialize audio engine
        Logger::log(Logger::Info, "Step 2/4: Initializing audio engine...");
        audioEngine = std::make_unique<AudioEngine>();
        if (!audioEngine->initialize()) {
            Logger::log(Logger::Error, "Failed to initialize audio engine");
            showError("Initialization Error", "Failed to initialize audio engine.");
            return false;
        }
        Logger::log(Logger::Info, QString("✓ Audio engine ready: %1 Hz, %2 channels").arg(audioEngine->sampleRate()).arg(audioEngine->channels()));
        Logger::log(Logger::Info, QString("  Device: %1").arg(audioEngine->deviceName()));

        // Initialize separation engine
        Logger::log(Logger::Info, "Step 3/4: Initializing separation engine...");
        separationEngine = std::make_unique<SeparationEngine>();
        if (!separationEngine->initialize()) {
            Logger::log(Logger::Error, "Failed to initialize separation engine");
            showError("Initialization Error", "Failed to initialize separation engine.");
            return false;
        }
        Logger::log(Logger::Info, "✓ Separation engine ready");

        // Create main window
        Logger::log(Logger::Info, "Step 4/4: Creating user interface...");
        mainWindow = std::make_unique<MainWindow>();
        mainWindow->setSeparationEngine(separationEngine.get());
        mainWindow->setAudioEngine(audioEngine.get());
        Logger::log(Logger::Info, "✓ User interface ready");

        Logger::log(Logger::Info, "===========================================");
        Logger::log(Logger::Info, "  Initialization Complete!");
        Logger::log(Logger::Info, "===========================================");
        return true;

    } catch (const std::exception &e) {
        Logger::log(Logger::Error, QString("Initialization failed: %1").arg(e.what()));
        showError("Initialization Error", QString("Initialization failed: %1").arg(e.what()));
        return false;
    }
}

void CarlosOkupaApplication::show()
{
    if (mainWindow) {
        mainWindow->showMaximized();
    }
}

void CarlosOkupaApplication::shutdown()
{
    Logger::log(Logger::Info, "Shutting down...");

    if (audioEngine) {
        audioEngine->shutdown();
        audioEngine = nullptr;
    }

    if (separationEngine) {
        separationEngine->shutdown();
        separationEngine = nullptr;
    }

    if (pythonRuntime) {
        pythonRuntime->shutdown();
        pythonRuntime = nullptr;
    }

    mainWindow = nullptr;

    Logger::log(Logger::Info, "Shutdown complete");
}

void CarlosOkupaApplication::showError(const QString &title, const QString &message)
{
    QMessageBox::critical(nullptr, title, message);
}
