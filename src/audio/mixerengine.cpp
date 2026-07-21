#include "mixerengine.h"
#include <QString>

MixerEngine::MixerEngine() = default;

MixerEngine::~MixerEngine() = default;

bool MixerEngine::loadTrack(const QString &filename, int trackIndex)
{
    if (trackIndex < 0 || trackIndex >= MaxTracks) {
        return false;
    }
    tracks[trackIndex].filename = filename;
    return true;
}

void MixerEngine::setTrackVolume(int trackIndex, float volume)
{
    if (trackIndex >= 0 && trackIndex < MaxTracks) {
        tracks[trackIndex].volume = volume;
    }
}

void MixerEngine::setTrackMute(int trackIndex, bool mute)
{
    if (trackIndex >= 0 && trackIndex < MaxTracks) {
        tracks[trackIndex].muted = mute;
    }
}

void MixerEngine::setTrackSolo(int trackIndex, bool solo)
{
    if (trackIndex >= 0 && trackIndex < MaxTracks) {
        tracks[trackIndex].solo = solo;
    }
}
