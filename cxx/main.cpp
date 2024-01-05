// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml/qqmlextensionplugin.h>
#include <QtQuickControls2/qquickstyle.h>
#include <QSettings>
#include <QQmlContext>
#include <QQuickStyle>

#include "translationclass.h"
#include "backupclass.h"

#ifdef Q_OS_ANDROID

/*#include <QtCore/private/qandroidextras_p.h>

"android.intent.action.OPEN_DOCUMENT"
bool createIntent()
{
	auto serviceIntent = QAndroidIntent(QtAndroidPrivate::androidActivity().object(), "com.example.MyService");

	bool success = true;
	if(QNativeInterface::QAndroidApplication::androidSdkVersion() >= 23)
	{
		static const QVector<QString> permissions({

		});

		for(const QString &permission : permissions)
		{
		QGuiApplication::nativeInterface()
			// check if permission is granded
			auto result = QNativeInterface::QAndroidApplication::checkPermission(permission);
			if(result != QNativeInterface::QAndroidApplication::PermissionResult::Granted)
			{
				// request permission
				auto resultHash = QNativeInterface::QAndroidApplication::requestPermissionsSync(QStringList({permission}));
				if(resultHash[permission] != QNativeInterface::QAndroidApplication::PermissionResult::Granted)
				{
					qDebug() << "Fail to get permission" << permission;
					success = false;
				}
				else
				{
					qDebug() << "Permission" << permission << "granted!";
				}
			}
			else
			{
				qDebug() << "Permission" << permission << "already granted!";
			}
		}
	}
	return success;
}*/
#endif //Q_OS_ANDROID

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

	TraslationClass trClass( appSettings );
	trClass.selectLanguage();
	QQmlApplicationEngine engine;
	//BackupClass backUpClass( engine.offlineStoragePath() );
	//backUpClass.checkIfDBFileIsMissing();
	engine.rootContext()->setContextProperty("trClass", &trClass);
	//engine.rootContext()->setContextProperty("backUpClass", &backUpClass);
	QQuickStyle::setStyle(appSettings.value("themeStyle").toString());

	const QStringList imagesLocation ( QStandardPaths::standardLocations(QStandardPaths::PicturesLocation) );
	const QUrl imagesPath( QUrl::fromLocalFile(imagesLocation.isEmpty() ? app.applicationDirPath() : imagesLocation.front() ) );
	engine.rootContext()->setContextProperty("imagesPath", imagesPath);

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

#ifdef Q_OS_ANDROID
	//checkPermissions();
#endif

	return app.exec();
}
