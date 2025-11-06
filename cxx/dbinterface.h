#pragma once

#include "tpdatabasetable.h"

#include <QObject>
#include <QMap>
#include <QQmlComponent>
#include <QFile>

QT_FORWARD_DECLARE_CLASS(DBExercisesListModel)
QT_FORWARD_DECLARE_CLASS(DBMesocyclesModel)
QT_FORWARD_DECLARE_CLASS(DBExercisesModel)
QT_FORWARD_DECLARE_CLASS(DBMesoCalendarManager)
QT_FORWARD_DECLARE_CLASS(DBExercisesModel)
QT_FORWARD_DECLARE_CLASS(DBUserModel)

class DBInterface : public QObject
{

Q_OBJECT

public:
	explicit inline DBInterface() : QObject{nullptr} { app_db_interface = this; init(); }
	inline DBInterface(const DBInterface &other) = delete;
	inline DBInterface &operator()(const DBInterface &other) = delete;
	inline ~DBInterface() {}

	void init();
	void sanityCheck();
	void executeExternalQuery(const QString &dbfilename, const QString &query);
	void createThread(TPDatabaseTable *worker, const std::function<void(void)> &execFunc);

	//-----------------------------------------------------------USER TABLE-----------------------------------------------------------
	void getAllUsers();
	void saveUser(const uint row);
	void removeUser(const uint row);
	void deleteUserTable(const bool bRemoveFile);
	//-----------------------------------------------------------USER TABLE-----------------------------------------------------------

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
	int getAllExercises();
	void saveExercises();
	void removeExercise(const uint row);
	void deleteExercisesTable(const bool bRemoveFile);
	void updateExercisesList();
	void getExercisesListVersion();
	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------
	void getAllMesocycles();
	void createMesoManager(const uint meso_idx);
	void saveMesocycle(const uint meso_idx);
	void removeMesocycle(const uint meso_idx);
	void deleteMesocyclesTable(const bool bRemoveFile);
	//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------
	int getMesoSplit(DBExercisesModel *model);
	void saveMesoSplit(DBExercisesModel *model);
	void removeMesoSplit(DBExercisesModel *model);
	void removeAllMesoSplits(const uint meso_idx);
	void deleteMesoSplitTable(const bool bRemoveFile);
	bool mesoHasAllSplitPlans(const uint meso_idx) const;
	bool mesoHasSplitPlan(const QString &meso_id, const QChar &split_letter) const;
	//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------
	int getMesoCalendar(const uint meso_idx);
	void saveMesoCalendar(const uint meso_idx);
	void removeMesoCalendar(const uint meso_idx);
	void deleteMesoCalendarTable(const uint meso_idx, const bool bRemoveFile);
	bool mesoCalendarSavedInDB(const uint meso_idx) const;
	//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------

	//-----------------------------------------------------------WORKOUT TABLE-----------------------------------------------------------
	int getWorkout(DBExercisesModel *model);
	int getPreviousWorkouts(DBExercisesModel *model);
	void saveWorkout(DBExercisesModel *model);
	void removeWorkout(DBExercisesModel *model);
	void removeAllWorkouts(const uint meso_idx);
	void deleteWorkoutsTable(const bool bRemoveFile);
	//-----------------------------------------------------------WORKOUT TABLE-----------------------------------------------------------

	//-----------------------------------------------------------STATISTICS-----------------------------------------------------------
	/*void getExercisesForSplitWithinMeso(const uint meso_idx, const QChar &splitLetter);
	void completedDaysForSplitWithinTimePeriod(const QChar &splitLetter, const QDate &startDate, const QDate &endDate);
	void workoutsInfoForTimePeriod(const QStringList &exercises, const QList<QDate> &workoutDates);*/
	//-----------------------------------------------------------STATISTICS-----------------------------------------------------------

signals:
	void databaseReady(const int _conn_id);
	void databaseReadyWithData(const uint table_idx, const QVariant &data);

private:
	struct workerLocks {
		inline TPDatabaseTable *nextObj() const { return dbObjs.at(++currentIndex); }
		inline TPDatabaseTable *at(const uint index) const { return dbObjs.at(index); }
		inline bool hasNext() const { return (currentIndex + 1) < dbObjs.count(); }
		inline uint count() const { return dbObjs.count(); }
		inline void appendObj(TPDatabaseTable *dbobj) { dbObjs.append(dbobj); }
		inline void removeAt(const uint index) { if (index < dbObjs.count()) { dbObjs.removeAt(index); if (currentIndex > 0) currentIndex--; } }
		int hasID(const uint id) const {
			for (uint i{0}; i < dbObjs.count(); ++i)
			{
				if (dbObjs.at(i)->uniqueId() == id)
					return i;
			}
			return -1;
		}
		workerLocks() : currentIndex(0) {}

		private:
			QList<TPDatabaseTable*> dbObjs;
			mutable uint currentIndex;
	};
	workerLocks m_WorkerLock[APP_TABLES_NUMBER+1];
	QString m_exercisesListVersion;

	void updateDB(TPDatabaseTable *worker);

	static DBInterface *app_db_interface;
	friend DBInterface *appDBInterface();
};

inline DBInterface *appDBInterface() { return DBInterface::app_db_interface; }
