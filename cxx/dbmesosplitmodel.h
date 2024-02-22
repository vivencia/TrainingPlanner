#ifndef DBMESOSPLITMODEL_H
#define DBMESOSPLITMODEL_H

#include "tplistmodel.h"

class DBMesoSplitModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(QString exerciseName READ exerciseName WRITE setExerciseName FINAL)
Q_PROPERTY(QString exerciseName1 READ exerciseName1 WRITE setExerciseName1 FINAL)
Q_PROPERTY(QString exerciseName2 READ exerciseName2 WRITE setExerciseName2 FINAL)
Q_PROPERTY(uint setType READ setType WRITE setSetTpe FINAL)
Q_PROPERTY(QString setsNumber READ setsNumber WRITE setSetsNumber FINAL)
Q_PROPERTY(QString setsReps1 READ setsReps1 WRITE setSetsReps1 FINAL)
Q_PROPERTY(QString setsReps2 READ setsReps2 WRITE setSetsReps2 FINAL)
Q_PROPERTY(QString setsWeight1 READ setsWeight1 WRITE setSetsWeight1 FINAL)
Q_PROPERTY(QString setsWeight2 READ setsWeight2 WRITE setSetsWeight2 FINAL)

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

	QString exerciseName() const { return static_cast<QString>(m_modeldata.at(currentRow()).at(0)).replace('&', QStringLiteral(" + ")); }
	void setExerciseName(const QString& new_name)
	{
		m_modeldata[currentRow()][0] = new_name;
		emit dataChanged(index(currentRow(), 0), index(currentRow(), 0), QList<int>() << exerciseNameRole);
	}
	QString exerciseName1() const;
	void setExerciseName1(const QString& new_name);
	QString exerciseName2() const;
	void setExerciseName2(const QString& new_name);

	Q_INVOKABLE void addExercise(const QString& exercise_name, const uint settype, const QString& sets, const QString& reps, const QString& weight)
	{
		appendList(QStringList() << exercise_name << QString::number(settype) << sets << reps << weight);
	}
	Q_INVOKABLE void removeExercise(const uint index) { removeFromList(index); }

	uint setType() const { return static_cast<QString>(m_modeldata.at(currentRow()).at(1)).toUInt(); }
	void setSetTpe(const uint new_type);

	QString setsNumber() const { return static_cast<QString>(m_modeldata.at(currentRow()).at(2)); }
	void setSetsNumber(const QString& new_setsnumber);

	QString setsReps1() const;
	void setSetsReps1(const QString& new_setsreps);
	QString setsReps2() const;
	void setSetsReps2(const QString& new_setsreps);

	QString setsWeight1() const;
	void setSetsWeight1(const QString& new_setsweight);
	QString setsWeight2() const;
	void setSetsWeight2(const QString& new_setsweight);

	Q_INVOKABLE int columnCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return 5; }
	Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const override;
	Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value, int role) override;
};

Q_DECLARE_METATYPE(DBMesoSplitModel*)

#endif // DBMESOSPLITMODEL_H
