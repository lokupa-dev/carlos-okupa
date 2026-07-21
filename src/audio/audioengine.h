#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H

#include "backends/audiobackend.h"
#include <QString>
#include <memory>
#include <cstdint>

class AudioEngineImpl;

class AudioEngine
{
public:
    AudioEngine();
    virtual ~AudioEngine();

    bool initialize();
    void shutdown();

    bool play();
    bool pause();
    bool stop();
    bool isPlaying() const;

    bool seek(int64_t samplePosition);
    int64_t currentPosition() const;
    int64_t totalSamples() const;

    void setAudioCallback(AudioBackend::AudioCallback callback);

    int sampleRate() const { return sampleRate_; }
    int channels() const { return channels_; }

    QString deviceName() const;
    int bufferSize() const;

private:
    std::unique_ptr<AudioEngineImpl> impl_;
    int sampleRate_ = 44100;
    int channels_ = 2;
};

#endif // AUDIOENGINE_H
