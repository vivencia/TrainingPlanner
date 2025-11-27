/******************************************************************************
 *   Copyright (C) 2011-2015 Frank Osterfeld <frank.osterfeld@gmail.com>	  *
 *																			*
 * This program is distributed in the hope that it will be useful, but		*
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE. For licensing and distribution		*
 * details, check the accompanying file 'COPYING'.							*
 *****************************************************************************/
#include "keychain.h"
#include "keychain_p.h"

using namespace QKeychain;
using namespace Qt::Literals::StringLiterals;

Job::Job(JobPrivate *q, QObject *parent) : QObject{parent}, d{q} {}
Job::~Job() { delete d; }

QString Job::service() const
{
	return d->service;
}

void Job::setService(const QString &new_service)
{
	d->service = new_service;
}

QSettings *Job::settings() const
{
	return d->settings;
}

void Job::setSettings(QSettings *settings)
{
	d->settings = settings;
}

void Job::start()
{
	QMetaObject::invokeMethod(this, "doStart", Qt::QueuedConnection);
}

bool Job::autoDelete() const
{
	return d->autoDelete;
}

void Job::setAutoDelete(bool autoDelete)
{
	d->autoDelete = autoDelete;
}

bool Job::insecureFallback() const
{
	return d->insecureFallback;
}

void Job::setInsecureFallback(bool insecureFallback)
{
	d->insecureFallback = insecureFallback;
}

void Job::doStart()
{
	JobExecutor::instance()->enqueue(this);
}

void Job::emitFinished()
{
	emit finished(this);
	if (d->autoDelete)
		deleteLater();
}

void Job::emitFinishedWithError(Error error, const QString &errorString)
{
	d->error = error;
	d->errorString = errorString;
	emitFinished();
}

void Job::scheduledStart()
{
	d->scheduledStart();
}

Error Job::error() const
{
	return d->error;
}

QString Job::errorString() const
{
	return d->errorString;
}

void Job::setError(Error error)
{
	d->error = error;
}

void Job::setErrorString(const QString &errorString)
{
	d->errorString = errorString;
}

ReadPasswordJob::ReadPasswordJob(const QString &service, QObject *parent)
	: Job{new ReadPasswordJobPrivate{service, this}, parent} {}
ReadPasswordJob::~ReadPasswordJob() {}

QString ReadPasswordJob::textData() const
{
	return QString::fromUtf8(d->data);
}

QByteArray ReadPasswordJob::binaryData() const
{
	return d->data;
}

QString Job::key() const
{
	return d->key;
}

void Job::setKey(const QString &key_)
{
	d->key = key_;
}

WritePasswordJob::WritePasswordJob(const QString &service, QObject *parent)
	: Job{new WritePasswordJobPrivate{service, this}, parent} {}
WritePasswordJob::~WritePasswordJob() {}

void WritePasswordJob::setBinaryData(const QByteArray &data)
{
	d->data = data;
	d->mode = JobPrivate::Binary;
}

void WritePasswordJob::setTextData(const QString &data)
{
	d->data = data.toUtf8();
	d->mode = JobPrivate::Text;
}

DeletePasswordJob::DeletePasswordJob(const QString &service, QObject *parent)
	: Job{new DeletePasswordJobPrivate{service, this}, parent} {}
DeletePasswordJob::~DeletePasswordJob() {}

void JobExecutor::enqueue(Job *job)
{
	m_queue.enqueue(job);
	startNextIfNoneRunning();
}

void JobExecutor::startNextIfNoneRunning()
{
	if (m_queue.isEmpty() || m_jobRunning)
		return;
	QPointer<Job> next;
	while (!next && !m_queue.isEmpty())
		next = m_queue.dequeue();
	if (next)
	{
		connect(next, &Job::finished, this, &JobExecutor::jobFinished);
		connect(next, &Job::destroyed, this, &JobExecutor::jobDestroyed);
		m_jobRunning = true;
		next->scheduledStart();
	}
}

void JobExecutor::jobDestroyed(QObject *object)
{
	Job *job{static_cast<Job *>(object)};
	Q_UNUSED(object) // for release mode
	job->disconnect(this);
	m_jobRunning = false;
	startNextIfNoneRunning();
}

void JobExecutor::jobFinished(Job *job)
{
	Q_UNUSED(job) // for release mode
	job->disconnect(this);
	m_jobRunning = false;
	startNextIfNoneRunning();
}

JobExecutor *JobExecutor::s_instance{nullptr};

QString JobPrivate::modeToString(Mode m)
{
	switch (m)
	{
		case Text:
			return "Text"_L1;
		case Binary:
			return "Binary"_L1;
	}
	Q_ASSERT_X(false, Q_FUNC_INFO, "Unhandled Mode value");
	return QString{};
}

JobPrivate::Mode JobPrivate::stringToMode(const QString &s)
{
	if (s == "Text"_L1 || s == "1"_L1)
		return Text;
	if (s == "Binary"_L1 || s == "2"_L1)
		return Binary;
	qCritical("Unexpected mode string '%s'", qPrintable(s));
	return Text;
}
