#ifndef WAVEFORMRENDERER_H
#define WAVEFORMRENDERER_H

class WaveformRenderer
{
public:
    WaveformRenderer();
    virtual ~WaveformRenderer();

    bool renderWaveform(const QString &audioFile);

private:
};

#endif // WAVEFORMRENDERER_H
