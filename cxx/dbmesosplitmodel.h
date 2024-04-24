#ifndef DBMESOSPLITMODEL_H
#define DBMESOSPLITMODEL_H

#include "tplistmodel.h"

class DBTrainingDayModel;

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

public:
	enum RoleNames {
		exerciseNameRole = Qt::UserRole,
		exerciseName1Role = Qt::UserRole+11,
		exerciseName2Role = Qt::UserRole+12,
		setTypeRole = Qt::UserRole+1,
		setsNumberRole = Qt::UserRole+2,
		setsSubsetsRole = Qt::UserRole+3,
		setsRepsRole = Qt::UserRole+4,
		setsReps1Role = Qt::UserRole+7,
		setsReps2Role = Qt::UserRole+9,
		setsWeightRole = Qt::UserRole+5,
		setsWeight1Role = Qt::UserRole+8,
		setsWeight2Role = Qt::UserRole+10,
		setsNotesRole = Qt::UserRole+6
	};

	explicit DBMesoSplitModel(QObject *parent = nullptr);
	void convertFromTDayModel(DBTrainingDayModel* tDayModel);

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

	Q_INVOKABLE void changeExercise(const QString& name, const QString& sets, const QString& reps, const QString& weight, const uint operation);

private:
	uint m_nextAddedExercisePos;

	void replaceCompositeValue(const uint row, const uint column, const uint pos, const QString& value);
};

Q_DECLARE_METATYPE(DBMesoSplitModel*)

#endif // DBMESOSPLITMODEL_H
