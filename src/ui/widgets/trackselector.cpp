#include "trackselector.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>

TrackSelector::TrackSelector(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

TrackSelector::~TrackSelector() = default;

void TrackSelector::setupUI()
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto titleLabel = new QLabel("Stems to Separate");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 12pt;");
    layout->addWidget(titleLabel);

    // Select All / None buttons
    auto buttonLayout = new QHBoxLayout();
    auto selectAllBtn = new QPushButton("Select All");
    auto selectNoneBtn = new QPushButton("Select None");
    selectAllBtn->setMaximumWidth(100);
    selectNoneBtn->setMaximumWidth(100);
    connect(selectAllBtn, &QPushButton::clicked, this, &TrackSelector::onSelectAll);
    connect(selectNoneBtn, &QPushButton::clicked, this, &TrackSelector::onDeselectAll);
    buttonLayout->addWidget(selectAllBtn);
    buttonLayout->addWidget(selectNoneBtn);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);

    // Checkboxes for each stem
    QStringList stems = {"Vocals", "Drums", "Bass", "Other"};
    for (const auto &stem : stems) {
        auto checkbox = std::make_unique<QCheckBox>(stem);
        checkbox->setChecked(true);
        checkboxes.push_back(std::move(checkbox));
        layout->addWidget(checkboxes.back().get());
    }

    layout->addStretch();
}

QStringList TrackSelector::selectedStems() const
{
    QStringList result;
    for (const auto &checkbox : checkboxes) {
        if (checkbox->isChecked()) {
            result << checkbox->text();
        }
    }
    return result;
}

void TrackSelector::onSelectAll()
{
    for (auto &checkbox : checkboxes) {
        checkbox->setChecked(true);
    }
}

void TrackSelector::onDeselectAll()
{
    for (auto &checkbox : checkboxes) {
        checkbox->setChecked(false);
    }
}
