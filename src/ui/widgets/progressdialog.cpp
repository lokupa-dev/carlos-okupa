#include "progressdialog.h"
#include <QVBoxLayout>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>

ProgressDialog::ProgressDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Separating...");
    setModal(true);
    setMinimumWidth(400);
    setupUI();
}

ProgressDialog::~ProgressDialog() = default;

void ProgressDialog::setupUI()
{
    auto layout = new QVBoxLayout(this);

    messageLabel = std::make_unique<QLabel>("Initializing separation...");
    layout->addWidget(messageLabel.get());

    progressBar = std::make_unique<QProgressBar>();
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    layout->addWidget(progressBar.get());

    cancelButton = std::make_unique<QPushButton>("Cancel");
    connect(cancelButton.get(), &QPushButton::clicked, this, &ProgressDialog::onCancel);
    layout->addWidget(cancelButton.get());
}

void ProgressDialog::setProgress(int percent)
{
    progressBar->setValue(percent);
}

void ProgressDialog::setMessage(const QString &message)
{
    messageLabel->setText(message);
}

void ProgressDialog::setError(const QString &error)
{
    messageLabel->setText("Error: " + error);
    messageLabel->setStyleSheet("color: red;");
}

void ProgressDialog::onCancel()
{
    // TODO: Cancel separation
    close();
}
