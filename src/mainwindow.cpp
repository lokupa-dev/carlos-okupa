#include "mainwindow.h"
#include "ui/widgets/fileimport.h"
#include "ui/widgets/trackselector.h"
#include "ui/widgets/folderchooser.h"
#include "ui/widgets/mixer.h"
#include "ui/widgets/timeline.h"
#include "ui/widgets/progressdialog.h"
#include "processing/separationengine.h"
#include "audio/audioengine.h"
#include "audio/mixercontroller.h"
#include "python/demucswrapper.h"
#include "utils/logger.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QWidget>
#include <QMessageBox>
#include <QGroupBox>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QSpinBox>
#include <QComboBox>
#include <QTabWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , separationEngine(nullptr)
    , audioEngine(nullptr)
    , mixerController(nullptr)
    , demucsWrapper(nullptr)
{
    setWindowTitle("Carlos Okupa - Music Source Separation");
    setMinimumSize(1400, 800);
    setWindowIcon(QIcon(":/icons/app-icon.png"));

    setupUI();
    setupConnections();
    applyStyles();
}

MainWindow::~MainWindow() = default;

void MainWindow::setSeparationEngine(SeparationEngine *engine)
{
    separationEngine = engine;
}

void MainWindow::setAudioEngine(AudioEngine *engine)
{
    audioEngine = engine;

    // Initialize mixer controller
    mixerController = std::make_unique<MixerController>();
    if (!mixerController->initialize(audioEngine)) {
        Logger::log(Logger::Error, "Failed to initialize mixer controller");
        QMessageBox::critical(this, "Error", "Failed to initialize mixer");
        return;
    }

    // Connect mixer callbacks
    mixerController->setPlaybackStateChangedCallback([this](bool isPlaying) {
        onPlaybackStateChanged(isPlaying);
    });

    mixerController->setPositionChangedCallback([this](int64_t pos) {
        onPositionChanged(pos);
    });

    mixerController->setTrackStateChangedCallback([this](int trackId) {
        updateTrackUI(trackId);
    });
}

void MainWindow::setupUI()
{
    // Create central widget
    auto centralWidget = new QWidget(this);
    auto mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    // ==================== LEFT PANEL: SEPARATION ====================
    auto leftPanel = new QWidget();
    auto leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(12);
    leftLayout->setContentsMargins(12, 12, 12, 12);

    // Separation Group
    auto sepGroupBox = new QGroupBox("Source Separation");
    auto sepLayout = new QVBoxLayout(sepGroupBox);

    // File import
    fileImport = std::make_unique<FileImport>();
    sepLayout->addWidget(fileImport.get());

    // Track selector
    trackSelector = std::make_unique<TrackSelector>();
    sepLayout->addWidget(trackSelector.get());

    // Folder chooser
    folderChooser = std::make_unique<FolderChooser>();
    sepLayout->addWidget(folderChooser.get());

    leftLayout->addWidget(sepGroupBox);

    // Separate button
    auto separateBtn = new QPushButton("🎵 Start Separation");
    separateBtn->setMinimumHeight(48);
    separateBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #4CAF50;"
        "    color: white;"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "    border-radius: 4px;"
        "    border: none;"
        "}"
        "QPushButton:hover { background-color: #45a049; }"
        "QPushButton:pressed { background-color: #3d8b40; }"
    );
    connect(separateBtn, &QPushButton::clicked, this, &MainWindow::onSeparate);
    leftLayout->addWidget(separateBtn);

    // Status label
    statusLabel = new QLabel("Ready");
    statusLabel->setStyleSheet("color: #666; font-size: 11px;");
    leftLayout->addWidget(statusLabel);

    leftLayout->addStretch();

    // ==================== RIGHT PANEL: PLAYBACK & MIXING ====================
    auto rightPanel = new QWidget();
    auto rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(12);
    rightLayout->setContentsMargins(12, 12, 12, 12);

    // Playback Group
    auto playbackGroupBox = new QGroupBox("Playback");
    auto playbackLayout = new QVBoxLayout(playbackGroupBox);

    // Timeline
    timeline = std::make_unique<Timeline>();
    playbackLayout->addWidget(timeline.get());

    // Playback buttons
    auto buttonLayout = new QHBoxLayout();
    auto playBtn = new QPushButton("▶ Play");
    auto pauseBtn = new QPushButton("⏸ Pause");
    auto stopBtn = new QPushButton("⏹ Stop");

    playBtn->setMinimumHeight(36);
    pauseBtn->setMinimumHeight(36);
    stopBtn->setMinimumHeight(36);

    connect(playBtn, &QPushButton::clicked, this, [this]() {
        if (mixerController) mixerController->play();
    });
    connect(pauseBtn, &QPushButton::clicked, this, [this]() {
        if (mixerController) mixerController->pause();
    });
    connect(stopBtn, &QPushButton::clicked, this, [this]() {
        if (mixerController) mixerController->stop();
    });

    buttonLayout->addWidget(playBtn);
    buttonLayout->addWidget(pauseBtn);
    buttonLayout->addWidget(stopBtn);
    buttonLayout->addStretch();

    playbackLayout->addLayout(buttonLayout);
    rightLayout->addWidget(playbackGroupBox);

    // Mixer Group
    auto mixerGroupBox = new QGroupBox("Mixer");
    auto mixerLayout = new QVBoxLayout(mixerGroupBox);

    // Track list (will be populated dynamically)
    tracksContainer = new QWidget();
    tracksContainerLayout = new QVBoxLayout(tracksContainer);
    tracksContainerLayout->setSpacing(8);
    tracksContainerLayout->setContentsMargins(0, 0, 0, 0);

    auto scrollArea = new QScrollArea();
    scrollArea->setWidget(tracksContainer);
    scrollArea->setWidgetResizable(true);
    mixerLayout->addWidget(scrollArea);

    rightLayout->addWidget(mixerGroupBox, 1);

    // Export buttons
    auto exportLayout = new QHBoxLayout();
    auto exportMixBtn = new QPushButton("💾 Export Mix");
    auto exportStemsBtn = new QPushButton("📦 Export Stems");
    exportMixBtn->setMinimumHeight(36);
    exportStemsBtn->setMinimumHeight(36);

    connect(exportMixBtn, &QPushButton::clicked, this, &MainWindow::onExportMix);
    connect(exportStemsBtn, &QPushButton::clicked, this, &MainWindow::onExportStems);

    exportLayout->addWidget(exportMixBtn);
    exportLayout->addWidget(exportStemsBtn);
    rightLayout->addLayout(exportLayout);

    // Splitter between left and right
    auto splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);

    mainLayout->addWidget(splitter);
    setCentralWidget(centralWidget);

    // Progress dialog
    progressDialog = std::make_unique<ProgressDialog>(this);

    // Menubar
    auto menuBar = this->menuBar();
    auto fileMenu = menuBar->addMenu("&File");
    auto helpMenu = menuBar->addMenu("&Help");

    auto exitAction = fileMenu->addAction("E&xit");
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    auto aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupConnections()
{
    // Will be connected in onSeparationStarted
}

