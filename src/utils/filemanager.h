#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QString>
#include <QStringList>

class FileManager
{
public:
    static bool createDirectory(const QString &path);
    static bool deleteFile(const QString &path);
    static bool deleteDirectory(const QString &path);
    static bool copyFile(const QString &src, const QString &dst);
    static qint64 fileSize(const QString &path);
};

#endif // FILEMANAGER_H
