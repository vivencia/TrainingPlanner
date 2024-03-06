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
	explicit DbManager(QSettings* appSettigs, QQmlApplicationEngine* QMlEngine, RunCommands* runcommands);
	~DbManager();

	void setWorkingMeso(const uint mesoId, const uint mesoIdx);
	void gotResult(TPDatabaseTable* dbObj);
	Q_INVOKABLE uint pass_object(QObject *obj) { m_model = static_cast<TPListModel*>(obj); return ++m_execId; }
	Q_INVOKABLE uint insertId() const { return m_insertid; }
	Q_INVOKABLE const QStringList result() const { return m_result; }
	Q_INVOKABLE void setAppStackView(QQuickItem* stackview) { m_appStackView = stackview; }
	Q_INVOKABLE QQuickItem* appStackView() const { return m_appStackView; }

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
	Q_INVOKABLE bool previousMesoHasPlan(const uint prev_meso_id, const QString& splitLetter) const;
	Q_INVOKABLE void loadSplitFromPreviousMeso(const uint prev_meso_id, const QString& splitLetter);
	//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------
	Q_INVOKABLE void getMesoCalendar();
	Q_INVOKABLE void createMesoCalendar();
	Q_INVOKABLE void newMesoCalendarEntry(const QDate& calDate, const uint calNDay, const QString& calSplit);
	Q_INVOKABLE void updateMesoCalendarEntry(const uint id, const QDate& calDate, const uint calNDay, const QString& calSplit);
	Q_INVOKABLE void deleteMesoCalendar(const uint id);
	Q_INVOKABLE void deleteMesoCalendarTable();
	//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------
	Q_INVOKABLE void getTrainingDay(const QDate& date);
	Q_INVOKABLE void getTrainingDayExercises(const QDate& date);
	Q_INVOKABLE void newTrainingDay(const QDate& date, const uint trainingDayNumber, const QString& splitLetter,
							const QString& timeIn, const QString& timeOut, const QString& location, const QString& notes);
	Q_INVOKABLE void updateTrainingDay(const uint id, const QDate& date, const uint trainingDayNumber, const QString& splitLetter,
							const QString& timeIn, const QString& timeOut, const QString& location, const QString& notes);
	Q_INVOKABLE void updateTrainingDayExercises(const uint id);
	Q_INVOKABLE void removeTrainingDay(const uint id);
	Q_INVOKABLE void deleteTrainingDayTable();

	Q_INVOKABLE void createExerciseObject(const QString& exerciseName, QQuickItem* parentLayout, const uint modelIdx);
	Q_INVOKABLE void createSetObject(const uint set_type, const uint set_number, const uint exercise_idx, DBTrainingDayModel* model);
	//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------

public slots:
	void receiveQMLSignal(int id, QVariant param, QQuickItem* qmlObject);

signals:
	void databaseReady(uint exec_id);
	void databaseFree();
	void getItem(QQuickItem* item, const uint id);

private:
	uint m_MesoId;
	uint m_MesoIdx;
	QString m_MesoIdStr;
	uint m_execId;
	QString m_DBFilePath;
	QSettings* m_appSettings;
	QQmlApplicationEngine* m_QMlEngine;
	RunCommands* m_runCommands;
	TPListModel* m_model;
	QMap<QString,uint> m_WorkerLock;
	uint m_insertid;
	QStringList m_result;
	QQuickItem* m_appStackView;
	QList<TPMesocycleClass*> m_MesoManager;

	DBMesocyclesModel* mesocyclesModel;
	DBMesoSplitModel* mesoSplitModel;
	DBMesoCalendarModel* mesoCalendarModel;
	DBExercisesModel* exercisesListModel;

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
