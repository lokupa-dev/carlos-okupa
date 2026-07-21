#include "realtimemixer.h"
#include "../utils/logger.h"
#include <algorithm>
#include <cmath>

RealtimeMixer::RealtimeMixer(int channels, int sampleRate)
    : channels_(channels)
    , sampleRate_(sampleRate)
    , isPlaying_(false)
    , masterPosition_(0)
    , maxDuration_(0)
    , nextTrackId_(0)
    , anySoloActive_(false)
{
    Logger::log(Logger::Info, QString("RealtimeMixer created: %1 channels, %2 Hz").arg(channels).arg(sampleRate));
}

RealtimeMixer::~RealtimeMixer()
{
    tracks_.clear();
    Logger::log(Logger::Info, "RealtimeMixer destroyed");
}

int RealtimeMixer::addTrack(const QString &name)
{
    int trackId = nextTrackId_++;
    TrackInfo info;
    info.track = std::make_unique<AudioTrack>(name, channels_, sampleRate_);
    info.id = trackId;
    info.soloActive = false;

    tracks_.push_back(std::move(info));
    Logger::log(Logger::Debug, QString("Track added: %1 (ID %2)").arg(name).arg(trackId));
    return trackId;
}

bool RealtimeMixer::removeTrack(int trackId)
{
    auto it = std::find_if(tracks_.begin(), tracks_.end(),
                          [trackId](const TrackInfo &info) { return info.id == trackId; });

    if (it != tracks_.end()) {
        tracks_.erase(it);
        Logger::log(Logger::Debug, QString("Track removed: ID %1").arg(trackId));
        calculateSoloState();
        return true;
    }

    return false;
}

bool RealtimeMixer::loadTrackAudio(int trackId, const QString &filePath)
{
    auto it = std::find_if(tracks_.begin(), tracks_.end(),
                          [trackId](const TrackInfo &info) { return info.id == trackId; });

    if (it != tracks_.end() && it->track) {
        bool success = it->track->loadFromFile(filePath);
        if (success) {
            maxDuration_ = std::max(maxDuration_, it->track->duration());
            Logger::log(Logger::Debug, QString("Track audio loaded: ID %1, duration %2 samples").arg(trackId).arg(it->track->duration()));
        }
        return success;
    }

    return false;
}

bool RealtimeMixer::isTrackLoaded(int trackId) const
{
    auto it = std::find_if(tracks_.begin(), tracks_.end(),
                          [trackId](const TrackInfo &info) { return info.id == trackId; });
    return it != tracks_.end() && it->track && it->track->isLoaded();
}

void RealtimeMixer::play()
{
    if (!isPlaying_) {
        isPlaying_ = true;
        Logger::log(Logger::Debug, "Mixer: play");
    }
}

void RealtimeMixer::pause()
{
    if (isPlaying_) {
        isPlaying_ = false;
        Logger::log(Logger::Debug, "Mixer: pause");
    }
}

void RealtimeMixer::stop()
{
    isPlaying_ = false;
    masterPosition_ = 0;

    // Reset all track positions
    for (auto &info : tracks_) {
        if (info.track) {
            info.track->setPosition(0);
        }
    }

    Logger::log(Logger::Debug, "Mixer: stop");
}

void RealtimeMixer::seek(int64_t samplePosition)
{
    if (samplePosition < 0) {
        samplePosition = 0;
    }
    if (samplePosition > maxDuration_) {
        samplePosition = maxDuration_;
    }

    masterPosition_ = samplePosition;

    // Seek all tracks to the same position
    for (auto &info : tracks_) {
        if (info.track) {
            info.track->setPosition(samplePosition);
        }
    }

    Logger::log(Logger::Debug, QString("Mixer: seek to sample %1").arg(samplePosition));
}

void RealtimeMixer::setTrackVolume(int trackId, float volumeLinear)
{
    auto it = std::find_if(tracks_.begin(), tracks_.end(),
                          [trackId](const TrackInfo &info) { return info.id == trackId; });

    if (it != tracks_.end() && it->track) {
        it->track->setVolume(volumeLinear);
    }
}

void RealtimeMixer::setTrackMute(int trackId, bool muted)
{
    auto it = std::find_if(tracks_.begin(), tracks_.end(),
                          [trackId](const TrackInfo &info) { return info.id == trackId; });

    if (it != tracks_.end() && it->track) {
        it->track->setMute(muted);
    }
}

void RealtimeMixer::setTrackSolo(int trackId, bool solo)
{
    auto it = std::find_if(tracks_.begin(), tracks_.end(),
                          [trackId](const TrackInfo &info) { return info.id == trackId; });

    if (it != tracks_.end() && it->track) {
        it->track->setSolo(solo);
        calculateSoloState();
    }
}

float RealtimeMixer::getTrackVolume(int trackId) const
{
    auto it = std::find_if(tracks_.begin(), tracks_.end(),
                          [trackId](const TrackInfo &info) { return info.id == trackId; });
    if (it != tracks_.end() && it->track) {
        return it->track->volume();
    }
    return 0.0f;
}

bool RealtimeMixer::isTrackMuted(int trackId) const
{
    auto it = std::find_if(tracks_.begin(), tracks_.end(),
                          [trackId](const TrackInfo &info) { return info.id == trackId; });
    if (it != tracks_.end() && it->track) {
        return it->track->isMuted();
    }
    return false;
}

bool RealtimeMixer::isTrackSolo(int trackId) const
{
    auto it = std::find_if(tracks_.begin(), tracks_.end(),
                          [trackId](const TrackInfo &info) { return info.id == trackId; });
    if (it != tracks_.end() && it->track) {
        return it->track->isSolo();
    }
    return false;
}

const QString &RealtimeMixer::trackName(int trackId) const
{
    static const QString emptyName;
    auto it = std::find_if(tracks_.begin(), tracks_.end(),
                          [trackId](const TrackInfo &info) { return info.id == trackId; });
    if (it != tracks_.end() && it->track) {
        return it->track->name();
    }
    return emptyName;
}

void RealtimeMixer::renderAudio(std::vector<float> &outputBuffer, int sampleCount)
{
    if (!isPlaying_) {
        // Fill with silence
        std::fill(outputBuffer.begin(), outputBuffer.end(), 0.0f);
        return;
    }

    // Clear output buffer
    std::fill(outputBuffer.begin(), outputBuffer.end(), 0.0f);

    // Mix all active tracks
    for (auto &info : tracks_) {
        if (info.track && info.track->isLoaded() && shouldTrackPlay(info.id)) {
            info.track->renderSamples(outputBuffer, sampleCount);
        }
    }

    // Update master position
    updateMasterPosition(sampleCount);
}

void RealtimeMixer::updateMasterPosition(int samplesProcessed)
{
    int64_t newPos = masterPosition_.load() + samplesProcessed;
    if (newPos > maxDuration_) {
        newPos = maxDuration_;
        pause();
    }
    masterPosition_ = newPos;
}

void RealtimeMixer::calculateSoloState()
{
    anySoloActive_ = false;
    for (const auto &info : tracks_) {
        if (info.track && info.track->isSolo()) {
            anySoloActive_ = true;
            break;
        }
    }
}

bool RealtimeMixer::shouldTrackPlay(int trackId) const
{
    auto it = std::find_if(tracks_.begin(), tracks_.end(),
                          [trackId](const TrackInfo &info) { return info.id == trackId; });

    if (it == tracks_.end() || !it->track) {
        return false;
    }

    if (it->track->isMuted()) {
        return false;
    }

    // If any solo is active, only play soloized tracks
    if (anySoloActive_) {
        return it->track->isSolo();
    }

    return true;
}
