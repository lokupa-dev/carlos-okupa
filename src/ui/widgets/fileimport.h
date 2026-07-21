#ifndef FILEIMPORT_H
#define FILEIMPORT_H

#include <QWidget>
#include <QString>
#include <memory>

class QLabel;
class QPushButton;

class FileImport : public QWidget
{
    Q_OBJECT

public:
    explicit FileImport(QWidget *parent = nullptr);
    ~FileImport();

    QString selectedFile() const { return currentFile; }
    QString fileName() const;
    QString fileInfo() const;

private slots:
    void onBrowse();
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    void setupUI();
    void setFile(const QString &filePath);
    bool validateAudioFile(const QString &filePath);

    QString currentFile;
    std::unique_ptr<QLabel> infoLabel;
    std::unique_ptr<QPushButton> browseButton;
};

#endif // FILEIMPORT_H
