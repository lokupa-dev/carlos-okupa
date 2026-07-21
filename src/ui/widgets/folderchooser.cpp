#include "folderchooser.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QStandardPaths>

FolderChooser::FolderChooser(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    // Default to Desktop
    setFolder(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
}

FolderChooser::~FolderChooser() = default;

void FolderChooser::setupUI()
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto titleLabel = new QLabel("Output Folder");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 12pt;");
    layout->addWidget(titleLabel);

    pathLabel = std::make_unique<QLabel>();
    pathLabel->setWordWrap(true);
    pathLabel->setStyleSheet("padding: 8px; background-color: #f5f5f5; border-radius: 4px;");
    layout->addWidget(pathLabel.get());

    auto browseBtn = new QPushButton("Browse");
    browseBtn->setMinimumHeight(32);
    connect(browseBtn, &QPushButton::clicked, this, &FolderChooser::onBrowse);
    layout->addWidget(browseBtn);
}

void FolderChooser::onBrowse()
{
    QString folder = QFileDialog::getExistingDirectory(this,
        "Select Output Folder", currentFolder);
    if (!folder.isEmpty()) {
        setFolder(folder);
    }
}

void FolderChooser::setFolder(const QString &folderPath)
{
    currentFolder = folderPath;
    pathLabel->setText(folderPath);
}
