#ifndef SEPARATIONENGINE_H
#define SEPARATIONENGINE_H

#include <QString>
#include <QStringList>
#include <memory>

class SeparationEngine
{
public:
    SeparationEngine();
    virtual ~SeparationEngine();

    bool initialize();
    void shutdown();

    bool startSeparation(const QString &inputFile,
                        const QStringList &stems,
                        const QString &outputFolder);
    void cancelSeparation();

private:
    bool loadModel(const QString &modelId);
    bool verifyProvenance();

    bool isInitialized_ = false;
};

#endif // SEPARATIONENGINE_H
