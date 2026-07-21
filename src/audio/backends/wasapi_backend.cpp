#ifdef Q_OS_WIN

#include "wasapi_backend.h"
#include "../../utils/logger.h"
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functiondiscoverykeys.h>
#include <cstring>
#include <thread>
#include <chrono>

#pragma comment(lib, "mmdevapi.lib")
#pragma comment(lib, "ole32.lib")

WasapiBackend::WasapiBackend()
    : audioEvent_(nullptr)
    , initialized_(false)
    , isPlaying_(false)
    , shouldExit_(false)
    , currentPosition_(0)
    , totalSamples_(0)
    , bufferFrameCount_(0)
{
}

WasapiBackend::~WasapiBackend()
{
    if (initialized_) {
        shutdown();
    }
}

bool WasapiBackend::initialize(const AudioFormat &format)
{
    Logger::log(Logger::Info, "Initializing WASAPI backend...");

    format_ = format;
    mixBuffer_.resize(format_.channels * 4096, 0.0f);

    if (!initializeWASAPI()) {
        Logger::log(Logger::Error, "Failed to initialize WASAPI");
        return false;
    }

    initialized_ = true;
    Logger::log(Logger::Info, QString("WASAPI initialized: %1 Hz, %2 channels").arg(format_.sampleRate).arg(format_.channels));
    return true;
}

void WasapiBackend::shutdown()
{
    if (isPlaying_) {
        stop();
    }
    cleanupWASAPI();
    initialized_ = false;
    Logger::log(Logger::Info, "WASAPI backend shutdown");
}

bool WasapiBackend::initializeWASAPI()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        Logger::log(Logger::Error, "CoInitializeEx failed");
        return false;
    }

    // Create device enumerator
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
                         nullptr,
                         CLSCTX_ALL,
                         __uuidof(IMMDeviceEnumerator),
                         (void **)deviceEnumerator_.GetAddressOf());
    if (FAILED(hr)) {
        Logger::log(Logger::Error, "Failed to create device enumerator");
        return false;
    }

    // Get default audio endpoint
    hr = deviceEnumerator_->GetDefaultAudioEndpoint(eRender, eConsole, device_.GetAddressOf());
    if (FAILED(hr)) {
        Logger::log(Logger::Error, "Failed to get default audio endpoint");
        return false;
    }

    // Activate audio client
    hr = device_->Activate(__uuidof(IAudioClient),
                          CLSCTX_ALL,
                          nullptr,
                          (void **)audioClient_.GetAddressOf());
    if (FAILED(hr)) {
        Logger::log(Logger::Error, "Failed to activate audio client");
        return false;
    }

    // Create WAVEFORMATEX structure
    WAVEFORMATEX waveFormat = {};
    waveFormat.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    waveFormat.nChannels = format_.channels;
    waveFormat.nSamplesPerSec = format_.sampleRate;
    waveFormat.wBitsPerSample = 32;
    waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.cbSize = 0;

    // Initialize audio client
    hr = audioClient_->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                 AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                                 10000000,  // 1 second
                                 0,
                                 &waveFormat,
                                 nullptr);
    if (FAILED(hr)) {
        Logger::log(Logger::Error, QString("IAudioClient::Initialize failed: %1").arg(static_cast<int>(hr)));
        return false;
    }

    // Get buffer size
    hr = audioClient_->GetBufferSize(&bufferFrameCount_);
    if (FAILED(hr)) {
        Logger::log(Logger::Error, "Failed to get buffer size");
        return false;
    }

    bufferSize_ = static_cast<int>(bufferFrameCount_);
    Logger::log(Logger::Info, QString("WASAPI buffer size: %1 frames").arg(bufferFrameCount_));

    // Get render client
    hr = audioClient_->GetService(__uuidof(IAudioRenderClient),
                                 (void **)renderClient_.GetAddressOf());
    if (FAILED(hr)) {
        Logger::log(Logger::Error, "Failed to get render client");
        return false;
    }

    // Create event for synchronization
    audioEvent_ = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
    if (!audioEvent_) {
        Logger::log(Logger::Error, "Failed to create audio event");
        return false;
    }

    hr = audioClient_->SetEventHandle(audioEvent_);
    if (FAILED(hr)) {
        Logger::log(Logger::Error, "Failed to set event handle");
        return false;
    }

    Logger::log(Logger::Info, "WASAPI initialized successfully");
    return true;
}

