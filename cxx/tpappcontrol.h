#ifndef TPAPPCONTROL_H
#define TPAPPCONTROL_H

class TranslationClass;
class TPUtils;
class DBInterface;
class DBUserModel;
class DBMesocyclesModel;
class DBExercisesModel;
class QmlItemManager;

class QQmlApplicationEngine;
class QSettings;

#include <QList>

class TPAppControl
{

public:
	TPAppControl();
	void cleanUp();

	Q_INVOKABLE inline QmlItemManager* itemManager(const uint meso_idx) const { return m_itemManager.at(meso_idx); }

private:
	void populateSettingsWithDefaultValue();
	void createItemManager();

	static TPAppControl* app_control;
	friend TPAppControl* appControl();

	static QSettings* app_settings;
	friend QSettings* appSettings();

	static TranslationClass* app_tr;
	friend TranslationClass* appTr();

	static TPUtils* app_utils;
	friend TPUtils* appUtils();

	static DBInterface* app_db_interface;
	friend DBInterface* appDBInterface();

	static DBUserModel* app_user_model;
	friend DBUserModel* appUserModel();

	static DBMesocyclesModel* app_meso_model;
	friend DBMesocyclesModel* appMesoModel();

	static DBExercisesModel* app_exercises_model;
	friend DBExercisesModel* appExercisesModel();

	static QQmlApplicationEngine* app_qml_engine;
	friend QQmlApplicationEngine* appQmlEngine();

	QList<QmlItemManager*> m_itemManager;
};

inline TPAppControl* appControl() { return TPAppControl::app_control; }
inline QSettings* appSettings() { return TPAppControl::app_settings; }
inline TranslationClass* appTr() { return TPAppControl::app_tr; }
inline TPUtils* appUtils() { return TPAppControl::app_utils; }
inline DBInterface* appDBInterface() { return TPAppControl::app_db_interface; }
inline DBUserModel* appUserModel() { return TPAppControl::app_user_model; }
inline DBMesocyclesModel* appMesoModel() { return TPAppControl::app_meso_model; }
inline DBExercisesModel* appExercisesModel() { return TPAppControl::app_exercises_model; }
inline QQmlApplicationEngine* appQmlEngine() { return TPAppControl::app_qml_engine; }

#endif // TPAPPCONTROL_H
