#ifdef Q_OS_MAC

#include "coreaudio_backend.h"
#include "../../utils/logger.h"
#include <AudioToolbox/AudioToolbox.h>
#include <AVFoundation/AVFoundation.h>
#include <cstring>

CoreAudioBackend::CoreAudioBackend()
    : audioUnit_(nullptr)
    , initialized_(false)
    , isPlaying_(false)
    , currentPosition_(0)
    , totalSamples_(0)
{
}

CoreAudioBackend::~CoreAudioBackend()
{
    if (initialized_) {
        shutdown();
    }
}

bool CoreAudioBackend::initialize(const AudioFormat &format)
{
    Logger::log(Logger::Info, "Initializing CoreAudio backend...");

    format_ = format;
    mixBuffer_.resize(format_.channels * 4096, 0.0f);

    if (!setupAudioEngine()) {
        Logger::log(Logger::Error, "Failed to setup AudioUnit");
        return false;
    }

    initialized_ = true;
    Logger::log(Logger::Info, QString("CoreAudio initialized: %1 Hz, %2 channels").arg(format_.sampleRate).arg(format_.channels));
    return true;
}

void CoreAudioBackend::shutdown()
{
    if (isPlaying_) {
        stop();
    }
    cleanupAudioEngine();
    initialized_ = false;
    Logger::log(Logger::Info, "CoreAudio backend shutdown");
}

bool CoreAudioBackend::setupAudioEngine()
{
    // Create an AudioComponentDescription for the default output unit
    AudioComponentDescription desc = {};
    desc.componentType = kAudioUnitType_Output;
    desc.componentSubType = kAudioUnitSubType_DefaultOutput;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;

    // Find component
    AudioComponent outputComponent = AudioComponentFindNext(nullptr, &desc);
    if (!outputComponent) {
        Logger::log(Logger::Error, "Could not find output component");
        return false;
    }

    // Create instance
    OSStatus status = AudioComponentInstanceNew(outputComponent, &audioUnit_);
    if (status != noErr) {
        Logger::log(Logger::Error, QString("AudioComponentInstanceNew failed: %1").arg(static_cast<int>(status)));
        return false;
    }

    // Set up render callback
    AURenderCallbackStruct renderCallback = {};
    renderCallback.inputProc = CoreAudioBackend::renderCallback;
    renderCallback.inputProcRefCon = this;

    status = AudioUnitSetProperty(audioUnit_,
                                  kAudioUnitProperty_SetRenderCallback,
                                  kAudioUnitScope_Global,
                                  0,
                                  &renderCallback,
                                  sizeof(renderCallback));
    if (status != noErr) {
        Logger::log(Logger::Error, QString("AudioUnitSetProperty failed: %1").arg(static_cast<int>(status)));
        return false;
    }

    // Set audio format
    AudioStreamBasicDescription asbd = {};
    asbd.mSampleRate = format_.sampleRate;
    asbd.mFormatID = kAudioFormatLinearPCM;
    asbd.mFormatFlags = kAudioFormatFlagsNativeFloatPacked;
    asbd.mBytesPerPacket = format_.channels * sizeof(float);
    asbd.mFramesPerPacket = 1;
    asbd.mBytesPerFrame = format_.channels * sizeof(float);
    asbd.mChannelsPerFrame = format_.channels;
    asbd.mBitsPerChannel = 32;

    status = AudioUnitSetProperty(audioUnit_,
                                  kAudioUnitProperty_StreamFormat,
                                  kAudioUnitScope_Input,
                                  0,
                                  &asbd,
                                  sizeof(asbd));
    if (status != noErr) {
        Logger::log(Logger::Error, QString("AudioUnitSetProperty format failed: %1").arg(static_cast<int>(status)));
        return false;
    }

    // Initialize
    status = AudioUnitInitialize(audioUnit_);
    if (status != noErr) {
        Logger::log(Logger::Error, QString("AudioUnitInitialize failed: %1").arg(static_cast<int>(status)));
        return false;
    }

    Logger::log(Logger::Info, "CoreAudio AudioUnit initialized successfully");
    return true;
}

