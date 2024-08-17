#ifndef DBMANAGER_H
#define DBMANAGER_H

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
class QSettings;
class DBMesocyclesModel;
class DBExercisesModel;
class DBMesoSplitModel;
class DBMesoCalendarModel;
class DBTrainingDayModel;
class DBUserModel;
class RunCommands;
class TPMesocycleClass;

class DbManager : public QObject
{

Q_OBJECT

public:
	explicit DbManager(QSettings* appSettigs);
	~DbManager();

	void init();
	Q_INVOKABLE void exitApp();
	void setQmlEngine(QQmlApplicationEngine* QMlEngine);
	void gotResult(TPDatabaseTable* dbObj);

	Q_INVOKABLE void verifyBackupPageProperties(QQuickItem* page) const;
	Q_INVOKABLE void copyDBFilesToUserDir(QQuickItem* page, const QString& targetPath, QVariantList backupFiles) const;
	Q_INVOKABLE void copyFileToAppDataDir(QQuickItem* page, const QString& sourcePath, QVariantList restoreFiles) const;

#ifndef Q_OS_ANDROID
	Q_INVOKABLE void processArguments();
	Q_INVOKABLE void restartApp();
#else
	Q_INVOKABLE void checkPendingIntents() const;
	bool sendFile(const QString& filePath, const QString& title, const QString& mimeType, const int& requestId) const;
	void androidOpenURL(const QString& address) const;
	bool androidSendMail(const QString& address, const QString& subject, const QString& attachment) const;
#endif

	void setExportFileName(const QString& filename) { m_exportFileName = mAppDataFilesPath + filename;}
	inline const QString& exportFileName() const { return m_exportFileName; }
	void openRequestedFile(const QString& filename);
	Q_INVOKABLE bool exportToFile(const TPListModel* model, const QString& filename, const bool bFancy, QFile* &outFile) const;
	Q_INVOKABLE int importFromFile(QString filename, QFile* inFile = nullptr);
	bool importFromModel(TPListModel* model);

	Q_INVOKABLE void saveFileDialogClosed(QString finalFileName, bool bResultOK);
	Q_INVOKABLE int parseFile(QString filename);
	Q_INVOKABLE void exportMeso(const bool bShare, const bool bFancy);
	Q_INVOKABLE void openURL(const QString& address) const;
	Q_INVOKABLE void startChatApp(const QString& phone, const QString& appname) const;
	Q_INVOKABLE void sendMail(const QString& address, const QString& subject, const QString& attachment_file) const;

