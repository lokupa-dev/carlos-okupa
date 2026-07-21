#ifndef PYTHONPROCESS_H
#define PYTHONPROCESS_H

#include <QString>
#include <QProcess>
#include <QJsonObject>
#include <memory>
#include <functional>

class PythonProcess : public QObject
{
    Q_OBJECT

public:
    using ProgressCallback = std::function<void(const QJsonObject &)>;
    using ErrorCallback = std::function<void(const QString &)>;
    using CompletedCallback = std::function<void()>;

    explicit PythonProcess(const QString &pythonPath, const QString &scriptPath);
    ~PythonProcess();

    bool start(const QStringList &arguments);
    void stop();
    bool isRunning() const;

    void setProgressCallback(ProgressCallback callback) { progressCallback_ = callback; }
    void setErrorCallback(ErrorCallback callback) { errorCallback_ = callback; }
    void setCompletedCallback(CompletedCallback callback) { completedCallback_ = callback; }

private slots:
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);

private:
    void processOutputLine(const QString &line);
    void emitProgress(const QJsonObject &data);
    void emitError(const QString &message);
    void emitCompleted();

    std::unique_ptr<QProcess> process_;
    QString pythonPath_;
    QString scriptPath_;
    QString outputBuffer_;

    ProgressCallback progressCallback_;
    ErrorCallback errorCallback_;
    CompletedCallback completedCallback_;
};

#endif // PYTHONPROCESS_H
