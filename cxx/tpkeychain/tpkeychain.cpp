#include "tpkeychain.h"
#include "keychain.h"

TPKeyChain *TPKeyChain::_appKeyChain{nullptr};

using namespace Qt::Literals::StringLiterals;

void TPKeyChain::readKey(const QString &key)
{
	QKeychain::ReadPasswordJob *readCredentialJob{new QKeychain::ReadPasswordJob{key, this}};
	readCredentialJob->setKey(key);
	readCredentialJob->setService(key);
	connect(readCredentialJob, &QKeychain::ReadPasswordJob::finished, this, [this,key] (QKeychain::Job *readCredentialJob) {
		const bool ok{readCredentialJob->error() == QKeychain::NoError};
		if (ok) Q_LIKELY_BRANCH
			emit keyRestored(true, key, static_cast<QKeychain::ReadPasswordJob*>(readCredentialJob)->binaryData(), QString{});
		else Q_UNLIKELY_BRANCH
			emit keyRestored(false, key, QString{}, "Read key failed: "_L1 % readCredentialJob->errorString());
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
		emit keyStored(ok, key, !ok ? "Write key failed: "_L1 % writeCredentialJob->errorString() : QString{});
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
		emit keyDeleted(ok, key, !ok ? "Delete key failed: "_L1 % deleteCredentialJob->errorString() : QString{});
	}, Qt::SingleShotConnection);
	deleteCredentialJob->start();
}
