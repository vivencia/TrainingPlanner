#ifndef TPAPPCONTROL_H
#define TPAPPCONTROL_H

#include <QDate>
#include <QList>

class QmlItemManager;
class TPListModel;

class QQmlApplicationEngine;
class QSettings;
class QQuickItem;

class TPAppControl
{

public:
	explicit inline TPAppControl(QSettings* settings) { app_control = this; app_settings = settings; }
	inline TPAppControl(const TPAppControl& other) : m_itemManager(other.m_itemManager) {}
	inline ~TPAppControl() { cleanUp(); }
	void init();
	void cleanUp();

	//inline QmlItemManager* itemManager(const uint meso_idx) const { return m_itemManager.at(meso_idx); }

	Q_INVOKABLE void getClientsOrCoachesPage(const bool bManageClients, const bool bManageCoaches);
	Q_INVOKABLE void getSettingsPage(const uint startPageIndex);
	Q_INVOKABLE void getExercisesPage(const bool bChooseButtonEnabled, QQuickItem* connectPage);

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
	void populateSettingsWithDefaultValue();
	void createItemManager();

	static TPAppControl* app_control;
	friend TPAppControl* appControl();

	static QSettings* app_settings;
	friend QSettings* appSettings();

private:
	QSettings* m_appSettings;
	QList<QmlItemManager*> m_itemManager;
	uint m_tempMesoIdx;
};
Q_DECLARE_METATYPE(TPAppControl*)

inline TPAppControl* appControl() { return TPAppControl::app_control; }
inline QSettings* appSettings() { return TPAppControl::app_settings; }

#endif // TPAPPCONTROL_H
