#include "tpstatistics.h"

#include <QtCharts/QtCharts>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QXYSeries>
#include <QRandomGenerator>
#include <QtMath>

TPStatistics* TPStatistics::_appStatistics(nullptr);

TPStatistics::TPStatistics(QObject* parent)
    : QObject{parent}
{
    generateData(1, 5, 1024);
}

void TPStatistics::update(QAbstractSeries *series)
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
}

void TPStatistics::generateData(int type, int rowCount, int colCount)
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
}
