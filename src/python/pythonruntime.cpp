#include "pythonruntime.h"
#include "../utils/logger.h"
#include <QStandardPaths>
#include <QDir>

PythonRuntime::PythonRuntime() = default;

PythonRuntime::~PythonRuntime() = default;

bool PythonRuntime::initialize()
{
    Logger::log(Logger::Info, "Initializing Python runtime...");
    return locatePythonFramework();
}

void PythonRuntime::shutdown()
{
    Logger::log(Logger::Info, "Shutting down Python runtime...");
}

bool PythonRuntime::locatePythonFramework()
{
    // TODO: Locate embedded Python (macOS: .app/Contents/Frameworks/Python.framework)
    // TODO: Locate embedded Python (Windows: .app/python/)
    Logger::log(Logger::Info, "Locating embedded Python runtime...");
    return true;
}

QString PythonRuntime::pythonPath() const
{
    return pythonPath_;
}
