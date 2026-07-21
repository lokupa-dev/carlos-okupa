#include "audioengine.h"
#include "backends/audiobackend.h"
#include "../utils/logger.h"
#include <memory>

#ifdef Q_OS_MAC
#include "backends/coreaudio_backend.h"
#elif defined(Q_OS_WIN)
#include "backends/wasapi_backend.h"
#endif

class AudioEngineImpl
{
public:
    std::unique_ptr<AudioBackend> backend;
};

AudioEngine::AudioEngine()
    : impl_(std::make_unique<AudioEngineImpl>())
    , sampleRate_(44100)
    , channels_(2)
{
}

AudioEngine::~AudioEngine() = default;

bool AudioEngine::initialize()
{
    Logger::log(Logger::Info, "Initializing audio engine...");

#ifdef Q_OS_MAC
    impl_->backend = std::make_unique<CoreAudioBackend>();
#elif defined(Q_OS_WIN)
    impl_->backend = std::make_unique<WasapiBackend>();
#else
    Logger::log(Logger::Error, "Unsupported platform");
    return false;
#endif

    AudioBackend::AudioFormat format;
    format.sampleRate = sampleRate_;
    format.channels = channels_;
    format.bitsPerSample = 32;

    if (!impl_->backend->initialize(format)) {
        Logger::log(Logger::Error, "Failed to initialize audio backend");
        return false;
    }

    Logger::log(Logger::Info, QString("Audio engine initialized: %1 Hz, %2 channels").arg(sampleRate_).arg(channels_));
    return true;
}

void AudioEngine::shutdown()
{
    if (impl_->backend) {
        impl_->backend->shutdown();
        impl_->backend = nullptr;
    }
    Logger::log(Logger::Info, "Audio engine shutdown");
}

bool AudioEngine::play()
{
    if (!impl_->backend) return false;
    return impl_->backend->play();
}

bool AudioEngine::pause()
{
    if (!impl_->backend) return false;
    return impl_->backend->pause();
}

bool AudioEngine::stop()
{
    if (!impl_->backend) return false;
    return impl_->backend->stop();
}

bool AudioEngine::isPlaying() const
{
    if (!impl_->backend) return false;
    return impl_->backend->isPlaying();
}

bool AudioEngine::seek(int64_t samplePosition)
{
    if (!impl_->backend) return false;
    return impl_->backend->seek(samplePosition);
}

int64_t AudioEngine::currentPosition() const
{
    if (!impl_->backend) return 0;
    return impl_->backend->currentPosition();
}

int64_t AudioEngine::totalSamples() const
{
    if (!impl_->backend) return 0;
    return impl_->backend->totalSamples();
}

void AudioEngine::setAudioCallback(AudioBackend::AudioCallback callback)
{
    if (impl_->backend) {
        impl_->backend->setAudioCallback(callback);
    }
}

QString AudioEngine::deviceName() const
{
    if (!impl_->backend) return "No Device";
    return impl_->backend->deviceName();
}

int AudioEngine::bufferSize() const
{
    if (!impl_->backend) return 512;
    return impl_->backend->bufferSize();
}
