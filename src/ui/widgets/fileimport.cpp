#include "fileimport.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QDropEvent>
#include <QMimeData>

FileImport::FileImport(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    setAcceptDrops(true);
}

FileImport::~FileImport() = default;

void FileImport::setupUI()
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto titleLabel = new QLabel("Audio File");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 12pt;");
    layout->addWidget(titleLabel);

    infoLabel = std::make_unique<QLabel>("Drag file here or click Browse");
    infoLabel->setStyleSheet("border: 2px dashed #ccc; padding: 20px; border-radius: 4px; text-align: center;");
    infoLabel->setMinimumHeight(80);
    infoLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(infoLabel.get());

    browseButton = std::make_unique<QPushButton>("Browse");
    browseButton->setMinimumHeight(32);
    connect(browseButton.get(), &QPushButton::clicked, this, &FileImport::onBrowse);
    layout->addWidget(browseButton.get());
}

void FileImport::onBrowse()
{
    QString file = QFileDialog::getOpenFileName(this,
        "Select Audio File", "",
        "Audio Files (*.mp3 *.wav *.flac *.ogg);;All Files (*)");
    if (!file.isEmpty() && validateAudioFile(file)) {
        setFile(file);
    }
}

void FileImport::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void FileImport::dropEvent(QDropEvent *event)
{
    const auto urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        QString file = urls.first().toLocalFile();
        if (validateAudioFile(file)) {
            setFile(file);
        }
    }
}

void FileImport::setFile(const QString &filePath)
{
    currentFile = filePath;
    QFileInfo info(filePath);
    infoLabel->setText(QString("File: %1\nSize: %2 MB").arg(info.fileName()).arg(info.size() / 1024 / 1024));
}

bool FileImport::validateAudioFile(const QString &filePath)
{
    QStringList supportedFormats = {"mp3", "wav", "flac", "ogg"};
    QFileInfo info(filePath);
    return supportedFormats.contains(info.suffix().toLower());
}

QString FileImport::fileName() const
{
    return QFileInfo(currentFile).fileName();
}

QString FileImport::fileInfo() const
{
    return currentFile;
}
