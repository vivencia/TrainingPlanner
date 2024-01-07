// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml/qqmlextensionplugin.h>
#include <QtQuickControls2/qquickstyle.h>
#include <QSettings>
#include <QQmlContext>
#include <QQuickStyle>
#include <QStandardPaths>

#include "translationclass.h"
#include "backupclass.h"
#include "runcommands.h"

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
		settingsObj.setValue( "fontSizePixelSize", qApp->font().pixelSize() );
		settingsObj.setValue( "titleFontSizePixelSize", qApp->font().pixelSize() * 1.25 );
		settingsObj.setValue( "hugeFontSizePixelSize", qApp->font().pixelSize() * 2 );
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

	QQmlApplicationEngine engine;
	engine.rootContext()->setContextProperty("trClass", &trClass);

	//BackupClass backUpClass( engine.offlineStoragePath() );
	//backUpClass.checkIfDBFileIsMissing();
	//engine.rootContext()->setContextProperty("backUpClass", &backUpClass);
	QQuickStyle::setStyle(appSettings.value("themeStyle").toString());

	RunCommands runCmd;
	engine.rootContext()->setContextProperty("runCmd", &runCmd);
	runCmd.updateExercisesList();

	const QUrl url(u"qrc:/qml/main.qml"_qs);
	QObject::connect(
				&engine, &QQmlApplicationEngine::objectCreated, &app,
				[url](QObject *obj, const QUrl &objUrl) {
		if (!obj && url == objUrl)
			QCoreApplication::exit(-1);
	});
	engine.addImportPath(":/");
	engine.load(url);
	if (engine.rootObjects().isEmpty())
		return -1;

	return app.exec();
}
