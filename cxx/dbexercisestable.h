#ifndef DBEXERCISESTABLE_H
#define DBEXERCISESTABLE_H

#include "dbexercisesmodel.h"

#include <QObject>
#include <QStringList>
#include <QSqlDatabase>
#include <QSettings>

static const QString DBExercisesFileName (QStringLiteral("ExercisesList.db.sqlite"));
static const uint EXERCISES_TABLE_ID = 0x0001;

class dbExercisesList : public QObject
{
Q_OBJECT

public:
	explicit dbExercisesList(const QString& dbFilePath, QSettings* appSettings, DBExercisesModel* model = nullptr);

	void createTable();
	void getAllExercises();
	void updateExercisesList();
	void newExercise();
	void updateExercise();
	void removeExercise();

	//Call before starting a thread that execs newExercise() and updateExercise()
	void setData(const QString& id, const QString& mainName = QString(), const QString& subName = QString(),
						const QString& muscularGroup = QString(), const qreal nSets = 0, const qreal nReps = 0, const qreal nWeight = 0,
						const QString& uWeight = QString(), const QString& mediaPath = QString());


	inline static uint exercisesTableLastId() { return m_exercisesTableLastId; }
	inline static void setExercisesTableLastId( const uint new_value ) { m_exercisesTableLastId = new_value; }
	inline const QStringList& data () const { return m_data; }

signals:
	void gotResult (const dbExercisesList* dbObj, const OP_CODES op);
	void done (const int result);

private:
	QSqlDatabase mSqlLiteDB;
	QSettings* m_appSettings;
	QStringList m_data;
	static uint m_exercisesTableLastId;
	QStringList m_ExercisesList;
	DBExercisesModel* m_model;

	void removePreviousListEntriesFromDB();
	void getExercisesList();
};

#endif // DBEXERCISESTABLE_H
