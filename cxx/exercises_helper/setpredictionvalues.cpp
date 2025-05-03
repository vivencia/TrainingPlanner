#include "setpredictionvalues.h"

#include "../tputils.h"
#include "../tpglobals.h"

QString SetPredictionValues::increaseStringTimeBy(const QString &strtime, const uint add_mins, const uint add_secs)
{
	uint secs{QStringView{strtime}.sliced(3, 2).toUInt()};
	uint mins{QStringView{strtime}.first(2).toUInt()};

	secs += add_secs;
	if (secs > 59)
	{
		secs -= 60;
		mins++;
	}
	mins += add_mins;
	const QString& ret{(mins <= 9 ? "0"_L1 + QString::number(mins) : QString::number(mins)) + QChar(':') +
		(secs <= 9 ? "0"_L1 + QString::number(secs) : QString::number(secs))};
	return ret;
}

QString SetPredictionValues::timeForSet(const QString &time_to_adjust, const TPSetTypes set_type)
{
	switch (set_type)
	{
		case Regular:
			return time_to_adjust;
		break;
		case Pyramid:
		case Drop:
		case ReversePyramid:
			return SetPredictionValues::increaseStringTimeBy(time_to_adjust, 0, 30);
		break;
		case Cluster:
			return SetPredictionValues::increaseStringTimeBy(time_to_adjust, 1, 0);
		break;
		case MyoReps:
			return SetPredictionValues::increaseStringTimeBy(time_to_adjust, 1, 30);
		break;
	}
}

QString SetPredictionValues::nextSetSuggestedTime(const uint set_number, const TPSetTypes set_type1, const QString &previous_set_time)
{
	QString *reference_time{};
	if (set_number == 0)
	{
		const QString &first_set_time{"1:00"_L1};
		reference_time = const_cast<QString*>(&first_set_time);
	}
	else
		reference_time = const_cast<QString*>(&previous_set_time);

	return SetPredictionValues::timeForSet(*reference_time, set_type1);
}

QString SetPredictionValues::nextSetSuggestedReps(const uint set_number, const TPSetTypes set_type, const QString &previous_set_reps,
												  const uint n_subsets)
{
	switch (set_type)
	{
		case Regular:
			return set_number == 0 ? "12"_L1 : previous_set_reps;
		break;
		case Pyramid:
			return set_number == 0 ? "15"_L1 : appUtils()->appLocale()->toString(qCeil(previous_set_reps.toUInt() * 0.8));
		break;
		case Drop:
			return SetPredictionValues::dropSetReps(set_number == 0 ? "15"_L1 : previous_set_reps, n_subsets, 0);
		break;
		case Cluster:
			return set_number == 0 ? SetPredictionValues::clusterReps(previous_set_reps.isEmpty() ? "24"_L1 : previous_set_reps, n_subsets) : previous_set_reps;
		case ReversePyramid:
			return set_number == 0 ? "5"_L1 : appUtils()->appLocale()->toString(qCeil(previous_set_reps.toUInt() * 1.25));
		break;
		case MyoReps:
			return set_number == 0 ? SetPredictionValues::myorepsReps(previous_set_reps.isEmpty() ? "20"_L1 : previous_set_reps, n_subsets) : previous_set_reps;
		break;
	}
}

QString SetPredictionValues::nextSetSuggestedWeight(const uint set_number, const TPSetTypes set_type, const QString &previous_set_weight,
													const uint n_subsets)
{
	switch (set_type)
	{
		case Regular:
			return set_number == 0 ? "20"_L1 : previous_set_weight;
		break;
		case Pyramid:
			if (set_number == 0)
				return "20"_L1;
			else
			{
				const float prev_set_weight{appUtils()->appLocale()->toFloat(previous_set_weight) * static_cast<float>(1.2)};
				return QString::number(qCeil(prev_set_weight));
			}

		break;
		case Drop:
			return SetPredictionValues::dropSetWeight(set_number == 0 ? "80"_L1 : previous_set_weight, n_subsets, 0);
		break;
		case Cluster:
			return set_number == 0 ?
				SetPredictionValues::clusterWeight(previous_set_weight.isEmpty() ? "100"_L1 : previous_set_weight, n_subsets) :
				previous_set_weight;
		case ReversePyramid:
			if (set_number == 0)
				return "60"_L1;
			else
			{
				const float prev_set_weight{appUtils()->appLocale()->toFloat(previous_set_weight) * static_cast<float>(0.8)};
				return appUtils()->appLocale()->toString(qFloor(prev_set_weight));
			}
		break;
		case MyoReps:
			return set_number == 0 ?
				SetPredictionValues::myorepsReps(previous_set_weight.isEmpty() ? "100"_L1 : previous_set_weight, n_subsets) :
				previous_set_weight;
		break;
	}
}

QString SetPredictionValues::dropSetReps(const QString& reps, const uint n_subsets, const uint from_subset)
{
	QString new_drop_reps{reps};
	float value{appUtils()->appLocale()->toFloat(appUtils()->getCompositeValue(from_subset, reps, record_separator))};

	for (uint subset{from_subset+1}, i{0}; i < n_subsets; ++subset)
	{
		value *= 0.8;
		QString res{std::move(appUtils()->appLocale()->toString(qCeil(value)))};
		appUtils()->setCompositeValue(subset, res, new_drop_reps, record_separator);
	}

	return new_drop_reps;
}

QString SetPredictionValues::clusterReps(const QString &total_reps, const uint n_subsets)
{
	const int value{qFloor(total_reps.toUInt()/n_subsets)};
	const QString &subSetValues{QString::number(value)};
	return appUtils()->makeCompositeValue(subSetValues, n_subsets, record_separator);
}

QString SetPredictionValues::myorepsReps(const QString &first_set_reps, const uint n_sets)
{
	return appUtils()->makeCompositeValue(first_set_reps, n_sets, set_separator);
}

QString SetPredictionValues::dropSetWeight(const QString& weight, const uint n_subsets, const uint from_subset)
{
	QString new_drop_weight{weight};
	float value{appUtils()->appLocale()->toFloat(appUtils()->getCompositeValue(from_subset, weight, record_separator))};

	for (uint subset{from_subset+1}, i{0}; i < n_subsets; ++subset)
	{
		value *= 0.5;
		appUtils()->setCompositeValue(subset, QString::number(qCeil(value)), new_drop_weight, record_separator);
	}

	return new_drop_weight;
}

QString SetPredictionValues::clusterWeight(const QString &constant_weight, const uint n_subsets)
{
	return appUtils()->makeCompositeValue(constant_weight, n_subsets, record_separator);
}

QString SetPredictionValues::myorepsWeight(const QString &first_set_weight, const uint n_sets)
{
	return appUtils()->makeCompositeValue(first_set_weight, n_sets, set_separator);
}
