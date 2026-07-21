#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <memory>

class MainWindow;
class PythonRuntime;
class SeparationEngine;
class AudioEngine;

class CarlosOkupaApplication : public QObject
{
    Q_OBJECT

public:
    CarlosOkupaApplication();
    ~CarlosOkupaApplication();

    bool initialize();
    void show();
    void shutdown();

private:
    void showError(const QString &title, const QString &message);

    std::unique_ptr<MainWindow> mainWindow;
    std::unique_ptr<PythonRuntime> pythonRuntime;
    std::unique_ptr<SeparationEngine> separationEngine;
    std::unique_ptr<AudioEngine> audioEngine;
};

#endif // APPLICATION_H
