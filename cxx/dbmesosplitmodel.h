#ifndef DBMESOSPLITMODEL_H
#define DBMESOSPLITMODEL_H

#include "tplistmodel.h"

#define MESOSPLIT_COL_EXERCISENAME 0
#define MESOSPLIT_COL_SETTYPE 1
#define MESOSPLIT_COL_SETSNUMBER 2
#define MESOSPLIT_COL_SUBSETSNUMBER 3
#define MESOSPLIT_COL_REPSNUMBER 4
#define MESOSPLIT_COL_WEIGHT 5
#define MESOSPLIT_COL_DROPSET 6
#define MESOSPLIT_COL_NOTES 7
#define MESOSPLIT_COL_WORKINGSET 8
#define MESOSPLIT_COL_EXERCISE1REPS 9
#define MESOSPLIT_COL_EXERCISE1WEIGHT 10
#define MESOSPLIT_COL_EXERCISE2REPS 11
#define MESOSPLIT_COL_EXERCISE2WEIGHT 12
#define MESOSPLIT_COL_EXERCISE1NAME 13
#define MESOSPLIT_COL_EXERCISE2NAME 14

class DBTrainingDayModel;
class DBExercisesModel;

class DBMesoSplitModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

public:
	enum RoleNames {
		exerciseNameRole = Qt::UserRole,
		exerciseName1Role = Qt::UserRole+MESOSPLIT_COL_EXERCISE1NAME,
		exerciseName2Role = Qt::UserRole+MESOSPLIT_COL_EXERCISE2NAME,
		setTypeRole = Qt::UserRole+MESOSPLIT_COL_SETTYPE,
		setsNumberRole = Qt::UserRole+MESOSPLIT_COL_SETSNUMBER,
		setsSubsetsRole = Qt::UserRole+MESOSPLIT_COL_SUBSETSNUMBER,
		setsRepsRole = Qt::UserRole+MESOSPLIT_COL_REPSNUMBER,
		setsReps1Role = Qt::UserRole+MESOSPLIT_COL_EXERCISE1REPS,
		setsReps2Role = Qt::UserRole+MESOSPLIT_COL_EXERCISE2REPS,
		setsWeightRole = Qt::UserRole+MESOSPLIT_COL_WEIGHT,
		setsWeight1Role = Qt::UserRole+MESOSPLIT_COL_EXERCISE1WEIGHT,
		setsWeight2Role = Qt::UserRole+MESOSPLIT_COL_EXERCISE2WEIGHT,
		setsDropSetRole = Qt::UserRole+MESOSPLIT_COL_DROPSET,
		setsNotesRole = Qt::UserRole+MESOSPLIT_COL_NOTES,
		setsWorkingSetRole = Qt::UserRole+MESOSPLIT_COL_WORKINGSET
	};

	explicit DBMesoSplitModel(QObject *parent = nullptr, const bool bComplete = true);
	void convertFromTDayModel(DBTrainingDayModel* tDayModel);
	inline bool completeSplit() const { return mb_Complete; }

	Q_INVOKABLE QString muscularGroup() const;
	Q_INVOKABLE void setMuscularGroup(const QString& muscularGroup);

	Q_INVOKABLE QString splitLetter() const;
	Q_INVOKABLE void setSplitLetter(const QChar& splitLetter );
	Q_INVOKABLE void setSplitLetter(const QString& splitLetter ) { setSplitLetter(splitLetter.at(0)); }

	Q_INVOKABLE const QString exerciseName(const uint row);
	Q_INVOKABLE void setExerciseName(const uint row, const QString& new_name);
	Q_INVOKABLE QString exerciseName1(const uint row) const;
	Q_INVOKABLE void setExerciseName1(const uint row, const QString& new_name);
	Q_INVOKABLE QString exerciseName2(const uint row) const;
	Q_INVOKABLE void setExerciseName2(const uint row, const QString& new_name);

	Q_INVOKABLE void addExercise(const QString& exercise_name, const uint settype, const QString& sets, const QString& reps, const QString& weight);
	Q_INVOKABLE void removeExercise(const uint row) { removeFromList(row); }

	Q_INVOKABLE uint setType(const uint row) const;
	Q_INVOKABLE void setSetType(const uint row, const uint new_type);

	Q_INVOKABLE uint setsNumber(const uint row) const;
	Q_INVOKABLE void setSetsNumber(const uint row, const uint new_setsnumber);

	Q_INVOKABLE uint workingSet(const uint row) const;
	Q_INVOKABLE void setWorkingSet(const uint row, const uint new_workingset);
	Q_INVOKABLE inline uint getWorkingSet(const uint row) const { return m_modeldata.at(row).at(MESOSPLIT_COL_WORKINGSET).toUInt(); }

	Q_INVOKABLE QString setsSubsets(const uint row) const;
	Q_INVOKABLE void setSetsSubsets(const uint row, const QString& new_setssubsets);

	Q_INVOKABLE QString setsReps(const uint row) const;
	Q_INVOKABLE void setSetsReps(const uint row, const QString& new_setsreps);
	Q_INVOKABLE QString setsReps1(const uint row) const;
	Q_INVOKABLE void setSetsReps1(const uint row, const QString& new_setsreps);
	Q_INVOKABLE QString setsReps2(const uint row) const;
	Q_INVOKABLE void setSetsReps2(const uint row, const QString& new_setsreps);

	Q_INVOKABLE QString setsWeight(const uint row) const;
	Q_INVOKABLE void setSetsWeight(const uint row, const QString& new_setsweight);
	Q_INVOKABLE QString setsWeight1(const uint row) const;
	Q_INVOKABLE void setSetsWeight1(const uint row, const QString& new_setsweight);
	Q_INVOKABLE QString setsWeight2(const uint row) const;
	Q_INVOKABLE void setSetsWeight2(const uint row, const QString& new_setsweight);

	Q_INVOKABLE bool setsDropSet(const uint row) const;
	Q_INVOKABLE void setSetsDropSet(const uint row, const bool bDropSet);

	Q_INVOKABLE QString setsNotes(const uint row) const;
	Q_INVOKABLE void setSetsNotes(const uint row, const QString& new_setsnotes);

	Q_INVOKABLE void changeExercise(DBExercisesModel* model);

	inline bool isFieldFormatSpecial (const uint field) const
	{
		if (mb_Complete)
			return field == MESOSPLIT_COL_SETTYPE || field == MESOSPLIT_COL_DROPSET;
		return false;
	}
	static QString formatFieldToExport(const QString& fieldValue, const uint field = MESOSPLIT_COL_SETTYPE);
	static QString formatFieldToImport(const QString& fieldValue, const uint field = MESOSPLIT_COL_SETTYPE);

	virtual void exportToText(QFile* outFile, const bool bFancy) const override;
	virtual const QString exportExtraInfo() const override;
	virtual bool importFromFancyText(QFile* inFile, QString& inData) override;
	virtual bool importExtraInfo(const QString& extrainfo) override;
	virtual bool updateFromModel(const TPListModel* model) override;

signals:
	void muscularGroupChanged();
	void splitLetterChanged();
	void exerciseNameChanged();
	void setTypeChanged();
	void workingSetChanged();

private:
	uint m_nextAddedExercisePos;
	QString m_muscularGroup;
	QChar m_splitLetter;
	bool mb_Complete;

	void replaceCompositeValue(const uint row, const uint column, const uint pos, const QString& value);
};

Q_DECLARE_METATYPE(DBMesoSplitModel*)

#endif // DBMESOSPLITMODEL_H
