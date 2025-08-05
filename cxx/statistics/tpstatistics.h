#pragma once

#include "../qmlitemmanager.h"

#include <QQmlEngine>
#include <QObject>

QT_FORWARD_DECLARE_CLASS(DBMesoSplitModel)
QT_FORWARD_DECLARE_CLASS(QAbstractSeries)
QT_FORWARD_DECLARE_STRUCT(DataSet)
QT_FORWARD_DECLARE_STRUCT(statsInfo)

class TPStatistics : public QObject
{

Q_OBJECT

Q_PROPERTY(int exercisesSeries READ exercisesSeries NOTIFY exercisesSeriesChanged)
Q_PROPERTY(double xMin READ xMin NOTIFY xAxisChanged)
Q_PROPERTY(double xMax READ xMax NOTIFY xAxisChanged)
Q_PROPERTY(double yMin READ yMin NOTIFY yAxisChanged)
Q_PROPERTY(double yMax READ yMax NOTIFY yAxisChanged)

public:
	explicit inline TPStatistics(QObject* parent = nullptr) : QObject{parent}, m_workingDataSet(nullptr),
		m_xmin(0.0), m_xmax(20.0), m_ymin(0.0), m_ymax(100.0) {}
	~TPStatistics();

	//Q_INVOKABLE void update(QAbstractSeries* series);
	Q_INVOKABLE inline void setStartDate(const QDate &startdate) { m_startDate = startdate; }
	Q_INVOKABLE inline void setEndDate(const QDate &enddate) { m_endDate = enddate; }
	Q_INVOKABLE void createDataSet(const uint meso_idx, const QChar &splitLetter);
	Q_INVOKABLE void includeExercise(const uint exercise_idx, const bool include);
	Q_INVOKABLE void generateDataSet();
	Q_INVOKABLE void update(const int exercise_idx, QAbstractSeries* series);
	Q_INVOKABLE QStringList exercisesList() const;
	Q_INVOKABLE QString exerciseName(const int exercise_idx) const;
	Q_INVOKABLE bool exerciseIncluded(const int exercise_idx) const;

	inline double xMin() const { return m_xmin; }
	inline double xMax() const { return m_xmax; }
	inline double yMin() const { return m_ymin; }
	inline double yMax() const { return m_ymax; }
	int exercisesSeries() const;

signals:
	void exercisesListChanged(const int meso_idx, const QString &splitLetter);
	void exercisesSeriesChanged();
	void xAxisChanged();
	void yAxisChanged();

private:
	QList<DataSet*> m_dataSet;
	DataSet *m_workingDataSet;
	QDate m_startDate, m_endDate;
	double m_xmin, m_xmax, m_ymin, m_ymax;

	void generateExercisesForPlotting(statsInfo *splitInfo = nullptr);
	void createXData(const QList<QDate> &dates);
	void createYData(const QList<QList<QStringList>> &workoutInfo);
	friend TPStatistics *appStatistics();
	static TPStatistics *_appStatistics;
};

inline TPStatistics* appStatistics()
{
	if (!TPStatistics::_appStatistics)
		TPStatistics::_appStatistics = new TPStatistics{appItemManager()};
	return TPStatistics::_appStatistics;
}
