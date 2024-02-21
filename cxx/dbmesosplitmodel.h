#ifndef DBMESOSPLITMODEL_H
#define DBMESOSPLITMODEL_H

#include "tplistmodel.h"

class DBMesoSplitModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

/*Q_PROPERTY(QString exerciseName READ exerciseName WRITE setExerciseName NOTIFY exerciseNameChanged)
Q_PROPERTY(uint setType READ setType WRITE setSetTpe NOTIFY setTypeChanged)
Q_PROPERTY(QString setsNumber READ setsNumber WRITE setSetsNumber NOTIFY setsNumberChanged)
Q_PROPERTY(QString setsReps READ setsReps WRITE setSetsReps NOTIFY setsRepsChanged)
Q_PROPERTY(QString setsWeight READ setsWeight WRITE setSetsWeight NOTIFY setsWeightChanged)*/

public:
	enum RoleNames {
		exerciseNameRole = Qt::UserRole,
		exerciseName1Role = Qt::UserRole+7,
		exerciseName2Role = Qt::UserRole+8,
		setTypeRole = Qt::UserRole+1,
		setsNumberRole = Qt::UserRole+2,
		setsReps1Role = Qt::UserRole+3,
		setsReps2Role = Qt::UserRole+5,
		setsWeight1Role = Qt::UserRole+4,
		setsWeight2Role = Qt::UserRole+6,
	};

	explicit DBMesoSplitModel(QObject *parent = nullptr);

	//QString exerciseName() const { return static_cast<QString>(m_modeldata.at(currentRow()).at(0)); }
	//void setExerciseName(const QString& new_name) { m_modeldata[currentRow()][0] = new_name; emit exerciseNameChanged(); }
	Q_INVOKABLE void addExercise(const QString& exercise_name, const uint settype, const QString& sets, const QString& reps, const QString& weight)
	{
		appendList(QStringList() << exercise_name << QString::number(settype) << sets << reps << weight);
	}
	Q_INVOKABLE void removeExercise(const uint index) { removeFromList(index); }

	/*uint setType() const { return static_cast<QString>(m_modeldata.at(currentRow()).at(1)).toUInt(); }
	void setSetTpe(const uint new_type) { m_modeldata[currentRow()][1] = QString::number(new_type); emit setTypeChanged(); }

	QString setsNumber() const { return static_cast<QString>(m_modeldata.at(currentRow()).at(2)); }
	void setSetsNumber(const QString& new_setsnumber) { m_modeldata[currentRow()][2] = new_setsnumber; emit setsNumberChanged(); }

	QString setsReps() const { return static_cast<QString>(m_modeldata.at(currentRow()).at(3)); }
	void setSetsReps(const QString& new_setsreps) { m_modeldata[currentRow()][3] = new_setsreps; emit setsRepsChanged(); }

	QString setsWeight() const { return static_cast<QString>(m_modeldata.at(currentRow()).at(4)); }
	void setSetsWeight(const QString& new_setsweight) { m_modeldata[currentRow()][4] = new_setsweight; emit setsWeightChanged(); }*/

	Q_INVOKABLE int columnCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return 5; }
	Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const override;
	Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value, int role) override;

signals:
	void exerciseNameChanged();
	void setTypeChanged();
	void setsNumberChanged();
	void setsRepsChanged();
	void setsWeightChanged();
};

Q_DECLARE_METATYPE(DBMesoSplitModel*)

#endif // DBMESOSPLITMODEL_H
