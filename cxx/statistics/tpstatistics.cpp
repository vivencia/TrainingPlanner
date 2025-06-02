#include "tpstatistics.h"

#include "../dbmesocalendarmanager.h"
#include "../dbmesocyclesmodel.h"
#include "../dbinterface.h"

#include <QtCharts/QtCharts>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QXYSeries>
#include <QtMath>
#include <QPointF>

TPStatistics *TPStatistics::_appStatistics(nullptr);

struct statsInfo {
	uint mesoIdx;
	QChar splitLetter;
	QStringList exercises;
};

struct DataSet {
	QList<QList<QPointF>> m_DataPoints;
	QChar m_SplitLetter;
	uint m_MesoIdx;
	QStringList m_ExercisesList;
	QList<bool> m_exercisesIncluded;
};

TPStatistics::~TPStatistics()
{
	for (uint i{0}; i < m_dataSet.count(); ++i)
		delete m_dataSet.at(i);
}

void TPStatistics::createDataSet(const uint meso_idx, const QChar &splitLetter)
{
	QList<DataSet*>::const_iterator itr(m_dataSet.constBegin());
	const QList<DataSet*>::const_iterator itr_end(m_dataSet.constEnd());
	while (itr != itr_end)
	{
		if ((*itr)->m_MesoIdx == meso_idx)
		{
			if ((*itr)->m_SplitLetter == splitLetter)
				m_workingDataSet = *itr;
		}
		++itr;
	}

	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(appDBInterface(), &DBInterface::databaseReadyWithData, this, [this,conn,meso_idx,splitLetter]
										(const uint table_idx, QVariant data) {
		if (table_idx == MESOSPLIT_TABLE_ID)
		{
			statsInfo receivedSplit{std::move(data.value<statsInfo>())};
			if (receivedSplit.mesoIdx == meso_idx &&receivedSplit.splitLetter == splitLetter)
			{
				disconnect(*conn);
				generateExercisesForPlotting(&receivedSplit);

			}
		}
	});
	//appDBInterface()->getExercisesForSplitWithinMeso(meso_idx, splitLetter);

	m_workingDataSet = new DataSet;
	m_workingDataSet->m_MesoIdx = meso_idx;
	m_workingDataSet->m_SplitLetter = splitLetter;
	m_dataSet.append(m_workingDataSet);
}

void TPStatistics::includeExercise(const uint exercise_idx, const bool include)
{
	m_workingDataSet->m_exercisesIncluded[exercise_idx] = include;
}

void TPStatistics::generateExercisesForPlotting(statsInfo *splitInfo)
{
	if (m_workingDataSet)
	{
		if (m_workingDataSet->m_ExercisesList.isEmpty() &&splitInfo)
		{
			m_workingDataSet->m_ExercisesList = std::move(splitInfo->exercises);
			for (uint i{0}; i < m_workingDataSet->m_ExercisesList.count(); ++i)
			{
				m_workingDataSet->m_exercisesIncluded.append(true);
				m_workingDataSet->m_DataPoints.append(QList<QPointF>());
			}
		}
		generateDataSet();
	}
	emit exercisesListChanged(m_workingDataSet->m_MesoIdx, m_workingDataSet->m_SplitLetter);
}

void TPStatistics::createXData(const QList<QDate> &dates)
{
	m_xmin = 0;
	m_xmax = dates.count();
	emit xAxisChanged();

	for (uint i{0}; i < m_workingDataSet->m_ExercisesList.count(); ++i)
	{
		QList<QPointF> *dataPoints{&(m_workingDataSet->m_DataPoints[i])};
		dataPoints->clear();
		dataPoints->reserve(dates.count());
		for (uint x(0); x < dates.count(); ++x)
		{
			dataPoints->append(QPointF{});
			(*dataPoints)[x].setX(i);
		}
	}
}

void TPStatistics::createYData(const QList<QList<QStringList>> &workoutInfo)
{
	m_ymin = 10000;
	for (uint i{0}; i < m_workingDataSet->m_ExercisesList.count(); ++i)
	{
		if (m_workingDataSet->m_exercisesIncluded.at(i))
		{
			QList<QPointF> *dataPoints{&(m_workingDataSet->m_DataPoints[i])};
			for (uint x(0); x < workoutInfo.count(); ++x)
			{
				int weight(0);
				const QStringList &weights{workoutInfo.at(x).at(1)};
				for(uint y(0); y < weights.count(); ++y)
				{
					const int w(weights.at(y).toUInt());
					if (weight < w)
						weight = w;
				}
				if (m_ymax < weight)
					m_ymax = weight;
				if (m_ymin > weight)
					m_ymin = weight;
				(*dataPoints)[i].setY(weight);
			}
		}
	}
	emit yAxisChanged();
	emit exercisesSeriesChanged();
}

void TPStatistics::generateDataSet()
{
	const uint mesoIdx(m_workingDataSet->m_MesoIdx);
	if (!appMesoModel()->mesoCalendarManager()->hasDBData(mesoIdx))
	{
		connect(appDBInterface(), &DBInterface::databaseReady, this, [this] (const uint db_id) {
			generateDataSet();
		}, static_cast<Qt::ConnectionType>(Qt::SingleShotConnection));
		appDBInterface()->getMesoCalendar(mesoIdx);
		return;
	}
	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(appDBInterface(), &DBInterface::databaseReadyWithData, this, [=,this] (const uint table_idx, QVariant data) {
		if (table_idx == MESOCALENDAR_TABLE_ID)
		{
			disconnect(*conn);
			const QList<QDate> &dates{data.value<QList<QDate>>()};
			createXData(dates);
			auto conn2 = std::make_shared<QMetaObject::Connection>();
			*conn2 = connect(appDBInterface(), &DBInterface::databaseReadyWithData, this, [=,this] (const uint table_idx, QVariant data2) {
				if (table_idx == WORKOUT_TABLE_ID)
				{
					disconnect(*conn2);
					createYData(data2.value<QList<QList<QStringList>>>());
				}
			});
			//appDBInterface()->workoutsInfoForTimePeriod(m_workingDataSet->m_ExercisesList, dates);
		}
	});
	//appDBInterface()->completedDaysForSplitWithinTimePeriod(m_workingDataSet->m_SplitLetter, m_startDate, m_endDate);
}

void TPStatistics::update(const int exercise_idx, QAbstractSeries *series)
{
	if (series)
	{
		auto xySeries = static_cast<QXYSeries *>(series);
		const QList<QPointF> &points{m_workingDataSet->m_DataPoints.at(exercise_idx)};
		// Use replace instead of clear + append, it's optimized for performance
		xySeries->replace(points);
	}
}

QStringList TPStatistics::exercisesList() const
{
	return m_workingDataSet->m_ExercisesList;
}

QString TPStatistics::exerciseName(const int exercise_idx) const
{
	if (exercise_idx >= 0 &&exercise_idx < m_workingDataSet->m_ExercisesList.count())
		return m_workingDataSet->m_ExercisesList.at(exercise_idx);
	else
		return QString{};
}

int TPStatistics::exercisesSeries() const
{
	return m_workingDataSet->m_ExercisesList.count();
}

bool TPStatistics::exerciseIncluded(const int exercise_idx) const
{
	return exercise_idx >= 0 ? m_workingDataSet->m_exercisesIncluded.at(exercise_idx) : false;
}
