#ifdef Q_OS_WIN

#ifndef WASAPI_BACKEND_H
#define WASAPI_BACKEND_H

#include "audiobackend.h"
#include <wrl/client.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <memory>
#include <atomic>
#include <thread>

using namespace Microsoft::WRL;

class WasapiBackend : public AudioBackend
{
public:
    WasapiBackend();
    ~WasapiBackend();

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
    bool initializeWASAPI();
    void cleanupWASAPI();
    void audioThread();

    ComPtr<IMMDeviceEnumerator> deviceEnumerator_;
    ComPtr<IMMDevice> device_;
    ComPtr<IAudioClient> audioClient_;
    ComPtr<IAudioRenderClient> renderClient_;
    ComPtr<ISimpleAudioVolume> volumeControl_;

    HANDLE audioEvent_ = nullptr;
    std::unique_ptr<std::thread> thread_;
    std::atomic<bool> initialized_ = false;
    std::atomic<bool> isPlaying_ = false;
    std::atomic<bool> shouldExit_ = false;
    std::atomic<int64_t> currentPosition_ = 0;
    int64_t totalSamples_ = 0;

    AudioFormat format_;
    int bufferSize_ = 512;
    UINT32 bufferFrameCount_ = 0;
    AudioCallback audioCallback_;
    std::vector<float> mixBuffer_;
};

#endif // WASAPI_BACKEND_H

#endif // Q_OS_WIN