void WasapiBackend::cleanupWASAPI()
{
    if (audioClient_) {
        audioClient_->Stop();
    }

    if (audioEvent_) {
        CloseHandle(audioEvent_);
        audioEvent_ = nullptr;
    }

    if (thread_ && thread_->joinable()) {
        shouldExit_ = true;
        thread_->join();
    }

    renderClient_.Reset();
    audioClient_.Reset();
    device_.Reset();
    deviceEnumerator_.Reset();

    CoUninitialize();
}

bool WasapiBackend::play()
{
    if (!initialized_ || !audioClient_) {
        return false;
    }

    HRESULT hr = audioClient_->Start();
    if (SUCCEEDED(hr)) {
        isPlaying_ = true;

        // Start audio thread
        shouldExit_ = false;
        thread_ = std::make_unique<std::thread>(&WasapiBackend::audioThread, this);

        Logger::log(Logger::Info, "WASAPI audio playback started");
        return true;
    }

    Logger::log(Logger::Error, QString("IAudioClient::Start failed: %1").arg(static_cast<int>(hr)));
    return false;
}

bool WasapiBackend::pause()
{
    if (!initialized_ || !audioClient_) {
        return false;
    }

    HRESULT hr = audioClient_->Stop();
    if (SUCCEEDED(hr)) {
        isPlaying_ = false;

        if (thread_ && thread_->joinable()) {
            shouldExit_ = true;
            thread_->join();
            thread_ = nullptr;
        }

        Logger::log(Logger::Info, "WASAPI audio playback paused");
        return true;
    }

    return false;
}

bool WasapiBackend::stop()
{
    return pause();
}

bool WasapiBackend::seek(int64_t samplePosition)
{
    if (samplePosition < 0 || samplePosition > totalSamples_) {
        return false;
    }

    currentPosition_ = samplePosition;
    Logger::log(Logger::Debug, QString("Seek to sample %1").arg(samplePosition));
    return true;
}

void WasapiBackend::audioThread()
{
    while (!shouldExit_) {
        DWORD waitResult = WaitForSingleObject(audioEvent_, 100);

        if (waitResult == WAIT_OBJECT_0 && isPlaying_) {
            UINT32 paddingFrames = 0;
            HRESULT hr = audioClient_->GetCurrentPadding(&paddingFrames);

            if (SUCCEEDED(hr)) {
                UINT32 availableFrames = bufferFrameCount_ - paddingFrames;

                if (availableFrames > 0) {
                    BYTE *pData = nullptr;
                    hr = renderClient_->GetBuffer(availableFrames, &pData);

                    if (SUCCEEDED(hr)) {
                        auto *floatData = reinterpret_cast<float *>(pData);

                        // Call audio callback
                        AudioBuffer audioBuffer;
                        audioBuffer.sampleCount = availableFrames;
                        audioBuffer.data.resize(availableFrames * format_.channels, 0.0f);

                        if (audioCallback_) {
                            try {
                                audioCallback_(audioBuffer);
                            } catch (...) {
                                Logger::log(Logger::Error, "Exception in audio callback");
                            }
                        }

                        // Copy to output
                        std::memcpy(floatData, audioBuffer.data.data(), audioBuffer.data.size() * sizeof(float));

                        renderClient_->ReleaseBuffer(availableFrames, 0);
                        currentPosition_ += availableFrames;
                    }
                }
            }
        }
    }
}

QString WasapiBackend::deviceName() const
{
    return QString("System Output (WASAPI)");
}

#endif // Q_OS_WIN
