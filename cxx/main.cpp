#include "tpappcontrol.h"

#include <QApplication>

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

	app.setOrganizationName(u"Vivencia Software"_qs);
	app.setOrganizationDomain(u"org.vivenciasoftware"_qs);
	app.setApplicationName(u"Training Planner"_qs);
	TPAppControl tpApp{};

	return app.exec();
}
