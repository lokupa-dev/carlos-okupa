#include "audiotrack.h"
#include "../utils/logger.h"
#include <sndfile.h>
#include <cmath>
#include <algorithm>
#include <cstring>

AudioTrack::AudioTrack(const QString &name, int channels, int sampleRate)
    : name_(name)
    , channels_(channels)
    , sampleRate_(sampleRate)
    , currentPosition_(0)
    , totalSamples_(0)
    , targetVolume_(1.0f)
    , currentVolume_(1.0f)
{
}

AudioTrack::~AudioTrack()
{
    unload();
}

bool AudioTrack::loadFromFile(const QString &filePath)
{
    Logger::log(Logger::Info, QString("Loading audio track: %1").arg(filePath));

    SF_INFO sfInfo = {};
    SNDFILE *file = sf_open(filePath.toUtf8().constData(), SFM_READ, &sfInfo);

    if (!file) {
        Logger::log(Logger::Error, QString("Failed to open audio file: %1").arg(sf_strerror(nullptr)));
        return false;
    }

    // Resample if needed
    int targetChannels = channels_;
    if (sfInfo.channels != targetChannels) {
        Logger::log(Logger::Warning, QString("Track channels mismatch: file has %1, expected %2").arg(sfInfo.channels).arg(targetChannels));
        targetChannels = std::min(static_cast<int>(sfInfo.channels), targetChannels);
    }

    // Read all samples
    std::vector<float> buffer(sfInfo.frames * sfInfo.channels);
    sf_count_t readCount = sf_readf_float(file, buffer.data(), sfInfo.frames);

    if (readCount != sfInfo.frames) {
        Logger::log(Logger::Error, QString("Failed to read audio samples: read %1 of %2").arg(readCount).arg(sfInfo.frames));
        sf_close(file);
        return false;
    }

    sf_close(file);

    // Convert to target channel count if needed
    if (targetChannels == sfInfo.channels) {
        audioData_ = std::move(buffer);
    } else {
        // Simple channel conversion (mixing down if needed)
        audioData_.resize(sfInfo.frames * channels_);
        for (sf_count_t i = 0; i < sfInfo.frames; ++i) {
            float sum = 0.0f;
            for (int ch = 0; ch < targetChannels; ++ch) {
                sum += buffer[i * sfInfo.channels + ch];
            }
            float avg = sum / targetChannels;
            for (int ch = 0; ch < channels_; ++ch) {
                audioData_[i * channels_ + ch] = (ch < targetChannels) ? avg : 0.0f;
            }
        }
    }

    totalSamples_ = sfInfo.frames;
    currentPosition_ = 0;
    currentVolume_ = targetVolume_;

    Logger::log(Logger::Info, QString("Track loaded: %1 samples, %2 Hz").arg(totalSamples_).arg(sfInfo.samplerate));
    return true;
}

void AudioTrack::unload()
{
    audioData_.clear();
    totalSamples_ = 0;
    currentPosition_ = 0;
    Logger::log(Logger::Debug, QString("Track unloaded: %1").arg(name_));
}

void AudioTrack::setPosition(int64_t samplePosition)
{
    if (samplePosition < 0) {
        samplePosition = 0;
    }
    if (samplePosition > totalSamples_) {
        samplePosition = totalSamples_;
    }
    currentPosition_ = samplePosition;
}

void AudioTrack::renderSamples(std::vector<float> &outputBuffer, int sampleCount)
{
    if (!isLoaded() || muted_) {
        // Output silence
        return;
    }

    int64_t pos = currentPosition_.load();
    int remaining = sampleCount;
    size_t outputIndex = 0;

    // Apply volume ramp
    applyGainRamp(outputBuffer, sampleCount);

    while (remaining > 0 && pos < totalSamples_) {
        int64_t samplesToRead = std::min(static_cast<int64_t>(remaining), totalSamples_ - pos);

        for (int i = 0; i < samplesToRead; ++i) {
            int trackIndex = (pos + i) * channels_;
            for (int ch = 0; ch < channels_; ++ch) {
                if (trackIndex + ch < static_cast<int>(audioData_.size())) {
                    outputBuffer[outputIndex++] += audioData_[trackIndex + ch] * currentVolume_;
                }
            }
        }

        pos += samplesToRead;
        remaining -= samplesToRead;
    }

    currentPosition_ = pos;
}

void AudioTrack::setVolume(float volumeLinear)
{
    targetVolume_ = std::max(0.0f, std::min(2.0f, volumeLinear));
}

void AudioTrack::setMute(bool muted)
{
    muted_ = muted;
}

void AudioTrack::setSolo(bool solo)
{
    solo_ = solo;
}

void AudioTrack::applyGainRamp(std::vector<float> &buffer, int sampleCount)
{
    // If volume hasn't changed significantly, no need to ramp
    float volumeDelta = std::abs(currentVolume_ - targetVolume_);
    if (volumeDelta < 0.001f) {
        currentVolume_ = targetVolume_;
        return;
    }

    // Calculate ramp rate
    int rampSamples = static_cast<int>(VolumeRampTime * sampleRate_);
    rampSamples = std::max(1, std::min(rampSamples, sampleCount));

    float volumeStep = (targetVolume_ - currentVolume_) / rampSamples;

    for (int i = 0; i < rampSamples; ++i) {
        currentVolume_ += volumeStep;
    }

    // Clamp to target
    if (std::abs(currentVolume_ - targetVolume_) < volumeStep) {
        currentVolume_ = targetVolume_;
    }
}

void AudioTrack::mixSamplesToBuffer(const std::vector<float> &trackSamples,
                                    std::vector<float> &outputBuffer,
                                    int sampleCount)
{
    (void)trackSamples;  // Unused in this simplified version
    (void)sampleCount;   // Unused in this simplified version
    (void)outputBuffer;  // Unused in this simplified version
}
