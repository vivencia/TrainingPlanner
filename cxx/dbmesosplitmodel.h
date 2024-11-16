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
#define COMPLETE_MESOSPLIT_TOTAL_COLS 8

#define MESOSPLIT_COL_ID 0
#define MESOSPLIT_COL_MESOID 1
#define MESOSPLIT_A 2
#define MESOSPLIT_B 3
#define MESOSPLIT_C 4
#define MESOSPLIT_D 5
#define MESOSPLIT_E 6
#define MESOSPLIT_F 7
#define SIMPLE_MESOSPLIT_TOTAL_COLS 8

class DBTrainingDayModel;
class DBExercisesModel;
class DBMesocyclesModel;

class DBMesoSplitModel : public TPListModel
{

Q_OBJECT

Q_PROPERTY(int workingSet READ workingSet WRITE setWorkingSet NOTIFY workingSetChanged)
Q_PROPERTY(QString instructionsLabel READ instructionsLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString typeLabel READ typeLabel NOTIFY labelsChanged FINAL)
Q_PROPERTY(QString subSetsLabel READ subSetsLabel NOTIFY labelsChanged FINAL)

public:
	explicit DBMesoSplitModel(QObject* parent, const bool bComplete, const uint meso_idx = 0);
	void convertFromTDayModel(const DBTrainingDayModel* const tDayModel);
	void addExerciseFromDatabase(QStringList* exercise_info);
	inline bool completeSplit() const { return mb_Complete; }

	inline const QString& id(const uint row) const { return m_modeldata.at(row).at(MESOSPLIT_COL_ID); }
	inline void setId(const uint row, const QString& new_id) { m_modeldata[row][MESOSPLIT_COL_ID] = new_id; }

	inline const QString& mesoId(const uint row) const { return m_modeldata.at(row).at(MESOSPLIT_COL_MESOID); }
	inline void setMesoId(const uint row, const QString& new_id) { m_modeldata[row][MESOSPLIT_COL_MESOID] = new_id; }

	inline const QString& splitA(const uint row) const { return m_modeldata.at(row).at(MESOSPLIT_A); }
	inline const QString& splitB(const uint row) const { return m_modeldata.at(row).at(MESOSPLIT_B); }
	inline const QString& splitC(const uint row) const { return m_modeldata.at(row).at(MESOSPLIT_C); }
	inline const QString& splitD(const uint row) const { return m_modeldata.at(row).at(MESOSPLIT_D); }
	inline const QString& splitE(const uint row) const { return m_modeldata.at(row).at(MESOSPLIT_E); }
	inline const QString& splitF(const uint row) const { return m_modeldata.at(row).at(MESOSPLIT_F); }
	inline const QString& splitX(const uint row, const uint split_idx) const { return m_modeldata.at(row).at(split_idx); }

	inline void setFast(const uint row, const uint field, const QString& value)
	{
		m_modeldata[row][field] = value;
	}

	inline QString instructionsLabel() const { return columnLabel(MESOSPLIT_COL_NOTES); }
	inline QString typeLabel() const { return columnLabel(MESOSPLIT_COL_SETTYPE); }
	inline QString subSetsLabel() const { return columnLabel(MESOSPLIT_COL_SUBSETSNUMBER); }

	Q_INVOKABLE inline QString muscularGroup() const { return m_muscularGroup; }
	Q_INVOKABLE inline void setMuscularGroup(const QString& muscularGroup) { m_muscularGroup = muscularGroup; }

	inline bool isExerciseNew(const uint row) const { return m_exerciseIsNew.at(row) != 0; }
	void setModified(const uint row, const uint field);

	inline const QChar& _splitLetter() const { return m_splitLetter; }
	Q_INVOKABLE inline QString splitLetter() const { return QString(m_splitLetter); }
	Q_INVOKABLE inline void setSplitLetter(const QChar& splitLetter) { m_splitLetter = splitLetter; }
	Q_INVOKABLE inline void setSplitLetter(const QString& splitLetter ) { setSplitLetter(splitLetter.at(0)); }

	Q_INVOKABLE void appendExercise();
	Q_INVOKABLE inline void removeExercise(const uint row) { removeRow(row); m_exerciseIsNew.remove(row); }
	Q_INVOKABLE void addSet(const uint row);
	Q_INVOKABLE void delSet(const uint row);

	inline const QString& _exerciseName(const uint row) const { return m_modeldata.at(row).at(MESOSPLIT_COL_EXERCISENAME); }
	Q_INVOKABLE const QString exerciseName(const int row);
	Q_INVOKABLE void setExerciseName(const uint row, const QString& new_name);
	Q_INVOKABLE QString exerciseName1(const uint row) const;
	Q_INVOKABLE void setExerciseName1(const uint row, const QString& new_name);
	Q_INVOKABLE QString exerciseName2(const uint row) const;
	Q_INVOKABLE void setExerciseName2(const uint row, const QString& new_name);

