#ifndef MODELPROVENANCE_H
#define MODELPROVENANCE_H

#include <QString>
#include <QJsonObject>

class ModelProvenance
{
public:
    ModelProvenance();
    virtual ~ModelProvenance();

    bool loadFromFile(const QString &filePath);
    bool verifyModel(const QString &modelId);
    bool isCommercialAuthorized(const QString &modelId) const;

private:
    QJsonObject provenanceData;
};

#endif // MODELPROVENANCE_H
