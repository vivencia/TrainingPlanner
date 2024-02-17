#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "tplistmodel.h"

#include <QObject>
#include <QMap>

class TPDatabaseTable;
class QQmlApplicationEngine;
class QSettings;

class DbManager : public QObject
{

Q_OBJECT

public:
	explicit DbManager(QSettings* appSettigs, QQmlApplicationEngine* QMlEngine);
	void gotResult(TPDatabaseTable* dbObj);
	Q_INVOKABLE void pass_object(QObject *obj) { m_model = static_cast<TPListModel*>(obj); }
	Q_INVOKABLE uint insertId() const { return m_insertid; }
	Q_INVOKABLE const QStringList result() const { return m_result; }

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
	Q_INVOKABLE void getCompleteMesoSplit(const uint meso_id, QLatin1Char splitLetter);
	Q_INVOKABLE void updateMesoSplitComplete(const uint meso_id, QLatin1Char splitLetter, const QString& splitGroup, const QString& exercises,
							const QString& types, const QString& nsets, const QString& nreps, const QString& nweights);
	Q_INVOKABLE bool previousMesoHasPlan(const uint prev_meso_id, QLatin1Char splitLetter) const;
	Q_INVOKABLE void loadSplitFromPreviousMeso(const uint meso_id, const uint prev_meso_id, QLatin1Char splitLetter);
	//-----------------------------------------------------------MESOSPLIT TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------
	Q_INVOKABLE void getMesoCalendar(const int meso_id);
	Q_INVOKABLE void createMesoCalendar();
	Q_INVOKABLE void newMesoCalendarEntry(const uint mesoId, const QDate& calDate, const uint calNDay, const QString& calSplit);
	Q_INVOKABLE void updateMesoCalendarEntry(const uint id, const uint mesoId, const QDate& calDate, const uint calNDay, const QString& calSplit);
	Q_INVOKABLE void deleteMesoCalendar(const uint id);
	Q_INVOKABLE void deleteMesoCalendarTable();
	//-----------------------------------------------------------MESOCALENDAR TABLE-----------------------------------------------------------

signals:
	void qmlReady();
	void databaseFree();

private:
	QString m_DBFilePath;
	QSettings* m_appSettings;
	QQmlApplicationEngine* m_QMlEngine;
	TPListModel* m_model;
	QMap<QString,uint> m_WorkerLock;
	uint m_insertid;
	QStringList m_result;

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
	QString m_exercisesListVersion;
	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

	void freeLocks(TPDatabaseTable* dbObj);
	void startThread(QThread* thread, TPDatabaseTable* dbObj);
	void cleanUp(TPDatabaseTable* dbObj);
	void createThread(TPDatabaseTable* worker, const std::function<void(void)>& execFunc);
};

#endif // DBMANAGER_H
