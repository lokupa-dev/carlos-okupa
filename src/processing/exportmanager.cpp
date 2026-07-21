#include "exportmanager.h"

ExportManager::ExportManager() = default;

ExportManager::~ExportManager() = default;

bool ExportManager::exportMix(const QString &outputFolder)
{
    // TODO: Implement offline mix rendering
    return true;
}

bool ExportManager::exportStems(const QString &outputFolder, const QStringList &stems)
{
    // TODO: Copy/link separated stems to output folder
    return true;
}
