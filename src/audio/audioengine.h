#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H

#include <QString>

class AudioEngine
{
public:
    AudioEngine();
    virtual ~AudioEngine();

    bool initialize();
    void shutdown();

    int sampleRate() const { return sampleRate_; }
    int channels() const { return channels_; }

private:
    bool initializePlatform();
    void shutdownPlatform();

    int sampleRate_ = 44100;
    int channels_ = 2;
};

#endif // AUDIOENGINE_H
