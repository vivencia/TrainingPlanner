#include "tpkeychain.h"
#include "keychain.h"

#include <QDebug>

TPKeyChain *TPKeyChain::_appKeyChain{nullptr};

using namespace Qt::Literals::StringLiterals;

void TPKeyChain::readKey(const QString &key)
{
	QKeychain::ReadPasswordJob *readCredentialJob{new QKeychain::ReadPasswordJob{key, this}};
	readCredentialJob->setKey(key);
	readCredentialJob->setService(key);
	connect(readCredentialJob, &QKeychain::ReadPasswordJob::finished, this, [this,key] (QKeychain::Job *readCredentialJob) {
		const bool ok{readCredentialJob->error() == QKeychain::NoError};
		if (!ok)
			qInfo() << "Read key failed: "_L1 << readCredentialJob->errorString();
		emit keyRestored(ok, key, ok
				? static_cast<QKeychain::ReadPasswordJob*>(readCredentialJob)->binaryData() : QString{});
	}, Qt::SingleShotConnection);
	readCredentialJob->start();
}

void TPKeyChain::writeKey(const QString &key, const QString &value)
{
	QKeychain::WritePasswordJob *writeCredentialJob{new QKeychain::WritePasswordJob{key, this}};
	writeCredentialJob->setKey(key);
	writeCredentialJob->setAutoDelete(true);
	connect(writeCredentialJob, &QKeychain::WritePasswordJob::finished, this, [this,key] (QKeychain::Job *writeCredentialJob) {
		const bool ok{writeCredentialJob->error() == QKeychain::NoError};
		if (!ok)
			qInfo() << "Write key failed: "_L1 << writeCredentialJob->errorString();
		emit keyStored(ok, key);
	}, Qt::SingleShotConnection);
	writeCredentialJob->setBinaryData(value.toLatin1());
	writeCredentialJob->start();
}

void TPKeyChain::deleteKey(const QString &key)
{
	QKeychain::DeletePasswordJob *deleteCredentialJob{new QKeychain::DeletePasswordJob{key, this}};
	deleteCredentialJob->setKey(key);
	deleteCredentialJob->setAutoDelete(true);
	connect(deleteCredentialJob, &QKeychain::DeletePasswordJob::finished, this, [this,key] (QKeychain::Job *deleteCredentialJob) {
		const bool ok{deleteCredentialJob->error() == QKeychain::NoError};
		if (!ok)
			qInfo() << "Delete key failed: "_L1 << deleteCredentialJob->errorString();
		emit keyDeleted(ok, key);
	}, Qt::SingleShotConnection);
	deleteCredentialJob->start();
}
