#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

class AudioPlayer
{
public:
    AudioPlayer();
    virtual ~AudioPlayer();

    void play();
    void pause();
    void stop();
    void seek(int positionMs);

private:
    bool isPlaying_ = false;
};

#endif // AUDIOPLAYER_H
