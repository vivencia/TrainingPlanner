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
	void gotResult(const dbExercisesList *dbObj, const OP_CODES op);

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
	Q_INVOKABLE void pass_object(QObject *obj) { m_model = static_cast<TPListModel*>(obj); }
	Q_INVOKABLE void getAllExercises();
	Q_INVOKABLE void newExercise(const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const qreal nSets, const qreal nReps, const qreal nWeight,
					 const QString& uWeight, const QString& mediaPath, TPListModel* model = nullptr );
	Q_INVOKABLE void updateExercise(const QString& id, const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const qreal nSets, const qreal nReps, const qreal nWeight,
					 const QString& uWeight, const QString& mediaPath, TPListModel* model = nullptr );
	Q_INVOKABLE void removeExercise(const QString& id, TPListModel* model = nullptr );
	void getExercisesListVersion();

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

public slots:
	void printOutInfo();
private:
	QString m_DBFilePath;
	QSettings* m_appSettings;
	QQmlApplicationEngine* m_QMlEngine;
	TPListModel* m_model;

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
	QString m_exercisesListVersion;
	uint m_exercisesLocked;
	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

	void freeLocks(const int res);
};

#endif // DBMANAGER_H
