#ifndef TPAPPCONTROL_H
#define TPAPPCONTROL_H

class TranslationClass;
class TPUtils;
class DBInterface;
class DBUserModel;
class DBMesocyclesModel;
class DBExercisesModel;
class QmlItemManager;
class TPListModel;
class OSInterface;

class QQmlApplicationEngine;
class QSettings;
class QQuickItem;

#include <QDate>
#include <QList>

enum {
	IFC_USER = 0x02,
	IFC_MESO = 0x04,
	IFC_MESOSPLIT = 0x08,
	IFC_EXERCISES = 0x10,
	IFC_TDAY = 0x10
} importFileContents;

class TPAppControl
{

public:
	TPAppControl();
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

	void openRequestedFile(const QString& filename);
	int importFromFile(const QString& filename, const bool bImportOptions[5]);
	void createItemManager();

	static TPAppControl* app_control;
	friend TPAppControl* appControl();

	static QSettings* app_settings;
	friend QSettings* appSettings();

	static TranslationClass* app_tr;
	friend TranslationClass* appTr();

	static DBInterface* app_db_interface;
	friend DBInterface* appDBInterface();

	static DBUserModel* app_user_model;
	friend DBUserModel* appUserModel();

	static DBMesocyclesModel* app_meso_model;
	friend DBMesocyclesModel* appMesoModel();

	static DBExercisesModel* app_exercises_model;
	friend DBExercisesModel* appExercisesModel();

	static QmlItemManager* app_root_items_manager;
	friend QmlItemManager* rootItemsManager();

	static QQmlApplicationEngine* app_qml_engine;
	friend QQmlApplicationEngine* appQmlEngine();

	static OSInterface* app_os_interface;
	friend OSInterface* appOsInterface();

	QList<QmlItemManager*> m_itemManager;
	uint m_tempMesoIdx;
};

inline TPAppControl* appControl() { return TPAppControl::app_control; }
inline QSettings* appSettings() { return TPAppControl::app_settings; }
inline TranslationClass* appTr() { return TPAppControl::app_tr; }
inline DBInterface* appDBInterface() { return TPAppControl::app_db_interface; }
inline DBUserModel* appUserModel() { return TPAppControl::app_user_model; }
inline DBMesocyclesModel* appMesoModel() { return TPAppControl::app_meso_model; }
inline DBExercisesModel* appExercisesModel() { return TPAppControl::app_exercises_model; }
inline QQmlApplicationEngine* appQmlEngine() { return TPAppControl::app_qml_engine; }
inline QmlItemManager* rootItemsManager() { return TPAppControl::app_root_items_manager; }
extern OSInterface* appOsInterface();

#endif // TPAPPCONTROL_H
