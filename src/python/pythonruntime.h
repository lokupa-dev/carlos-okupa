#ifndef PYTHONRUNTIME_H
#define PYTHONRUNTIME_H

#include <QString>
#include <QStandardPaths>

class PythonRuntime
{
public:
    PythonRuntime();
    virtual ~PythonRuntime();

    bool initialize();
    void shutdown();
    bool isInitialized() const { return initialized_; }

    QString pythonExecutable() const { return pythonExecutable_; }
    QString pythonHome() const { return pythonHome_; }
    QString modulePath() const { return modulePath_; }

private:
    bool locatePython();
    bool verifyPython();
    QString findEmbeddedPython();

    QString pythonExecutable_;
    QString pythonHome_;
    QString modulePath_;
    bool initialized_ = false;
};

#endif // PYTHONRUNTIME_H
