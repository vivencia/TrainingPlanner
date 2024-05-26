#ifndef DBTRAININGDAYMODEL_H
#define DBTRAININGDAYMODEL_H

#include "tplistmodel.h"

#define TDAY_COL_ID 0
#define TDAY_COL_MESOID 1
#define TDAY_COL_DATE 2
#define TDAY_COL_TRAININGDAYNUMBER 3
#define TDAY_COL_SPLITLETTER 4
#define TDAY_COL_TIMEIN 5
#define TDAY_COL_TIMEOUT 6
#define TDAY_COL_LOCATION 7
#define TDAY_COL_NOTES 8

class DBExercisesModel;
class DBMesoSplitModel;

class DBTrainingDayModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint exerciseCount READ exerciseCount NOTIFY exerciseCountChanged)
Q_PROPERTY(bool dayIsFinished READ dayIsFinished WRITE setDayIsFinished NOTIFY dayIsFinishedChanged FINAL)

public:
	explicit DBTrainingDayModel(QObject *parent = nullptr) : TPListModel{parent}, m_tDayModified(false), mb_DayIsFinished(false)
				{ m_tableId = TRAININGDAY_TABLE_ID; setObjectName(DBTrainingDayObjectName); }
	~DBTrainingDayModel() { for(uint i(0); i < m_ExerciseData.count(); ++i) delete m_ExerciseData[i]; }

	inline void clearExercises() { for(uint i(0); i < m_ExerciseData.count(); ++i) delete m_ExerciseData[i]; m_ExerciseData.clear(); setModified(true); }
	void fromDataBase(const QStringList& list);
	void getSaveInfo(QStringList& data) const;
	void convertMesoModelToTDayModel(DBMesoSplitModel* splitModel);
	virtual void updateFromModel(TPListModel* model) override;

	//So far, date format for exporting is not locale specific
	inline virtual const QString exportExtraInfo() const override { return tr("Date: ") + date().toString("d/M/yyyy"); }
	virtual bool importExtraInfo(const QString& extraInfo) override;
	virtual void exportToText(QFile* outFile, const bool bFancy) const override;
	virtual bool importFromFancyText(QFile* inFile) override;
	virtual bool importFromText(const QString& data) override;

	Q_INVOKABLE void appendRow() { appendList(QStringList(9)); setId("-1"); }
	bool dayIsFinished() const { return mb_DayIsFinished; }
	void setDayIsFinished(const bool finished) { mb_DayIsFinished = finished; emit dayIsFinishedChanged(); }
	void moveExercise(const uint from, const uint to);

	void setModified(const bool bModified)
	{
		if (m_bModified != bModified)
		{
			m_bModified = bModified;
			if (++m_nModified == 5)
			{
				emit modifiedChanged();
				m_nModified = 0;
			}
		}
	}

	Q_INVOKABLE const int id() const { return count() == 1 ? m_modeldata.at(0).at(TDAY_COL_ID).toInt() : -1; }
	inline const QString& idStr() const { return m_modeldata.at(0).at(TDAY_COL_ID); }
	inline void setId(const QString& new_id) { m_modeldata[0][TDAY_COL_ID] = new_id; }

	Q_INVOKABLE const int mesoId() const { return count() == 1 ? m_modeldata.at(0).at(TDAY_COL_MESOID).toInt(): -1; }
	inline const QString& mesoIdStr() const { return m_modeldata.at(0).at(TDAY_COL_MESOID); }
	Q_INVOKABLE inline void setMesoId(const QString& mesoid) { m_modeldata[0][TDAY_COL_MESOID] = mesoid; }

	Q_INVOKABLE QDate date() const { return count() == 1 ? QDate::fromJulianDay(m_modeldata.at(0).at(TDAY_COL_DATE).toLongLong()) : QDate::currentDate(); }
	inline const QString& dateStr() const { return m_modeldata.at(0).at(TDAY_COL_DATE); }
	Q_INVOKABLE void setDate(const QDate& date) { m_modeldata[0][TDAY_COL_DATE] = QString::number(date.toJulianDay()); setModified(true); }

	Q_INVOKABLE QString trainingDay() const { return m_modeldata.at(0).at(TDAY_COL_TRAININGDAYNUMBER); }
	//Do not set model's modified to true because this is called from onTextChanged on TrainingDayInfo.qml, which gets called when the property
	//is changed even for the first time, i.e., when the page is receiving default values
	Q_INVOKABLE void setTrainingDay(const QString& trainingday )
	{ if (trainingday != m_modeldata.at(0).at(TDAY_COL_TRAININGDAYNUMBER)) { m_modeldata[0][TDAY_COL_TRAININGDAYNUMBER] = trainingday; setModified(true); } }

	Q_INVOKABLE QString splitLetter() const { return m_modeldata.at(0).at(TDAY_COL_SPLITLETTER); }
	Q_INVOKABLE void setSplitLetter(const QString& splitletter )
	{ if (splitletter != m_modeldata.at(0).at(TDAY_COL_SPLITLETTER)) { m_modeldata[0][TDAY_COL_SPLITLETTER] = splitletter; setModified(true); } }

	Q_INVOKABLE QString timeIn() const { return m_modeldata.at(0).at(TDAY_COL_TIMEIN); }
	Q_INVOKABLE void setTimeIn(const QString& timein)
	{ if (timein != m_modeldata.at(0).at(TDAY_COL_TIMEIN)) { m_modeldata[0][TDAY_COL_TIMEIN] = timein; setModified(true); } }

	Q_INVOKABLE QString timeOut() const { return m_modeldata.at(0).at(TDAY_COL_TIMEOUT); }
	Q_INVOKABLE void setTimeOut(const QString& timeout)
	{ if (timeout != m_modeldata.at(0).at(TDAY_COL_TIMEOUT)) { m_modeldata[0][TDAY_COL_TIMEOUT] = timeout; setModified(true); } }

	Q_INVOKABLE QString location() const { return m_modeldata.at(0).at(TDAY_COL_LOCATION); }
	Q_INVOKABLE void setLocation(const QString& location) { m_modeldata[0][TDAY_COL_LOCATION] = location; setModified(true); }

	Q_INVOKABLE QString dayNotes() const { return m_modeldata.at(0).at(TDAY_COL_NOTES); }
	Q_INVOKABLE void setDayNotes(const QString& day_notes) { m_modeldata[0][TDAY_COL_NOTES] = day_notes; setModified(true); }

	const uint exerciseCount() const { return m_ExerciseData.count(); }
	Q_INVOKABLE const uint setsNumber(const uint exercise_idx) const { return m_ExerciseData.at(exercise_idx)->nsets; }

	Q_INVOKABLE QString exerciseName(const uint exercise_idx) const;
	Q_INVOKABLE void setExerciseName(const QString& new_name, const uint exercise_idx);
	void newExercise(const QString& new_exercise, const uint idx);
	void removeExercise(const uint exercise_idx);
	Q_INVOKABLE void changeExerciseName(const uint exercise_idx, DBExercisesModel* model);

	Q_INVOKABLE QString exerciseName1(const uint exercise_idx) const;
	Q_INVOKABLE void setExerciseName1(const QString& name1, const uint exercise_idx);

	Q_INVOKABLE QString exerciseName2(const uint exercise_idx) const;
	Q_INVOKABLE void setExerciseName2(const QString& name2, const uint exercise_idx);

	void newFirstSet(const uint exercise_idx, const uint type, const QString& nReps, const QString& nWeight,
						const QString& nSubsets = u"0"_qs, const QString& notes = u" "_qs);
	const QString& nextSetSuggestedReps(const uint exercise_idx, const uint type, const uint set_number = 100, const uint sub_set = 100) const;
	const QString& nextSetSuggestedWeight(const uint exercise_idx, const uint type, const uint set_number = 100, const uint sub_set = 100) const;
	void newSet(const uint set_number, const uint exercise_idx, const uint type,
					const QString& nReps = QString(), const QString& nWeight = QString(), const QString& nSubSets = QString());
	bool removeSet(const uint set_number, const uint exercise_idx);

	Q_INVOKABLE uint setType(const uint set_number, const uint exercise_idx) const;
	Q_INVOKABLE void setSetType(const uint set_number, const uint exercise_idx, const uint new_type);
	void changeSetType(const uint set_number, const uint exercise_idx, const uint new_type);

	Q_INVOKABLE QString setRestTime(const uint set_number, const uint exercise_idx) const;
	Q_INVOKABLE void setSetRestTime(const uint set_number, const uint exercise_idx, const QString& new_time);

	Q_INVOKABLE QString setSubSets(const uint set_number, const uint exercise_idx) const;
	Q_INVOKABLE void newSetSubSet(const uint set_number, const uint exercise_idx);
	Q_INVOKABLE uint setSubSets_int(const uint set_number, const uint exercise_idx) const { return setSubSets(set_number, exercise_idx).toUInt(); }
	Q_INVOKABLE void setSetSubSets(const uint set_number, const uint exercise_idx, const QString& new_subsets);

	Q_INVOKABLE QString setReps(const uint set_number, const uint exercise_idx) const;
	Q_INVOKABLE uint setReps_int(const uint set_number, const uint exercise_idx) const { return setReps(set_number, exercise_idx).toUInt(); }
	Q_INVOKABLE void setSetReps(const uint set_number, const uint exercise_idx, const QString& new_reps);

	Q_INVOKABLE QString setWeight(const uint set_number, const uint exercise_idx) const;
	Q_INVOKABLE void setSetWeight(const uint set_number, const uint exercise_idx, const QString& new_weight);

	Q_INVOKABLE QString setNotes(const uint set_number, const uint exercise_idx) const;
	Q_INVOKABLE void setSetNotes(const uint set_number, const QString& new_notes, const uint exercise_idx);

	Q_INVOKABLE QString setReps(const uint set_number, const uint subset, const uint exercise_idx) const;
	Q_INVOKABLE void setSetReps(const uint set_number, const uint exercise_idx, const uint subset, const QString& new_reps);

	Q_INVOKABLE QString setWeight(const uint set_number, const uint subset, const uint exercise_idx) const;
	Q_INVOKABLE void setSetWeight(const uint set_number, const uint exercise_idx, const uint subset, const QString& new_weight);

signals:
	void exerciseCountChanged();
	void dayIsFinishedChanged();

private:
	struct exerciseEntry {
		QString name;
		uint nsets;
		QStringList type;
		QStringList resttime;
		QStringList subsets;
		QStringList reps;
		QStringList weight;
		QStringList notes;

		exerciseEntry() : nsets(0) {}
	};

	QList<exerciseEntry*> m_ExerciseData;
	bool m_tDayModified;
	bool mb_DayIsFinished;
	uint m_nModified;

	friend class DBMesoSplitModel;
};

Q_DECLARE_METATYPE(DBTrainingDayModel*)

#endif // DBTRAININGDAYMODEL_H
