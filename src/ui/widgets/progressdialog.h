#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#include <QString>
#include <memory>

class QProgressBar;
class QLabel;
class QPushButton;

class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = nullptr);
    ~ProgressDialog();

    void setProgress(int percent);
    void setMessage(const QString &message);
    void setError(const QString &error);

private slots:
    void onCancel();

private:
    void setupUI();

    std::unique_ptr<QProgressBar> progressBar;
    std::unique_ptr<QLabel> messageLabel;
    std::unique_ptr<QPushButton> cancelButton;
};

#endif // PROGRESSDIALOG_H
