#pragma once

#include <QString>

namespace SetPredictionValues
{
	enum {
		Regular = 0,
		Pyramid = 1,
		Drop =  2,
		Cluster = 3,
		MyoReps = 4,
		ReversePyramid = 5
	} typedef TPSetTypes;

	QString increaseStringTimeBy(const QString &strtime, const uint add_mins, const uint add_secs);
	QString timeForSet(const QString &time_to_adjust, const TPSetTypes set_type);
	QString nextSetSuggestedTime(const uint set_number, const TPSetTypes set_type1, const QString &previous_set_time = QString{});
	QString nextSetSuggestedReps(const uint set_number, const TPSetTypes set_type, const QString &previous_set_reps = QString{},
										const uint n_subsets = 0);
	QString nextSetSuggestedWeight(const uint set_number, const TPSetTypes set_type, const QString &previous_set_weight,
								   const uint n_subsets = 0);
	QString dropSetReps(const QString &reps, const uint n_subsets, const uint from_subset = 0);
	QString clusterReps(const QString &total_reps, const uint n_subsets);
	QString myorepsReps(const QString &first_set_reps, const uint n_sets);
	QString dropSetWeight(const QString &weight, const uint n_subsets, const uint from_subset = 0);
	QString clusterWeight(const QString &constant_weight, const uint n_subsets);
	QString myorepsWeight(const QString &first_set_weight, const uint n_sets);
}
