#ifndef DBMESOSPLITMODEL_H
#define DBMESOSPLITMODEL_H

#include "tplistmodel.h"

#define MESOSPLIT_COL_EXERCISENAME 0
#define MESOSPLIT_COL_SETTYPE 1
#define MESOSPLIT_COL_SETSNUMBER 2
#define MESOSPLIT_COL_SUBSETSNUMBER 3
#define MESOSPLIT_COL_REPSNUMBER 4
#define MESOSPLIT_COL_WEIGHT 5
#define MESOSPLIT_COL_NOTES 6
#define MESOSPLIT_COL_EXERCISE1REPS 7
#define MESOSPLIT_COL_EXERCISE1WEIGHT 8
#define MESOSPLIT_COL_EXERCISE2REPS 9
#define MESOSPLIT_COL_EXERCISE2WEIGHT 10
#define MESOSPLIT_COL_EXERCISE1NAME 11
#define MESOSPLIT_COL_EXERCISE2NAME 12

class DBTrainingDayModel;
class DBExercisesModel;

class DBMesoSplitModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(QString exerciseName READ exerciseName WRITE setExerciseName)
Q_PROPERTY(QString exerciseName1 READ exerciseName1 WRITE setExerciseName1)
Q_PROPERTY(QString exerciseName2 READ exerciseName2 WRITE setExerciseName2)
Q_PROPERTY(uint setType READ setType WRITE setSetType)
Q_PROPERTY(QString setsNumber READ setsNumber WRITE setSetsNumber)
Q_PROPERTY(QString setsSubsets READ setsSubsets WRITE setSetsSubsets)
Q_PROPERTY(QString setsReps READ setsReps WRITE setSetsReps)
Q_PROPERTY(QString setsReps1 READ setsReps1 WRITE setSetsReps1)
Q_PROPERTY(QString setsReps2 READ setsReps2 WRITE setSetsReps2)
Q_PROPERTY(QString setsWeight READ setsWeight WRITE setSetsWeight)
Q_PROPERTY(QString setsWeight1 READ setsWeight1 WRITE setSetsWeight1)
Q_PROPERTY(QString setsWeight2 READ setsWeight2 WRITE setSetsWeight2)
Q_PROPERTY(QString setsNotes READ setsNotes WRITE setSetsNotes)
Q_PROPERTY(QString muscularGroup READ muscularGroup WRITE setMuscularGroup NOTIFY muscularGroupChanged)
Q_PROPERTY(QString splitLetter READ splitLetter WRITE setSplitLetter NOTIFY splitLetterChanged)

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
		setsNotesRole = Qt::UserRole+MESOSPLIT_COL_NOTES
	};

	explicit DBMesoSplitModel(QObject *parent = nullptr);
	void convertFromTDayModel(DBTrainingDayModel* tDayModel);

	QString muscularGroup() const { return m_muscularGroup; }
	void setMuscularGroup(const QString& muscularGroup ) { m_muscularGroup = muscularGroup; setModified(true); emit muscularGroupChanged(); }

	QString splitLetter() const { return QString(m_splitLetter); }
	void setSplitLetter(const QChar& splitLetter ) { m_splitLetter = splitLetter; setModified(true); emit splitLetterChanged(); }
	void setSplitLetter(const QString& splitLetter ) { m_splitLetter = splitLetter.at(0); setModified(true); emit splitLetterChanged(); }

	QString exerciseName() const { return data(index(currentRow(), 0), exerciseNameRole).toString(); }
	void setExerciseName(const QString& new_name) { setData(index(currentRow(), 0), new_name, exerciseNameRole); }
	QString exerciseName1() const { return data(index(currentRow(), 0), exerciseName1Role).toString(); }
	void setExerciseName1(const QString& new_name) { setData(index(currentRow(), 0), new_name, exerciseName1Role); }
	QString exerciseName2() const { return data(index(currentRow(), 0), exerciseName2Role).toString(); }
	void setExerciseName2(const QString& new_name) { setData(index(currentRow(), 0), new_name, exerciseName2Role); }

	Q_INVOKABLE void addExercise(const QString& exercise_name, const uint settype, const QString& sets, const QString& reps, const QString& weight)
	{
		appendList(QStringList() << exercise_name << QString::number(settype) << sets << u"0"_qs << reps << weight << u" "_qs);
		setCurrentRow(count() - 1);
	}
	Q_INVOKABLE void removeExercise(const uint index) { removeFromList(index); }

	uint setType() const { return data(index(currentRow(), 0), setTypeRole).toUInt(); }
	void setSetType(const uint new_type) { setData(index(currentRow(), 0), new_type, setTypeRole); }

	QString setsNumber() const { return data(index(currentRow(), 0), setsNumberRole).toString(); }
	void setSetsNumber(const QString& new_setsnumber) { setData(index(currentRow(), 0), new_setsnumber, setsNumberRole); }

	QString setsSubsets() const { return data(index(currentRow(), 0), setsSubsetsRole).toString(); }
	void setSetsSubsets(const QString& new_setssubsets) { setData(index(currentRow(), 0), new_setssubsets, setsSubsetsRole); }

	QString setsReps() const { return data(index(currentRow(), 0), setsRepsRole).toString(); }
	void setSetsReps(const QString& new_setsreps) { setData(index(currentRow(), 0), new_setsreps, setsRepsRole); }
	QString setsReps1() const { return data(index(currentRow(), 0), setsRepsRole).toString(); }
	void setSetsReps1(const QString& new_setsreps) { setData(index(currentRow(), 0), new_setsreps, setsReps1Role); }
	QString setsReps2() const { return data(index(currentRow(), 0), setsReps2Role).toString(); }
	void setSetsReps2(const QString& new_setsreps) { setData(index(currentRow(), 0), new_setsreps, setsReps2Role); }

	QString setsWeight() const { return data(index(currentRow(), 0), setsWeightRole).toString(); }
	void setSetsWeight(const QString& new_setsweight) { setData(index(currentRow(), 0), new_setsweight, setsWeightRole); }
	QString setsWeight1() const { return data(index(currentRow(), 0), setsWeight1Role).toString(); }
	void setSetsWeight1(const QString& new_setsweight) { setData(index(currentRow(), 0), new_setsweight, setsWeight1Role); }
	QString setsWeight2() const { return data(index(currentRow(), 0), setsWeight2Role).toString(); }
	void setSetsWeight2(const QString& new_setsweight) { setData(index(currentRow(), 0), new_setsweight, setsWeight2Role); }

	QString setsNotes() const { return data(index(currentRow(), 0), setsNotesRole).toString(); }
	void setSetsNotes(const QString& new_setsnotes) { setData(index(currentRow(), 0), new_setsnotes, setsNotesRole); }

	Q_INVOKABLE int columnCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return 5; }
	Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const override;
	Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value, int role) override;

	Q_INVOKABLE void changeExercise(DBExercisesModel* model);

	inline virtual const QString exportExtraInfo() const override { return tr("Split: ") + m_splitLetter + u" - "_qs + m_muscularGroup; }
	virtual bool importExtraInfo(const QString& extrainfo) override;
	virtual void updateFromModel(TPListModel* model) override;

signals:
	void muscularGroupChanged();
	void splitLetterChanged();

private:
	uint m_nextAddedExercisePos;
	QString m_muscularGroup;
	QChar m_splitLetter;

	void replaceCompositeValue(const uint row, const uint column, const uint pos, const QString& value);
};

Q_DECLARE_METATYPE(DBMesoSplitModel*)

#endif // DBMESOSPLITMODEL_H