void CoreAudioBackend::cleanupAudioEngine()
{
    if (audioUnit_) {
        AudioUnitUninitialize(audioUnit_);
        AudioComponentInstanceDispose(audioUnit_);
        audioUnit_ = nullptr;
    }
}

bool CoreAudioBackend::play()
{
    if (!initialized_ || !audioUnit_) {
        return false;
    }

    OSStatus status = AudioOutputUnitStart(audioUnit_);
    if (status == noErr) {
        isPlaying_ = true;
        Logger::log(Logger::Info, "Audio playback started");
        return true;
    }

    Logger::log(Logger::Error, QString("AudioOutputUnitStart failed: %1").arg(static_cast<int>(status)));
    return false;
}

bool CoreAudioBackend::pause()
{
    if (!initialized_ || !audioUnit_) {
        return false;
    }

    OSStatus status = AudioOutputUnitStop(audioUnit_);
    if (status == noErr) {
        isPlaying_ = false;
        Logger::log(Logger::Info, "Audio playback paused");
        return true;
    }

    return false;
}

bool CoreAudioBackend::stop()
{
    return pause();
}

bool CoreAudioBackend::seek(int64_t samplePosition)
{
    if (samplePosition < 0 || samplePosition > totalSamples_) {
        return false;
    }

    currentPosition_ = samplePosition;
    Logger::log(Logger::Debug, QString("Seek to sample %1").arg(samplePosition));
    return true;
}

OSStatus CoreAudioBackend::renderCallback(void *inRefCon,
                                         AudioUnitRenderActionFlags *ioActionFlags,
                                         const AudioTimeStamp *inTimeStamp,
                                         UInt32 inOutputBusNumber,
                                         UInt32 inNumberFrames,
                                         AudioBufferList *ioData)
{
    auto *this_ptr = static_cast<CoreAudioBackend *>(inRefCon);
    return this_ptr->handleRenderCallback(ioActionFlags, inTimeStamp, inOutputBusNumber, inNumberFrames, ioData);
}

OSStatus CoreAudioBackend::handleRenderCallback(AudioUnitRenderActionFlags *ioActionFlags,
                                               const AudioTimeStamp *inTimeStamp,
                                               UInt32 inOutputBusNumber,
                                               UInt32 inNumberFrames,
                                               AudioBufferList *ioData)
{
    (void)inOutputBusNumber;
    (void)inTimeStamp;
    (void)ioActionFlags;

    if (!isPlaying_ || !audioCallback_) {
        // Fill with silence
        for (UInt32 i = 0; i < ioData->mNumberBuffers; ++i) {
            AudioBuffer &buffer = ioData->mBuffers[i];
            std::memset(buffer.mData, 0, buffer.mDataByteSize);
        }
        return noErr;
    }

    // Call the audio callback
    AudioBuffer audioBuffer;
    audioBuffer.sampleCount = inNumberFrames;
    audioBuffer.data.resize(inNumberFrames * format_.channels, 0.0f);

    try {
        audioCallback_(audioBuffer);
    } catch (...) {
        Logger::log(Logger::Error, "Exception in audio callback");
    }

    // Copy to output buffers
    size_t offset = 0;
    for (UInt32 i = 0; i < ioData->mNumberBuffers; ++i) {
        AudioBuffer &buffer = ioData->mBuffers[i];
        auto *floatData = static_cast<float *>(buffer.mData);

        for (UInt32 j = 0; j < inNumberFrames && offset < audioBuffer.data.size(); ++j) {
            floatData[j] = audioBuffer.data[offset++];
        }
    }

    currentPosition_ += inNumberFrames;
    return noErr;
}

QString CoreAudioBackend::deviceName() const
{
    return QString("System Output (CoreAudio)");
}

#endif // Q_OS_MAC
