#ifndef PYTHONRUNTIME_H
#define PYTHONRUNTIME_H

#include <QString>

class PythonRuntime
{
public:
    PythonRuntime();
    virtual ~PythonRuntime();

    bool initialize();
    void shutdown();

    QString pythonPath() const;

private:
    bool locatePythonFramework();
    QString pythonPath_;
};

#endif // PYTHONRUNTIME_H
