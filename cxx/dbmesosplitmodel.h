#ifndef DBMESOSPLITMODEL_H
#define DBMESOSPLITMODEL_H

#include "tplistmodel.h"

#define MESOSPLIT_COL_EXERCISENAME 0
#define MESOSPLIT_COL_SETSNUMBER 1
#define MESOSPLIT_COL_NOTES 2
#define MESOSPLIT_COL_SETTYPE 3
#define MESOSPLIT_COL_SUBSETSNUMBER 4
#define MESOSPLIT_COL_REPSNUMBER 5
#define MESOSPLIT_COL_WEIGHT 6
#define MESOSPLIT_COL_WORKINGSET 7

class DBTrainingDayModel;
class DBExercisesModel;

class DBMesoSplitModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(int nbrSets READ nbrSets NOTIFY nbrSetsChanged)
Q_PROPERTY(int workingSet READ workingSet WRITE setWorkingSet NOTIFY workingSetChanged)

public:
	explicit DBMesoSplitModel(QObject* parent = nullptr, const bool bComplete = true);
	void convertFromTDayModel(DBTrainingDayModel* tDayModel);
	inline bool completeSplit() const { return mb_Complete; }

	Q_INVOKABLE QString muscularGroup() const;
	Q_INVOKABLE void setMuscularGroup(const QString& muscularGroup);

	Q_INVOKABLE QString splitLetter() const;
	Q_INVOKABLE void setSplitLetter(const QChar& splitLetter );
	Q_INVOKABLE void setSplitLetter(const QString& splitLetter ) { setSplitLetter(splitLetter.at(0)); }

	Q_INVOKABLE const QString exerciseName(const int row);
	Q_INVOKABLE void setExerciseName(const uint row, const QString& new_name);
	Q_INVOKABLE QString exerciseName1(const uint row) const;
	Q_INVOKABLE void setExerciseName1(const uint row, const QString& new_name);
	Q_INVOKABLE QString exerciseName2(const uint row) const;
	Q_INVOKABLE void setExerciseName2(const uint row, const QString& new_name);

	Q_INVOKABLE void addExercise(const QString& exercise_name, const uint settype, const QString& sets, const QString& reps, const QString& weight);
	Q_INVOKABLE void removeExercise(const uint row) { removeFromList(row); }

	Q_INVOKABLE uint setsNumber(const uint row) const;
	void setSetsNumber(const uint row, const uint new_setsnumber);
	Q_INVOKABLE void addSet(const uint row);
	Q_INVOKABLE void delSet(const uint row);
	int nbrSets() const { return 1; }

	Q_INVOKABLE QString setsNotes(const uint row) const;
	Q_INVOKABLE void setSetsNotes(const uint row, const QString& new_setsnotes);

	Q_INVOKABLE uint setType(const uint row) const;
	Q_INVOKABLE void setSetType(const uint row, const uint new_type);

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

	uint workingSet() const { return workingSet(currentRow()); }
	inline uint workingSet(const int row) const { return row >= 0 ? m_modeldata.at(row).at(MESOSPLIT_COL_WORKINGSET).toUInt() : 0; }
	void setWorkingSet(const uint new_workingset) { setWorkingSet(currentRow(), new_workingset, true); }
	void setWorkingSet(const uint row, const uint new_workingset, const bool emitSignal = true);

	Q_INVOKABLE void changeExercise(DBExercisesModel* model);

	virtual inline bool isFieldFormatSpecial (const uint field) const override
	{
		if (mb_Complete)
			return field == MESOSPLIT_COL_SETTYPE;
		return false;
	}
	virtual QString formatFieldToExport(const uint field, const QString& fieldValue) const override;
	QString formatFieldToImport(const uint field, const QString& fieldValue) const;

	virtual void exportToText(QFile* outFile) const override;
	virtual const QString exportExtraInfo() const override;
	virtual bool importFromText(QFile* inFile, QString& inData) override;
	virtual bool importExtraInfo(const QString& extrainfo) override;
	virtual bool updateFromModel(const TPListModel* model) override;

signals:
	void muscularGroupChanged();
	void splitLetterChanged();
	void exerciseNameChanged();
	void setTypeChanged();
	void workingSetChanged();
	void nbrSetsChanged();

private:
	uint m_nextAddedExercisePos;
	QString m_muscularGroup;
	QChar m_splitLetter;
	bool mb_Complete;

	QString getFromCompositeValue(const uint row, const uint column, const uint pos) const;
	void replaceCompositeValue(const uint row, const uint column, const uint pos, const QString& value);
};

Q_DECLARE_METATYPE(DBMesoSplitModel*)

#endif // DBMESOSPLITMODEL_H
