#ifndef QMLMESOCALENDARINTERFACE_H
#define QMLMESOCALENDARINTERFACE_H

#include <QDate>
#include <QObject>
#include <QVariantMap>

class QMLMesoInterface;
class QQmlComponent;
class QQuickItem;

class QmlMesoCalendarInterface : public QObject
{

Q_OBJECT

Q_PROPERTY(QString nameLabel READ nameLabel NOTIFY nameLabelChanged FINAL)
Q_PROPERTY(QString dateLabel READ dateLabel NOTIFY dateLabelChanged FINAL)
Q_PROPERTY(QString selectedSplitLetter READ selectedSplitLetter NOTIFY selectedSplitLetterChanged FINAL)

public:
	explicit inline QmlMesoCalendarInterface(QObject* parent, const uint meso_idx)
		: QObject{parent}, m_calComponent(nullptr), m_calPage(nullptr), m_mesoIdx(meso_idx) {}
	~QmlMesoCalendarInterface();

	inline void setMesoIdx(const uint new_meso_idx) { m_mesoIdx = new_meso_idx; }
	void getMesoCalendarPage();

	Q_INVOKABLE void changeCalendar(const bool bUntillTheEnd, const QString& newSplitLetter);
	Q_INVOKABLE void getTrainingDayPage(const QDate& date);
	Q_INVOKABLE QString dayInfo(const uint year, const uint month, const uint day);
	QString nameLabel() const;
	QString dateLabel() const;
	inline QString selectedSplitLetter() const { return m_selectedSplitLetter; }

signals:
	void addPageToMainMenu(const QString& label, QQuickItem* page);
	void removePageFromMainMenu(QQuickItem* page);
	void nameLabelChanged();
	void dateLabelChanged();
	void selectedSplitLetterChanged();

private:
	QQmlComponent* m_calComponent;
	QQuickItem* m_calPage;
	QVariantMap m_calProperties;
	uint m_mesoIdx;
	QString m_selectedTrainingDay, m_selectedSplitLetter;
	QDate m_selectedDate;

	void createMesoCalendarPage();
	void createMesoCalendarPage_part2();
};

#endif // QMLMESOCALENDARINTERFACE_H
