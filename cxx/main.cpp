#include "translationclass.h"
#include "tputils.h"
#include "dbinterface.h"
#include "tpimageprovider.h"
#include "tpimage.h"

#include <QApplication>
#include <QtQml/qqmlextensionplugin.h>
#include <QQmlApplicationEngine>

#ifdef Q_OS_ANDROID

#include "urihandler.h"

/*
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
*/
#endif //Q_OS_ANDROID

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
	QApplication app(argc, argv);

	/*#ifdef Q_OS_ANDROID
	qInstallMessageHandler(tpMessageHandler);
	#endif*/

	app.setOrganizationName("Vivencia Software");
	app.setOrganizationDomain("org.vivenciasoftware");
	app.setApplicationName("Training Planner");

	QSettings appSettings;
	TPUtils appUtils(&appSettings);
	TranslationClass trClass{};
	trClass.selectLanguage();

	DBInterface db{};
	#ifdef Q_OS_ANDROID
	new URIHandler(&db, &db);
	#endif

	QQmlApplicationEngine engine;
	engine.addImageProvider(QLatin1String("tpimageprovider"), new TPImageProvider());
	qmlRegisterType<TPImage>("com.vivenciasoftware.qmlcomponents", 1, 0, "TPImage");

	const QUrl url(u"qrc:/qml/main.qml"_qs);
	QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
					&app, [url](QObject *obj, const QUrl &objUrl) { if (!obj && url == objUrl) QCoreApplication::exit(-1); });
	engine.addImportPath(QStringLiteral(":/"));
	engine.load(url);
	if (engine.rootObjects().isEmpty())
		return -1;

	db.init(&engine);

	return app.exec();
}
