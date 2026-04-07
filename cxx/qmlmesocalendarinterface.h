#pragma once
#include "dbcalendarmodel.h"

#include <QDate>
#include <QObject>
#include <QVariantMap>

QT_FORWARD_DECLARE_CLASS(DBMesocyclesModel)
QT_FORWARD_DECLARE_CLASS(QMLMesoInterface)
QT_FORWARD_DECLARE_CLASS(QQmlComponent)
QT_FORWARD_DECLARE_CLASS(QQuickItem)

class QmlMesoCalendarInterface : public QObject
{

Q_OBJECT
QML_VALUE_TYPE(CalendarManager)
QML_UNCREATABLE("")

Q_PROPERTY(QString nameLabel READ nameLabel NOTIFY nameLabelChanged FINAL)
Q_PROPERTY(QString dateLabel READ dateLabel NOTIFY dateLabelChanged FINAL)

public:
	explicit QmlMesoCalendarInterface(QObject *parent, DBMesocyclesModel *meso_model,
																		DBCalendarModel *calendar_model, const uint meso_idx);
	inline ~QmlMesoCalendarInterface() { cleanUp(); }
	void cleanUp();

	inline void setMesoIdx(const uint new_meso_idx) { m_mesoIdx = new_meso_idx; }
	void getMesoCalendarPage();

	Q_INVOKABLE void changeSplitLetter(const QString &newSplitLetter, const bool bUntillTheEnd);
	Q_INVOKABLE void getWorkoutPage();
	Q_INVOKABLE QString dayInfo();
	QString nameLabel() const;
	QString dateLabel() const;

signals:
	void nameLabelChanged();
	void dateLabelChanged();

private:
	DBMesocyclesModel *m_mesoModel{nullptr};
	QQmlComponent *m_calComponent{nullptr};
	QQuickItem *m_calPage{nullptr};
	QVariantMap m_calProperties;
	DBCalendarModel *m_calendarModel{nullptr};
	uint m_mesoIdx;

	void createMesoCalendarPage();
};
