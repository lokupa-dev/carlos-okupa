#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

class SeparationEngine;
class AudioEngine;
class FileImport;
class TrackSelector;
class FolderChooser;
class Mixer;
class Timeline;
class ProgressDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setSeparationEngine(SeparationEngine *engine);
    void setAudioEngine(AudioEngine *engine);

private slots:
    void onSeparate();
    void onExportMix();
    void onOpenFolder();
    void onSeparationProgress(int percent, const QString &message);
    void onSeparationComplete();
    void onSeparationError(const QString &error);

private:
    void setupUI();
    void setupConnections();

    SeparationEngine *separationEngine = nullptr;
    AudioEngine *audioEngine = nullptr;

    std::unique_ptr<FileImport> fileImport;
    std::unique_ptr<TrackSelector> trackSelector;
    std::unique_ptr<FolderChooser> folderChooser;
    std::unique_ptr<Mixer> mixer;
    std::unique_ptr<Timeline> timeline;
    std::unique_ptr<ProgressDialog> progressDialog;
};

#endif // MAINWINDOW_H
