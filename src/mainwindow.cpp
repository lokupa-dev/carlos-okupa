#include "mainwindow.h"
#include "ui/widgets/fileimport.h"
#include "ui/widgets/trackselector.h"
#include "ui/widgets/folderchooser.h"
#include "ui/widgets/mixer.h"
#include "ui/widgets/timeline.h"
#include "ui/widgets/progressdialog.h"
#include "processing/separationengine.h"
#include "audio/audioengine.h"
#include "utils/logger.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Carlos Okupa - Music Source Separation");
    setMinimumSize(1200, 700);

    setupUI();
    setupConnections();
}

MainWindow::~MainWindow() = default;

void MainWindow::setSeparationEngine(SeparationEngine *engine)
{
    separationEngine = engine;
}

void MainWindow::setAudioEngine(AudioEngine *engine)
{
    audioEngine = engine;
}

void MainWindow::setupUI()
{
    // Create central widget with layout
    auto centralWidget = new QWidget(this);
    auto mainLayout = new QHBoxLayout(centralWidget);

    // Left panel: Input & Settings
    auto leftPanel = new QWidget();
    auto leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(12);
    leftLayout->setContentsMargins(12, 12, 12, 12);

    // File import
    fileImport = std::make_unique<FileImport>();
    leftLayout->addWidget(fileImport.get());

    // Track selector
    trackSelector = std::make_unique<TrackSelector>();
    leftLayout->addWidget(trackSelector.get());

    // Folder chooser
    folderChooser = std::make_unique<FolderChooser>();
    leftLayout->addWidget(folderChooser.get());

    // Separate button
    auto separateBtn = new QPushButton("Separate & Export");
    separateBtn->setMinimumHeight(44);
    connect(separateBtn, &QPushButton::clicked, this, &MainWindow::onSeparate);
    leftLayout->addWidget(separateBtn);

    leftLayout->addStretch();

    // Right panel: Playback & Mixer
    auto rightPanel = new QWidget();
    auto rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(12);
    rightLayout->setContentsMargins(12, 12, 12, 12);

    // Timeline
    timeline = std::make_unique<Timeline>();
    rightLayout->addWidget(timeline.get());

    // Mixer
    mixer = std::make_unique<Mixer>();
    rightLayout->addWidget(mixer.get(), 1);

    // Export buttons
    auto exportLayout = new QHBoxLayout();
    auto exportMixBtn = new QPushButton("Export Mix");
    auto openFolderBtn = new QPushButton("Open Folder");
    exportLayout->addWidget(exportMixBtn);
    exportLayout->addWidget(openFolderBtn);
    rightLayout->addLayout(exportLayout);

    connect(exportMixBtn, &QPushButton::clicked, this, &MainWindow::onExportMix);
    connect(openFolderBtn, &QPushButton::clicked, this, &MainWindow::onOpenFolder);

    // Splitter between left and right
    auto splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

    mainLayout->addWidget(splitter);
    setCentralWidget(centralWidget);

    // Progress dialog
    progressDialog = std::make_unique<ProgressDialog>(this);
}

void MainWindow::setupConnections()
{
    // Note: Connections to be implemented
}

void MainWindow::onSeparate()
{
    if (!separationEngine) return;

    QString inputFile = fileImport->selectedFile();
    QStringList selectedStems = trackSelector->selectedStems();
    QString outputFolder = folderChooser->selectedFolder();

    if (inputFile.isEmpty() || selectedStems.isEmpty() || outputFolder.isEmpty()) {
        Logger::log(Logger::Warning, "Missing input file, stems, or output folder");
        return;
    }

    progressDialog->show();
    // TODO: Start separation
}

void MainWindow::onExportMix()
{
    // TODO: Export mix
}

void MainWindow::onOpenFolder()
{
    QString folder = folderChooser->selectedFolder();
    if (!folder.isEmpty()) {
        // TODO: Open folder in file explorer
    }
}

void MainWindow::onSeparationProgress(int percent, const QString &message)
{
    progressDialog->setProgress(percent);
    progressDialog->setMessage(message);
}

void MainWindow::onSeparationComplete()
{
    progressDialog->close();
    // TODO: Load stems into mixer
}

void MainWindow::onSeparationError(const QString &error)
{
    progressDialog->setError(error);
}