	//-----------------------------------------------------------USER TABLE-----------------------------------------------------------
	void getAllUsers();
	Q_INVOKABLE void saveUser(const uint row);
	Q_INVOKABLE int removeUser(const uint row, const bool bCoach);
	void deleteUserTable(const bool bRemoveFile);
	//-----------------------------------------------------------USER TABLE-----------------------------------------------------------

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
	Q_INVOKABLE void getAllExercises();
	Q_INVOKABLE void saveExercise(const QString& id, const QString& mainName, const QString& subName, const QString& muscularGroup,
									const QString& nSets, const QString& nReps, const QString& nWeight,
									const QString& uWeight, const QString& mediaPath);
	Q_INVOKABLE void removeExercise(const QString& id);
	Q_INVOKABLE void deleteExercisesTable(const bool bRemoveFile);
	Q_INVOKABLE void updateExercisesList(DBExercisesModel* model = nullptr);
	Q_INVOKABLE void openExercisesListPage(const bool bChooseButtonEnabled, QQuickItem* connectPage = nullptr);
	void createExercisesListPage(QQuickItem *connectPage);
	void getExercisesListVersion();
	Q_INVOKABLE void exportExercisesList(const bool bShare, const bool bFancy);
	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------
	void getAllMesocycles();
	Q_INVOKABLE void setWorkingMeso(int meso_idx);
	Q_INVOKABLE void getMesocycle(const uint meso_idx);
	Q_INVOKABLE void createNewMesocycle(const bool bRealMeso, const QString& name, const bool bCreatePage);
	Q_INVOKABLE void saveMesocycle(const bool bNewMeso, const bool bChangeCalendar = false, const bool bPreserveOldCalendar = false,
									const bool bPreserveUntillYesterday = false);
	Q_INVOKABLE void removeMesocycle(const uint meso_idx);
	Q_INVOKABLE void scheduleMesocycleRemoval(const uint meso_idx);
	void deleteMesocyclesTable(const bool bRemoveFile);
	//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------
	void getMesoSplit(const QString& mesoid);
	void saveMesoSplit();
	void removeMesoSplit(const QString& id);
	void deleteMesoSplitTable(const bool bRemoveFile);
	Q_INVOKABLE void createExercisesPlannerPage();
	void loadCompleteMesoSplits(const bool bThreaded = true);
	Q_INVOKABLE void getCompleteMesoSplit();
	Q_INVOKABLE void saveMesoSplitComplete(DBMesoSplitModel* model);
	Q_INVOKABLE bool mesoHasPlan(const uint meso_id, const QString& splitLetter) const;
	Q_INVOKABLE void loadSplitFromPreviousMeso(const uint prev_meso_id, DBMesoSplitModel* model);
	Q_INVOKABLE QString checkIfSplitSwappable(const QString& splitLetter) const;
	Q_INVOKABLE void swapMesoPlans(const QString& splitLetter1, const QString& splitLetter2);
	Q_INVOKABLE void exportMesoSplit(const QString& splitLetter, const bool bShare, const bool bFancy, QFile *outFileInUse = nullptr);
	//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------
	Q_INVOKABLE void getMesoCalendar(const bool bCreatePage);
	Q_INVOKABLE void createMesoCalendar();
	Q_INVOKABLE void changeMesoCalendar(const QDate& newStartDate, const QDate& newEndDate, const QString& newSplit,
								const bool bPreserveOldInfo, const bool bPreserveOldInfoUntilToday);
	Q_INVOKABLE void updateMesoCalendarModel(const QString& mesoSplit, const QDate& startDate, const QString& splitLetter);
	Q_INVOKABLE void updateMesoCalendarEntry(const QDate& calDate, const uint calNDay, const QString& calSplit, const bool bDayIsFinished);
	Q_INVOKABLE void setDayIsFinished(const QDate& date, const bool bFinished);
	Q_INVOKABLE void removeMesoCalendar(const uint meso_id);
	Q_INVOKABLE void deleteMesoCalendarTable(const bool bRemoveFile);
	//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------
	Q_INVOKABLE void getTrainingDay(const QDate& date);
	void getTrainingDayExercises(const QDate& date);
	Q_INVOKABLE void verifyTDayOptions(const QDate& date, const QString& splitLetter = QString());
	Q_INVOKABLE void clearExercises();
	Q_INVOKABLE void loadExercisesFromDate(const QString& strDate);
	Q_INVOKABLE void loadExercisesFromMesoPlan(const QString& splitLetter);
	Q_INVOKABLE void convertTDayToPlan(DBTrainingDayModel* tDayModel);
	Q_INVOKABLE void saveTrainingDay();
	Q_INVOKABLE void removeTrainingDay();
	Q_INVOKABLE void deleteTrainingDayTable(const bool bRemoveFile);
	Q_INVOKABLE void exportTrainingDay(const QDate& date, const QString& splitLetter, const bool bShare, const bool bFancy);
	Q_INVOKABLE uint getWorkoutNumberForTrainingDay(const QDate& date) const;
	//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------

	//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------
	Q_INVOKABLE void openSettingsPage(const uint startPageIndex);
	void createSettingsPage();
	Q_INVOKABLE void openClientsOrCoachesPage(const bool bManageCoaches);
	void createClientsOrCoachesPage();

	void addMainMenuShortCut(const QString& label, QQuickItem* page);
	void removeMainMenuShortCut(QQuickItem* page);
	Q_INVOKABLE void addMainMenuShortCutEntry(QQuickItem* entry) { m_mainMenuShortcutEntries.append(entry); }
	//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------

signals:
	void databaseReady(const uint db_id);
	void getPage(QQuickItem* item, const uint id);
	void getItem(QQuickItem* item, const uint id);
	void internalSignal(const uint id);

public slots:
	void cleanUp();
	void bridge(QQuickItem* item, const uint id);
	void openMainMenuShortCut(const int button_id);

private:
	int m_MesoId;
	int m_MesoIdx;
	uint m_expectedPageId;
	bool mb_splitsLoaded;
	bool mb_importMode;
	uint m_nSplits;
	uint m_totalSplits;
	QString m_MesoIdStr;
	QString m_DBFilePath;
	QSettings* m_appSettings;
	QQmlApplicationEngine* m_QMlEngine;
	QList<TPMesocycleClass*> m_MesoManager;
	TPMesocycleClass* m_currentMesoManager;
	QQuickWindow* m_mainWindow;

	DBUserModel* userModel;
	DBMesocyclesModel* mesocyclesModel;
	DBMesoSplitModel* mesoSplitModel;
	DBExercisesModel* exercisesListModel;
	DBMesoCalendarModel* mesoCalendarModel;

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
	QQuickItem* m_exercisesPage;
	QQmlComponent* m_exercisesComponent;
	QVariantMap m_exercisesProperties;
	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

	//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------
	QQuickItem* m_settingsPage, *m_clientsOrCoachesPage;
	QQmlComponent* m_settingsComponent, *m_clientsOrCoachesComponent;
	QVariantMap m_settingsProperties, m_clientsOrCoachesProperties;

	QList<QQuickItem*> m_mainMenuShortcutPages;
	QList<QQuickItem*> m_mainMenuShortcutEntries;
	//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------

	void updateDB(TPDatabaseTable* worker);
	void createThread(TPDatabaseTable* worker, const std::function<void(void)>& execFunc);

	QString m_exportFileName;
	QString mAppDataFilesPath;
};

#endif // DBMANAGER_H
