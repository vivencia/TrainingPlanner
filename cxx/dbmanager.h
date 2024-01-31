#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QQmlEngine>
#include <QSqlDatabase>
#include <QSettings>
#include <QQmlApplicationEngine>


class dbWorkerThread;

class DbManager : public QObject
{

Q_OBJECT
Q_PROPERTY(QString exercisesListVersion READ exercisesListVersion CONSTANT FINAL)

public:
	explicit DbManager(const QString& dbFilename, QSettings* appSettigs, QQmlApplicationEngine* QMlEngine, const QString& mostRecentListVersion);
	Q_INVOKABLE void updateExercisesList();

	inline const QString exercisesListVersion() const { return m_exercisesListVersion; }

private:
	QString m_DBFileName;
	QSettings* m_appSettings;
	QQmlApplicationEngine* m_QMlEngine;
	QString m_exercisesListVersion;
};

class dbWorkerThread : public QObject
{
Q_OBJECT

public:
	explicit dbWorkerThread(const QString& dbFileName, QSettings* appSettings);

	inline void receiveParameter(const QStringList& qslist) { m_qslist = qslist; }
	inline void receiveParameter(const QString& string) { m_string = string; }
	void updateExercisesList();

signals:
	void gotResult ( const QString& info );
	void done ();

private:
	QSqlDatabase mSqlLiteDB;
	QSettings* m_appSettings;

	QStringList m_qslist;
	QString m_string;

	void removePreviousListEntriesFromDB();
};

#endif // DBMANAGER_H
