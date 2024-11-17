#ifndef TPSTATISTICS_H
#define TPSTATISTICS_H

#include "../qmlitemmanager.h"

#include <QQmlEngine>
#include <QObject>

QT_FORWARD_DECLARE_CLASS(DBMesoSplitModel)
QT_FORWARD_DECLARE_CLASS(QAbstractSeries)
QT_FORWARD_DECLARE_STRUCT(DataSet);

class TPStatistics : public QObject
{

Q_OBJECT

public:
	explicit inline TPStatistics(QObject* parent = nullptr) : QObject{parent} {}
	~TPStatistics();
	//Q_INVOKABLE void update(QAbstractSeries* series);
	Q_INVOKABLE void createDataSet(const uint meso_idx, const QChar& splitLetter);
	Q_INVOKABLE void includeExercise(const uint exercise_idx, const bool include);
	Q_INVOKABLE void generateDataSet(const QDate& startDate, const QDate& endDate);
	Q_INVOKABLE QStringList exercisesList() const;
	Q_INVOKABLE bool exerciseIncluded(const int exercise_idx) const;

signals:
	void exercisesListChanged(const int meso_idx, const QString& splitLetter);

private:
	QList<DataSet*> m_dataSet;
	DataSet* m_workingDataSet;

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
