#ifndef GRAPH_H
#define GRAPH_H

#include <QtQuick/QQuickPaintedItem>
#include <QColor>

class Graph : public QQuickPaintedItem
{
	Q_OBJECT
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
	Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged FINAL)
	QML_ELEMENT

public:
	Graph(QQuickItem *parent = nullptr) : QQuickPaintedItem(parent) {}

	inline QString name() const { return m_name; }
	inline void setName(const QString &name) { m_name = name; emit nameChanged(); }

	inline QColor color() const { return m_color;}
	inline void setColor(const QColor &color) { m_color = color; emit colorChanged(); }

	void paint(QPainter *painter) override;
	Q_INVOKABLE void clearGraph();

signals:
	void graphCleared();
	void nameChanged();
	void colorChanged();

private:
	QString m_name;
	QColor m_color;
};
#endif // GRAPH_H
