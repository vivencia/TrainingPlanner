/******************************************************************************
 *   Copyright (C) 2011-2015 Frank Osterfeld <frank.osterfeld@gmail.com>	  *
 *																			*
 * This program is distributed in the hope that it will be useful, but		*
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE. For licensing and distribution		*
 * details, check the accompanying file 'COPYING'.							*
 *****************************************************************************/
#pragma once

#include <QCoreApplication>
#include <QObject>
#include <QPointer>
#include <QSettings>
#include <QQueue>

#include "keychain.h"

namespace QKeychain {

class JobExecutor;

class JobPrivate : public QObject
{

Q_OBJECT

public:
	enum Mode { Text, Binary };

	virtual void scheduledStart() = 0;

	static QString modeToString(Mode m);
	static Mode stringToMode(const QString &s);

	Job *const q;
	Mode mode;
	QByteArray data;

protected:
	JobPrivate(const QString &service_, Job *q);

	QKeychain::Error error;
	QString errorString;
	QString service;
	bool autoDelete;
	bool insecureFallback;
	QPointer<QSettings> settings;
	QString key;

	friend class Job;
	friend class JobExecutor;
	friend class ReadPasswordJob;
	friend class WritePasswordJob;
	friend class PlainTextStore;
};

class ReadPasswordJobPrivate : public JobPrivate
{
	Q_OBJECT
public:
	explicit ReadPasswordJobPrivate(const QString &service_, ReadPasswordJob *qq);
	void scheduledStart() override;
	void fallbackOnError();

	friend class ReadPasswordJob;
};

class WritePasswordJobPrivate : public JobPrivate
{
	Q_OBJECT
public:
	explicit WritePasswordJobPrivate(const QString &service_, WritePasswordJob *qq);
	void scheduledStart() override;
	void fallbackOnError();

	friend class WritePasswordJob;
};

class DeletePasswordJobPrivate : public JobPrivate
{
	Q_OBJECT
public:
	explicit DeletePasswordJobPrivate(const QString &service_, DeletePasswordJob *qq);
	void scheduledStart() override;
	void fallbackOnError();

protected:
	void doStart();

	friend class DeletePasswordJob;
};

class JobExecutor : public QObject
{
	Q_OBJECT
public:
	static JobExecutor *instance();

	void enqueue(Job *job);

private:
	explicit JobExecutor();
	void startNextIfNoneRunning();

private Q_SLOTS:
	void jobFinished(QKeychain::Job *);
	void jobDestroyed(QObject *object);

private:
	static JobExecutor *s_instance;
	QQueue<QPointer<Job>> m_queue;
	bool m_jobRunning;
};

} // namespace QKeychain
