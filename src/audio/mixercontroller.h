#ifndef MIXERCONTROLLER_H
#define MIXERCONTROLLER_H

#include <QString>
#include <memory>
#include <vector>
#include <functional>

class RealtimeMixer;
class AudioEngine;

class MixerController
{
public:
    struct TrackInfo {
        int id = -1;
        QString name;
        bool isLoaded = false;
        float volume = 1.0f;
        bool muted = false;
        bool solo = false;
    };

    using TrackListChangedCallback = std::function<void()>;
    using TrackStateChangedCallback = std::function<void(int trackId)>;
    using PlaybackStateChangedCallback = std::function<void(bool isPlaying)>;
    using PositionChangedCallback = std::function<void(int64_t position)>;

    MixerController();
    ~MixerController();

    // Initialization
    bool initialize(AudioEngine *audioEngine);
    void shutdown();

    // Stem loading
    bool loadStem(const QString &stemName, const QString &filePath);
    bool unloadStem(int trackId);
    void unloadAllStems();

    // Playback control
    void play();
    void pause();
    void stop();
    void seek(int64_t samplePosition);

    // Query
    std::vector<TrackInfo> getTracksInfo() const;
    int64_t currentPosition() const;
    int64_t totalDuration() const;
    bool isPlaying() const;

    // Real-time track controls
    void setTrackVolume(int trackId, float volumeLinear);
    void setTrackMute(int trackId, bool muted);
    void setTrackSolo(int trackId, bool solo);

    // Callbacks
    void setTrackListChangedCallback(TrackListChangedCallback cb) { trackListChanged_ = cb; }
    void setTrackStateChangedCallback(TrackStateChangedCallback cb) { trackStateChanged_ = cb; }
    void setPlaybackStateChangedCallback(PlaybackStateChangedCallback cb) { playbackStateChanged_ = cb; }
    void setPositionChangedCallback(PositionChangedCallback cb) { positionChanged_ = cb; }

private:
    void onAudioCallback(std::vector<float> &buffer, int sampleCount);

    std::unique_ptr<RealtimeMixer> mixer_;
    AudioEngine *audioEngine_ = nullptr;

    std::vector<int> trackIds_;
    std::vector<QString> trackNames_;
    bool isPlaying_ = false;

    TrackListChangedCallback trackListChanged_;
    TrackStateChangedCallback trackStateChanged_;
    PlaybackStateChangedCallback playbackStateChanged_;
    PositionChangedCallback positionChanged_;
};

#endif // MIXERCONTROLLER_H
