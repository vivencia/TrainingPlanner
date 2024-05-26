// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml/qqmlextensionplugin.h>
#include <QSettings>
#include <QQmlContext>
#include <QQuickStyle>

#include "translationclass.h"
#include "runcommands.h"
#include "dbmanager.h"

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
	app.setOrganizationName("Vivencia Software");
	app.setOrganizationDomain("org.vivenciasoftware");
	app.setApplicationName("Training Planner");

	QSettings appSettings;
	populateSettingsWithDefaultValue(appSettings);

	TranslationClass trClass( appSettings );
	trClass.selectLanguage();

	QQuickStyle::setStyle(appSettings.value("themeStyle").toString());

	RunCommands runCmd(&appSettings);
	DbManager db(&appSettings, &runCmd, argv[0]);
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
