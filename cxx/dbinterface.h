#ifndef DBINTERFACE_H
#define DBINTERFACE_H

#include "tplistmodel.h"
#include "tpdatabasetable.h"

#include <QObject>
#include <QMap>
#include <QQmlComponent>
#include <QFile>
#include <QTimer>

class QQmlApplicationEngine;
class QQuickItem;
class QQuickWindow;
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

class DBInterface : public QObject
{

Q_OBJECT

public:
	explicit DBInterface();

	void init();
	Q_INVOKABLE void exitApp();
	void gotResult(TPDatabaseTable* dbObj);

	Q_INVOKABLE inline QmlItemManager* itemManager(const uint meso_idx) const { return m_itemManager.at(meso_idx); }
	//Q_INVOKABLE void verifyBackupPageProperties(QQuickItem* page) const;
	//Q_INVOKABLE void copyDBFilesToUserDir(QQuickItem* page, const QString& targetPath, QVariantList backupFiles) const;
	//Q_INVOKABLE void copyFileToAppDataDir(QQuickItem* page, const QString& sourcePath, QVariantList restoreFiles) const;

#ifndef Q_OS_ANDROID
	Q_INVOKABLE void processArguments();
	Q_INVOKABLE void restartApp();
#else
	Q_INVOKABLE void checkPendingIntents() const;
	bool sendFile(const QString& filePath, const QString& title, const QString& mimeType, const int& requestId) const;
	void androidOpenURL(const QString& address) const;
	bool androidSendMail(const QString& address, const QString& subject, const QString& attachment) const;
	bool viewFile(const QString& filePath, const QString& title) const;
	void appStartUpNotifications();
#endif

	void setExportFileName(const QString& filename) { m_exportFileName = mAppDataFilesPath + filename;}
	inline const QString& exportFileName() const { return m_exportFileName; }
	void openRequestedFile(const QString& filename);
	bool exportToFile(const TPListModel* const model, const QString& filename, QFile* &outFile) const;
	Q_INVOKABLE int importFromFile(QString filename, QFile* inFile = nullptr);
	bool importFromModel(TPListModel* model);

	Q_INVOKABLE void saveFileDialogClosed(QString finalFileName, bool bResultOK);
	Q_INVOKABLE int parseFile(QString filename);
	Q_INVOKABLE void exportMeso(const uint meso_idx, const bool bShare, const bool bCoachInfo);
	Q_INVOKABLE void openURL(const QString& address) const;
	Q_INVOKABLE void startChatApp(const QString& phone, const QString& appname) const;
	Q_INVOKABLE void sendMail(const QString& address, const QString& subject, const QString& attachment_file) const;
	Q_INVOKABLE void viewExternalFile(const QString& filename) const;

