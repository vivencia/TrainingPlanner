#include "tpkeychain.h"

using namespace Qt::Literals::StringLiterals;

TPKeyChain::TPKeyChain(QObject *parent)
	: QObject{parent},
	  m_readCredentialJob{"org.vivenciasoftware.TrainingPlanner"_L1},
	  m_writeCredentialJob{"org.vivenciasoftware.TrainingPlanner"_L1},
	  m_deleteCredentialJob{"org.vivenciasoftware.TrainingPlanner"_L1}
{
	m_readCredentialJob.setAutoDelete(false);
	m_writeCredentialJob.setAutoDelete(false);
	m_deleteCredentialJob.setAutoDelete(false);
}

void TPKeyChain::readKey(const QString &key)
{
	m_readCredentialJob.setKey(key);

	connect(&m_readCredentialJob, &QKeychain::ReadPasswordJob::finished, this, [this,key] (QKeychain::Job *) {
		if (m_readCredentialJob.error()) {
			emit error(tr("Read key failed: %1").arg(qPrintable(m_readCredentialJob.errorString())));
			return;
		}
		emit keyRestored(key, m_readCredentialJob.textData());
	});

	m_readCredentialJob.start();
}

void TPKeyChain::writeKey(const QString &key, const QString &value)
{
	m_writeCredentialJob.setKey(key);

	connect(&m_writeCredentialJob, &QKeychain::WritePasswordJob::finished, this, [this,key] (QKeychain::Job *) {
		if (m_writeCredentialJob.error()) {
			emit error(tr("Write key failed: %1").arg(qPrintable(m_writeCredentialJob.errorString())));
			return;
		}
		emit keyStored(key);
	});

	m_writeCredentialJob.setTextData(value);
	m_writeCredentialJob.start();
}

void TPKeyChain::deleteKey(const QString &key)
{
	m_deleteCredentialJob.setKey(key);

	connect(&m_deleteCredentialJob, &QKeychain::DeletePasswordJob::finished, this, [this,key] (QKeychain::Job *) {
		if (m_deleteCredentialJob.error()) {
			emit error(tr("Delete key failed: %1").arg(qPrintable(m_deleteCredentialJob.errorString())));
			return;
		}
		emit keyDeleted(key);
	});

	m_deleteCredentialJob.start();
}
