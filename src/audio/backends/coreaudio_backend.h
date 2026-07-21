#ifdef Q_OS_MAC

#ifndef COREAUDIO_BACKEND_H
#define COREAUDIO_BACKEND_H

#include "audiobackend.h"
#include <AudioToolbox/AudioToolbox.h>
#include <memory>
#include <atomic>

class CoreAudioBackend : public AudioBackend
{
public:
    CoreAudioBackend();
    ~CoreAudioBackend();

    bool initialize(const AudioFormat &format) override;
    void shutdown() override;
    bool isInitialized() const override { return initialized_; }

    bool play() override;
    bool pause() override;
    bool stop() override;
    bool isPlaying() const override { return isPlaying_; }

    bool seek(int64_t samplePosition) override;
    int64_t currentPosition() const override { return currentPosition_; }
    int64_t totalSamples() const override { return totalSamples_; }

    void setAudioCallback(AudioCallback callback) override { audioCallback_ = callback; }

    QString deviceName() const override;
    int bufferSize() const override { return bufferSize_; }
    AudioFormat format() const override { return format_; }

private:
    bool setupAudioEngine();
    void cleanupAudioEngine();

    static OSStatus renderCallback(void *inRefCon,
                                  AudioUnitRenderActionFlags *ioActionFlags,
                                  const AudioTimeStamp *inTimeStamp,
                                  UInt32 inOutputBusNumber,
                                  UInt32 inNumberFrames,
                                  AudioBufferList *ioData);

    OSStatus handleRenderCallback(AudioUnitRenderActionFlags *ioActionFlags,
                                 const AudioTimeStamp *inTimeStamp,
                                 UInt32 inOutputBusNumber,
                                 UInt32 inNumberFrames,
                                 AudioBufferList *ioData);

    AudioComponentInstance audioUnit_ = nullptr;
    AudioFormat format_;
    int bufferSize_ = 512;
    std::atomic<bool> initialized_ = false;
    std::atomic<bool> isPlaying_ = false;
    std::atomic<int64_t> currentPosition_ = 0;
    int64_t totalSamples_ = 0;
    AudioCallback audioCallback_;
    std::vector<float> mixBuffer_;
};

#endif // COREAUDIO_BACKEND_H

#endif // Q_OS_MAC
