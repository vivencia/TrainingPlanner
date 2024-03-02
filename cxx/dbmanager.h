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
class DBMesoSplitModel;
class DBMesocyclesModel;
class DBExercisesModel;
class DBMesoCalendarModel;
class DBTrainingDayModel;
class RunCommands;

class DbManager : public QObject
{

Q_OBJECT

public:
	explicit DbManager(QSettings* appSettigs, QQmlApplicationEngine* QMlEngine, RunCommands* runcommands);
	~DbManager();

	void gotResult(TPDatabaseTable* dbObj);
	Q_INVOKABLE uint pass_object(QObject *obj) { m_model = static_cast<TPListModel*>(obj); return ++m_execId; }
	Q_INVOKABLE uint insertId() const { return m_insertid; }
	Q_INVOKABLE const QStringList result() const { return m_result; }
	Q_INVOKABLE void setMainQMLProperties(QQuickWindow* mainwindow, QQuickItem* stackView);
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
	Q_INVOKABLE void updateMesocycle(const QString& id, const QString& mesoName, const QDate& mesoStartDate, const QDate& mesoEndDate,
									const QString& mesoNote, const QString& mesoWeeks, const QString& mesoSplit, const QString& mesoDrugs);
	Q_INVOKABLE void removeMesocycle(const QString& id);
	Q_INVOKABLE void deleteMesocyclesTable();
	//-----------------------------------------------------------MESOCYCLES TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------
	Q_INVOKABLE void getMesoSplit(const int meso_id);
	Q_INVOKABLE void newMesoSplit(const uint meso_id, const QString& splitA, const QString& splitB, const QString& splitC,
									const QString& splitD, const QString& splitE, const QString& splitF);
	Q_INVOKABLE void updateMesoSplit(const uint meso_id, const QString& splitA, const QString& splitB,
										const QString& splitC, const QString& splitD,
										const QString& splitE, const QString& splitF);
	Q_INVOKABLE void removeMesoSplit(const uint meso_id);
	Q_INVOKABLE void deleteMesoSplitTable();
	Q_INVOKABLE void getCompleteMesoSplit(const uint meso_id, const uint meso_idx, const QString& mesoSplit);
	void createMesoSlitPlanner();
	Q_INVOKABLE void updateMesoSplitComplete(const uint meso_id, const QString& splitLetter);
	Q_INVOKABLE bool previousMesoHasPlan(const uint prev_meso_id, const QString& splitLetter) const;
	Q_INVOKABLE void loadSplitFromPreviousMeso(const uint prev_meso_id, const QString& splitLetter);
	//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------
	Q_INVOKABLE void getMesoCalendar(const int meso_id);
	Q_INVOKABLE void createMesoCalendar();
	Q_INVOKABLE void createMesoCalendarPage(const uint meso_id, const uint meso_idx);
	void createMesoCalendarPage_part2();
	Q_INVOKABLE void newMesoCalendarEntry(const uint mesoId, const QDate& calDate, const uint calNDay, const QString& calSplit);
	Q_INVOKABLE void updateMesoCalendarEntry(const uint id, const uint mesoId, const QDate& calDate, const uint calNDay, const QString& calSplit);
	Q_INVOKABLE void deleteMesoCalendar(const uint id);
	Q_INVOKABLE void deleteMesoCalendarTable();
	//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------
	Q_INVOKABLE void getTrainingDay(const uint meso_id, const QDate& date);
	void createTrainingDayPage(const int exec_id = -1);
	Q_INVOKABLE void getTrainingDayExercises(const QDate& date);
	Q_INVOKABLE void newTrainingDay(const uint meso_id, const QDate& date, const uint trainingDayNumber, const QString& splitLetter,
							const QString& timeIn, const QString& timeOut, const QString& location, const QString& notes);
	Q_INVOKABLE void updateTrainingDay(const uint id, const uint meso_id, const QDate& date, const uint trainingDayNumber, const QString& splitLetter,
							const QString& timeIn, const QString& timeOut, const QString& location, const QString& notes);
	Q_INVOKABLE void updateTrainingDayExercises(const uint id);
	Q_INVOKABLE void removeTrainingDay(const uint id);
	Q_INVOKABLE void deleteTrainingDayTable();

	Q_INVOKABLE void createExerciseObject(const QString& exerciseName, QQuickItem* parentLayout, const uint modelIdx);
	void createExerciseObject_part2();
	//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------

public slots:
	void receiveQMLSignal(int id, QVariant param, QQuickItem* qmlObject);

signals:
	void qmlReady(uint exec_id);
	void databaseFree();
	void getQmlObject(QQuickItem* item, const bool bFirstTime);

private:
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

	DBMesocyclesModel* mesocyclesModel;
	DBExercisesModel* exercisesListModel;
	DBMesoSplitModel* mesoSplitModel;
	DBMesoCalendarModel* mesosCalendarModel;

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
	QString m_exercisesListVersion;
	QQuickItem* m_exercisesPage;
	QQmlComponent* m_exercisesComponent;
	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------
	QQmlComponent* m_splitComponent;
	QMap<QChar,QQuickItem*> m_splitItems;
	QMap<QChar,DBMesoSplitModel*> m_splitModels;
	QVariantMap m_splitProperties;
	uint m_lastUsedSplitMesoID;
	QString m_createdSplits;
	QQuickItem* m_qmlSplitObjectParent;
	QQuickItem* m_qmlSplitObjectContainer;
	//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------
	QQmlComponent* m_calComponent;
	QQuickItem* m_calPage;
	QVariantMap m_calProperties;
	uint m_lastUsedCalMesoID;
	//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------
	QMap<QDate,uint> m_tDayObjects;
	QList<DBTrainingDayModel*> m_tDayModels;
	QList<QQuickItem*> m_tDayPages;
	QQmlComponent* m_tDayComponent;
	QVariantMap m_tDayProperties;

	QVariantMap m_tDayExerciseEntryProperties;
	QList<QQuickItem*> m_tDayExercises;
	QQmlComponent* m_tDayExercisesComponent;
	//-----------------------------------------------------------TRAININGDAY TABLE-----------------------------------------------------------

	void freeLocks(TPDatabaseTable* dbObj);
	void startThread(QThread* thread, TPDatabaseTable* dbObj);
	void cleanUp(TPDatabaseTable* dbObj);
	void createThread(TPDatabaseTable* worker, const std::function<void(void)>& execFunc);
};

#endif // DBMANAGER_H