	inline const QString& _setsNumber(const uint row) const { return m_modeldata.at(row).at(MESOSPLIT_COL_SETSNUMBER); }
	Q_INVOKABLE uint setsNumber(const int row) const;
	void setSetsNumber(const uint row, const uint new_setsnumber);

	inline const QString& _setsNotes(const uint row) const { return m_modeldata.at(row).at(MESOSPLIT_COL_NOTES); }
	Q_INVOKABLE QString setsNotes(const int row) const;
	Q_INVOKABLE void setSetsNotes(const uint row, const QString& new_setsnotes);

	inline const QString& _setsTypes(const uint row) const { return m_modeldata.at(row).at(MESOSPLIT_COL_SETTYPE); }
	Q_INVOKABLE uint setType(const int row, const uint set_number) const;
	Q_INVOKABLE void setSetType(const uint row, const uint set_number, const uint new_type);

	inline const QString& _setsSubSets(const uint row) const { return m_modeldata.at(row).at(MESOSPLIT_COL_SUBSETSNUMBER); }
	Q_INVOKABLE QString setSubsets(const int row, const uint set_number) const;
	Q_INVOKABLE void setSetsSubsets(const uint row, const uint set_number, const QString& new_setssubsets);

	inline const QString& _setsReps(const uint row) const { return m_modeldata.at(row).at(MESOSPLIT_COL_REPSNUMBER); }
	QString setReps(const int row, const uint set_number) const;
	void setSetReps(const uint row, const uint set_number, const QString& new_setsreps);
	Q_INVOKABLE QString setReps1(const int row, const uint set_number) const;
	Q_INVOKABLE void setSetReps1(const uint row, const uint set_number, const QString& new_setsreps);
	Q_INVOKABLE QString setReps2(const int row, const uint set_number) const;
	Q_INVOKABLE void setSetReps2(const uint row, const uint set_number, const QString& new_setsreps);

	inline const QString& _setsWeights(const uint row) const { return m_modeldata.at(row).at(MESOSPLIT_COL_WEIGHT); }
	QString setWeight(const int row, const uint set_number) const;
	void setSetWeight(const uint row, const uint set_number, const QString& new_setsweight);
	Q_INVOKABLE QString setWeight1(const int row, const uint set_number) const;
	Q_INVOKABLE void setSetWeight1(const uint row, const uint set_number, const QString& new_setsweight);
	Q_INVOKABLE QString setWeight2(const int row, const uint set_number) const;
	Q_INVOKABLE void setSetWeight2(const uint row, const uint set_number, const QString& new_setsweight);

	inline uint workingSet() const { return workingSet(currentRow()); }
	inline uint workingSet(const int row) const { return row >= 0 ? m_modeldata.at(row).at(MESOSPLIT_COL_WORKINGSET).toUInt() : 0; }
	void setWorkingSet(const uint new_workingset) { setWorkingSet(currentRow(), new_workingset, true); }
	void setWorkingSet(const uint row, const uint new_workingset, const bool emitSignal = true);

	Q_INVOKABLE QString findSwappableModel() const;

	int exportToFile(const QString& filename, const bool = true, const bool = true) const override;
	int importFromFile(const QString& filename) override;
	bool updateFromModel(const TPListModel* model) override;

	inline bool isFieldFormatSpecial (const uint field) const override
	{
		if (mb_Complete)
			return field == MESOSPLIT_COL_SETTYPE;
		return false;
	}
	QString formatFieldToExport(const uint field, const QString& value) const override;
	QString formatFieldToImport(const uint field, const QString& fieldValue) const;
	const QString exportExtraInfo() const;
	bool importExtraInfo(const QString& extrainfo);

signals:
	void exerciseNameChanged(const int row);
	void setsNumberChanged(const int row);
	void setTypeChanged(const int row);
	void workingSetChanged(const int row);
	void splitChanged(const uint row, const uint field);
	void labelsChanged();

private:
	uint m_nextAddedExercisePos;
	QString m_muscularGroup;
	QChar m_splitLetter;
	QList<uchar> m_exerciseIsNew;
	bool mb_Complete;

	QString getFromCompositeValue(const uint row, const uint set_number, const uint field, const uint pos) const;
	void replaceCompositeValue(const uint row, const uint set_number, const uint field, const uint pos, const QString& value);
	void fillColumnNames();
};

//Q_DECLARE_METATYPE(DBMesoSplitModel*)

#endif // DBMESOSPLITMODEL_H
