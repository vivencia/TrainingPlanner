#include "graph.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsRectItem>

void Graph::paint(QPainter *painter)
{
	QPen pen(m_color, 2);
	painter->setPen(pen);
	painter->setRenderHints(QPainter::Antialiasing, true);
	painter->drawPie(boundingRect().adjusted(1, 1, -1, -1), 90 * 16, 290 * 16);
}

void Graph::clearGraph()
{
	setColor(QColor(Qt::transparent));
	update();
	emit graphCleared();
}
