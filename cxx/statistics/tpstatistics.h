#ifndef TPSTATISTICS_H
#define TPSTATISTICS_H

#include "../qmlitemmanager.h"

#include <QQmlEngine>
#include <QObject>
#include <QPointF>

QT_FORWARD_DECLARE_CLASS(QAbstractSeries)

class TPStatistics : public QObject
{

Q_OBJECT

public:
	explicit TPStatistics(QObject* parent = nullptr);
    Q_INVOKABLE void generateData(int type, int rowCount, int colCount);
    Q_INVOKABLE void update(QAbstractSeries* series);

private:
    QList<QList<QPointF>> m_data;
    int m_index = -1;

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
