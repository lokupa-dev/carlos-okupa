#ifndef FOLDERCHOOSER_H
#define FOLDERCHOOSER_H

#include <QWidget>
#include <QString>
#include <memory>

class QLabel;
class QPushButton;

class FolderChooser : public QWidget
{
    Q_OBJECT

public:
    explicit FolderChooser(QWidget *parent = nullptr);
    ~FolderChooser();

    QString selectedFolder() const { return currentFolder; }

private slots:
    void onBrowse();

private:
    void setupUI();
    void setFolder(const QString &folderPath);

    QString currentFolder;
    std::unique_ptr<QLabel> pathLabel;
};

#endif // FOLDERCHOOSER_H
