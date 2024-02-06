#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "tplistmodel.h"
#include "dbexercisestable.h"
#include "dbexercisesmodel.h"

#include <QQmlEngine>
#include <QSettings>
#include <QQmlApplicationEngine>

#include <functional>

class DbManager : public QObject
{

Q_OBJECT

public:
	explicit DbManager(QSettings* appSettigs, QQmlApplicationEngine* QMlEngine);
	void gotResult(const dbExercisesTable *dbObj, const OP_CODES op);

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
	Q_INVOKABLE void pass_object(QObject *obj) { m_model = static_cast<TPListModel*>(obj); }
	Q_INVOKABLE void getAllExercises();
	Q_INVOKABLE void newExercise(const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const QString& nSets, const QString& nReps, const QString& nWeight,
					 const QString& uWeight, const QString& mediaPath);
	Q_INVOKABLE void updateExercise(const QString& id, const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const QString& nSets, const QString& nReps, const QString& nWeight,
					 const QString& uWeight, const QString& mediaPath);
	Q_INVOKABLE void removeExercise(const QString& id);
	void getExercisesListVersion();

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

signals:
	void qmlReady();
	void databaseFree();

public slots:
	//void printOutInfo();

private:
	QString m_DBFilePath;
	QSettings* m_appSettings;
	QQmlApplicationEngine* m_QMlEngine;
	TPListModel* m_model;

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
	QString m_exercisesListVersion;
	uint m_exercisesLocked;
	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

	void freeLocks(const bool res);
	void startThread(QThread* thread);
};

#endif // DBMANAGER_H
