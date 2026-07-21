#include "modelprovenance.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>

ModelProvenance::ModelProvenance() = default;

ModelProvenance::~ModelProvenance() = default;

bool ModelProvenance::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject()) {
        return false;
    }

    provenanceData = doc.object();
    return true;
}

bool ModelProvenance::verifyModel(const QString &modelId)
{
    // TODO: Verify model exists and hash matches
    return true;
}

bool ModelProvenance::isCommercialAuthorized(const QString &modelId) const
{
    QJsonArray models = provenanceData["models"].toArray();
    for (const auto &modelRef : models) {
        QJsonObject model = modelRef.toObject();
        if (model["id"].toString() == modelId) {
            return model["commercial_authorized"].toBool();
        }
    }
    return false;
}
