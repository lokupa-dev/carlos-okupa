#ifndef DEMUCSWRAPPER_H
#define DEMUCSWRAPPER_H

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <memory>
#include <functional>

class PythonProcess;

class DemucsWrapper
{
public:
    struct SeparationProgress {
        int percent = 0;
        QString stage;
        QString model;
        int currentChunk = 0;
        int totalChunks = 0;
    };

    using ProgressCallback = std::function<void(const SeparationProgress &)>;
    using ErrorCallback = std::function<void(const QString &)>;
    using CompletedCallback = std::function<void(const QString &)>; // Output folder path

    DemucsWrapper(const QString &pythonPath, const QString &wrapperScriptPath);
    ~DemucsWrapper();

    bool separate(const QString &inputFile,
                 const QStringList &stems,
                 const QString &outputFolder);
    void cancel();
    bool isRunning() const;

    void setProgressCallback(ProgressCallback callback) { progressCallback_ = callback; }
    void setErrorCallback(ErrorCallback callback) { errorCallback_ = callback; }
    void setCompletedCallback(CompletedCallback callback) { completedCallback_ = callback; }

private:
    void onProgress(const QJsonObject &data);
    void onError(const QString &message);
    void onCompleted();

    std::unique_ptr<PythonProcess> process_;
    QString pythonPath_;
    QString wrapperScriptPath_;

    ProgressCallback progressCallback_;
    ErrorCallback errorCallback_;
    CompletedCallback completedCallback_;
};

#endif // DEMUCSWRAPPER_H
