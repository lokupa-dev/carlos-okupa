#include "filemanager.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>

bool FileManager::createDirectory(const QString &path)
{
    QDir dir;
    return dir.mkpath(path);
}

bool FileManager::deleteFile(const QString &path)
{
    QFile file(path);
    return file.remove();
}

bool FileManager::deleteDirectory(const QString &path)
{
    QDir dir(path);
    return dir.removeRecursively();
}

bool FileManager::copyFile(const QString &src, const QString &dst)
{
    QFile file(src);
    return file.copy(dst);
}

qint64 FileManager::fileSize(const QString &path)
{
    QFileInfo info(path);
    return info.size();
}
