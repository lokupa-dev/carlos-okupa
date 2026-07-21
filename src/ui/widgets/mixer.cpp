#include "mixer.h"
#include <QVBoxLayout>
#include <QLabel>

Mixer::Mixer(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

Mixer::~Mixer() = default;

void Mixer::setupUI()
{
    auto layout = new QVBoxLayout(this);
    auto label = new QLabel("Mixer (To be implemented)");
    layout->addWidget(label);
}
