// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml/qqmlextensionplugin.h>
#include <QSettings>
#include <QQmlContext>
#include <QQuickStyle>

#include "translationclass.h"
#include "backupclass.h"
#include "runcommands.h"
#include "dbmanager.h"

void populateSettingsWithDefaultValue( QSettings& settingsObj)
{
	if ( settingsObj.childKeys().isEmpty() ) {
		settingsObj.setValue( "appLocale", QLocale::system().name() );
		settingsObj.setValue( "weightUnit", "(kg)" );
		settingsObj.setValue( "themeStyle", "Material" );
		settingsObj.setValue( "themeStyleColorIndex", 0 );
		settingsObj.setValue( "themeStyleIndex", 4 );
		settingsObj.setValue( "fontSize", 14 );
		settingsObj.setValue( "fontSizeLists", 9 );
		settingsObj.setValue( "fontSizeText", 12 );
		settingsObj.setValue( "fontSizeTitle", 21 );
		settingsObj.setValue( "exercisesListVersion", "0");
		settingsObj.sync();
	}
}

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
	QGuiApplication app(argc, argv);
	app.setOrganizationName("Vivencia Software");
	app.setOrganizationDomain("org.vivenciasoftware");
	app.setApplicationName("Training Planner");

	QSettings appSettings;
	populateSettingsWithDefaultValue(appSettings);

	TranslationClass trClass( appSettings );
	trClass.selectLanguage();

	QQuickStyle::setStyle(appSettings.value("themeStyle").toString());

	RunCommands runCmd(&appSettings);
	QString db_filepath (appSettings.value("dbFilePath").toString());
	if (db_filepath.isEmpty())
	{
		QQmlApplicationEngine* tempEngine(new QQmlApplicationEngine());
		db_filepath = runCmd.getAppDir(runCmd.searchForDatabaseFile(tempEngine->offlineStoragePath()));
		appSettings.setValue("dbFilePath", db_filepath);
		appSettings.sync();
		delete tempEngine;
	}
	DbManager db(&appSettings, &runCmd);

	QQmlApplicationEngine engine;
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
