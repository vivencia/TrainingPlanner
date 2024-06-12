#include "translationclass.h"
#include "runcommands.h"
#include "dbmanager.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml/qqmlextensionplugin.h>
#include <QSettings>
#include <QQmlContext>
#include <QQuickStyle>

#ifdef Q_OS_ANDROID

#include "urihandler.h"
#include <android/log.h>

const char*const applicationName="tp_app";

void tpMessageHandler(
  QtMsgType type,
  const QMessageLogContext& context,
  const QString& msg
) {
  QString report=msg;
  if (context.file && !QString(context.file).isEmpty()) {
	report+=" in file ";
	report+=QString(context.file);
	report+=" line ";
	report+=QString::number(context.line);
  }
  if (context.function && !QString(context.function).isEmpty()) {
	report+=+" function ";
	report+=QString(context.function);
  }
  const char*const local=report.toLocal8Bit().constData();
  switch (type) {
  case QtDebugMsg:
	__android_log_write(ANDROID_LOG_DEBUG,applicationName,local);
	break;
  case QtInfoMsg:
	__android_log_write(ANDROID_LOG_INFO,applicationName,local);
	break;
  case QtWarningMsg:
	__android_log_write(ANDROID_LOG_WARN,applicationName,local);
	break;
  case QtCriticalMsg:
	__android_log_write(ANDROID_LOG_ERROR,applicationName,local);
	break;
  case QtFatalMsg:
  default:
	__android_log_write(ANDROID_LOG_FATAL,applicationName,local);
	abort();
  }
}

#endif //Q_OS_ANDROID

void populateSettingsWithDefaultValue( QSettings& settingsObj)
{
	if (settingsObj.childKeys().isEmpty() || settingsObj.value("appLocale").toString().isEmpty())
	{
		appLocale = QLocale::system();
		settingsObj.setValue("appVersion", TP_APP_VERSION);
		settingsObj.setValue("appLocale", appLocale.name());
		settingsObj.setValue("weightUnit", u"(kg)"_qs);
		settingsObj.setValue("themeStyle", u"Material"_qs);
		settingsObj.setValue("colorScheme", u"Blue"_qs);
		settingsObj.setValue("primaryDarkColor", u"#1976D2"_qs);
		settingsObj.setValue("primaryColor", u"#25b5f3"_qs);
		settingsObj.setValue("primaryLightColor", u"#BBDEFB"_qs);
		settingsObj.setValue("paneBackgroundColor", u"#1976d2"_qs);
		settingsObj.setValue("entrySelectedColor", u"#6495ed"_qs);
		settingsObj.setValue("exercisesListVersion", u"0"_qs);
		settingsObj.setValue("backupFolder", u""_qs);
		settingsObj.setValue("fontColor", u"white"_qs);
		settingsObj.setValue("disabledFontColor", u"lightgray"_qs);
		settingsObj.setValue("iconFolder", u"white/"_qs);
		settingsObj.setValue("fontSize", 12);
		settingsObj.setValue("fontSizeLists", 8);
		settingsObj.setValue("fontSizeText", 10);
		settingsObj.setValue("fontSizeTitle", 18);
		settingsObj.setValue("alwaysAskConfirmation", true);
		settingsObj.setValue("firstTime", true);
		settingsObj.sync();
	}
	else
		appLocale.setDefault(QLocale(settingsObj.value("appLocale").toString()));
	appLocale.setNumberOptions(QLocale::IncludeTrailingZeroesAfterDot);
}

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
	QApplication app(argc, argv);

	#ifdef Q_OS_ANDROID
	qInstallMessageHandler(tpMessageHandler);
	#endif

	app.setOrganizationName("Vivencia Software");
	app.setOrganizationDomain("org.vivenciasoftware");
	app.setApplicationName("Training Planner");

	QSettings appSettings;
	populateSettingsWithDefaultValue(appSettings);

	TranslationClass trClass( appSettings );
	trClass.selectLanguage();

	QQuickStyle::setStyle(appSettings.value("themeStyle").toString());

	RunCommands runCmd(&appSettings);
	DbManager db(&appSettings, &runCmd);
	#ifdef Q_OS_ANDROID
	new URIHandler(&db, &db);
	#endif
	QQmlApplicationEngine engine;

	QString db_filepath (appSettings.value("dbFilePath").toString());
	if (db_filepath.isEmpty())
	{
		db_filepath = runCmd.getAppDir(engine.offlineStoragePath());
		appSettings.setValue("dbFilePath", db_filepath);
		appSettings.sync();
	}
	db.init();

	const QUrl url(u"qrc:/qml/main.qml"_qs);
	QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
					&app, [url](QObject *obj, const QUrl &objUrl) { if (!obj && url == objUrl) QCoreApplication::exit(-1); });
	engine.addImportPath(QStringLiteral(":/"));
	engine.load(url);
	if (engine.rootObjects().isEmpty())
		return -1;

	db.setQmlEngine(&engine);

	return app.exec();
}
