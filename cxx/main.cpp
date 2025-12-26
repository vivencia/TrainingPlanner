#include "thread_manager.h"
#include "dbexerciseslistmodel.h"

#include "dbusermodel.h"
#include "osinterface.h"
#include "qmlitemmanager.h"
#include "tpsettings.h"
#include "tputils.h"
#include "translationclass.h"
#include "thread_manager.h"
#include "online_services/tponlineservices.h"
#include "tpkeychain/tpkeychain.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QSurfaceFormat>

#ifdef Q_OS_ANDROID
#include <QtCore/private/qandroidextras_p.h>
#endif

int main(int argc, char *argv[])
{
#ifdef Q_OS_ANDROID
	if (argc > 1 && strcmp(argv[1], "-service") == 0)
	{
		qInfo() << "Service starting with from the same .so file";
		QAndroidService app(argc, argv);
		return app.exec();
	}
#endif
	QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
	// Set the default surface format before creating the application
	QSurfaceFormat format;
	format.setVersion(3, 1); // Request OpenGL ES 3.1
	format.setProfile(QSurfaceFormat::CoreProfile); // Use Core Profile
	QSurfaceFormat::setDefaultFormat(format);
	QApplication app{argc, argv};

	app.setOrganizationName("Vivencia Software"_L1);
	app.setOrganizationDomain("org.vivenciasoftware"_L1);
	app.setApplicationName("TrainingPlanner"_L1);

	TPUtils tpUtils{};
	ThreadManager threadManager{};
	TPSettings tpSettings{};
	TPKeyChain tpKeyChain{};
	TranslationClass appTranslations{};
	OSInterface osInterface{};
	TPOnlineServices appTOS{};
	DBUserModel userModel{};
	DBExercisesListModel exercisesModel{};
	QQmlApplicationEngine qmlEngine;
	QmlItemManager rootQmlManager{&qmlEngine};
	return app.exec();
}
