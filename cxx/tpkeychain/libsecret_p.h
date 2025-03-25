#pragma once

#include <QtGlobal>

#ifndef Q_OS_ANDROID
#ifdef Q_OS_LINUX
#include <QLibrary>

#include "keychain_p.h"

class LibSecretKeyring : public QLibrary
{
public:
    static bool isAvailable();

    static bool findPassword(const QString &user, const QString &server,
                             QKeychain::JobPrivate *self);

    static bool writePassword(const QString &display_name, const QString &user,
                              const QString &server, const QKeychain::JobPrivate::Mode type,
                              const QByteArray &password, QKeychain::JobPrivate *self);

    static bool deletePassword(const QString &key, const QString &service,
                               QKeychain::JobPrivate *self);

private:
    LibSecretKeyring();

    static LibSecretKeyring &instance();
};

#endif //Q_OS_LINUX
#endif //Q_OS_ANDROID
