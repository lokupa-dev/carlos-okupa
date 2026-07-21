#include "audioplayer.h"

AudioPlayer::AudioPlayer() = default;

AudioPlayer::~AudioPlayer() = default;

void AudioPlayer::play()
{
    isPlaying_ = true;
}

void AudioPlayer::pause()
{
    isPlaying_ = false;
}

void AudioPlayer::stop()
{
    isPlaying_ = false;
}

void AudioPlayer::seek(int positionMs)
{
    // TODO: Implement seek
}