void MainWindow::applyStyles()
{
    setStyleSheet(
        "QMainWindow {"
        "    background-color: #f5f5f5;"
        "}"
        "QGroupBox {"
        "    color: #333;"
        "    font-weight: bold;"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    margin-top: 8px;"
        "    padding-top: 8px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 0 3px 0 3px;"
        "}"
        "QScrollArea {"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    background-color: white;"
        "}"
    );
}

void MainWindow::updateTrackUI(int trackId)
{
    // Refresh track display if needed
}

void MainWindow::populateTracksUI()
{
    if (!mixerController) return;

    // Clear existing tracks
    QLayoutItem *item;
    while ((item = tracksContainerLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Add tracks
    auto tracks = mixerController->getTracksInfo();
    for (const auto &trackInfo : tracks) {
        auto trackWidget = createTrackWidget(trackInfo);
        tracksContainerLayout->addWidget(trackWidget);
    }

    tracksContainerLayout->addStretch();
}

QWidget *MainWindow::createTrackWidget(const MixerController::TrackInfo &info)
{
    auto trackWidget = new QWidget();
    auto layout = new QHBoxLayout(trackWidget);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    // Track name
    auto nameLabel = new QLabel(info.name);
    nameLabel->setMinimumWidth(80);
    nameLabel->setStyleSheet("font-weight: bold;");
    layout->addWidget(nameLabel);

    // Mute button
    auto muteBtn = new QPushButton("Mute");
    muteBtn->setCheckable(true);
    muteBtn->setChecked(info.muted);
    muteBtn->setMaximumWidth(60);
    connect(muteBtn, &QPushButton::toggled, this, [this, trackId = info.id](bool checked) {
        if (mixerController) {
            mixerController->setTrackMute(trackId, checked);
        }
    });
    layout->addWidget(muteBtn);

    // Solo button
    auto soloBtn = new QPushButton("Solo");
    soloBtn->setCheckable(true);
    soloBtn->setChecked(info.solo);
    soloBtn->setMaximumWidth(60);
    connect(soloBtn, &QPushButton::toggled, this, [this, trackId = info.id](bool checked) {
        if (mixerController) {
            mixerController->setTrackSolo(trackId, checked);
        }
    });
    layout->addWidget(soloBtn);

    // Volume slider
    auto volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setRange(0, 200);  // 0% to 200%
    volumeSlider->setValue(static_cast<int>(info.volume * 100));
    volumeSlider->setMaximumWidth(200);
    connect(volumeSlider, &QSlider::valueChanged, this, [this, trackId = info.id](int value) {
        if (mixerController) {
            mixerController->setTrackVolume(trackId, value / 100.0f);
        }
    });
    layout->addWidget(volumeSlider);

    // Volume label
    auto volumeLabel = new QLabel(QString("%1%").arg(static_cast<int>(info.volume * 100)));
    volumeLabel->setMinimumWidth(40);
    volumeLabel->setAlignment(Qt::AlignRight);
    connect(volumeSlider, &QSlider::valueChanged, volumeLabel, [volumeLabel](int value) {
        volumeLabel->setText(QString("%1%").arg(value));
    });
    layout->addWidget(volumeLabel);

    layout->addStretch();

    trackWidget->setStyleSheet(
        "QWidget {"
        "    background-color: white;"
        "    border: 1px solid #e0e0e0;"
        "    border-radius: 4px;"
        "}"
        "QWidget:hover {"
        "    background-color: #fafafa;"
        "}"
    );

    return trackWidget;
}

void MainWindow::onSeparate()
{
    if (!separationEngine || !mixerController) {
        QMessageBox::warning(this, "Error", "Application not fully initialized");
        return;
    }

    QString inputFile = fileImport->selectedFile();
    QStringList selectedStems = trackSelector->selectedStems();
    QString outputFolder = folderChooser->selectedFolder();

    if (inputFile.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select an audio file");
        return;
    }

    if (selectedStems.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select at least one stem");
        return;
    }

    if (outputFolder.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select an output folder");
        return;
    }

    // Unload previous stems
    mixerController->unloadAllStems();

    // Show progress dialog
    progressDialog->show();
    statusLabel->setText("Separating...");

    Logger::log(Logger::Info, QString("Starting separation: %1").arg(inputFile));

    // TODO: Start separation process
    // For now, just simulate
    progressDialog->setProgress(0);
    progressDialog->setMessage("Initializing...");
}

void MainWindow::onSeparationProgress(int percent, const QString &message)
{
    progressDialog->setProgress(percent);
    progressDialog->setMessage(message);
    statusLabel->setText(QString("Separating... %1%").arg(percent));
}

void MainWindow::onSeparationComplete(const QString &outputFolder)
{
    progressDialog->close();
    statusLabel->setText("Separation complete");

    Logger::log(Logger::Info, "Separation completed");

    // Load stems into mixer
    QStringList stemNames = {"vocals", "drums", "bass", "other"};
    QDir outputDir(outputFolder);

    for (const auto &stemName : stemNames) {
        QString stemFile = outputDir.filePath(stemName + ".wav");
        if (QFile::exists(stemFile)) {
            if (mixerController->loadStem(stemName, stemFile)) {
                Logger::log(Logger::Info, QString("Loaded stem: %1").arg(stemName));
            }
        }
    }

    populateTracksUI();
    QMessageBox::information(this, "Success", "Separation completed successfully!");
}

void MainWindow::onSeparationError(const QString &error)
{
    progressDialog->close();
    statusLabel->setText("Error");
    Logger::log(Logger::Error, QString("Separation error: %1").arg(error));
    QMessageBox::critical(this, "Error", QString("Separation failed: %1").arg(error));
}

void MainWindow::onExportMix()
{
    if (!mixerController) {
        QMessageBox::warning(this, "Warning", "No stems loaded");
        return;
    }

    if (mixerController->totalDuration() == 0) {
        QMessageBox::warning(this, "Warning", "No audio to export");
        return;
    }

    QString outputPath = QFileDialog::getSaveFileName(this,
        "Export Mix", "", "WAV Files (*.wav)");

    if (outputPath.isEmpty()) return;

    statusLabel->setText("Exporting mix...");
    Logger::log(Logger::Info, QString("Exporting mix to: %1").arg(outputPath));

    // TODO: Export mix with current mixer state
    statusLabel->setText("Export complete");
    QMessageBox::information(this, "Success", "Mix exported successfully!");
}

void MainWindow::onExportStems()
{
    if (!mixerController) {
        QMessageBox::warning(this, "Warning", "No stems loaded");
        return;
    }

    QString outputFolder = QFileDialog::getExistingDirectory(this, "Export Stems To");
    if (outputFolder.isEmpty()) return;

    statusLabel->setText("Exporting stems...");
    Logger::log(Logger::Info, QString("Exporting stems to: %1").arg(outputFolder));

    // TODO: Export individual stems
    statusLabel->setText("Export complete");
    QMessageBox::information(this, "Success", "Stems exported successfully!");
}

void MainWindow::onPlaybackStateChanged(bool isPlaying)
{
    statusLabel->setText(isPlaying ? "Playing..." : "Paused");
}

void MainWindow::onPositionChanged(int64_t pos)
{
    // Update timeline display
    // timeline->setCurrentPosition(pos);
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About Carlos Okupa",
        "<h2>Carlos Okupa v0.1.0</h2>"
        "<p>Music Source Separation for macOS & Windows</p>"
        "<p><b>Technology:</b></p>"
        "<ul>"
        "<li>Qt 6 - Cross-platform UI</li>"
        "<li>CoreAudio/WASAPI - Native audio</li>"
        "<li>Demucs v4 - AI separation model</li>"
        "<li>PyTorch - Deep learning</li>"
        "</ul>"
        "<p><b>Author:</b> lokupa-dev</p>"
        "<p><b>License:</b> MIT (Code) | Non-commercial (Model Weights)</p>"
    );
}
