#ifndef QMLMESOINTERFACE_H
#define QMLMESOINTERFACE_H

#include <QDate>
#include <QObject>
#include <QVariantMap>

class QQmlApplicationEngine;
class QQmlComponent;
class QQuickItem;
class QQuickWindow;

class QMLMesoInterface : public QObject
{

Q_OBJECT

public:
	explicit QMLMesoInterface(QObject* parent, QQmlApplicationEngine* qmlEngine, QQuickWindow* mainWindow, const uint meso_idx);
	~QMLMesoInterface();

	Q_INVOKABLE void changeMesoCalendar(const bool preserve_old_cal, const bool preserve_untilyesterday);

	void getMesocyclePage();
	void exportMeso(const bool bShare, const bool bCoachInfo);
	void importMeso(const QString& filename = QString());

signals:
	void displayMessageOnAppWindow(const int message_id, const QString& filename = QString());
	void addPageToMainMenu(const QString& label, QQuickItem* page);
	void removePageFromMainMenu(QQuickItem* page);

private:
	QQmlApplicationEngine* m_qmlEngine;
	QQuickWindow* m_mainWindow;
	QQmlComponent* m_mesoComponent;
	QQuickItem* m_mesoPage;
	QVariantMap m_mesoProperties;
	uint m_mesoIdx, m_mesoMuscularGroupId;
	bool m_mesoCalendarChanged;

	void createMesocyclePage(const QDate& minimumMesoStartDate = QDate(), const QDate& maximumMesoEndDate = QDate(),
								const QDate& calendarStartDate = QDate());
	void createMesocyclePage_part2();

	friend class TPAppControl;
};

#endif // QMLMESOINTERFACE_H
