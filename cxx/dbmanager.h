#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QQmlEngine>
#include <QSqlDatabase>
#include <QSettings>
#include <QQmlApplicationEngine>

class DbManager : public QObject
{

Q_OBJECT
QML_ELEMENT

public:
	explicit DbManager(const QString& dbFileName, QSettings* appSettigs, QQmlApplicationEngine* QMlEngine, float mostRecentListVersion);
	Q_INVOKABLE bool updateExercisesList(const QStringList& exercisesList);
	void removePreviousListEntriesFromDB();

private:
	QSqlDatabase mSqlLiteDB;
	QSettings* m_appSettings;
	QQmlApplicationEngine* m_QMlEngine;
	float m_mostRecentListVersion;
};

#endif // DBMANAGER_H
