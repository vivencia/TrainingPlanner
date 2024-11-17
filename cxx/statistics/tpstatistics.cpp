#include "tpstatistics.h"

#include "../dbmesocyclesmodel.h"
#include "../dbmesosplitmodel.h"
#include "../dbinterface.h"

#include <QtCharts/QtCharts>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QXYSeries>
#include <QtMath>
#include <QPointF>

TPStatistics* TPStatistics::_appStatistics(nullptr);

struct DataSet {
	QList<QPointF> m_DataPoints;
	DBMesoSplitModel* m_Source;
	QChar m_SplitLetter;
	uint m_MesoIdx;
	QStringList m_ExercisesList;
	QList<bool> m_exercisesIncluded;
};

TPStatistics::~TPStatistics()
{
	for (uint i(0); i < m_dataSet.count(); ++i)
		delete m_dataSet.at(i);
}

/*void TPStatistics::update(QAbstractSeries *series)
{
	if (series)
	{
		auto xySeries = static_cast<QXYSeries *>(series);
		m_index++;
		if (m_index > m_data.count() - 1)
			m_index = 0;

		const QList<QPointF>& points{m_data.at(m_index)};
		// Use replace instead of clear + append, it's optimized for performance
		xySeries->replace(points);
	}
}*/

void TPStatistics::createDataSet(const uint meso_idx, const QChar& splitLetter)
{
	QList<DataSet*>::const_iterator itr(m_dataSet.constBegin());
	const QList<DataSet*>::const_iterator itr_end(m_dataSet.constEnd());
	while (itr != itr_end)
	{
		if ((*itr)->m_MesoIdx == meso_idx)
		{
			if ((*itr)->m_SplitLetter == splitLetter)
			{
				m_workingDataSet = *itr;
				generateExercisesForPlotting((*itr)->m_Source);
			}
		}
		++itr;
	}
	DBMesoSplitModel* splitModel{new DBMesoSplitModel{this, true, meso_idx}};
	splitModel->setSplitLetter(splitLetter);

	auto conn = std::make_shared<QMetaObject::Connection>();
	*conn = connect(appDBInterface(), &DBInterface::databaseReadyWithData, this, [this,conn,meso_idx,splitLetter] (const uint table_idx, const QVariant data) {
		if (table_idx == MESOSPLIT_TABLE_ID)
		{
			DBMesoSplitModel* receivedSplit(data.value<DBMesoSplitModel*>());
			if (receivedSplit->mesoIdx() == meso_idx && receivedSplit->_splitLetter() == splitLetter)
			{
				disconnect(*conn);
				generateExercisesForPlotting(receivedSplit);

			}
		}
	});
	appDBInterface()->loadCompleteMesoSplit(splitModel);

	m_workingDataSet = new DataSet;
	m_workingDataSet->m_MesoIdx = meso_idx;
	m_workingDataSet->m_SplitLetter = splitLetter;
	m_workingDataSet->m_Source = splitModel;
	m_dataSet.append(m_workingDataSet);
}

void TPStatistics::includeExercise(const uint exercise_idx, const bool include)
{
	m_workingDataSet->m_exercisesIncluded[exercise_idx] = include;
}

void TPStatistics::generateExercisesForPlotting(const DBMesoSplitModel* const splitModel)
{
	if (m_workingDataSet)
	{
		if (m_workingDataSet->m_ExercisesList.isEmpty())
		{
			for (uint i(0); i < splitModel->count(); ++i)
			{
				m_workingDataSet->m_ExercisesList.append(std::move(splitModel->exerciseName(i)));
				m_workingDataSet->m_exercisesIncluded.append(true);
			}
		}
	}
	emit exercisesListChanged(splitModel->mesoIdx(), splitModel->_splitLetter());
}

void TPStatistics::generateDataSet(const QDate& startDate, const QDate& endDate)
{

}

QStringList TPStatistics::exercisesList() const
{
	return m_workingDataSet->m_ExercisesList;
}

bool TPStatistics::exerciseIncluded(const int exercise_idx) const
{
	return exercise_idx >= 0 ? m_workingDataSet->m_exercisesIncluded.at(exercise_idx) : false;
}

/*void TPStatistics::generateData(int type, int rowCount, int colCount)
{
	// Remove previous data
	m_data.clear();

	// Append the new data depending on the type
	for (int i(0); i < rowCount; i++)
	{
		QList<QPointF> points;
		points.reserve(colCount);
		for (int j(0); j < colCount; j++)
		{
			qreal x(0);
			qreal y(0);
			switch (type)
			{
			case 0:
				// data with sin + random component
				y = qSin(M_PI / 50 * j) + 0.5 + QRandomGenerator::global()->generateDouble();
				x = j;
				break;
			case 1:
				// linear data
				x = j;
				y = (qreal) i / 10;
				break;
			default:
				// unknown, do nothing
				break;
			}
			points.append(QPointF(x, y));
		}
		m_data.append(points);
	}
}*/
