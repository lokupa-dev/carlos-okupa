#include "hashverifier.h"
#include <QCryptographicHash>
#include <QFile>

QString HashVerifier::calculateSHA256(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    while (true) {
        QByteArray chunk = file.read(4096);
        if (chunk.isEmpty()) {
            break;
        }
        hash.addData(chunk);
    }
    file.close();

    return QString(hash.result().toHex());
}

bool HashVerifier::verifySHA256(const QString &filePath, const QString &expectedHash)
{
    QString calculatedHash = calculateSHA256(filePath);
    return calculatedHash.compare(expectedHash, Qt::CaseInsensitive) == 0;
}
