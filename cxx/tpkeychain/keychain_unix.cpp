/******************************************************************************
 *   Copyright (C) 2011-2015 Frank Osterfeld <frank.osterfeld@gmail.com>      *
 *                                                                            *
 * This program is distributed in the hope that it will be useful, but        *
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE. For licensing and distribution        *
 * details, check the accompanying file 'COPYING'.                            *
 *****************************************************************************/
#include <QtGlobal>

#ifndef Q_OS_ANDROID
#ifdef Q_OS_LINUX

#include "keychain_p.h"
#include "libsecret_p.h"
#include "plaintextstore_p.h"

#include <QScopedPointer>

using namespace QKeychain;

// the following detection algorithm is derived from chromium,
// licensed under BSD, see base/nix/xdg_util.cc

void ReadPasswordJobPrivate::scheduledStart()
{
	if (!LibSecretKeyring::findPassword(key, q->service(), this))
	{
		#ifndef QT_NO_DEBUG
		qDebug() << "keychain_unix::ReadPasswordJobPrivate::scheduledStart" << "Read password failed. Using fallback()";
		#endif
		fallbackOnError();
	}
}

void ReadPasswordJobPrivate::fallbackOnError()
{
	PlainTextStore plainTextStore{q->service(), q->settings()};
	if (q->insecureFallback() && plainTextStore.contains(key))
	{
        mode = plainTextStore.readMode(key);
        data = plainTextStore.readData(key);

        if (plainTextStore.error() != NoError)
            q->emitFinishedWithError(plainTextStore.error(), plainTextStore.errorString());
        else
            q->emitFinished();
    }
}

void WritePasswordJobPrivate::scheduledStart()
{
	if (!LibSecretKeyring::writePassword(service, key, service, mode, data, this))
	{
		#ifndef QT_NO_DEBUG
		qDebug() << "keychain_unix::WritePasswordJobPrivate::scheduledStart() " << "Write password failed. Using fallback()";
		#endif
		fallbackOnError();
	}
}

void WritePasswordJobPrivate::fallbackOnError()
{
	PlainTextStore plainTextStore{q->service(), q->settings()};
    plainTextStore.write(key, data, mode);

    if (plainTextStore.error() != NoError)
        q->emitFinishedWithError(plainTextStore.error(), plainTextStore.errorString());
    else
        q->emitFinished();
}

void DeletePasswordJobPrivate::scheduledStart()
{
	if (!LibSecretKeyring::deletePassword(key, q->service(), this))
	{
		#ifndef QT_NO_DEBUG
		qDebug() << "keychain_unix::DeletePasswordJobPrivate::scheduledStart()" << "Delete password failed. Using fallback()";
		#endif
		fallbackOnError();
	}
}

void DeletePasswordJobPrivate::fallbackOnError()
{
	QScopedPointer<QSettings> local{!q->settings() ? new QSettings{q->service()} : nullptr};
	QSettings *actual{q->settings() ? q->settings() : local.data()};
    actual->remove(key);
    actual->sync();
    q->emitFinished();
}

bool QKeychain::isAvailable()
{
    return LibSecretKeyring::isAvailable();
}

#endif //Q_OS_LINUX
#endif //Q_OS_ANDROID
