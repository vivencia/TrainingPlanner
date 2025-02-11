/******************************************************************************
 *   Copyright (C) 2011-2015 Frank Osterfeld <frank.osterfeld@gmail.com>      *
 *                                                                            *
 * This program is distributed in the hope that it will be useful, but        *
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE. For licensing and distribution        *
 * details, check the accompanying file 'COPYING'.                            *
 *****************************************************************************/
#include "keychain_p.h"
#include "libsecret_p.h"
#include "plaintextstore_p.h"

#include "../tpglobals.h"

#include <QScopedPointer>

using namespace QKeychain;

// the following detection algorithm is derived from chromium,
// licensed under BSD, see base/nix/xdg_util.cc

void ReadPasswordJobPrivate::scheduledStart()
{
	if (LibSecretKeyring::findPassword(key, q->service(), this))
		q->emitFinished();
	else
	{
		DEFINE_SOURCE_LOCATION
		ERROR_MESSAGE("LibSecret", "Read password failed. Using fallback()")
		fallbackOnError();
	}
}

void ReadPasswordJobPrivate::fallbackOnError()
{
    PlainTextStore plainTextStore(q->service(), q->settings());

    if (q->insecureFallback() && plainTextStore.contains(key)) {
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
	if (LibSecretKeyring::writePassword(service, key, service, mode, data, this))
        q->emitFinished();
	else
	{
		DEFINE_SOURCE_LOCATION
		ERROR_MESSAGE("LibSecret", "Write password failed. Using fallback()")
		fallbackOnError();
	}
}

void WritePasswordJobPrivate::fallbackOnError()
{
    PlainTextStore plainTextStore(q->service(), q->settings());
    plainTextStore.write(key, data, mode);

    if (plainTextStore.error() != NoError)
        q->emitFinishedWithError(plainTextStore.error(), plainTextStore.errorString());
    else
        q->emitFinished();
}

void DeletePasswordJobPrivate::scheduledStart()
{
	if (LibSecretKeyring::deletePassword(key, q->service(), this))
		q->emitFinished();
	else
	{
		DEFINE_SOURCE_LOCATION
		ERROR_MESSAGE("LibSecret", "Delete password failed. Using fallback()")
		fallbackOnError();
	}
}

void DeletePasswordJobPrivate::fallbackOnError()
{
    QScopedPointer<QSettings> local(!q->settings() ? new QSettings(q->service()) : nullptr);
    QSettings *actual = q->settings() ? q->settings() : local.data();

    actual->remove(key);
    actual->sync();
    q->emitFinished();
}

bool QKeychain::isAvailable()
{
    return LibSecretKeyring::isAvailable();
}
