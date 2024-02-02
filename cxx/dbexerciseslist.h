#ifndef DBEXERCISESLIST_H
#define DBEXERCISESLIST_H

#include "dbexercisesmodel.h"

#include <QObject>
#include <QStringList>
#include <QSqlDatabase>
#include <QSettings>

class dbExercisesList : public QObject
{
Q_OBJECT

public:
	explicit dbExercisesList(const QString& dbFilePath, QSettings* appSettings);

	void createTable();
	void getAllExercises();
	void updateExercisesList();
	void newExercise();
	void updateExercise();

	//Call before starting a thread that execs newExercise() and updateExercise()
	void setData(const int id, const QString& mainName, const QString& subName, const QString& muscularGroup,
					 const qreal nSets, const qreal nReps, const qreal nWeight,
					 const QString& uWeight, const QString& mediaPath);

signals:
	void gotResult ( const DBExercisesModel& results );
	void done (const int result);

private:
	QSqlDatabase mSqlLiteDB;
	QSettings* m_appSettings;
	QStringList m_data;

	void removePreviousListEntriesFromDB();
};

#endif // DBEXERCISESLIST_H