	//-----------------------------------------------------------USER TABLE-----------------------------------------------------------
	void getAllUsers();
	Q_INVOKABLE void saveUser(const uint row);
	Q_INVOKABLE void removeUser(const uint row, const bool bCoach);
	void deleteUserTable(const bool bRemoveFile);
	//-----------------------------------------------------------USER TABLE-----------------------------------------------------------

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
	Q_INVOKABLE void getAllExercises();
	Q_INVOKABLE void getExercisesPage(const bool bChooseButtonEnabled, QQuickItem* connectPage);
	Q_INVOKABLE void saveExercise(const QString& id, const QString& mainName, const QString& subName, const QString& muscularGroup,
									const QString& nSets, const QString& nReps, const QString& nWeight,
									const QString& uWeight, const QString& mediaPath);
	Q_INVOKABLE void removeExercise(const QString& id);
	Q_INVOKABLE void deleteExercisesTable(const bool bRemoveFile);
	Q_INVOKABLE void updateExercisesList(DBExercisesModel* model = nullptr);
	void getExercisesListVersion();
	Q_INVOKABLE void exportExercisesList(const bool bShare);
	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------
	void getAllMesocycles();
	void createMesoManager(const uint meso_idx);
	Q_INVOKABLE void getMesocyclePage(const uint meso_idx);
	Q_INVOKABLE uint createNewMesocycle(const bool bCreatePage);
	Q_INVOKABLE void saveMesocycle(const uint meso_idx);
	Q_INVOKABLE void removeMesocycle(const uint meso_idx);
	Q_INVOKABLE void scheduleMesocycleRemoval(const uint meso_idx);
	void deleteMesocyclesTable(const bool bRemoveFile);
	//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------
	void saveMesoSplit(const uint meso_idx);
	void removeMesoSplit(const uint meso_idx);
	void deleteMesoSplitTable(const bool bRemoveFile);
	Q_INVOKABLE void getExercisesPlannerPage(QmlItemManager* const itemMngr);
	void loadCompleteMesoSplits(const uint meso_idx, const bool bThreaded = true);
	void getCompleteMesoSplit(const uint meso_idx);
	Q_INVOKABLE void saveMesoSplitComplete(DBMesoSplitModel* model);
	Q_INVOKABLE bool mesoHasPlan(const uint meso_id, const QString& splitLetter) const;
	Q_INVOKABLE void loadSplitFromPreviousMeso(const uint prev_meso_id, DBMesoSplitModel* model);
	Q_INVOKABLE QString checkIfSplitSwappable(const DBMesoSplitModel* const splitModel) const;
	Q_INVOKABLE void swapMesoPlans(QmlItemManager* const itemMngr, const QString& splitLetter1, const QString& splitLetter2);
	Q_INVOKABLE void exportMesoSplit(const QmlItemManager* const itemMngr, const QString& splitLetter, const bool bShare, QFile* outFileInUse = nullptr);
	//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------
	Q_INVOKABLE void getMesoCalendar(const uint meso_idx, const bool bCreatePage);
	Q_INVOKABLE void changeMesoCalendar(const uint meso_idx, const bool bPreserveOldInfo, const bool bPreserveOldInfoUntilDayBefore);
	Q_INVOKABLE void updateMesoCalendarModel(const DBTrainingDayModel* const tDayModel);
	Q_INVOKABLE void updateMesoCalendarEntry(const DBTrainingDayModel* const tDayModel);
	Q_INVOKABLE void setDayIsFinished(DBTrainingDayModel* const tDayModel, const bool bFinished);
	void removeMesoCalendar(const uint meso_id);
	void deleteMesoCalendarTable(const uint meso_idx, const bool bRemoveFile);
	//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------
	Q_INVOKABLE void getTrainingDay(const uint meso_idx, const QDate& date);
	void getTrainingDayExercises(const uint meso_idx, const QDate& date);
	Q_INVOKABLE void verifyTDayOptions(const DBTrainingDayModel* const tDayModel);
	Q_INVOKABLE void clearExercises(QmlItemManager* const itemMngr);
	Q_INVOKABLE void loadExercisesFromDate(QmlItemManager* const itemMngr, const QString& strDate);
	Q_INVOKABLE void loadExercisesFromMesoPlan(QmlItemManager* const itemMngr, const QString& splitLetter);
	Q_INVOKABLE void convertTDayToPlan(const DBTrainingDayModel* const tDayModel);
	Q_INVOKABLE void saveTrainingDay(DBTrainingDayModel* const tDayModel);
	void removeTrainingDay(const uint meso_idx);
	void deleteTrainingDayTable(const uint meso_idx, const bool bRemoveFile);
	Q_INVOKABLE void exportTrainingDay(const DBTrainingDayModel* tDayModel, const bool bShare);
	Q_INVOKABLE uint getWorkoutNumberForTrainingDay(const DBTrainingDayModel* const tDayModel) const;
	//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------

signals:
	void databaseReady(const uint db_id);
	void internalSignal(const uint id);

public slots:
	void cleanUp();
	void cleanUpThreads();

private:
	bool mb_splitsLoaded;
	bool mb_importMode;
	QString m_DBFilePath;
	QQmlApplicationEngine* m_QMlEngine;
	QList<QmlItemManager*> m_itemManager;

	#ifdef Q_OS_ANDROID
	TPAndroidNotification* m_AndroidNotification;
	#endif

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

	QString m_exportFileName;
	QString mAppDataFilesPath;
};
#endif // DBINTERFACE_H
