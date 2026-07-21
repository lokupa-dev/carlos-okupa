#ifndef MIXERENGINE_H
#define MIXERENGINE_H

class MixerEngine
{
public:
    MixerEngine();
    virtual ~MixerEngine();

    bool loadTrack(const QString &filename, int trackIndex);
    void setTrackVolume(int trackIndex, float volume);
    void setTrackMute(int trackIndex, bool mute);
    void setTrackSolo(int trackIndex, bool solo);

private:
    struct Track {
        QString filename;
        float volume = 1.0f;
        bool muted = false;
        bool solo = false;
    };

    static constexpr int MaxTracks = 16;
    Track tracks[MaxTracks];
};

#endif // MIXERENGINE_H
