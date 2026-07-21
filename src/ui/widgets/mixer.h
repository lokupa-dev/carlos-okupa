#ifndef MIXER_H
#define MIXER_H

#include <QWidget>

class Mixer : public QWidget
{
    Q_OBJECT

public:
    explicit Mixer(QWidget *parent = nullptr);
    ~Mixer();

private:
    void setupUI();
};

#endif // MIXER_H
