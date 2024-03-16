#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "tplistmodel.h"

#include <QObject>
#include <QMap>
#include <QQmlComponent>

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

	void setQmlEngine(QQmlApplicationEngine* QMlEngine);
	void setWorkingMeso(const int mesoId, const uint mesoIdx);
	void gotResult(TPDatabaseTable* dbObj);
	Q_INVOKABLE void pass_object(QObject *obj) { m_model = static_cast<TPListModel*>(obj); }
	Q_INVOKABLE uint insertId() const { return m_insertid; }

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
	Q_INVOKABLE void getAllExercises();
	Q_INVOKABLE void newExercise(const QString& mainName, const QString& subName, const QString& muscularGroup,
									const QString& nSets, const QString& nReps, const QString& nWeight,
									const QString& uWeight, const QString& mediaPath);
	Q_INVOKABLE void updateExercise(const QString& id, const QString& mainName, const QString& subName, const QString& muscularGroup,
									const QString& nSets, const QString& nReps, const QString& nWeight,
									const QString& uWeight, const QString& mediaPath);
	Q_INVOKABLE void removeExercise(const QString& id);
	Q_INVOKABLE void deleteExercisesTable();
	Q_INVOKABLE void openExercisesListPage();
	void createExercisesListPage();
	void getExercisesListVersion();
	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------
	Q_INVOKABLE void getAllMesocycles();
	Q_INVOKABLE void getMesocycle(const uint meso_idx);
	Q_INVOKABLE void createNewMesocycle(const bool bRealMeso, const QString& name);
	Q_INVOKABLE void newMesocycle(const QString& mesoName, const QDate& mesoStartDate, const QDate& mesoEndDate, const QString& mesoNote,
									const QString& mesoWeeks, const QString& mesoSplit, const QString& mesoDrugs);
	Q_INVOKABLE void updateMesocycle(const QString& mesoName, const QDate& mesoStartDate, const QDate& mesoEndDate,
									const QString& mesoNote, const QString& mesoWeeks, const QString& mesoSplit, const QString& mesoDrugs);
	Q_INVOKABLE void removeMesocycle();
	Q_INVOKABLE void deleteMesocyclesTable();
	//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------
	Q_INVOKABLE void getMesoSplit();
	Q_INVOKABLE void newMesoSplit(const QString& splitA, const QString& splitB, const QString& splitC,
									const QString& splitD, const QString& splitE, const QString& splitF);
	Q_INVOKABLE void updateMesoSplit(const QString& splitA, const QString& splitB, const QString& splitC, const QString& splitD,
										const QString& splitE, const QString& splitF);
	Q_INVOKABLE void removeMesoSplit();
	Q_INVOKABLE void deleteMesoSplitTable();
	Q_INVOKABLE void getCompleteMesoSplit(const QString& mesoSplit);
	Q_INVOKABLE void updateMesoSplitComplete(const QString& splitLetter);
	Q_INVOKABLE bool mesoHasPlan(const uint meso_id, const QString& splitLetter) const;
	Q_INVOKABLE void loadSplitFromPreviousMeso(const uint prev_meso_id, const QString& splitLetter);
	//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------
	Q_INVOKABLE void getMesoCalendar(const bool bCreatePage);
	Q_INVOKABLE void createMesoCalendar();
	Q_INVOKABLE void changeMesoCalendar(const QDate& newStartDate, const QDate& newEndDate, const QString& newSplit,
								const bool bPreserveOldInfo, const bool bPreserveOldInfoUntilToday);
	Q_INVOKABLE void updateMesoCalendarEntry(const QDate& calDate, const uint calNDay, const QString& calSplit);
	Q_INVOKABLE void removeMesoCalendar(const uint meso_id);
	Q_INVOKABLE void deleteMesoCalendarTable();
	//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------
	Q_INVOKABLE void getTrainingDay(const QDate& date);
	void getTrainingDayExercises(const QDate& date);
	Q_INVOKABLE void verifyTDayOptions(const QDate& date, const QString& splitLetter = QString());
	Q_INVOKABLE void loadExercisesFromDate(const QString& strDate);
	Q_INVOKABLE void loadExercisesFromMesoPlan(const QString& splitLetter);
	Q_INVOKABLE void newTrainingDay(const QDate& date, const uint trainingDayNumber, const QString& splitLetter,
							const QString& timeIn, const QString& timeOut, const QString& location, const QString& notes);
	Q_INVOKABLE void updateTrainingDay(const uint id, const QDate& date, const uint trainingDayNumber, const QString& splitLetter,
							const QString& timeIn, const QString& timeOut, const QString& location, const QString& notes);
	Q_INVOKABLE void updateTrainingDayExercises(const uint id);
	Q_INVOKABLE void removeTrainingDay(const uint id);
	Q_INVOKABLE void deleteTrainingDayTable();
	Q_INVOKABLE void createExerciseObject(const QString& exerciseName);
	Q_INVOKABLE void removeExerciseObject(const uint exercise_idx);
	Q_INVOKABLE void createSetObject(const uint set_type, const uint set_number, const uint exercise_idx);
	Q_INVOKABLE void createSetObjects(const uint exercise_idx);
	Q_INVOKABLE void removeSetObject(const uint set_number, const uint exercise_idx);
	//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------

signals:
	void databaseReady();
	void databaseFree();
	void getPage(QQuickItem* item, const uint id);
	void getItem(QQuickItem* item, const uint id);
	void internalSignal(const uint id);

public slots:
	void bridge(QQuickItem* item, const uint id);

private:
	int m_MesoId;
	uint m_MesoIdx;
	uint m_expectedPageId;
	QString m_MesoIdStr;
	QString m_DBFilePath;
	QSettings* m_appSettings;
	QQmlApplicationEngine* m_QMlEngine;
	RunCommands* m_runCommands;
	TPListModel* m_model;
	QMap<QString,int> m_WorkerLock;
	uint m_insertid;
	QList<TPMesocycleClass*> m_MesoManager;

	DBMesocyclesModel* mesocyclesModel;
	DBMesoSplitModel* mesoSplitModel;
	DBExercisesModel* exercisesListModel;
	DBMesoCalendarModel* mesoCalendarModel;

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
	QString m_exercisesListVersion;
	QQuickItem* m_exercisesPage;
	QQmlComponent* m_exercisesComponent;
	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

	void freeLocks(TPDatabaseTable* dbObj);
	void startThread(QThread* thread, TPDatabaseTable* dbObj);
	void cleanUp(TPDatabaseTable* dbObj);
	void createThread(TPDatabaseTable* worker, const std::function<void(void)>& execFunc);
};

#endif // DBMANAGER_H
