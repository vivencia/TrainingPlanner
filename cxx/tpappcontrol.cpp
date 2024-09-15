#include "tpappcontrol.h"
#include "translationclass.h"
#include "tputils.h"
#include "dbinterface.h"
#include "dbusermodel.h"
#include "dbmesocyclesmodel.h"
#include "dbexercisesmodel.h"
#include "qmlitemmanager.h"

#include <QQmlApplicationEngine>

TPAppControl* TPAppControl::app_control(nullptr);
QSettings* TPAppControl::app_settings(nullptr);
TranslationClass* TPAppControl::app_tr(nullptr);
TPUtils* TPAppControl::app_utils(nullptr);
DBInterface* TPAppControl::app_db_interface(nullptr);
DBUserModel* TPAppControl::app_user_model(nullptr);
DBMesocyclesModel* TPAppControl::app_meso_model(nullptr);
DBExercisesModel* TPAppControl::app_exercises_model(nullptr);
QQmlApplicationEngine* TPAppControl::app_qml_engine(nullptr);

#ifdef Q_OS_ANDROID
	#define FONT_POINT_SIZE 15
	#define FONT_POINT_SIZE_LISTS 11
	#define FONT_POINT_SIZE_TEXT 13
	#define FONT_POINT_SIZE_TITLE 20
#else
	#define FONT_POINT_SIZE 12
	#define FONT_POINT_SIZE_LISTS 8
	#define FONT_POINT_SIZE_TEXT 10
	#define FONT_POINT_SIZE_TITLE 18
#endif

TPAppControl::TPAppControl()
{
	app_control = this;
	QSettings _appSettings{};
	app_settings = &_appSettings;
	TranslationClass _trClass{};
	app_tr = &_trClass;
	_trClass.selectLanguage();
	TPUtils _tPUtils;
	app_utils = &_tPUtils;
	DBInterface _dbInterface;
	app_db_interface = &_dbInterface;
	DBUserModel _dbUserModel;
	app_user_model = &_dbUserModel;
	DBMesocyclesModel _dbMesocyclesModel;
	app_meso_model = &_dbMesocyclesModel;
	DBExercisesModel _dbExercisesModel;
	app_exercises_model = &_dbExercisesModel;
	QQmlApplicationEngine _qmlEngine;
	app_qml_engine = &_qmlEngine;
	#ifdef Q_OS_ANDROID
	new URIHandler(&db, &db);
	#endif

	populateSettingsWithDefaultValue();
	appDBInterface()->init();

	QmlItemManager::configureQmlEngine();
	createItemManager();

#ifdef Q_OS_ANDROID
	// if App was launched from VIEW or SEND Intent there's a race collision: the event will be lost,
	// because App and UI wasn't completely initialized. Workaround: QShareActivity remembers that an Intent is pending
	connect(appUtils(), &TPUtils::appResumed, this, &DBInterface::checkPendingIntents);
	connect(handlerInstance(), &URIHandler::activityFinishedResult, this, [&] (const int requestCode, const int resultCode) {
		QMetaObject::invokeMethod(appMainWindow(), "activityResultMessage", Q_ARG(int, requestCode), Q_ARG(int, resultCode));
		QFile::remove(exportFileName());
	});
	appStartUpNotifications();
#endif

	const QUrl url(u"qrc:/qml/main.qml"_qs);
	QObject::connect(appQmlEngine(), &QQmlApplicationEngine::objectCreated, appQmlEngine(), [url] (QObject *obj, const QUrl &objUrl) {
		if (!obj && url == objUrl)
			QCoreApplication::exit(-1);
	});
	appQmlEngine()->addImportPath(u":/"_qs);
	appQmlEngine()->load(url);
	if (appQmlEngine()->rootObjects().isEmpty())
		QCoreApplication::exit(-1);
}

void TPAppControl::cleanUp()
{
	appDBInterface()->cleanUpThreads();
	for(uint i(0); i < m_itemManager.count(); ++i)
		delete m_itemManager.at(i);
}

void TPAppControl::populateSettingsWithDefaultValue()
{
	if (appSettings()->value("appVersion").toString().isEmpty())
	{
		appSettings()->setValue("appVersion", TP_APP_VERSION);
		appSettings()->setValue("weightUnit", u"(kg)"_qs);
		appSettings()->setValue("themeStyle", u"Material"_qs);
		appSettings()->setValue("colorScheme", u"Blue"_qs);
		appSettings()->setValue("primaryDarkColor", u"#1976D2"_qs);
		appSettings()->setValue("primaryColor", u"#25b5f3"_qs);
		appSettings()->setValue("primaryLightColor", u"#BBDEFB"_qs);
		appSettings()->setValue("paneBackgroundColor", u"#1976d2"_qs);
		appSettings()->setValue("entrySelectedColor", u"#6495ed"_qs);
		appSettings()->setValue("exercisesListVersion", u"0"_qs);
		appSettings()->setValue("backupFolder", u""_qs);
		appSettings()->setValue("fontColor", u"white"_qs);
		appSettings()->setValue("disabledFontColor", u"lightgray"_qs);
		appSettings()->setValue("iconFolder", u"white/"_qs);
		appSettings()->setValue("fontSize", FONT_POINT_SIZE);
		appSettings()->setValue("fontSizeLists", FONT_POINT_SIZE_LISTS);
		appSettings()->setValue("fontSizeText", FONT_POINT_SIZE_TEXT);
		appSettings()->setValue("fontSizeTitle", FONT_POINT_SIZE_TITLE);
		appSettings()->setValue("lastViewedOwnMesoIdx", -1);
		appSettings()->setValue("lastViewedOtherMesoIdx", -1);
		appSettings()->setValue("alwaysAskConfirmation", true);
		appSettings()->sync();
	}
}

void TPAppControl::createItemManager()
{
	const uint n_mesos(appMesoModel()->count());
	m_itemManager.reserve(n_mesos);
	for (uint i(0); i < n_mesos; ++i)
		m_itemManager.append(new QmlItemManager{i});
}
