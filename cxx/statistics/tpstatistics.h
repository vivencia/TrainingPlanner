#ifndef TPSTATISTICS_H
#define TPSTATISTICS_H

#include "../qmlitemmanager.h"

#include <QQmlEngine>
#include <QObject>
#include <QPointF>

QT_FORWARD_DECLARE_CLASS(DBMesoSplitModel)
QT_FORWARD_DECLARE_CLASS(QAbstractSeries)
QT_FORWARD_DECLARE_STRUCT(DataSet);

class TPStatistics : public QObject
{

Q_OBJECT

public:
	explicit inline TPStatistics(QObject* parent = nullptr) : QObject{parent}, m_exercisesForPlotting(nullptr) {}
	~TPStatistics();
	Q_INVOKABLE void update(QAbstractSeries* series);
	Q_INVOKABLE uint createDataSet(const uint meso_idx, const QChar& splitLetter);
	Q_INVOKABLE void includeExercise(const uint exercise_idx, const bool include);
	Q_INVOKABLE void generateDataSet(const uint dataSetIndex, const QDate& startDate, const QDate& endDate);
	Q_INVOKABLE inline QStringList exercisesList() const { return m_exercisesForPlotting ? *m_exercisesForPlotting : QStringList(); }

signals:
	void exercisesListChanged(const uint meso_idx, const QChar& splitLetter);

private:
	QList<DataSet*> m_dataSet;
	QStringList* m_exercisesForPlotting;
	int m_index = -1;

	DataSet* findDataSet(const DBMesoSplitModel* const splitModel) const;
	void generateExercisesForPlotting(const DBMesoSplitModel* const splitModel);
	friend TPStatistics* appStatistics();
	static TPStatistics* _appStatistics;
};

inline TPStatistics* appStatistics()
{
	if (!TPStatistics::_appStatistics)
		TPStatistics::_appStatistics = new TPStatistics{appItemManager()};
	return TPStatistics::_appStatistics;
}

#endif // TPSTATISTICS_H
