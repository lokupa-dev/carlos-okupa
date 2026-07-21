#ifndef EXPORTMANAGER_H
#define EXPORTMANAGER_H

#include <QString>
#include <QStringList>

class ExportManager
{
public:
    ExportManager();
    virtual ~ExportManager();

    bool exportMix(const QString &outputFolder);
    bool exportStems(const QString &outputFolder, const QStringList &stems);

private:
};

#endif // EXPORTMANAGER_H
