#ifndef REALTIMEMIXER_H
#define REALTIMEMIXER_H

#include "audiotrack.h"
#include <QString>
#include <vector>
#include <memory>
#include <atomic>
#include <cstdint>
#include <functional>

class RealtimeMixer
{
public:
    using MixerCallback = std::function<void(std::vector<float> &, int)>;

    RealtimeMixer(int channels = 2, int sampleRate = 44100);
    ~RealtimeMixer();

    // Track management
    int addTrack(const QString &name);
    bool removeTrack(int trackId);
    bool loadTrackAudio(int trackId, const QString &filePath);
    bool isTrackLoaded(int trackId) const;

    // Playback control (sync'd across all tracks)
    void play();
    void pause();
    void stop();
    bool isPlaying() const { return isPlaying_; }

    // Master position (sample-accurate)
    void seek(int64_t samplePosition);
    int64_t currentPosition() const { return masterPosition_; }
    int64_t totalDuration() const { return maxDuration_; }

    // Per-track controls (real-time, no audio glitches)
    void setTrackVolume(int trackId, float volumeLinear);
    void setTrackMute(int trackId, bool muted);
    void setTrackSolo(int trackId, bool solo);

    float getTrackVolume(int trackId) const;
    bool isTrackMuted(int trackId) const;
    bool isTrackSolo(int trackId) const;

    // Query
    int trackCount() const { return static_cast<int>(tracks_.size()); }
    const QString &trackName(int trackId) const;

    // Audio rendering (called from audio callback)
    void renderAudio(std::vector<float> &outputBuffer, int sampleCount);

private:
    void updateMasterPosition(int samplesProcessed);
    void calculateSoloState();
    bool shouldTrackPlay(int trackId) const;

    struct TrackInfo {
        std::unique_ptr<AudioTrack> track;
        int id = -1;
        bool soloActive = false;
    };

    std::vector<TrackInfo> tracks_;
    int nextTrackId_ = 0;

    int channels_;
    int sampleRate_;

    std::atomic<bool> isPlaying_ = false;
    std::atomic<int64_t> masterPosition_ = 0;
    int64_t maxDuration_ = 0;

    bool anySoloActive_ = false;
};

#endif // REALTIMEMIXER_H
