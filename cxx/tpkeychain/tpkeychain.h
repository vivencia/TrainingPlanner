#pragma once

#include <QObject>

class TPKeyChain : public QObject
{

Q_OBJECT

public:
    explicit inline TPKeyChain(QObject *parent = nullptr): QObject{parent} { _appKeyChain = this; }

    Q_INVOKABLE void readKey(const QString &key);
    Q_INVOKABLE void writeKey(const QString &key, const QString &value);
    Q_INVOKABLE void deleteKey(const QString &key);

signals:
    void keyStored(const QString &key);
    void keyRestored(const QString &key, const QString &value);
    void keyDeleted(const QString &key);
    void error(const QString &errorText);

private:
    static TPKeyChain *_appKeyChain;
    friend TPKeyChain *appKeyChain();
};

inline TPKeyChain *appKeyChain() { return TPKeyChain::_appKeyChain; }
