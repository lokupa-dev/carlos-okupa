#include "mixercontroller.h"
#include "realtimemixer.h"
#include "audioengine.h"
#include "../utils/logger.h"

MixerController::MixerController()
    : mixer_(nullptr)
    , audioEngine_(nullptr)
    , isPlaying_(false)
{
}

MixerController::~MixerController()
{
    shutdown();
}

bool MixerController::initialize(AudioEngine *audioEngine)
{
    if (!audioEngine) {
        Logger::log(Logger::Error, "AudioEngine pointer is null");
        return false;
    }

    audioEngine_ = audioEngine;
    mixer_ = std::make_unique<RealtimeMixer>(audioEngine->channels(), audioEngine->sampleRate());

    // Set audio callback
    audioEngine_->setAudioCallback([this](std::vector<float> &buffer, int sampleCount) {
        onAudioCallback(buffer, sampleCount);
    });

    Logger::log(Logger::Info, "MixerController initialized");
    return true;
}

void MixerController::shutdown()
{
    if (mixer_) {
        mixer_->stop();
        mixer_ = nullptr;
    }
    audioEngine_ = nullptr;
    trackIds_.clear();
    trackNames_.clear();
    Logger::log(Logger::Info, "MixerController shutdown");
}

bool MixerController::loadStem(const QString &stemName, const QString &filePath)
{
    if (!mixer_) {
        Logger::log(Logger::Error, "Mixer not initialized");
        return false;
    }

    int trackId = mixer_->addTrack(stemName);
    if (!mixer_->loadTrackAudio(trackId, filePath)) {
        Logger::log(Logger::Error, QString("Failed to load stem: %1").arg(stemName));
        mixer_->removeTrack(trackId);
        return false;
    }

    trackIds_.push_back(trackId);
    trackNames_.push_back(stemName);

    if (trackListChanged_) {
        trackListChanged_();
    }

    Logger::log(Logger::Info, QString("Stem loaded: %1").arg(stemName));
    return true;
}

bool MixerController::unloadStem(int trackId)
{
    if (!mixer_) {
        return false;
    }

    auto it = std::find(trackIds_.begin(), trackIds_.end(), trackId);
    if (it != trackIds_.end()) {
        size_t index = std::distance(trackIds_.begin(), it);
        trackIds_.erase(it);
        trackNames_.erase(trackNames_.begin() + index);
        mixer_->removeTrack(trackId);

        if (trackListChanged_) {
            trackListChanged_();
        }

        return true;
    }

    return false;
}

void MixerController::unloadAllStems()
{
    if (!mixer_) {
        return;
    }

    std::vector<int> toRemove = trackIds_;
    for (int trackId : toRemove) {
        unloadStem(trackId);
    }
}

void MixerController::play()
{
    if (mixer_ && !isPlaying_) {
        mixer_->play();
        if (audioEngine_) {
            audioEngine_->play();
        }
        isPlaying_ = true;

        if (playbackStateChanged_) {
            playbackStateChanged_(true);
        }

        Logger::log(Logger::Debug, "Mixer playback started");
    }
}

void MixerController::pause()
{
    if (mixer_ && isPlaying_) {
        mixer_->pause();
        if (audioEngine_) {
            audioEngine_->pause();
        }
        isPlaying_ = false;

        if (playbackStateChanged_) {
            playbackStateChanged_(false);
        }

        Logger::log(Logger::Debug, "Mixer playback paused");
    }
}

void MixerController::stop()
{
    if (mixer_) {
        mixer_->stop();
        if (audioEngine_) {
            audioEngine_->stop();
        }
        isPlaying_ = false;

        if (playbackStateChanged_) {
            playbackStateChanged_(false);
        }

        Logger::log(Logger::Debug, "Mixer playback stopped");
    }
}

void MixerController::seek(int64_t samplePosition)
{
    if (mixer_) {
        mixer_->seek(samplePosition);

        if (positionChanged_) {
            positionChanged_(samplePosition);
        }
    }
}

std::vector<MixerController::TrackInfo> MixerController::getTracksInfo() const
{
    std::vector<TrackInfo> result;

    if (!mixer_) {
        return result;
    }

    for (size_t i = 0; i < trackIds_.size(); ++i) {
        int trackId = trackIds_[i];
        TrackInfo info;
        info.id = trackId;
        info.name = trackNames_[i];
        info.isLoaded = mixer_->isTrackLoaded(trackId);
        info.volume = mixer_->getTrackVolume(trackId);
        info.muted = mixer_->isTrackMuted(trackId);
        info.solo = mixer_->isTrackSolo(trackId);
        result.push_back(info);
    }

    return result;
}

int64_t MixerController::currentPosition() const
{
    if (mixer_) {
        return mixer_->currentPosition();
    }
    return 0;
}

int64_t MixerController::totalDuration() const
{
    if (mixer_) {
        return mixer_->totalDuration();
    }
    return 0;
}

bool MixerController::isPlaying() const
{
    return isPlaying_;
}

void MixerController::setTrackVolume(int trackId, float volumeLinear)
{
    if (mixer_) {
        mixer_->setTrackVolume(trackId, volumeLinear);

        if (trackStateChanged_) {
            trackStateChanged_(trackId);
        }
    }
}

void MixerController::setTrackMute(int trackId, bool muted)
{
    if (mixer_) {
        mixer_->setTrackMute(trackId, muted);

        if (trackStateChanged_) {
            trackStateChanged_(trackId);
        }
    }
}

void MixerController::setTrackSolo(int trackId, bool solo)
{
    if (mixer_) {
        mixer_->setTrackSolo(trackId, solo);

        if (trackStateChanged_) {
            trackStateChanged_(trackId);
        }
    }
}

void MixerController::onAudioCallback(std::vector<float> &buffer, int sampleCount)
{
    if (mixer_) {
        mixer_->renderAudio(buffer, sampleCount);

        if (positionChanged_) {
            positionChanged_(mixer_->currentPosition());
        }
    }
}
