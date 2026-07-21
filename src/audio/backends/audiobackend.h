#ifndef AUDIOBACKEND_H
#define AUDIOBACKEND_H

#include <QString>
#include <cstdint>
#include <functional>
#include <vector>

class AudioBackend
{
public:
    struct AudioFormat {
        int sampleRate = 44100;
        int channels = 2;
        int bitsPerSample = 32;
    };

    struct AudioBuffer {
        std::vector<float> data;
        int sampleCount = 0;
    };

    using AudioCallback = std::function<void(AudioBuffer &)>;

    AudioBackend() = default;
    virtual ~AudioBackend() = default;

    // Lifecycle
    virtual bool initialize(const AudioFormat &format) = 0;
    virtual void shutdown() = 0;
    virtual bool isInitialized() const = 0;

    // Playback control
    virtual bool play() = 0;
    virtual bool pause() = 0;
    virtual bool stop() = 0;
    virtual bool isPlaying() const = 0;

    // Seek & position
    virtual bool seek(int64_t samplePosition) = 0;
    virtual int64_t currentPosition() const = 0;
    virtual int64_t totalSamples() const = 0;

    // Audio callback
    virtual void setAudioCallback(AudioCallback callback) = 0;

    // Device info
    virtual QString deviceName() const = 0;
    virtual int bufferSize() const = 0;
    virtual AudioFormat format() const = 0;
};

#endif // AUDIOBACKEND_H
