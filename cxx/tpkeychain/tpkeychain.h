#ifndef TPKEYCHAIN_H
#define TPKEYCHAIN_H

#include "keychain.h"

#include <QObject>

class TPKeyChain : public QObject
{

Q_OBJECT

public:
    explicit TPKeyChain(QObject *parent = nullptr);

    Q_INVOKABLE void readKey(const QString &key);
    Q_INVOKABLE void writeKey(const QString &key, const QString &value);
    Q_INVOKABLE void deleteKey(const QString &key);

Q_SIGNALS:
    void keyStored(const QString &key);
    void keyRestored(const QString &key, const QString &value);
    void keyDeleted(const QString &key);
    void error(const QString &errorText);

private:
    QKeychain::ReadPasswordJob m_readCredentialJob;
    QKeychain::WritePasswordJob m_writeCredentialJob;
    QKeychain::DeletePasswordJob m_deleteCredentialJob;

    static TPKeyChain *_appKeyChain;
    friend TPKeyChain *appkeyChain();
};

inline TPKeyChain *appkeyChain() { return TPKeyChain::_appKeyChain; }

#endif // TPKEYCHAIN_H
