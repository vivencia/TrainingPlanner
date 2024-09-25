#include "tpappcontrol.h"
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
#include <QSettings>

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
	QApplication app(argc, argv);

	app.setOrganizationName(u"Vivencia Software"_qs);
	app.setOrganizationDomain(u"org.vivenciasoftware"_qs);
	app.setApplicationName(u"Training Planner"_qs);

	QSettings settings;
	TPAppControl tpApp{&settings};
	TPUtils appUtils{};
	TranslationClass appTr{};
	OSInterface appOsInterface{};
	DBInterface appDB{};
	DBUserModel userModel{};
	DBMesocyclesModel mesocyclesModel{};
	DBExercisesModel exercisesModel{};
	QmlItemManager rootQmlManager{0xFFFF};
	QQmlApplicationEngine qmlEngine;
	tpApp.init(&qmlEngine);
	return app.exec();
}
