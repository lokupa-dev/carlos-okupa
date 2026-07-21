#ifndef HASHVERIFIER_H
#define HASHVERIFIER_H

#include <QString>

class HashVerifier
{
public:
    static QString calculateSHA256(const QString &filePath);
    static bool verifySHA256(const QString &filePath, const QString &expectedHash);
};

#endif // HASHVERIFIER_H
