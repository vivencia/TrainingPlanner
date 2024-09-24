#include "translationclass.h"
#include "tpappcontrol.h"
#include "tputils.h"
#include "qmlitemmanager.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSettings>
#include <QTranslator>

TranslationClass* TranslationClass::app_tr(nullptr);

TranslationClass::TranslationClass(QObject* parent)
	: QObject{parent}
{
	app_tr = this;
	mbOK = true;
	mTranslator = new QTranslator(this);
	selectLanguage();
}

TranslationClass::~TranslationClass()
{
	delete mTranslator;
}

void TranslationClass::selectLanguage()
{
	QString strLocale(appSettings()->value("appLocale").toString());
	const bool bConfigEmpty(strLocale.isEmpty());
	if (bConfigEmpty)
	{
		#ifndef Q_OS_ANDROID
		const QString& sysLocale(std::setlocale(LC_NAME, ""));
		strLocale = sysLocale.left(sysLocale.indexOf('.'));
		#else
		strLocale = QLocale::system().name();
		#endif
	}
	if (strLocale != u"en_US"_qs)
	{
		mbOK = mTranslator->load(u"tplanner.%1.qm"_qs.arg(strLocale), u":/translations/"_qs, u"qm"_qs);
		if (mbOK)
			qApp->installTranslator(mTranslator);
	}
	if (mbOK)
	{
		appUtils()->setAppLocale(strLocale);
		if (bConfigEmpty)
			appSettings()->setValue("appLocale", strLocale);
	}
	else
		appUtils()->setAppLocale(u"en_US"_qs); //If any part of the program calls RunCommands::appLocale() we will hava an error
}

void TranslationClass::switchToLanguage(const QString& language)
{
	QCoreApplication::removeTranslator(mTranslator);
	const bool bEnglish(language == u"en_US"_qs);
	mbOK = bEnglish;
	if (!bEnglish)
		mbOK = mTranslator->load(u"tplanner.%1.qm"_qs.arg(language), u":/translations/"_qs, u"qm"_qs);
	if (mbOK)
	{
		appUtils()->setAppLocale(language);
		appSettings()->setValue("appLocale", language);
		if (!bEnglish)
			qApp->installTranslator(mTranslator);
		appQmlEngine()->retranslate();
	}
}
