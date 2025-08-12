/******************************************************************************
 *   Copyright (C) 2011-2015 Frank Osterfeld <frank.osterfeld@gmail.com>      *
 *   Copyright (C) 2016 Mathias Hasselmann <mathias.hasselmann@kdab.com>      *
 *                                                                            *
 * This program is distributed in the hope that it will be useful, but        *
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE. For licensing and distribution        *
 * details, check the accompanying file 'COPYING'.                            *
 *****************************************************************************/

#include "plaintextstore_p.h"

using namespace QKeychain;
using namespace Qt::Literals::StringLiterals;

namespace {

inline QString dataKey(const QString &key)
{
	return key + "/data"_L1;
}
inline QString typeKey(const QString &key)
{
	return key + "/type"_L1;
}

} // namespace

PlainTextStore::PlainTextStore(const QString &service, QSettings *settings)
	: m_localSettings{settings ? nullptr : new QSettings{service}},
	  m_actualSettings{settings ? settings : m_localSettings.data()},
	  m_error{NoError}
{
}

bool PlainTextStore::contains(const QString &key) const
{
    return m_actualSettings->contains(dataKey(key));
}

QByteArray PlainTextStore::readData(const QString &key)
{
    return read(dataKey(key)).toByteArray();
}

#ifndef Q_OS_WIN

JobPrivate::Mode PlainTextStore::readMode(const QString &key)
{
    return JobPrivate::stringToMode(read(typeKey(key)).toString());
}

#endif // Q_OS_WIN

void PlainTextStore::write(const QString &key, const QByteArray &data, JobPrivate::Mode mode)
{
    if (m_actualSettings->status() != QSettings::NoError)
        return;

    m_actualSettings->setValue(typeKey(key), JobPrivate::modeToString(mode));
    m_actualSettings->setValue(dataKey(key), data);
    m_actualSettings->sync();

	if (m_actualSettings->status() == QSettings::AccessError)
        setError(AccessDenied, tr("Could not store data in settings: access error"));
	else if (m_actualSettings->status() != QSettings::NoError)
        setError(OtherError, tr("Could not store data in settings: format error"));
	else
		setError(NoError, QString{});
}

void PlainTextStore::remove(const QString &key)
{
    if (m_actualSettings->status() != QSettings::NoError)
        return;

    m_actualSettings->remove(typeKey(key));
    m_actualSettings->remove(dataKey(key));
    m_actualSettings->sync();

	if (m_actualSettings->status() == QSettings::AccessError)
        setError(AccessDenied, tr("Could not delete data from settings: access error"));
	else if (m_actualSettings->status() != QSettings::NoError)
        setError(OtherError, tr("Could not delete data from settings: format error"));
	else
		setError(NoError, QString{});
}

void PlainTextStore::setError(Error error, const QString &errorString)
{
    m_error = error;
    m_errorString = errorString;
}

QVariant PlainTextStore::read(const QString &key)
{
	const QVariant &value{m_actualSettings->value(key)};

	if (value.isNull())
        setError(EntryNotFound, tr("Entry not found"));
	else
		setError(NoError, QString{});

    return value;
}
