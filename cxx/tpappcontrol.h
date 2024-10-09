#ifndef TPAPPCONTROL_H
#define TPAPPCONTROL_H

#include <QDate>
#include <QObject>
#include <QList>

class QmlItemManager;
class TPListModel;

class QQmlApplicationEngine;
class QQuickItem;

class TPAppControl : public QObject
{

Q_OBJECT

public:
	explicit inline TPAppControl(QObject* parent = nullptr) : QObject{nullptr} { app_control = this; }
	void init(QQmlApplicationEngine* qml_engine);

	Q_INVOKABLE void getMesocyclePage(const uint meso_idx);
	Q_INVOKABLE uint createNewMesocycle(const bool bCreatePage);
	Q_INVOKABLE void removeMesocycle(const uint meso_idx);
	Q_INVOKABLE void exportMeso(const uint meso_idx, const bool bShare, const bool bCoachInfo);
	Q_INVOKABLE void getExercisesPlannerPage(const uint meso_idx);
	Q_INVOKABLE void getMesoCalendarPage(const uint meso_idx);
	Q_INVOKABLE void getTrainingDayPage(const uint meso_idx, const QDate& date);

	void openRequestedFile(const QString& filename, const int wanted_content = 0xFF);
	void importFromFile(const QString& filename, const int wanted_content = 0xFF);
	void incorporateImportedData(const TPListModel* const model);
	void createItemManager();

	static TPAppControl* app_control;
	friend TPAppControl* appControl();

private:
	QList<QmlItemManager*> m_itemManager;
};

inline TPAppControl* appControl() { return TPAppControl::app_control; }

#endif // TPAPPCONTROL_H
