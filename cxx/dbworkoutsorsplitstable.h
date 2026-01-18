#pragma once

#include "tpdatabasetable.h"

#include <QObject>

QT_FORWARD_DECLARE_CLASS(DBModelInterfaceExercises)

class DBWorkoutsOrSplitsTable final : public TPDatabaseTable
{

Q_OBJECT

public:
	explicit DBWorkoutsOrSplitsTable(const uint tableid);

	QString dbFileName(const bool fullpath = true) const override final;
	void updateTable() override final {}

	bool getExercises(DBModelInterfaceExercises *dbmi);
	std::pair<QVariant,QVariant> mesoHasAllSplitPlans(const QString &meso_id, const QString &split);
	std::pair<QVariant,QVariant> mesoHasSplitPlan();
	std::pair<QVariant,QVariant> getPreviousWorkoutsIds();
	std::pair<QVariant,QVariant> removeAllMesoExercises(const QString &mesoid);

signals:
	void exercisesLoaded(const uint meso_idx, const bool success, const QVariant &extra_info);
};
