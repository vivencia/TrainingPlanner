#include "dbinterface.h"
#include "dbexercisesmodel.h"
#include "dbmesocyclesmodel.h"
#include "dbusermodel.h"
#include "osinterface.h"
#include "qmlitemmanager.h"
#include "tpsettings.h"
#include "tputils.h"
#include "translationclass.h"
#include "online_services/tponlineservices.h"

#include <QApplication>
#include <QQmlApplicationEngine>

#ifdef Q_OS_ANDROID
#include <QtCore/private/qandroidextras_p.h>
#endif

int main(int argc, char *argv[])
{
#ifdef Q_OS_ANDROID
	if (argc <= 1)
	{
#endif
		QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
		QApplication app{argc, argv};

		app.setOrganizationName("Vivencia Software"_L1);
		app.setOrganizationDomain("org.vivenciasoftware"_L1);
		app.setApplicationName("TrainingPlanner"_L1);

		TPSettings tpSettings{};
		TPUtils tpUtils{};
		TranslationClass appTranslations{};
		TPOnlineServices appTOS{};
		OSInterface osInterface{};
		DBInterface appDB{};
		DBUserModel userModel{};
		DBMesocyclesModel mesocyclesModel{};
		DBExercisesModel exercisesModel{};
		QQmlApplicationEngine qmlEngine;
		QmlItemManager rootQmlManager{&qmlEngine};
		return app.exec();
#ifdef Q_OS_ANDROID
	}
	else if (argc > 1 && strcmp(argv[1], "-service") == 0)
	{
		qInfo() << "Service starting with from the same .so file";
		QAndroidService app(argc, argv);
		return app.exec();
	}
	else
	{
		qWarning() << "Unrecognized command line argument";
		return -1;
	}
#endif
}

