#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "tplistmodel.h"

#include <QObject>
#include <QMap>
#include <QQmlComponent>
#include <QFile>

class TPDatabaseTable;
class QQmlApplicationEngine;
class QQuickItem;
class QQuickWindow;
class QSettings;
class DBMesocyclesModel;
class DBExercisesModel;
class DBMesoSplitModel;
class DBMesoCalendarModel;
class DBTrainingDayModel;
class RunCommands;
class TPMesocycleClass;

class DbManager : public QObject
{

Q_OBJECT

public:
	explicit DbManager(QSettings* appSettigs, RunCommands* runcommands);
	~DbManager();

	void init();
	Q_INVOKABLE void exitApp();
	Q_INVOKABLE void restartApp();
	void setQmlEngine(QQmlApplicationEngine* QMlEngine);
	Q_INVOKABLE void setWorkingMeso(int mesoId, const uint mesoIdx);
	void removeWorkingMeso();
	void gotResult(TPDatabaseTable* dbObj);

	Q_INVOKABLE void verifyBackupPageProperties(QQuickItem* page) const;
	Q_INVOKABLE void copyDBFilesToUserDir(QQuickItem* page, const QString& targetPath, QVariantList backupFiles) const;
	Q_INVOKABLE void copyFileToAppDataDir(QQuickItem* page, const QString& sourcePath, QVariantList restoreFiles) const;

#ifndef Q_OS_ANDROID
	void processArguments();
#else
	void checkPendingIntents();
	bool sendFile(const QString& filePath, const QString& title, const QString& mimeType, const int& requestId);
#endif

	void setExportFileName(const QString& filename) { m_exportFileName = mAppDataFilesPath + filename;}
	inline const QString& exportFileName() const { return m_exportFileName; }
	void openRequestedFile(const QString& filename);
	Q_INVOKABLE bool exportToFile(const TPListModel* model, const QString& filename, const bool bFancy, QFile* &outFile) const;
	Q_INVOKABLE int importFromFile(QString filename, QFile* inFile = nullptr);
	void importFromModel(TPListModel* model);

	Q_INVOKABLE void saveFileDialogClosed(QString finalFileName, bool bResultOK);
	Q_INVOKABLE int parseFile(QString filename);
	Q_INVOKABLE void exportMeso(const bool bShare, const bool bFancy);

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
	Q_INVOKABLE void getAllExercises();
	Q_INVOKABLE void newExercise(const QString& mainName, const QString& subName, const QString& muscularGroup,
									const QString& nSets, const QString& nReps, const QString& nWeight,
									const QString& uWeight, const QString& mediaPath);
	Q_INVOKABLE void updateExercise(const QString& id, const QString& mainName, const QString& subName, const QString& muscularGroup,
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
	Q_INVOKABLE void getAllMesocycles();
	Q_INVOKABLE void getMesocycle(const uint meso_idx);
	Q_INVOKABLE void createNewMesocycle(const bool bRealMeso, const QString& name);
	Q_INVOKABLE void saveMesocycle(const bool bNewMeso, const QString& mesoName, const QDate& mesoStartDate, const QDate& mesoEndDate,
									const QString& mesoNote, const QString& mesoWeeks, const QString& mesoSplit, const QString& mesoDrugs,
										const QString& splitA, const QString& splitB, const QString& splitC,
										const QString& splitD, const QString& splitE, const QString& splitF,
											const bool bChangeCalendar, const bool bPreserveOldCalendar, const bool bPreserveUntillYesterday);
	Q_INVOKABLE void removeMesocycle();
	Q_INVOKABLE void deleteMesocyclesTable(const bool bRemoveFile);
	//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------
	Q_INVOKABLE void getMesoSplit(const QString& mesoid);
	Q_INVOKABLE void newMesoSplit(const QString& splitA, const QString& splitB, const QString& splitC,
									const QString& splitD, const QString& splitE, const QString& splitF);
	Q_INVOKABLE void updateMesoSplit(const QString& splitA, const QString& splitB, const QString& splitC, const QString& splitD,
										const QString& splitE, const QString& splitF);
	Q_INVOKABLE void removeMesoSplit();
	Q_INVOKABLE void deleteMesoSplitTable(const bool bRemoveFile);
	Q_INVOKABLE void createExercisesPlannerPage();
	void loadCompleteMesoSplits(const bool bThreaded = true);
	Q_INVOKABLE void getCompleteMesoSplit();
	Q_INVOKABLE void updateMesoSplitComplete(DBMesoSplitModel* model);
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
	Q_INVOKABLE void removeMesoCalendar();
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
	//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------

	//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------
	void addMainMenuShortCut(const QString& label, QQuickItem* page);
	void removeMainMenuShortCut(QQuickItem* page);
	Q_INVOKABLE void addMainMenuShortCutEntry(QQuickItem* entry) { m_mainMenuShortcutEntries.append(entry); }
	//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------

signals:
	void databaseReady();
	void getPage(QQuickItem* item, const uint id);
	void getItem(QQuickItem* item, const uint id);
	void internalSignal(const uint id);

public slots:
	void bridge(QQuickItem* item, const uint id);
	void openMainMenuShortCut(const int button_id);

private:
	int m_MesoId;
	int m_MesoIdx;
	uint m_expectedPageId;
	bool mb_splitsLoaded;
	uint m_nSplits;
	uint m_totalSplits;
	QString m_MesoIdStr;
	QString m_DBFilePath;
	QSettings* m_appSettings;
	QQmlApplicationEngine* m_QMlEngine;
	RunCommands* m_runCommands;
	QMap<QString,int> m_WorkerLock;
	QList<TPMesocycleClass*> m_MesoManager;
	TPMesocycleClass* m_currentMesoManager;
	QQuickWindow* m_mainWindow;

	DBMesocyclesModel* mesocyclesModel;
	DBMesoSplitModel* mesoSplitModel;
	DBExercisesModel* exercisesListModel;
	DBMesoCalendarModel* mesoCalendarModel;

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
	QString m_exercisesListVersion;
	QQuickItem* m_exercisesPage;
	QQmlComponent* m_exercisesComponent;
	QVariantMap m_exercisesProperties;
	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

	//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------
	QList<QQuickItem*> m_mainMenuShortcutPages;
	QList<QQuickItem*> m_mainMenuShortcutEntries;
	//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------

	void updateDB(TPDatabaseTable* worker);
	void startThread(QThread* thread, TPDatabaseTable* dbObj);
	void cleanUp(TPDatabaseTable* dbObj);
	void createThread(TPDatabaseTable* worker, const std::function<void(void)>& execFunc);

	QString m_exportFileName;
	QString mAppDataFilesPath;
};

#endif // DBMANAGER_H
