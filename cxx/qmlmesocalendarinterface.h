#pragma once
#include "dbcalendarmodel.h"

#include <QDate>
#include <QObject>
#include <QVariantMap>

QT_FORWARD_DECLARE_CLASS(QMLMesoInterface)
QT_FORWARD_DECLARE_CLASS(QQmlComponent)
QT_FORWARD_DECLARE_CLASS(QQuickItem)

class QmlMesoCalendarInterface : public QObject
{

Q_OBJECT

Q_PROPERTY(QDate selectedDate READ selectedDate WRITE setSelectedDate NOTIFY selectedDateChanged FINAL)
Q_PROPERTY(QString nameLabel READ nameLabel NOTIFY nameLabelChanged FINAL)
Q_PROPERTY(QString dateLabel READ dateLabel NOTIFY dateLabelChanged FINAL)
Q_PROPERTY(QString selectedSplitLetter READ selectedSplitLetter NOTIFY selectedSplitLetterChanged FINAL)

public:
	explicit QmlMesoCalendarInterface(QObject *parent, const uint meso_idx);
	inline ~QmlMesoCalendarInterface() { cleanUp(); }
	void cleanUp();

	inline void setMesoIdx(const uint new_meso_idx) { m_mesoIdx = new_meso_idx; }
	void getMesoCalendarPage();

	inline DBCalendarModel *calendarModel() const { return m_calendarModel; }
	inline QDate selectedDate() const { return m_selectedDate; }
	void setSelectedDate(const QDate &new_date);

	Q_INVOKABLE void changeSplitLetter(const QString &newSplitLetter, const bool bUntillTheEnd);
	Q_INVOKABLE void getWorkoutPage();
	Q_INVOKABLE QString dayInfo();
	QString nameLabel() const;
	QString dateLabel() const;
	inline QString selectedSplitLetter() const { return m_selectedSplitLetter; }

signals:
	void selectedDateChanged();
	void nameLabelChanged();
	void dateLabelChanged();
	void selectedSplitLetterChanged();

private:
	QQmlComponent *m_calComponent;
	QQuickItem *m_calPage;
	QVariantMap m_calProperties;
	DBCalendarModel *m_calendarModel;
	uint m_mesoIdx;
	QString m_selectedWorkout, m_selectedSplitLetter;
	QDate m_selectedDate;

	void createMesoCalendarPage();
	void createMesoCalendarPage_part2();
};
