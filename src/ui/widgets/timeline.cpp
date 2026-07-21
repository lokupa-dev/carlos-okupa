#include "timeline.h"
#include <QVBoxLayout>
#include <QLabel>

Timeline::Timeline(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

Timeline::~Timeline() = default;

void Timeline::setupUI()
{
    auto layout = new QVBoxLayout(this);
    auto label = new QLabel("Timeline (To be implemented)");
    layout->addWidget(label);
}
