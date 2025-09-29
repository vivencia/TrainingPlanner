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
		if (readCredentialJob->error())
		{
			emit error(tr("Read key failed: %1").arg(qPrintable(readCredentialJob->errorString())));
			return;
		}
		emit keyRestored(key, static_cast<QKeychain::ReadPasswordJob*>(readCredentialJob)->binaryData());
	}, Qt::SingleShotConnection);

	readCredentialJob->start();
}

void TPKeyChain::writeKey(const QString &key, const QString &value)
{
	QKeychain::WritePasswordJob *writeCredentialJob{new QKeychain::WritePasswordJob{key, this}};
	writeCredentialJob->setKey(key);
	writeCredentialJob->setAutoDelete(true);

	connect(writeCredentialJob, &QKeychain::WritePasswordJob::finished, this, [this,key] (QKeychain::Job *writeCredentialJob) {
		if (writeCredentialJob->error())
		{
			emit error(tr("Write key failed: %1").arg(qPrintable(writeCredentialJob->errorString())));
			return;
		}
		emit keyStored(key);
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
		if (deleteCredentialJob->error())
		{
			emit error(tr("Delete key failed: %1").arg(qPrintable(deleteCredentialJob->errorString())));
			return;
		}
		emit keyDeleted(key);
	}, Qt::SingleShotConnection);

	deleteCredentialJob->start();
}
