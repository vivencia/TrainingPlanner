#include "tpsettings.h"
#include "tputils.h"
#include "translationclass.h"
#include "osinterface.h"
#include "dbinterface.h"
#include "dbusermodel.h"
#include "dbmesocyclesmodel.h"
#include "dbexercisesmodel.h"
#include "qmlitemmanager.h"

#include <QApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
	QApplication app(argc, argv);

	app.setOrganizationName(u"Vivencia Software"_s);
	app.setOrganizationDomain(u"org.vivenciasoftware"_s);
	app.setApplicationName(u"TrainingPlanner"_s);

	TPSettings tpSettings{};
	TPUtils tpUtils{};
	TranslationClass appTranslations{};
	OSInterface osInterface{};
	DBInterface appDB{};
	DBUserModel userModel{};
	DBMesocyclesModel mesocyclesModel{};
	DBExercisesModel exercisesModel{};
	QQmlApplicationEngine qmlEngine;
	QmlItemManager rootQmlManager{&qmlEngine};
	return app.exec();
}
