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
	explicit dbExercisesList(const QString& dbFilePath, QSettings* appSettings, DBExercisesModel* model);

	void createTable();
	void getAllExercises();
	void updateExercisesList();
	void newExercise();
	void updateExercise();
	void removeExercise();

	//Call before starting a thread that execs newExercise() and updateExercise()
	static void setData(const QString& id, const QString& mainName = QString(), const QString& subName = QString(),
						const QString& muscularGroup = QString(), const qreal nSets = 0, const qreal nReps = 0, const qreal nWeight = 0,
						const QString& uWeight = QString(), const QString& mediaPath = QString());


	inline static uint exercisesTableLastId() { return m_exercisesTableLastId; }
	inline static void setExercisesTableLastId( const uint new_value ) { m_exercisesTableLastId = new_value; }
	inline static const QStringList& data () { return m_data; }
	inline static const QString DBFileName() { return QStringLiteral("ExercisesList.db.sqlite"); }

signals:
	void gotResult (const uint db_id, const OP_CODES op);
	void done (const int result);

private:
	QSqlDatabase mSqlLiteDB;
	QSettings* m_appSettings;
	static QStringList m_data;
	static uint m_exercisesTableLastId;
	QStringList m_ExercisesList;
	DBExercisesModel* m_model;

	void removePreviousListEntriesFromDB();
	void getExercisesList();
};

#endif // DBEXERCISESLIST_H
