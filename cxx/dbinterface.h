#ifndef DBINTERFACE_H
#define DBINTERFACE_H

#include "tplistmodel.h"
#include "tpdatabasetable.h"

#include <QObject>
#include <QMap>
#include <QQmlComponent>
#include <QFile>
#include <QTimer>

class DBMesocyclesModel;
class DBExercisesModel;
class DBMesoSplitModel;
class DBMesoCalendarModel;
class DBTrainingDayModel;
class DBUserModel;
class RunCommands;
class QmlItemManager;

#ifdef Q_OS_ANDROID
class TPAndroidNotification;
#endif

//****THIS CLASS MUST NOT ALTER THE MODELS IT USES, UNLESS WHEN LOADING DATA FROM THE DATABASE*****

class DBInterface : public QObject
{

Q_OBJECT

public:
	explicit inline DBInterface()
		: QObject (nullptr), mb_splitsLoaded(false), mb_importMode(false) {}

	void init();
	void threadFinished(TPDatabaseTable* dbObj);

	//Q_INVOKABLE void verifyBackupPageProperties(QQuickItem* page) const;
	//Q_INVOKABLE void copyDBFilesToUserDir(QQuickItem* page, const QString& targetPath, QVariantList backupFiles) const;
	//Q_INVOKABLE void copyFileToAppDataDir(QQuickItem* page, const QString& sourcePath, QVariantList restoreFiles) const;

	//-----------------------------------------------------------USER TABLE-----------------------------------------------------------
	void getAllUsers();
	Q_INVOKABLE void saveUser(const uint row);
	void removeUser(const uint row, const bool bCoach);
	void deleteUserTable(const bool bRemoveFile);
	//-----------------------------------------------------------USER TABLE-----------------------------------------------------------

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
	void getAllExercises();
	Q_INVOKABLE void saveExercise(const QString& id, const QString& mainName, const QString& subName, const QString& muscularGroup,
									const QString& nSets, const QString& nReps, const QString& nWeight,
									const QString& uWeight, const QString& mediaPath);
	void removeExercise(const uint row);
	void deleteExercisesTable(const bool bRemoveFile);
	void updateExercisesList();
	void getExercisesListVersion();
	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------
	void getAllMesocycles();
	void createMesoManager(const uint meso_idx);
	Q_INVOKABLE void saveMesocycle(const uint meso_idx);
	void removeMesocycle(const uint meso_idx);
	void deleteMesocyclesTable(const bool bRemoveFile);
	//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------
	void saveMesoSplit(const uint meso_idx);
	void removeMesoSplit(const uint meso_idx);
	void deleteMesoSplitTable(const bool bRemoveFile);
	void loadCompleteMesoSplits(const uint meso_idx, QMap<QChar,DBMesoSplitModel*>& splitModels, const bool bThreaded = true);
	Q_INVOKABLE void saveMesoSplitComplete(DBMesoSplitModel* model);
	Q_INVOKABLE bool mesoHasPlan(const uint meso_id, const QString& splitLetter) const;
	Q_INVOKABLE void loadSplitFromPreviousMeso(const uint prev_meso_id, DBMesoSplitModel* model);
	//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------
	void getMesoCalendar(const uint meso_idx);
	Q_INVOKABLE void changeMesoCalendar(const uint meso_idx, const bool bPreserveOldInfo, const bool bPreserveOldInfoUntilDayBefore);
	Q_INVOKABLE void updateMesoCalendarModel(const DBTrainingDayModel* const tDayModel);
	Q_INVOKABLE void updateMesoCalendarEntry(const DBTrainingDayModel* const tDayModel);
	Q_INVOKABLE void setDayIsFinished(DBTrainingDayModel* const tDayModel, const bool bFinished);
	void removeMesoCalendar(const uint meso_id);
	void deleteMesoCalendarTable(const uint meso_idx, const bool bRemoveFile);
	//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------
	void getTrainingDay(DBTrainingDayModel* tDayModel);
	void getTrainingDayExercises(DBTrainingDayModel* tDayModel);
	void verifyTDayOptions(DBTrainingDayModel* tDayModel);
	void loadExercisesFromDate(const QString& strDate, DBTrainingDayModel* tDayModel);
	void loadExercisesFromMesoPlan(DBTrainingDayModel* tDayModel, QMap<QChar, DBMesoSplitModel *>& splitModels);
	void convertTDayToPlan(const DBTrainingDayModel* const tDayModel, QMap<QChar, DBMesoSplitModel *>& splitModels);
	Q_INVOKABLE void saveTrainingDay(DBTrainingDayModel* const tDayModel);
	void removeTrainingDay(const uint meso_idx);
	void deleteTrainingDayTable(const bool bRemoveFile);
	Q_INVOKABLE void exportTrainingDay(const DBTrainingDayModel* tDayModel, const bool bShare);
	//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------

signals:
	void databaseReady(const uint db_id);
	void databaseReadyWithData(const QVariant data);
	void internalSignal(const uint id);

public slots:
	void cleanUpThreads();

private:
	bool mb_splitsLoaded;
	bool mb_importMode;
	QString m_DBFilePath;

	struct workerLocks {
		inline TPDatabaseTable* nextObj() const { return dbObjs.at(++currentIndex); }
		inline TPDatabaseTable* at(const uint index) const { return dbObjs.at(index); }
		inline bool hasNext() const { return (currentIndex + 1) < dbObjs.count(); }
		inline bool canStartThread() const { return (dbObjs.count() == 1 || dbObjs.at(currentIndex)->resolved()); }
		inline uint count() const { return dbObjs.count(); }
		inline void appendObj(TPDatabaseTable* dbobj) { dbObjs.append(dbobj); }
		inline void removeAt(const uint index) { if (index < dbObjs.count()) { dbObjs.removeAt(index); if (currentIndex > 0) currentIndex--; } }
		bool hasID(const uint id) const {
			for (uint i(0); i < dbObjs.count(); ++i)
			{
				if (dbObjs.at(i)->uniqueID() == id)
					return true;
			}
			return false;
		}
		workerLocks() : currentIndex(0) {}

		private:
			QList<TPDatabaseTable*> dbObjs;
			mutable uint currentIndex;
	};
	workerLocks m_WorkerLock[APP_TABLES_NUMBER+1];
	QTimer m_threadCleaner;

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
	QString m_exercisesListVersion;
	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

	void updateDB(TPDatabaseTable* worker);
	void createThread(TPDatabaseTable* worker, const std::function<void(void)>& execFunc);
};
#endif // DBINTERFACE_H
