#pragma once

#include "tpdatabasetable.h"

#include <QObject>

QT_FORWARD_DECLARE_CLASS(DBModelInterfaceExercises)

class DBWorkoutsOrSplitsTable final : public TPDatabaseTable
{

Q_OBJECT

public:
	explicit DBWorkoutsOrSplitsTable(const uint tableid);

	QString dbFilePath() const override final;
	QString dbFileName(const bool fullpath = true) const override final;
	void updateTable() override final {}

	bool getExercises();
	std::pair<QVariant,QVariant> mesoHasAllSplitPlans(const QString &meso_id, const QString &split);
	std::pair<QVariant,QVariant> mesoHasSplitPlan();
	std::pair<QVariant,QVariant> getPreviousWorkoutsIds();

signals:
	void exercisesLoaded(const uint meso_idx, const bool success, const QVariant &extra_info);

private:
	DBModelInterfaceExercises *m_dbmodelInterface;
};
