#ifndef AUDIOTRACK_H
#define AUDIOTRACK_H

#include <QString>
#include <vector>
#include <atomic>
#include <cstdint>

class AudioTrack
{
public:
    AudioTrack(const QString &name, int channels, int sampleRate);
    ~AudioTrack();

    // File loading
    bool loadFromFile(const QString &filePath);
    void unload();
    bool isLoaded() const { return !audioData_.empty(); }

    // Playback position
    void setPosition(int64_t samplePosition);
    int64_t position() const { return currentPosition_; }
    int64_t duration() const { return totalSamples_; }

    // Rendering
    void renderSamples(std::vector<float> &outputBuffer, int sampleCount);

    // Real-time controls (no audio glitches)
    void setVolume(float volumeLinear);  // 0.0 to 2.0
    void setMute(bool muted);
    void setSolo(bool solo);

    float volume() const { return targetVolume_; }
    bool isMuted() const { return muted_; }
    bool isSolo() const { return solo_; }

    // Info
    const QString &name() const { return name_; }
    int channels() const { return channels_; }
    int sampleRate() const { return sampleRate_; }

private:
    void applyGainRamp(std::vector<float> &buffer, int sampleCount);
    void mixSamplesToBuffer(const std::vector<float> &trackSamples,
                           std::vector<float> &outputBuffer,
                           int sampleCount);

    QString name_;
    int channels_;
    int sampleRate_;

    // Audio data
    std::vector<float> audioData_;  // Interleaved samples
    int64_t totalSamples_ = 0;
    std::atomic<int64_t> currentPosition_ = 0;

    // Volume control with ramping
    float targetVolume_ = 1.0f;
    float currentVolume_ = 1.0f;
    static constexpr float VolumeRampTime = 0.005f;  // 5ms ramp

    // Mute/Solo
    std::atomic<bool> muted_ = false;
    std::atomic<bool> solo_ = false;
};

#endif // AUDIOTRACK_H
