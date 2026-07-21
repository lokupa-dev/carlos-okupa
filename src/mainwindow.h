#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <memory>
#include <cstdint>

class SeparationEngine;
class AudioEngine;
class MixerController;
class DemucsWrapper;
class FileImport;
class TrackSelector;
class FolderChooser;
class Mixer;
class Timeline;
class ProgressDialog;
class QVBoxLayout;

namespace MixerControllerNS {
struct TrackInfo;
}

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
    void onExportStems();
    void onSeparationProgress(int percent, const QString &message);
    void onSeparationComplete(const QString &outputFolder);
    void onSeparationError(const QString &error);
    void onPlaybackStateChanged(bool isPlaying);
    void onPositionChanged(int64_t pos);
    void onAbout();
    void updateTrackUI(int trackId);

private:
    void setupUI();
    void setupConnections();
    void applyStyles();
    void populateTracksUI();
    QWidget *createTrackWidget(const class MixerController::TrackInfo &info);

    SeparationEngine *separationEngine = nullptr;
    AudioEngine *audioEngine = nullptr;
    std::unique_ptr<MixerController> mixerController;
    std::unique_ptr<DemucsWrapper> demucsWrapper;

    std::unique_ptr<FileImport> fileImport;
    std::unique_ptr<TrackSelector> trackSelector;
    std::unique_ptr<FolderChooser> folderChooser;
    std::unique_ptr<Mixer> mixer;
    std::unique_ptr<Timeline> timeline;
    std::unique_ptr<ProgressDialog> progressDialog;

    QWidget *tracksContainer = nullptr;
    QVBoxLayout *tracksContainerLayout = nullptr;
    QLabel *statusLabel = nullptr;
};

#endif // MAINWINDOW_H
