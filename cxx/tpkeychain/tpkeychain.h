#pragma once

#include <QObject>

class TPKeyChain : public QObject
{

Q_OBJECT

public:
	explicit inline TPKeyChain(QObject *parent = nullptr): QObject{parent} { _appKeyChain = this; }

	Q_INVOKABLE void readKey(const QString &key);
	Q_INVOKABLE void writeKey(const QString &key, const QString &value);
	Q_INVOKABLE void deleteKey(const QString &key);

signals:
	void keyStored(const bool ok, const QString &key, const QString &error_string);
	void keyRestored(const bool ok, const QString &key, const QString &value, const QString &error_string);
	void keyDeleted(const bool ok, const QString &key, const QString &error_string);

private:
	static TPKeyChain *_appKeyChain;
	friend TPKeyChain *appKeyChain();
};

inline TPKeyChain *appKeyChain() { return TPKeyChain::_appKeyChain; }
