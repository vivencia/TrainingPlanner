#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "dbexercisestable.h"
#include "dbexercisesmodel.h"

#include <QQmlEngine>
#include <QSettings>
#include <QQmlApplicationEngine>

#include <functional>

class DbManager : public QObject
{

Q_OBJECT

Q_PROPERTY(DBExercisesModel dbExercisesModel READ dbExercisesModel NOTIFY dbExercisesModelModified)

public:
	explicit DbManager(QSettings* appSettigs, QQmlApplicationEngine* QMlEngine);
	void gotResult(const dbExercisesList *dbObj, const OP_CODES op);

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
	Q_INVOKABLE void getAllExercises();
	Q_INVOKABLE void newExercise(const uint row, const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const qreal nSets, const qreal nReps, const qreal nWeight,
					 const QString& uWeight, const QString& mediaPath);
	Q_INVOKABLE void updateExercise(const uint row, const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const qreal nSets, const qreal nReps, const qreal nWeight,
					 const QString& uWeight, const QString& mediaPath);
	Q_INVOKABLE void removeExercise(const uint row);
	void getExercisesListVersion();
	inline const DBExercisesModel& dbExercisesModel() const { return m_dbExercisesModel; }

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

signals:
	void dbExercisesModelModified();

private:
	QString m_DBFilePath;
	QSettings* m_appSettings;
	QQmlApplicationEngine* m_QMlEngine;
	uint m_workingRow;

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
	DBExercisesModel m_dbExercisesModel;
	QString m_exercisesListVersion;
	uint m_exercisesLocked;
	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

	void freeLocks(const int res);
};

#endif // DBMANAGER_H
